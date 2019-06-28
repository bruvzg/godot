/*************************************************************************/
/*  font_implementation_fb.cpp                                           */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "font_implementation_fb.h"
#include "core/os/file_access.h"
#include "core/os/os.h"

#include FT_STROKER_H
#include FT_TRUETYPE_TAGS_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H

#define __STDC_LIMIT_MACROS
#include <stdint.h>

/*************************************************************************/
/*  Bitmap Font                                                          */
/*************************************************************************/

void FontImplementationBitmap::clear() {
	valid = false;
	ascent = 0.f;
	height = 0.f;
	name = String();
	supported_scripts.clear();
	supported_langs.clear();
	textures.clear();
	glyph_map.clear();
	kerning_map.clear();
}

void FontImplementationBitmap::invalidate() {
	valid = false;
}

bool FontImplementationBitmap::require_reload() const {
	return !valid;
}

void *FontImplementationBitmap::get_native_handle() const {
	//not used in the fallback implementation
	return NULL;
}

float FontImplementationBitmap::get_font_scale() const {
	return 1.0f;
}

uint32_t FontImplementationBitmap::get_glyph(uint32_t p_unicode, uint32_t p_variation_selector) const {
	if (!valid)
		return 0;

	const Glyph *gl = glyph_map.getptr(p_unicode);
	if (!gl) {
		return 0;
	}
	//variation selectors are not supported by bitmap font
	return p_unicode;
}

float FontImplementationBitmap::FontImplementationBitmap::get_advance(uint32_t p_glyph) const {
	if (!valid)
		return 0.f;
	const Glyph *gl = glyph_map.getptr(p_glyph);
	if (!gl) {
		return 0.f;
	}
	return gl->advance;
}

float FontImplementationBitmap::get_kerning(uint32_t p_glyph_a, uint32_t p_glyph_b) const {
	if (!valid)
		return 0.f;
	KerningPairKey kpk;
	kpk.A = p_glyph_a;
	kpk.B = p_glyph_b;

	const Map<KerningPairKey, int>::Element *E = kerning_map.find(kpk);
	if (E)
		return E->get();

	return 0.f;
}

float FontImplementationBitmap::FontImplementationBitmap::get_v_advance(uint32_t p_glyph) const {
	return get_advance(p_glyph);
}

float FontImplementationBitmap::get_v_kerning(uint32_t p_glyph_a, uint32_t p_glyph_b) const {
	return get_kerning(p_glyph_a, p_glyph_b);
}

Size2 FontImplementationBitmap::get_glyph_size(uint32_t p_glyph) const {
	if (!valid)
		return Size2();
	const Glyph *gl = glyph_map.getptr(p_glyph);
	if (!gl) {
		return Size2();
	}
	return gl->rect.size;
}

float FontImplementationBitmap::get_ascent() const {
	if (!valid)
		return 0.f;
	return ascent;
}

float FontImplementationBitmap::get_descent() const {
	if (!valid)
		return 0.f;
	return height - ascent;
}

float FontImplementationBitmap::get_line_gap() const {
	//not supported
	return 0.f;
}

float FontImplementationBitmap::get_underline_position() const {
	return 2.0f;
}

float FontImplementationBitmap::get_underline_thickness() const {
	return 1.0f;
}

String FontImplementationBitmap::get_name() const {
	if (!valid)
		return "";
	return name;
}

int32_t FontImplementationBitmap::get_script_support_priority(uint32_t p_script) const {
	if (!valid)
		return 0;
	const int32_t *pri = supported_scripts.getptr(p_script);
	if (!pri) {
		return 0;
	}
	return *pri;
}

int32_t FontImplementationBitmap::get_language_support_priority(const String &p_lang) const {
	return (supported_langs.find(p_lang) == -1) ? 0 : 1000;
}

