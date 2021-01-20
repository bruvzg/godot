/*************************************************************************/
/*  font.cpp                                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "font.h"

#include "core/io/resource_loader.h"
#include "core/method_bind_ext.gen.inc"
#include "core/os/file_access.h"
#include "core/translation.h"

#include "scene/resources/text_line.h"
#include "scene/resources/text_paragraph.h"

void Font::draw(RID p_canvas_item, const Point2 &p_pos, const String &p_text, const Color &p_modulate, int p_clip_w, const Color &p_outline_modulate) const {
	draw_halign(p_canvas_item, p_pos, HALIGN_LEFT, p_clip_w, p_text, p_modulate, p_outline_modulate);
}

void Font::draw_halign(RID p_canvas_item, const Point2 &p_pos, HAlign p_align, float p_width, const String &p_text, const Color &p_modulate, const Color &p_outline_modulate) const {
	uint64_t hash = p_text.hash64();

	Ref<TextLine> buffer;
	if (cache.has(hash)) {
		buffer = cache.get(hash);
	} else {
		buffer.instance();
		buffer->add_string(p_text, Ref<Font>(this), base_size, Dictionary(), TranslationServer::get_singleton()->get_tool_locale());
		cache.insert(hash, buffer);
	}

	Vector2 ofs = p_pos;
	if (buffer->get_orientation() == TextServer::ORIENTATION_HORIZONTAL) {
		ofs.y += spacing_top - buffer->get_line_ascent();
	} else {
		ofs.x += spacing_top - buffer->get_line_ascent();
	}

	buffer->set_width(p_width);
	buffer->set_align(p_align);

	if (outline_size > 0 && p_outline_modulate.a != 0.0f) {
		buffer->draw_outline(p_canvas_item, ofs, outline_size, p_outline_modulate);
	}
	buffer->draw(p_canvas_item, ofs, p_modulate);
}

void Font::draw_wordwrap_string(RID p_canvas_item, const Point2 &p_pos, HAlign p_align, float p_width, const String &p_text, const Color &p_modulate, const Color &p_outline_modulate) const {
	uint64_t hash = p_text.hash64();
	uint64_t wrp_hash = hash_djb2_one_64(hash_djb2_one_float(p_width), hash);

	Ref<TextParagraph> lines_buffer;
	if (cache_wrap.has(wrp_hash)) {
		lines_buffer = cache_wrap.get(wrp_hash);
	} else {
		lines_buffer.instance();
		lines_buffer->add_string(p_text, Ref<Font>(this), base_size, Dictionary(), TranslationServer::get_singleton()->get_tool_locale());
		lines_buffer->set_width(p_width);
		cache_wrap.insert(wrp_hash, lines_buffer);
	}

	lines_buffer->set_align(p_align);

	Vector2 lofs = p_pos;
	for (int i = 0; i < lines_buffer->get_line_count(); i++) {
		if (lines_buffer->get_orientation() == TextServer::ORIENTATION_HORIZONTAL) {
			lofs.y += spacing_top;
			if (i == 0) {
				lofs.y -= lines_buffer->get_line_ascent(0);
			}
		} else {
			lofs.x += spacing_top;
			if (i == 0) {
				lofs.x -= lines_buffer->get_line_ascent(0);
			}
		}
		if (p_width > 0) {
			lines_buffer->set_align(p_align);
		}

		if (outline_size > 0 && p_outline_modulate.a != 0.0f) {
			lines_buffer->draw_line_outline(p_canvas_item, lofs, i, outline_size, p_outline_modulate);
		}
		lines_buffer->draw_line(p_canvas_item, lofs, i, p_modulate);

		Size2 line_size = lines_buffer->get_line_size(i);
		if (lines_buffer->get_orientation() == TextServer::ORIENTATION_HORIZONTAL) {
			lofs.y += line_size.y + spacing_bottom;
		} else {
			lofs.x += line_size.x + spacing_bottom;
		}
	}
}

Size2 Font::get_string_size(const String &p_string) const {
	uint64_t hash = p_string.hash64();

	Ref<TextLine> buffer;
	if (cache.has(hash)) {
		buffer = cache.get(hash);
	} else {
		buffer.instance();
		buffer->add_string(p_string, Ref<Font>(this), base_size, Dictionary(), TranslationServer::get_singleton()->get_tool_locale());
		cache.insert(hash, buffer);
	}
	if (buffer->get_orientation() == TextServer::ORIENTATION_HORIZONTAL) {
		return buffer->get_size() + Vector2(0, spacing_top + spacing_bottom);
	} else {
		return buffer->get_size() + Vector2(spacing_top + spacing_bottom, 0);
	}
}

Size2 Font::get_wordwrap_string_size(const String &p_string, float p_width) const {
	uint64_t hash = p_string.hash64();
	uint64_t wrp_hash = hash_djb2_one_64(hash_djb2_one_float(p_width), hash);

	Ref<TextParagraph> lines_buffer;
	if (cache_wrap.has(wrp_hash)) {
		lines_buffer = cache_wrap.get(wrp_hash);
	} else {
		lines_buffer.instance();
		lines_buffer->add_string(p_string, Ref<Font>(this), base_size, Dictionary(), TranslationServer::get_singleton()->get_tool_locale());
		lines_buffer->set_width(p_width);
		cache_wrap.insert(wrp_hash, lines_buffer);
	}

	Size2 ret;
	for (int i = 0; i < lines_buffer->get_line_count(); i++) {
		Size2 line_size = lines_buffer->get_line_size(i);
		if (lines_buffer->get_orientation() == TextServer::ORIENTATION_HORIZONTAL) {
			ret.x = MAX(ret.x, line_size.x);
			ret.y += line_size.y + spacing_top + spacing_bottom;
		} else {
			ret.y = MAX(ret.y, line_size.y);
			ret.x += line_size.x + spacing_top + spacing_bottom;
		}
	}
	return ret;
}

int Font::get_spacing(int p_type) const {
	if (p_type == SPACING_TOP) {
		return spacing_top;
	} else if (p_type == SPACING_BOTTOM) {
		return spacing_bottom;
	}

	return 0;
}

void Font::update_changes() {

	emit_changed();
}

void Font::_bind_methods() {

	ClassDB::bind_method(D_METHOD("draw", "canvas_item", "position", "string", "modulate", "clip_w", "outline_modulate"), &Font::draw, DEFVAL(Color(1, 1, 1)), DEFVAL(-1), DEFVAL(Color(1, 1, 1)));
	ClassDB::bind_method(D_METHOD("draw_wordwrap_string", "canvas_item", "position", "align", "string", "clip_w", "modulate", "outline_modulate"), &Font::draw_wordwrap_string, DEFVAL(Color(1, 1, 1)), DEFVAL(-1), DEFVAL(Color(1, 1, 1)));
	ClassDB::bind_method(D_METHOD("get_size"), &Font::get_size);
	ClassDB::bind_method(D_METHOD("get_outline_size"), &Font::get_outline_size);
	ClassDB::bind_method(D_METHOD("get_ascent"), &Font::get_ascent);
	ClassDB::bind_method(D_METHOD("get_descent"), &Font::get_descent);
	ClassDB::bind_method(D_METHOD("get_height"), &Font::get_height);
	ClassDB::bind_method(D_METHOD("is_distance_field_hint"), &Font::is_distance_field_hint);
	ClassDB::bind_method(D_METHOD("get_char_size", "char", "next"), &Font::get_char_size, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("get_string_size", "string"), &Font::get_string_size);
	ClassDB::bind_method(D_METHOD("get_wordwrap_string_size", "string", "width"), &Font::get_wordwrap_string_size);
	ClassDB::bind_method(D_METHOD("has_outline"), &Font::has_outline);
	ClassDB::bind_method(D_METHOD("draw_char", "canvas_item", "position", "char", "next", "modulate", "outline"), &Font::draw_char, DEFVAL(-1), DEFVAL(Color(1, 1, 1)), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("update_changes"), &Font::update_changes);

	BIND_ENUM_CONSTANT(SPACING_TOP);
	BIND_ENUM_CONSTANT(SPACING_BOTTOM);
}

Font::Font() {}

Font::~Font() {
	cache.clear();
	cache_wrap.clear();
}

/////////////////////////////////////////////////////////////////

Error BitmapFont::create_from_memory(const uint8_t *p_data, size_t p_size) {
	if (rid != RID()) {
		TS->free(rid);
	}
	rid = TS->create_font_memory(p_data, p_size, "fnt", 0);
	base_size = TS->font_get_height(rid, 0);
	cache.clear();
	cache_wrap.clear();
	emit_changed();

	return OK;
}

Error BitmapFont::create_from_fnt(const String &p_file) {
	if (rid != RID()) {
		TS->free(rid);
	}
	rid = TS->create_font_resource(p_file, 0);
	base_size = TS->font_get_height(rid, 0);
	cache.clear();
	cache_wrap.clear();
	emit_changed();

	return OK;
}

float BitmapFont::get_height() const {
	if (rid == RID()) {
		return 0.f; // Do not raise errors in getters, to prevent editor from spamming errors on incomplete (without data_path set) fonts.
	}
	return TS->font_get_height(rid, -1);
}

float BitmapFont::get_ascent() const {
	if (rid == RID()) {
		return 0.f;
	}
	return TS->font_get_ascent(rid, -1);
}

float BitmapFont::get_descent() const {
	if (rid == RID()) {
		return 0.f;
	}
	return TS->font_get_descent(rid, -1);
}

void BitmapFont::set_distance_field_hint(bool p_distance_field) {
	ERR_FAIL_COND(rid == RID());
	TS->font_set_distance_field_hint(rid, p_distance_field);
	emit_changed();
}

bool BitmapFont::is_distance_field_hint() const {
	if (rid == RID()) {
		return false;
	}
	return TS->font_get_distance_field_hint(rid);
}

void BitmapFont::set_fallback(const Ref<BitmapFont> &p_fallback) {

	for (Ref<BitmapFont> fallback_child = p_fallback; fallback_child != NULL; fallback_child = fallback_child->get_fallback()) {
		ERR_FAIL_COND_MSG(fallback_child == this, "Can't set as fallback one of its parents to prevent crashes due to recursive loop.");
	}

	fallback = p_fallback;

	cache.clear();
	cache_wrap.clear();
}

Ref<BitmapFont> BitmapFont::get_fallback() const {

	return fallback;
}

float BitmapFont::draw_char(RID p_canvas_item, const Point2 &p_pos, char32_t p_char, char32_t p_next, const Color &p_modulate, bool p_outline) const {
	ERR_FAIL_COND_V(rid == RID(), 0);
	float adv = 0.f;
	uint32_t index_a = TS->font_get_glyph_index(rid, p_char, 0x0000);
	if (p_next != 0) {
		uint32_t index_b = TS->font_get_glyph_index(rid, p_next, 0x0000);
		adv += TS->font_get_glyph_kerning(rid, index_a, index_b, base_size).x;
	}
	return TS->font_draw_glyph(rid, p_canvas_item, base_size, p_pos, index_a, p_modulate).x + adv;
}

Size2 BitmapFont::get_char_size(char32_t p_char, char32_t p_next) const {
	ERR_FAIL_COND_V(rid == RID(), Size2());
	float adv = 0.f;
	uint32_t index_a = TS->font_get_glyph_index(rid, p_char, 0x0000);
	if (p_next != 0) {
		uint32_t index_b = TS->font_get_glyph_index(rid, p_next, 0x0000);
		adv += TS->font_get_glyph_kerning(rid, index_a, index_b, base_size).x;
	}
	return TS->font_get_glyph_advance(rid, index_a, base_size) + Vector2(adv, 0);
}

Vector<RID> BitmapFont::get_rids() const {
	Vector<RID> rids;

	rids.push_back(rid);
	Ref<BitmapFont> fb = fallback;
	while (fb.is_valid()) {
		rids.push_back(fb->rid);
		fb = fallback;
	}

	return rids;
}

void BitmapFont::set_height(float p_height) {
	if (rid != RID()) {
		TS->font_legacy_bitmap_set_height(rid, p_height);
	}
}

void BitmapFont::set_ascent(float p_ascent) {
	if (rid != RID()) {
		TS->font_legacy_bitmap_set_ascent(rid, p_ascent);
	}
}

void BitmapFont::add_texture(const Ref<Texture> &p_texture) {
	if (rid != RID()) {
		TS->font_legacy_bitmap_add_texture(rid, p_texture);
	}
}

void BitmapFont::add_char(char32_t p_char, int p_texture_idx, const Rect2 &p_rect, const Size2 &p_align, float p_advance) {
	if (rid != RID()) {
		TS->font_legacy_bitmap_add_char(rid, p_char, p_texture_idx, p_rect, p_align, p_advance);
	}
}

int BitmapFont::get_texture_count() const {
	if (rid != RID()) {
		return TS->font_legacy_bitmap_get_texture_count(rid);
	} else {
		return 0;
	}
}

Ref<Texture> BitmapFont::get_texture(int p_idx) const {
	if (rid != RID()) {
		return TS->font_legacy_bitmap_get_texture(rid, p_idx);
	} else {
		return Ref<Texture>();
	}
}

void BitmapFont::add_kerning_pair(char32_t p_A, char32_t p_B, int p_kerning) {
	if (rid != RID()) {
		TS->font_legacy_bitmap_add_kerning_pair(rid, p_A, p_B, p_kerning);
	}
}

int BitmapFont::get_kerning_pair(char32_t p_A, char32_t p_B) const {
	if (rid != RID()) {
		return TS->font_get_glyph_kerning(rid, p_A, p_B, base_size).x;
	} else {
		return 0;
	}
}

void BitmapFont::clear() {
	if (rid != RID()) {
		TS->font_legacy_bitmap_clear(rid);
	}
}

void BitmapFont::_set_chars(const PoolVector<int> &p_chars) {

	int len = p_chars.size();
	//char 1 charsize 1 texture, 4 rect, 2 align, advance 1
	ERR_FAIL_COND(len % 9);
	if (!len)
		return; //none to do
	int chars = len / 9;

	PoolVector<int>::Read r = p_chars.read();
	for (int i = 0; i < chars; i++) {

		const int *data = &r[i * 9];
		add_char(data[0], data[1], Rect2(data[2], data[3], data[4], data[5]), Size2(data[6], data[7]), data[8]);
	}
}

PoolVector<int> BitmapFont::_get_chars() const {
	PoolVector<int> chars;
	String txt = TS->font_get_supported_chars(rid);
	for (int i = 0; i < txt.length(); i++) {
		chars.push_back(txt[i]);
	}
	return chars;
}

void BitmapFont::_set_kernings(const PoolVector<int> &p_kernings) {

	int len = p_kernings.size();
	ERR_FAIL_COND(len % 3);
	if (!len)
		return;
	PoolVector<int>::Read r = p_kernings.read();

	for (int i = 0; i < len / 3; i++) {

		const int *data = &r[i * 3];
		add_kerning_pair(data[0], data[1], data[2]);
	}
}

PoolVector<int> BitmapFont::_get_kernings() const {
	PoolVector<int> kernings;

	int ctn = TS->font_legacy_bitmap_get_kerning_count(rid);

	for (int i = 0; i < ctn; i++) {
		char32_t A, B;
		int kern;

		TS->font_legacy_bitmap_get_kerning_pair(rid, i, &A, &B, &kern);

		kernings.push_back(A);
		kernings.push_back(B);
		kernings.push_back(kern);
	}

	return kernings;
}

void BitmapFont::_set_textures(const Vector<Variant> &p_textures) {

	TS->font_legacy_bitmap_clear_texture(rid);
	for (int i = 0; i < p_textures.size(); i++) {
		Ref<Texture> tex = p_textures[i];
		ERR_CONTINUE(!tex.is_valid());
		add_texture(tex);
	}
}

Vector<Variant> BitmapFont::_get_textures() const {

	Vector<Variant> rtextures;
	int ts = TS->font_legacy_bitmap_get_texture_count(rid);
	for (int i = 0; i < ts; i++)
		rtextures.push_back(TS->font_legacy_bitmap_get_texture(rid, i).get_ref_ptr());
	return rtextures;
}

Dictionary BitmapFont::get_variation_list() const {
	if (rid == RID()) {
		return Dictionary();
	}
	return TS->font_get_variation_list(rid);
}

void BitmapFont::set_variation(const String &p_name, double p_value) {
	ERR_FAIL_COND(rid == RID());
	TS->font_set_variation(rid, p_name, p_value);
	emit_changed();
}

double BitmapFont::get_variation(const String &p_name) const {
	if (rid == RID()) {
		return 0;
	}
	return TS->font_get_variation(rid, p_name);
}

void BitmapFont::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_variation_list"), &BitmapFont::get_variation_list);

	ClassDB::bind_method(D_METHOD("set_variation", "tag", "value"), &BitmapFont::set_variation);
	ClassDB::bind_method(D_METHOD("get_variation", "tag"), &BitmapFont::get_variation);

	ClassDB::bind_method(D_METHOD("create_from_fnt", "path"), &BitmapFont::create_from_fnt);
	ClassDB::bind_method(D_METHOD("set_height", "px"), &BitmapFont::set_height);

	ClassDB::bind_method(D_METHOD("set_ascent", "px"), &BitmapFont::set_ascent);

	ClassDB::bind_method(D_METHOD("add_kerning_pair", "char_a", "char_b", "kerning"), &BitmapFont::add_kerning_pair);
	ClassDB::bind_method(D_METHOD("get_kerning_pair", "char_a", "char_b"), &BitmapFont::get_kerning_pair);

	ClassDB::bind_method(D_METHOD("add_texture", "texture"), &BitmapFont::add_texture);
	ClassDB::bind_method(D_METHOD("add_char", "character", "texture", "rect", "align", "advance"), &BitmapFont::add_char, DEFVAL(Point2()), DEFVAL(-1));

	ClassDB::bind_method(D_METHOD("get_texture_count"), &BitmapFont::get_texture_count);
	ClassDB::bind_method(D_METHOD("get_texture", "idx"), &BitmapFont::get_texture);

	ClassDB::bind_method(D_METHOD("set_distance_field_hint", "enable"), &BitmapFont::set_distance_field_hint);

	ClassDB::bind_method(D_METHOD("clear"), &BitmapFont::clear);

	ClassDB::bind_method(D_METHOD("_set_chars"), &BitmapFont::_set_chars);
	ClassDB::bind_method(D_METHOD("_get_chars"), &BitmapFont::_get_chars);

	ClassDB::bind_method(D_METHOD("_set_kernings"), &BitmapFont::_set_kernings);
	ClassDB::bind_method(D_METHOD("_get_kernings"), &BitmapFont::_get_kernings);

	ClassDB::bind_method(D_METHOD("_set_textures"), &BitmapFont::_set_textures);
	ClassDB::bind_method(D_METHOD("_get_textures"), &BitmapFont::_get_textures);

	ClassDB::bind_method(D_METHOD("set_fallback", "fallback"), &BitmapFont::set_fallback);
	ClassDB::bind_method(D_METHOD("get_fallback"), &BitmapFont::get_fallback);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "textures", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "_set_textures", "_get_textures");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_INT_ARRAY, "chars", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "_set_chars", "_get_chars");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_INT_ARRAY, "kernings", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "_set_kernings", "_get_kernings");

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "height", PROPERTY_HINT_RANGE, "1,1024,1"), "set_height", "get_height");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "ascent", PROPERTY_HINT_RANGE, "0,1024,1"), "set_ascent", "get_ascent");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "distance_field"), "set_distance_field_hint", "is_distance_field_hint");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "fallback", PROPERTY_HINT_RESOURCE_TYPE, "BitmapFont"), "set_fallback", "get_fallback");
}

BitmapFont::BitmapFont() {}

BitmapFont::~BitmapFont() {
	if (rid != RID()) {
		TS->free(rid);
	}
}

////////////

RES ResourceFormatLoaderBMFont::load(const String &p_path, const String &p_original_path, Error *r_error) {

	if (r_error)
		*r_error = ERR_FILE_CANT_OPEN;

	Ref<BitmapFont> font;
	font.instance();

	Error err = font->create_from_fnt(p_path);

	if (err) {
		if (r_error)
			*r_error = err;
		return RES();
	}

	return font;
}

void ResourceFormatLoaderBMFont::get_recognized_extensions(List<String> *p_extensions) const {

	p_extensions->push_back("fnt");
}

bool ResourceFormatLoaderBMFont::handles_type(const String &p_type) const {

	return (p_type == "BitmapFont");
}

String ResourceFormatLoaderBMFont::get_resource_type(const String &p_path) const {

	String el = p_path.get_extension().to_lower();
	if (el == "fnt")
		return "BitmapFont";
	return "";
}
