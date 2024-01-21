/**************************************************************************/
/*  pck_packer.cpp                                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "pck_packer.h"
#include "pck_packer.compat.inc"

#include "core/crypto/crypto_core.h"
#include "core/io/file_access.h"
#include "core/io/file_access_encrypted.h"
#include "core/io/file_access_pack.h" // PACK_HEADER_MAGIC, PACK_FORMAT_VERSION
#include "core/version.h"

static int _get_pad(int p_alignment, int p_n) {
	int rest = p_n % p_alignment;
	int pad = 0;
	if (rest > 0) {
		pad = p_alignment - rest;
	}

	return pad;
}

void PCKPacker::_bind_methods() {
	ClassDB::bind_method(D_METHOD("pck_start", "pck_name", "alignment", "key", "encrypt_directory"), &PCKPacker::pck_start, DEFVAL(32), DEFVAL("0000000000000000000000000000000000000000000000000000000000000000"), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("add_file", "pck_path", "source_path", "encrypt", "require_verification"), &PCKPacker::add_file, DEFVAL(false), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("flush", "verbose"), &PCKPacker::flush, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("flush_and_sign", "private_key", "curve", "verbose"), &PCKPacker::flush_and_sign, DEFVAL(ECP_DP_SECP256R1), DEFVAL(false));

	BIND_ENUM_CONSTANT(ECP_DP_NONE);
	BIND_ENUM_CONSTANT(ECP_DP_SECP192R1);
	BIND_ENUM_CONSTANT(ECP_DP_SECP224R1);
	BIND_ENUM_CONSTANT(ECP_DP_SECP256R1);
	BIND_ENUM_CONSTANT(ECP_DP_SECP384R1);
	BIND_ENUM_CONSTANT(ECP_DP_SECP521R1);
	BIND_ENUM_CONSTANT(ECP_DP_BP256R1);
	BIND_ENUM_CONSTANT(ECP_DP_BP384R1);
	BIND_ENUM_CONSTANT(ECP_DP_BP512R1);
	BIND_ENUM_CONSTANT(ECP_DP_CURVE25519);
	BIND_ENUM_CONSTANT(ECP_DP_SECP192K1);
	BIND_ENUM_CONSTANT(ECP_DP_SECP224K1);
	BIND_ENUM_CONSTANT(ECP_DP_SECP256K1);
	BIND_ENUM_CONSTANT(ECP_DP_CURVE448);
}

Error PCKPacker::pck_start(const String &p_file, int p_alignment, const String &p_key, bool p_encrypt_directory) {
	ERR_FAIL_COND_V_MSG((p_key.is_empty() || !p_key.is_valid_hex_number(false) || p_key.length() != 64), ERR_CANT_CREATE, "Invalid Encryption Key (must be 64 characters long).");
	ERR_FAIL_COND_V_MSG(p_alignment <= 0, ERR_CANT_CREATE, "Invalid alignment, must be greater then 0.");

	String _key = p_key.to_lower();
	key.resize(32);
	for (int i = 0; i < 32; i++) {
		int v = 0;
		if (i * 2 < _key.length()) {
			char32_t ct = _key[i * 2];
			if (is_digit(ct)) {
				ct = ct - '0';
			} else if (ct >= 'a' && ct <= 'f') {
				ct = 10 + ct - 'a';
			}
			v |= ct << 4;
		}

		if (i * 2 + 1 < _key.length()) {
			char32_t ct = _key[i * 2 + 1];
			if (is_digit(ct)) {
				ct = ct - '0';
			} else if (ct >= 'a' && ct <= 'f') {
				ct = 10 + ct - 'a';
			}
			v |= ct;
		}
		key.write[i] = v;
	}
	enc_dir = p_encrypt_directory;

	file = FileAccess::open(p_file, FileAccess::WRITE);
	ERR_FAIL_COND_V_MSG(file.is_null(), ERR_CANT_CREATE, "Can't open file to write: " + String(p_file) + ".");

	alignment = p_alignment;

	file->store_32(PACK_HEADER_MAGIC);
	file->store_32(PACK_FORMAT_VERSION);
	file->store_32(VERSION_MAJOR);
	file->store_32(VERSION_MINOR);
	file->store_32(VERSION_PATCH);

	uint32_t pack_flags = 0;
	if (enc_dir) {
		pack_flags |= PACK_DIR_ENCRYPTED;
	}
	file->store_32(pack_flags); // flags

	files.clear();
	ofs = 0;

	return OK;
}

Error PCKPacker::add_file(const String &p_file, const String &p_src, bool p_encrypt, bool require_verification) {
	ERR_FAIL_COND_V_MSG(file.is_null(), ERR_INVALID_PARAMETER, "File must be opened before use.");

	Ref<FileAccess> f = FileAccess::open(p_src, FileAccess::READ);
	if (f.is_null()) {
		return ERR_FILE_CANT_OPEN;
	}

	File pf;
	// Simplify path here and on every 'files' access so that paths that have extra '/'
	// symbols in them still match to the MD5 hash for the saved path.
	pf.path = p_file.simplify_path();
	pf.src_path = p_src;
	pf.ofs = ofs;
	pf.size = f->get_length();

	Vector<uint8_t> data = FileAccess::get_file_as_bytes(p_src);
	{
		pf.md5.resize(16);
		CryptoCore::md5(data.ptr(), data.size(), pf.md5.ptrw());

		pf.sha256.resize(32);
		CryptoCore::sha256(data.ptr(), data.size(), pf.sha256.ptrw());
	}
	pf.encrypted = p_encrypt;
	pf.require_verification = require_verification;

	uint64_t _size = pf.size;
	if (p_encrypt) { // Add encryption overhead.
		if (_size % 16) { // Pad to encryption block size.
			_size += 16 - (_size % 16);
		}
		_size += 16; // hash
		_size += 8; // data size
		_size += 16; // iv
	}

	int pad = _get_pad(alignment, ofs + _size);
	ofs = ofs + _size + pad;

	files.push_back(pf);

	return OK;
}

Error PCKPacker::flush(bool p_verbose) {
	return flush_and_sign(String(), ECP_DP_NONE, p_verbose);
}

Error PCKPacker::flush_and_sign(const String &p_private_key, PCKPacker::CurveType p_curve, bool p_verbose) {
	ERR_FAIL_COND_V_MSG(file.is_null(), ERR_INVALID_PARAMETER, "File must be opened before use.");

#if !defined(ECDSA_ENABLED)
	ERR_FAIL_COND_V_MSG(!p_private_key.is_empty(), ERR_INVALID_PARAMETER, "Engine was built without ECDSA support.");
#endif

	int64_t file_base_ofs = file->get_position();
	file->store_64(0); // files base

#if defined(ECDSA_ENABLED)
	uint64_t signature_info_ofs = file->get_position();
#endif
	file->store_64(0); // signature ofs
	file->store_64(0); // signature size
	file->store_32(0); // signature curve

	for (int i = 0; i < 11; i++) {
		file->store_32(0); // reserved
	}

	// write the index
	file->store_32(files.size());

	Ref<FileAccessEncrypted> fae;
	Ref<FileAccess> fhead = file;

	if (enc_dir) {
		fae.instantiate();
		ERR_FAIL_COND_V(fae.is_null(), ERR_CANT_CREATE);

		Error err = fae->open_and_parse(file, key, FileAccessEncrypted::MODE_WRITE_AES256, false);
		ERR_FAIL_COND_V(err != OK, ERR_CANT_CREATE);

		fhead = fae;
	}

	Vector<uint8_t> directory_hash;
	directory_hash.resize(32);
	CryptoCore::SHA256Context sha_ctx;
	sha_ctx.start();

	for (int i = 0; i < files.size(); i++) {
		static uint8_t zero = 0;
		int string_len = files[i].path.utf8().length();
		int pad = _get_pad(4, string_len);

		uint32_t full_len = string_len + pad;
		fhead->store_32(full_len);
		sha_ctx.update((const unsigned char *)&full_len, 4);
		fhead->store_buffer((const uint8_t *)files[i].path.utf8().get_data(), string_len);
		sha_ctx.update((const unsigned char *)files[i].path.utf8().get_data(), string_len);
		for (int j = 0; j < pad; j++) {
			fhead->store_8(0);
			sha_ctx.update((const unsigned char *)&zero, 1);
		}

		fhead->store_64(files[i].ofs);
		sha_ctx.update((const unsigned char *)&files[i].ofs, 8);
		fhead->store_64(files[i].size); // pay attention here, this is where file is
		sha_ctx.update((const unsigned char *)&files[i].size, 8);
		fhead->store_buffer(files[i].md5.ptr(), 16); //also save md5 for file
		sha_ctx.update((const unsigned char *)files[i].md5.ptr(), 16);
		fhead->store_buffer(files[i].sha256.ptr(), 32); //also save sha256 for file
		sha_ctx.update((const unsigned char *)files[i].sha256.ptr(), 32);

		uint32_t flags = 0;
		if (files[i].encrypted) {
			flags |= PACK_FILE_ENCRYPTED;
		}
		if (files[i].require_verification) {
			flags |= PACK_FILE_REQUIRE_VERIFICATION;
		}
		fhead->store_32(flags);
		sha_ctx.update((const unsigned char *)&flags, 4);
	}
	sha_ctx.finish((unsigned char *)directory_hash.ptrw());

#if defined(ECDSA_ENABLED)
	Vector<uint8_t> signature;
	size_t signature_size = 4096;
	if (!p_private_key.is_empty()) {
		CharString priv_key = p_private_key.ascii();

		size_t priv_key_len = 0;
		Vector<uint8_t> buf_priv;
		buf_priv.resize(1024);
		uint8_t *w = buf_priv.ptrw();
		ERR_FAIL_COND_V(CryptoCore::b64_decode(&w[0], buf_priv.size(), &priv_key_len, (unsigned char *)priv_key.ptr(), priv_key.length()) != OK, ERR_CANT_CREATE);

		signature.resize(signature_size);
		CryptoCore::ECDSAContext ecdsa_ctx((CryptoCore::ECDSAContext::CurveType)p_curve);
		ecdsa_ctx.set_private_key(buf_priv.ptr(), priv_key_len);
		ERR_FAIL_COND_V_MSG(ecdsa_ctx.sign((const unsigned char *)directory_hash.ptr(), (unsigned char *)signature.ptr(), &signature_size) != OK, ERR_CANT_CREATE, "Pack directory signing failed.");
		signature.resize(signature_size);
	}
#endif

	if (fae.is_valid()) {
		fhead.unref();
		fae.unref();
	}

	int header_padding = _get_pad(alignment, file->get_position());
	for (int i = 0; i < header_padding; i++) {
		file->store_8(0);
	}

	int64_t file_base = file->get_position();
	file->seek(file_base_ofs);
	file->store_64(file_base); // update files base
	file->seek(file_base);

	const uint32_t buf_max = 65536;
	uint8_t *buf = memnew_arr(uint8_t, buf_max);

	int count = 0;
	for (int i = 0; i < files.size(); i++) {
		Ref<FileAccess> src = FileAccess::open(files[i].src_path, FileAccess::READ);
		uint64_t to_write = files[i].size;

		Ref<FileAccess> ftmp = file;
		if (files[i].encrypted) {
			fae.instantiate();
			ERR_FAIL_COND_V(fae.is_null(), ERR_CANT_CREATE);

			Error err = fae->open_and_parse(file, key, FileAccessEncrypted::MODE_WRITE_AES256, false);
			ERR_FAIL_COND_V(err != OK, ERR_CANT_CREATE);
			ftmp = fae;
		}

		while (to_write > 0) {
			uint64_t read = src->get_buffer(buf, MIN(to_write, buf_max));
			ftmp->store_buffer(buf, read);
			to_write -= read;
		}

		if (fae.is_valid()) {
			ftmp.unref();
			fae.unref();
		}

		int pad = _get_pad(alignment, file->get_position());
		for (int j = 0; j < pad; j++) {
			file->store_8(0);
		}

		count += 1;
		const int file_num = files.size();
		if (p_verbose && (file_num > 0)) {
			print_line(vformat("[%d/%d - %d%%] PCKPacker flush: %s -> %s", count, file_num, float(count) / file_num * 100, files[i].src_path, files[i].path));
		}
	}

#if defined(ECDSA_ENABLED)
	if (!p_private_key.is_empty()) {
		int signature_padding = _get_pad(alignment, file->get_position());
		for (int i = 0; i < signature_padding; i++) {
			file->store_8(0);
		}
		uint64_t signature_offset = file->get_position();
		file->store_buffer(signature);
		uint64_t signature_end = file->get_position();

		// Update signature offset and size.
		file->seek(signature_info_ofs);
		file->store_64(signature_offset);
		file->store_64(signature_size);
		file->store_32(p_curve);
		file->seek(signature_end);
	}
#endif

	file.unref();
	memdelete_arr(buf);

	return OK;
}