void FontImplementationBitmap::draw_glyph(RID p_canvas_item, const Point2 &p_pos, uint32_t p_glyph, const Color &p_modulate, bool p_rotate_cw) const {
	if (!valid)
		return;
	const Glyph *gl = glyph_map.getptr(p_glyph);
	if (!gl) {
		return;
	}
	ERR_FAIL_COND(gl->texture_idx < -1 || gl->texture_idx >= textures.size());
	if (gl->texture_idx != -1) {
		Rect2 crect = Rect2(p_pos, gl->rect.size);
		crect.position.x += gl->h_align;
		crect.position.y -= ascent;
		crect.position.y += gl->v_align;
		if (p_rotate_cw) {
			crect.size.x *= -1;
		}
		//if (p_rotate_ccw) {
		//	crect.size.y *= -1;
		//}
		VisualServer::get_singleton()->canvas_item_add_texture_rect_region(p_canvas_item, crect, textures[gl->texture_idx]->get_rid(), gl->rect, p_modulate, p_rotate_cw, RID(), false);
	}
}

Error FontImplementationBitmap::create(const FontData *p_data, FontData::CacheID p_cache_id) {
	//fnt format used by angelcode bmfont
	//http://www.angelcode.com/products/bmfont/

	clear();

	if (p_cache_id.outline_size != 0) {
		return OK; //no outlines!
	}

	if (p_data->get_font_data_path() == String()) {

		const uint8_t *data_mem = NULL;
		p_data->_get_static_font_data(&data_mem, NULL);

		const void **data = (const void **)data_mem;

		height = *(int *)data[0];
		ascent = *(int *)data[1];
		int charcount = *(int *)data[2];
		const int *char_rects = (const int *)data[3];
		int kerning_count = *(int *)data[4];
		const int *kernings = (const int *)data[5];
		//int w = *(int *)data[6];
		//int h = *(int *)data[7];
		const uint8_t *img = (const uint8_t *)data[8];

		if (!img || charcount == 0) {
			ERR_FAIL_V_MSG(ERR_UNCONFIGURED, "BitmapFont uninitialized.");
		}

		name = "[Static]";

		Ref<Image> image = memnew(Image(img));
		Ref<ImageTexture> tex = memnew(ImageTexture);
		tex->create_from_image(image);

		add_texture(tex);

		for (int i = 0; i < charcount; i++) {

			const int *c = &char_rects[i * 8];

			int chr = c[0];
			Rect2 frect;
			frect.position.x = c[1];
			frect.position.y = c[2];
			frect.size.x = c[3];
			frect.size.y = c[4];
			Point2 align(c[6], c[5]);
			int advance = c[7];

			add_glyph(chr, 0, frect, align, advance);
		}

		for (int i = 0; i < kerning_count; i++) {
			add_kerning_pair(kernings[i * 3 + 0], kernings[i * 3 + 1], kernings[i * 3 + 2]);
		}

		valid = true;

		return OK;
	} else {
		FileAccess *f = FileAccess::open(p_data->get_font_data_path(), FileAccess::READ);

		ERR_FAIL_COND_V_MSG(!f, ERR_FILE_NOT_FOUND, "Can't open font: " + p_data->get_font_data_path() + ".");

		name = "[Unknown]";

		while (true) {

			String line = f->get_line();

			int delimiter = line.find(" ");
			String type = line.substr(0, delimiter);
			int pos = delimiter + 1;
			Map<String, String> keys;

			while (pos < line.size() && line[pos] == ' ')
				pos++;

			while (pos < line.size()) {

				int eq = line.find("=", pos);
				if (eq == -1)
					break;
				String key = line.substr(pos, eq - pos);
				int end = -1;
				String value;
				if (line[eq + 1] == '"') {
					end = line.find("\"", eq + 2);
					if (end == -1)
						break;
					value = line.substr(eq + 2, end - 1 - eq - 1);
					pos = end + 1;
				} else {
					end = line.find(" ", eq + 1);
					if (end == -1)
						end = line.size();

					value = line.substr(eq + 1, end - eq);

					pos = end;
				}

				while (pos < line.size() && line[pos] == ' ')
					pos++;

				keys[key] = value;
			}

			if (type == "info") {

				if (keys.has("face"))
					set_name(keys["face"]);
				//if (keys.has("size"))
				//	set_size(keys["size"].to_int());

			} else if (type == "common") {

				if (keys.has("lineHeight"))
					set_height(keys["lineHeight"].to_int());
				if (keys.has("base"))
					set_ascent(keys["base"].to_int());

			} else if (type == "page") {

				if (keys.has("file")) {

					String base_dir = p_data->get_font_data_path().get_base_dir();
					String file = base_dir.plus_file(keys["file"]);
					Ref<Texture> tex = ResourceLoader::load(file);
					if (tex.is_null()) {
						ERR_PRINT("Can't load font texture!");
					} else {
						add_texture(tex);
					}
				}
			} else if (type == "char") {

				CharType idx = 0;
				if (keys.has("id"))
					idx = keys["id"].to_int();

				Rect2 rect;

				if (keys.has("x"))
					rect.position.x = keys["x"].to_int();
				if (keys.has("y"))
					rect.position.y = keys["y"].to_int();
				if (keys.has("width"))
					rect.size.width = keys["width"].to_int();
				if (keys.has("height"))
					rect.size.height = keys["height"].to_int();

				Point2 ofs;

				if (keys.has("xoffset"))
					ofs.x = keys["xoffset"].to_int();
				if (keys.has("yoffset"))
					ofs.y = keys["yoffset"].to_int();

				int texture = 0;
				if (keys.has("page"))
					texture = keys["page"].to_int();
				int advance = -1;
				if (keys.has("xadvance"))
					advance = keys["xadvance"].to_int();

				add_glyph(idx, texture, rect, ofs, advance);
			} else if (type == "kerning") {

				CharType first = 0, second = 0;
				int k = 0;

				if (keys.has("first"))
					first = keys["first"].to_int();
				if (keys.has("second"))
					second = keys["second"].to_int();
				if (keys.has("amount"))
					k = keys["amount"].to_int();

				add_kerning_pair(first, second, -k);
			}

			if (f->eof_reached())
				break;
		}

		memdelete(f);

		supported_langs = p_data->get_force_supported_languages().split(",");

		valid = true;

		return OK;
	}

	ERR_FAIL_V_MSG(ERR_UNCONFIGURED, "BitmapFont uninitialized.");
}

void FontImplementationBitmap::set_name(const String &p_name) {
	name = p_name;
}

void FontImplementationBitmap::set_ascent(float p_ascent) {
	ascent = p_ascent;
}

void FontImplementationBitmap::set_height(float p_height) {
	height = p_height;
}

void FontImplementationBitmap::add_texture(const Ref<Texture> &p_texture) {
	ERR_FAIL_COND(p_texture.is_null());
	textures.push_back(p_texture);
}

void FontImplementationBitmap::add_glyph(uint32_t p_glyph, int p_texture_idx, const Rect2 &p_rect, const Size2 &p_align, int p_advance) {
	Glyph gl;
	gl.rect = p_rect;
	gl.texture_idx = p_texture_idx;
	gl.v_align = p_align.y;
	gl.advance = (p_advance < 0) ? p_rect.size.width : p_advance;
	gl.h_align = p_align.x;
	glyph_map[p_glyph] = gl;
}

void FontImplementationBitmap::add_kerning_pair(uint32_t p_A, uint32_t p_B, int p_kerning) {
	KerningPairKey kpk;
	kpk.A = p_A;
	kpk.B = p_B;
	if (p_kerning == 0 && kerning_map.has(kpk)) {
		kerning_map.erase(kpk);
	} else {
		kerning_map[kpk] = p_kerning;
	}
}

FontImplementationBitmap::FontImplementationBitmap() {
	clear();
}

FontImplementationBitmap::~FontImplementationBitmap() {
	clear();
}

/*************************************************************************/
/*  Dynamic Font                                                         */
/*************************************************************************/

#ifdef MODULE_FREETYPE_ENABLED

HashMap<String, Vector<uint8_t>> FontImplementationDynamic::_fontdata;

FontImplementationDynamic::Glyph FontImplementationDynamic::Glyph::not_found() {
	Glyph gl;
	gl.texture_idx = -1;

	gl.advance = 0.f;
	gl.found = false;
	return gl;
}

FontImplementationDynamic::Glyph FontImplementationDynamic::_make_outline_glyph(uint32_t p_index) {
	Glyph ret = Glyph::not_found();

	if (FT_Load_Glyph(face, p_index, FT_LOAD_NO_BITMAP | (id.force_autohinter ? FT_LOAD_FORCE_AUTOHINT : 0)) != 0)
		return ret;

	FT_Stroker stroker;
	if (FT_Stroker_New(library, &stroker) != 0)
		return ret;

	FT_Stroker_Set(stroker, (int)(id.outline_size * oversampling * 64.0), FT_STROKER_LINECAP_BUTT, FT_STROKER_LINEJOIN_ROUND, 0);
	FT_Glyph glyph;
	FT_BitmapGlyph glyph_bitmap;

	if (FT_Get_Glyph(face->glyph, &glyph) != 0)
		goto cleanup_stroker;
	if (FT_Glyph_Stroke(&glyph, stroker, 1) != 0)
		goto cleanup_glyph;
	if (FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1) != 0)
		goto cleanup_glyph;

	glyph_bitmap = (FT_BitmapGlyph)glyph;

	ret = _bitmap_to_glyph(glyph_bitmap->bitmap, glyph_bitmap->top, glyph_bitmap->left, glyph->advance.x / 1024.f);

cleanup_glyph:
	FT_Done_Glyph(glyph);
cleanup_stroker:
	FT_Stroker_Done(stroker);
	return ret;
}

FontImplementationDynamic::TexturePosition FontImplementationDynamic::_find_texture_pos_for_glyph(int p_color_size, Image::Format p_image_format, int p_width, int p_height) {
	TexturePosition ret;
	ret.index = -1;
	ret.x = 0;
	ret.y = 0;

	int mw = p_width;
	int mh = p_height;

	for (int i = 0; i < textures.size(); i++) {

		const GlyphTexture &ct = textures[i];

		if (ct.texture->get_format() != p_image_format)
			continue;

		if (mw > ct.texture_size || mh > ct.texture_size) //too big for this texture
			continue;

		ret.y = 0x7FFFFFFF;
		ret.x = 0;

		for (int j = 0; j < ct.texture_size - mw; j++) {

			int max_y = 0;

			for (int k = j; k < j + mw; k++) {

				int y = ct.offsets[k];
				if (y > max_y)
					max_y = y;
			}

			if (max_y < ret.y) {
				ret.y = max_y;
				ret.x = j;
			}
		}

		if (ret.y == 0x7FFFFFFF || ret.y + mh > ct.texture_size)
			continue; //fail, could not fit it here

		ret.index = i;
		break;
	}

	if (ret.index == -1) {
		//could not find texture to fit, create one
		ret.x = 0;
		ret.y = 0;

		int texsize = MAX(id.size * oversampling * 8, 256);
		if (mw > texsize)
			texsize = mw; //special case, adapt to it?
		if (mh > texsize)
			texsize = mh; //special case, adapt to it?

		texsize = next_power_of_2(texsize);

		texsize = MIN(texsize, 4096);

		GlyphTexture tex;
		tex.texture_size = texsize;
		tex.imgdata.resize(texsize * texsize * p_color_size); //grayscale alpha

		{
			//zero texture
			PoolVector<uint8_t>::Write w = tex.imgdata.write();
			ERR_FAIL_COND_V(texsize * texsize * p_color_size > tex.imgdata.size(), ret);
			for (int i = 0; i < texsize * texsize * p_color_size; i++) {
				w[i] = 0;
			}
		}
		tex.offsets.resize(texsize);
		for (int i = 0; i < texsize; i++) //zero offsets
			tex.offsets.write[i] = 0;

		textures.push_back(tex);
		ret.index = textures.size() - 1;
	}

	return ret;
}

FontImplementationDynamic::Glyph FontImplementationDynamic::_bitmap_to_glyph(FT_Bitmap bitmap, int yofs, int xofs, float h_advance) {
	int w = bitmap.width;
	int h = bitmap.rows;

	int mw = w + rect_margin * 2;
	int mh = h + rect_margin * 2;

	ERR_FAIL_COND_V(mw > 4096, Glyph::not_found());
	ERR_FAIL_COND_V(mh > 4096, Glyph::not_found());

	int color_size = bitmap.pixel_mode == FT_PIXEL_MODE_BGRA ? 4 : 2;
	Image::Format require_format = color_size == 4 ? Image::FORMAT_RGBA8 : Image::FORMAT_LA8;

	TexturePosition tex_pos = _find_texture_pos_for_glyph(color_size, require_format, mw, mh);
	ERR_FAIL_COND_V(tex_pos.index < 0, Glyph::not_found());

	//fit glyph in glpyh texture

	GlyphTexture &tex = textures.write[tex_pos.index];

	{
		PoolVector<uint8_t>::Write wr = tex.imgdata.write();

		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {

				int ofs = ((i + tex_pos.y + rect_margin) * tex.texture_size + j + tex_pos.x + rect_margin) * color_size;
				ERR_FAIL_COND_V(ofs >= tex.imgdata.size(), Glyph::not_found());
				switch (bitmap.pixel_mode) {
					case FT_PIXEL_MODE_MONO: {
						int byte = i * bitmap.pitch + (j >> 3);
						int bit = 1 << (7 - (j % 8));
						wr[ofs + 0] = 255; //grayscale as 1
						wr[ofs + 1] = (bitmap.buffer[byte] & bit) ? 255 : 0;
					} break;
					case FT_PIXEL_MODE_GRAY:
						wr[ofs + 0] = 255; //grayscale as 1
						wr[ofs + 1] = bitmap.buffer[i * bitmap.pitch + j];
						break;
					case FT_PIXEL_MODE_BGRA: {
						int ofs_color = i * bitmap.pitch + (j << 2);
						wr[ofs + 2] = bitmap.buffer[ofs_color + 0];
						wr[ofs + 1] = bitmap.buffer[ofs_color + 1];
						wr[ofs + 0] = bitmap.buffer[ofs_color + 2];
						wr[ofs + 3] = bitmap.buffer[ofs_color + 3];
					} break;
					// TODO: FT_PIXEL_MODE_LCD
					default:
						ERR_FAIL_V_MSG(Glyph::not_found(), "Font uses unsupported pixel format: " + itos(bitmap.pixel_mode) + ".");
						break;
				}
			}
		}
	}

	//blit to image and texture
	{

		Ref<Image> img = memnew(Image(tex.texture_size, tex.texture_size, 0, require_format, tex.imgdata));

		if (tex.texture.is_null()) {
			tex.texture.instance();
			tex.texture->create_from_image(img, Texture::FLAG_VIDEO_SURFACE | texture_flags);
		} else {
			tex.texture->set_data(img); //update
		}
	}

	// update height array
	for (int k = tex_pos.x; k < tex_pos.x + mw; k++) {
		tex.offsets.write[k] = tex_pos.y + mh;
	}

	Glyph glph;
	glph.align.x = xofs * get_font_scale();
	glph.align.y = ascent - (yofs * get_font_scale()); // + ascent - descent;
	glph.advance = h_advance * get_font_scale() / 64.0f;
	glph.texture_idx = tex_pos.index;
	glph.found = true;

	glph.rect_uv = Rect2(tex_pos.x + rect_margin, tex_pos.y + rect_margin, w, h);
	glph.rect = glph.rect_uv;
	glph.rect.position /= oversampling;
	glph.rect.size = glph.rect.size * get_font_scale();

	return glph;
}

_FORCE_INLINE_ void FontImplementationDynamic::_update_glyph(uint32_t p_index) {
	if (glyph_map.has(p_index))
		return;

	//_THREAD_SAFE_METHOD_

	Glyph glyph = Glyph::not_found();

	FT_GlyphSlot slot = face->glyph;

	int ft_hinting;

	switch (id.hinting) {
		case FontData::HINTING_NONE:
			ft_hinting = FT_LOAD_NO_HINTING;
			break;
		case FontData::HINTING_LIGHT:
			ft_hinting = FT_LOAD_TARGET_LIGHT;
			break;
		default:
			ft_hinting = FT_LOAD_TARGET_NORMAL;
			break;
	}

	int error = FT_Load_Glyph(face, p_index, FT_HAS_COLOR(face) ? FT_LOAD_COLOR : FT_LOAD_DEFAULT | (id.force_autohinter ? FT_LOAD_FORCE_AUTOHINT : 0) | ft_hinting);
	if (error) {
		glyph_map[p_index] = glyph;
		return;
	}

	if (id.outline_size > 0) {
		glyph = _make_outline_glyph(p_index);
	} else {
		error = FT_Render_Glyph(face->glyph, id.antialiased ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO);
		if (!error) {
			glyph = _bitmap_to_glyph(slot->bitmap, slot->bitmap_top, slot->bitmap_left, slot->advance.x);
		}
	}

	glyph_map[p_index] = glyph;
}

unsigned long FontImplementationDynamic::_ft_stream_io(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count) {
	FileAccess *f = (FileAccess *)stream->descriptor.pointer;

	if (f->get_position() != offset) {
		f->seek(offset);
	}

	if (count == 0)
		return 0;

	return f->get_buffer(buffer, count);
}

void FontImplementationDynamic::_ft_stream_close(FT_Stream stream) {
	FileAccess *f = (FileAccess *)stream->descriptor.pointer;
	f->close();
	memdelete(f);
}

void FontImplementationDynamic::invalidate() {
	clear();
}

bool FontImplementationDynamic::require_reload() const {
	return !valid;
}

void *FontImplementationDynamic::get_native_handle() const {
	//not used in the fallback implementation
	return NULL;
}

float FontImplementationDynamic::get_font_scale() const {
	return scale_color_font / oversampling;
}

uint32_t FontImplementationDynamic::get_glyph(uint32_t p_unicode, uint32_t p_variation_selector) const {
	if (!valid)
		return 0;

	if (p_variation_selector == 0) {
		return FT_Get_Char_Index(face, p_unicode);
	} else {
		return FT_Face_GetCharVariantIndex(face, p_unicode, p_variation_selector);
	}
}

float FontImplementationDynamic::get_advance(uint32_t p_glyph) const {
	if (!valid)
		return 0.f;
	const_cast<FontImplementationDynamic *>(this)->_update_glyph(p_glyph);

	const Glyph *gl = glyph_map.getptr(p_glyph);

	ERR_FAIL_COND_V(!gl, 0.f);
	return gl->advance;
}

float FontImplementationDynamic::get_kerning(uint32_t p_glyph_a, uint32_t p_glyph_b) const {
	if (!valid)
		return 0.f;
	FT_Vector kerningv;

	if (FT_Get_Kerning(face, p_glyph_a, p_glyph_b, FT_KERNING_DEFAULT, &kerningv)) {
		return float(kerningv.x) / get_font_scale();
	} else {
		return 0.f;
	}
}

float FontImplementationDynamic::get_v_advance(uint32_t p_glyph) const {
	if (!valid)
		return 0.f;
	FT_Fixed v;

	int ft_hinting;

	switch (id.hinting) {
		case FontData::HINTING_NONE:
			ft_hinting = FT_LOAD_NO_HINTING;
			break;
		case FontData::HINTING_LIGHT:
			ft_hinting = FT_LOAD_TARGET_LIGHT;
			break;
		default:
			ft_hinting = FT_LOAD_TARGET_NORMAL;
			break;
	}

	if (unlikely(FT_Get_Advance(face, p_glyph, FT_LOAD_DEFAULT | (id.force_autohinter ? FT_LOAD_FORCE_AUTOHINT : 0) | ft_hinting | FT_LOAD_VERTICAL_LAYOUT, &v)))
		return 0.f;

	return (-v + (1 << 9)) >> 10;
}

float FontImplementationDynamic::get_v_kerning(uint32_t p_glyph_a, uint32_t p_glyph_b) const {
	if (!valid)
		return 0.f;
	FT_Vector kerningv;

	if (FT_Get_Kerning(face, p_glyph_a, p_glyph_b, FT_KERNING_DEFAULT | FT_LOAD_VERTICAL_LAYOUT, &kerningv)) {
		return float(kerningv.x) / get_font_scale();
	} else {
		return 0.f;
	}
}

Size2 FontImplementationDynamic::get_glyph_size(uint32_t p_glyph) const {
	if (!valid)
		return Size2();
	const_cast<FontImplementationDynamic *>(this)->_update_glyph(p_glyph);

	const Glyph *gl = glyph_map.getptr(p_glyph);

	ERR_FAIL_COND_V(!gl, Size2());
	return gl->rect.size;
}

float FontImplementationDynamic::get_ascent() const {
	if (!valid)
		return 0.f;
	return ascent;
}

float FontImplementationDynamic::get_descent() const {
	if (!valid)
		return 0.f;
	return descent;
}

float FontImplementationDynamic::get_line_gap() const {
	if (!valid)
		return 0.f;
	return linegap;
}

float FontImplementationDynamic::get_underline_position() const {
	if (!valid)
		return 0.f;
	return underline_position;
}

float FontImplementationDynamic::get_underline_thickness() const {
	if (!valid)
		return 0.f;
	return underline_thickness;
}

String FontImplementationDynamic::get_name() const {
	if (!valid)
		return "";
	return name;
}

int32_t FontImplementationDynamic::get_script_support_priority(uint32_t p_script) const {
	if (!valid)
		return 0;
	const int32_t *pri = supported_scripts.getptr(p_script);
	if (!pri) {
		return 0;
	}
	return *pri;
}

int32_t FontImplementationDynamic::get_language_support_priority(const String &p_lang) const {
	return (supported_langs.find(p_lang) == -1) ? 0 : 1000;
}

void FontImplementationDynamic::draw_glyph(RID p_canvas_item, const Point2 &p_pos, uint32_t p_glyph, const Color &p_modulate, bool p_rotate_cw) const {
	if (!valid)
		return;

	const_cast<FontImplementationDynamic *>(this)->_update_glyph(p_glyph);
	const Glyph *gl = glyph_map.getptr(p_glyph);

	if (gl->found) {
		ERR_FAIL_COND(gl->texture_idx < -1 || gl->texture_idx >= textures.size());
		if (gl->texture_idx != -1) {
			Rect2 uvrect = gl->rect_uv;
			Rect2 crect = Rect2(p_pos, gl->rect.size);
			crect.position.x += gl->align.x;
			crect.position.y -= ascent;
			crect.position.y += gl->align.y;
			if (p_rotate_cw) {
				crect.size.x *= -1;
			}
			//if (p_rotate_ccw) {
			//	crect.size.y *= -1;
			//}
			Color modulate = p_modulate;
			if (FT_HAS_COLOR(face)) {
				modulate.r = modulate.g = modulate.b = 1.0;
			}
			RID texture = textures[gl->texture_idx].texture->get_rid();
			VisualServer::get_singleton()->canvas_item_add_texture_rect_region(p_canvas_item, crect, texture, uvrect, modulate, p_rotate_cw, RID(), false);
		}
	}
}

void FontImplementationDynamic::clear() {
	if (valid) {
		FT_Done_FreeType(library);
	}
	valid = false;
	glyph_map.clear();
	supported_scripts.clear();
	supported_langs.clear();
	rect_margin = 1;
	ascent = 1;
	descent = 1;
	linegap = 1;
	texture_flags = 0;
	scale_color_font = 1;
	name = "";
}

Error FontImplementationDynamic::create(const FontData *p_data, FontData::CacheID p_cache_id) {
	clear();

	id = p_cache_id;

	int error = FT_Init_FreeType(&library);
	ERR_FAIL_COND_V_MSG(error != 0, ERR_CANT_CREATE, "Error initializing FreeType.");

	const uint8_t *data_mem = NULL;
	size_t data_mem_size = 0;
	p_data->_get_static_font_data(&data_mem, &data_mem_size);

	name = "[Unknown]";

	// FT_OPEN_STREAM is extremely slow only on Android.
	if (OS::get_singleton()->get_name() == "Android" && data_mem == NULL && p_data->get_font_data_path() != String()) {
		// cache font only once for each fd->get_font_data_path()
		if (_fontdata.has(p_data->get_font_data_path())) {
			data_mem = _fontdata[p_data->get_font_data_path()].ptr();
			data_mem_size = _fontdata[p_data->get_font_data_path()].size();
		} else {
			FileAccess *f = FileAccess::open(p_data->get_font_data_path(), FileAccess::READ);
			ERR_FAIL_COND_V(!f, ERR_CANT_OPEN);

			size_t len = f->get_len();
			_fontdata[p_data->get_font_data_path()] = Vector<uint8_t>();
			Vector<uint8_t> &fontdata = _fontdata[p_data->get_font_data_path()];
			fontdata.resize(len);
			f->get_buffer(fontdata.ptrw(), len);
			data_mem = fontdata.ptr();
			data_mem_size = len;
			f->close();
		}
	}

	if (data_mem == NULL && p_data->get_font_data_path() != String()) {
		FileAccess *f = FileAccess::open(p_data->get_font_data_path(), FileAccess::READ);
		ERR_FAIL_COND_V(!f, ERR_CANT_OPEN);

		memset(&stream, 0, sizeof(FT_StreamRec));
		stream.base = NULL;
		stream.size = f->get_len();
		stream.pos = 0;
		stream.descriptor.pointer = f;
		stream.read = _ft_stream_io;
		stream.close = _ft_stream_close;

		FT_Open_Args fargs;
		memset(&fargs, 0, sizeof(FT_Open_Args));
		fargs.flags = FT_OPEN_STREAM;
		fargs.stream = &stream;
		error = FT_Open_Face(library, &fargs, 0, &face);
	} else if (data_mem) {
		memset(&stream, 0, sizeof(FT_StreamRec));
		stream.base = (unsigned char *)data_mem;
		stream.size = data_mem_size;
		stream.pos = 0;

		FT_Open_Args fargs;
		memset(&fargs, 0, sizeof(FT_Open_Args));
		fargs.memory_base = (unsigned char *)data_mem;
		fargs.memory_size = data_mem_size;
		fargs.flags = FT_OPEN_MEMORY;
		fargs.stream = &stream;
		error = FT_Open_Face(library, &fargs, 0, &face);
	} else {
		ERR_FAIL_V_MSG(ERR_UNCONFIGURED, "DynamicFont uninitialized.");
	}

	if (error == FT_Err_Unknown_File_Format) {
		ERR_EXPLAIN("Unknown font format.");
		FT_Done_FreeType(library);
	} else if (error) {
		ERR_EXPLAIN("Error loading font.");
		FT_Done_FreeType(library);
	}

	ERR_FAIL_COND_V(error, ERR_FILE_CANT_OPEN);

	if (FT_HAS_COLOR(face)) {
		int best_match = 0;
		int diff = ABS(id.size - ((int64_t)face->available_sizes[0].width));
		scale_color_font = float(id.size) / face->available_sizes[0].width;
		oversampling = 1.0f;
		for (int i = 1; i < face->num_fixed_sizes; i++) {
			int ndiff = ABS(id.size - ((int64_t)face->available_sizes[i].width));
			if (ndiff < diff) {
				best_match = i;
				diff = ndiff;
				scale_color_font = float(id.size) / face->available_sizes[i].width;
			}
		}
		FT_Select_Size(face, best_match);
	} else {
		oversampling = Math::floor(id.size * Font::get_oversampling()) / id.size;
		FT_Set_Pixel_Sizes(face, 0, id.size * oversampling);
	}

	ascent = (face->size->metrics.ascender * (scale_color_font / oversampling)) / 64.0;
	descent = (-face->size->metrics.descender * (scale_color_font / oversampling)) / 64.0;
	underline_position = (-face->underline_position * (scale_color_font / oversampling)) / 64.0;
	underline_thickness = (face->underline_thickness * (scale_color_font / oversampling)) / 64.0;

	linegap = 0.f;
	texture_flags = 0;
	if (id.mipmaps)
		texture_flags |= Texture::FLAG_MIPMAPS;
	if (id.filter)
		texture_flags |= Texture::FLAG_FILTER;

	FT_Select_Charmap(face, FT_ENCODING_UNICODE);

	int ncnt = FT_Get_Sfnt_Name_Count(face);
	for (int i = 0; i < ncnt; i++) {
		FT_SfntName ft_name_info;
		FT_Get_Sfnt_Name(face, i, &ft_name_info);
		if (ft_name_info.name_id == TT_NAME_ID_FULL_NAME) {
			if ((ft_name_info.platform_id == TT_PLATFORM_APPLE_UNICODE) || ((ft_name_info.platform_id == TT_PLATFORM_MICROSOFT) && ((ft_name_info.encoding_id == TT_MS_ID_UNICODE_CS) || (ft_name_info.encoding_id == TT_MS_ID_UCS_4)))) {
				name = String((const CharType *)ft_name_info.string, ft_name_info.string_len);
				break;
			}
		}
	}

	supported_langs = p_data->get_force_supported_languages().split(",");

	valid = true;

	return OK;
}

FontImplementationDynamic::FontImplementationDynamic() {
	valid = false;
	rect_margin = 1;
	underline_position = 1.f;
	underline_thickness = 1.f;
	ascent = 1.f;
	descent = 1.f;
	linegap = 1.f;
	texture_flags = 0;
	scale_color_font = 1.f;
	oversampling = 1.f;
}

FontImplementationDynamic::~FontImplementationDynamic() {
	clear();
}

#endif
