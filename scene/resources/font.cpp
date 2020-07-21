/*************************************************************************/
/*  font.cpp                                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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
#include "core/hashfuncs.h"

/*************************************************************************/
/* FontData                                                              */
/*************************************************************************/

void FontData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_rid"), &FontData::get_rid);

	ClassDB::bind_method(D_METHOD("load_system", "name"), &FontData::load_system);
	ClassDB::bind_method(D_METHOD("load_resource", "filename"), &FontData::load_resource);
	ClassDB::bind_method(D_METHOD("load_memory", "data"), &FontData::load_memory);

	ClassDB::bind_method(D_METHOD("get_height", "size"), &FontData::get_height);
	ClassDB::bind_method(D_METHOD("get_ascent", "size"), &FontData::get_ascent);
	ClassDB::bind_method(D_METHOD("get_descent", "size"), &FontData::get_descent);
	ClassDB::bind_method(D_METHOD("get_underline_position", "size"), &FontData::get_underline_position);
	ClassDB::bind_method(D_METHOD("get_underline_thickness", "size"), &FontData::get_underline_thickness);

	ClassDB::bind_method(D_METHOD("has_feature", "feature"), &FontData::has_feature);

	ClassDB::bind_method(D_METHOD("get_language_supported", "locale"), &FontData::get_language_supported);
	ClassDB::bind_method(D_METHOD("set_language_supported", "locale", "supported"), &FontData::set_language_supported);

	ClassDB::bind_method(D_METHOD("get_script_supported", "script"), &FontData::get_script_supported);
	ClassDB::bind_method(D_METHOD("set_script_supported", "script", "supported"), &FontData::set_script_supported);

	ClassDB::bind_method(D_METHOD("draw_glyph", "canvas", "size", "pos", "index", "color"), &FontData::draw_glyph, DEFVAL(Color(1, 1, 1)));
	ClassDB::bind_method(D_METHOD("draw_glyph_outline", "canvas", "size", "pos", "index", "color"), &FontData::draw_glyph_outline, DEFVAL(Color(1, 1, 1)));
	ClassDB::bind_method(D_METHOD("draw_invalid_glpyh", "canvas", "size", "pos", "index", "color"), &FontData::draw_invalid_glpyh, DEFVAL(Color(1, 1, 1)));
}

RID FontData::get_rid() const {
	return font_data;
}

bool FontData::load_system(const String &p_name) {
	ERR_FAIL_COND_V(TX != nullptr, false);
	ERR_FAIL_COND_V(!TX->has_feature(TextServer::FEATURE_SERVER_SYSTEM_FONTS), false);
	RID new_data = TX->create_font_system(p_name);
	ERR_FAIL_COND_V(new_data == RID(), false);
	if (font_data != RID()) {
		TX->free(font_data);
	}
	font_data = new_data;
	return true;
}

bool FontData::load_resource(const String &p_filename) {
	ERR_FAIL_COND_V(TX != nullptr, false);
	RID new_data = TX->create_font_resource(p_filename);
	ERR_FAIL_COND_V(new_data == RID(), false);
	if (font_data != RID()) {
		TX->free(font_data);
	}
	font_data = new_data;
	return true;
}

bool FontData::load_memory(const Vector<uint8_t> &p_data) {
	ERR_FAIL_COND_V(TX != nullptr, false);
	RID new_data = TX->create_font_memory(p_data);
	ERR_FAIL_COND_V(new_data == RID(), false);
	if (font_data != RID()) {
		TX->free(font_data);
	}
	font_data = new_data;
	return true;
}

float FontData::get_height(float p_size) const {
	ERR_FAIL_COND_V(TX != nullptr, 0.f);
	ERR_FAIL_COND_V(!font_data.is_valid(), 0.f);
	return TX->font_get_height(font_data, p_size);
}

float FontData::get_ascent(float p_size) const {
	ERR_FAIL_COND_V(TX != nullptr, 0.f);
	ERR_FAIL_COND_V(!font_data.is_valid(), 0.f);
	return TX->font_get_ascent(font_data, p_size);
}

float FontData::get_descent(float p_size) const {
	ERR_FAIL_COND_V(TX != nullptr, 0.f);
	ERR_FAIL_COND_V(!font_data.is_valid(), 0.f);
	return TX->font_get_descent(font_data, p_size);
}

float FontData::get_underline_position(float p_size) const {
	ERR_FAIL_COND_V(TX != nullptr, 0.f);
	ERR_FAIL_COND_V(!font_data.is_valid(), 0.f);
	return TX->font_get_underline_position(font_data, p_size);
}

float FontData::get_underline_thickness(float p_size) const {
	ERR_FAIL_COND_V(TX != nullptr, 0.f);
	ERR_FAIL_COND_V(!font_data.is_valid(), 0.f);
	return TX->font_get_underline_thickness(font_data, p_size);
}

bool FontData::has_feature(TextServer::FontFeature p_feature) const {
	ERR_FAIL_COND_V(TX != nullptr, false);
	ERR_FAIL_COND_V(!font_data.is_valid(), false);
	return TX->font_has_feature(font_data, p_feature);
}

bool FontData::FontData::get_language_supported(const String &p_locale) const {
	ERR_FAIL_COND_V(TX != nullptr, false);
	ERR_FAIL_COND_V(!font_data.is_valid(), false);
	return TX->font_get_language_supported(font_data, p_locale);
}

bool FontData::get_script_supported(const String &p_script) const {
	ERR_FAIL_COND_V(TX != nullptr, false);
	ERR_FAIL_COND_V(!font_data.is_valid(), false);
	return TX->font_get_script_supported(font_data, p_script);
}

void FontData::FontData::set_language_supported(const String &p_locale, bool p_value) {
	ERR_FAIL_COND_V(TX != nullptr, false);
	ERR_FAIL_COND_V(!font_data.is_valid(), false);
	TX->font_set_language_supported(font_data, p_locale, p_value);
}

void FontData::set_script_supported(const String &p_script, bool p_value) {
	ERR_FAIL_COND_V(TX != nullptr, false);
	ERR_FAIL_COND_V(!font_data.is_valid(), false);
	TX->font_set_script_supported(font_data, p_script, p_value);
}

void FontData::draw_glyph(RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color) const{
	ERR_FAIL_COND(TX != nullptr);
	ERR_FAIL_COND(!font_data.is_valid());
	TX->font_draw_glyph(font_data, p_canvas, p_size, p_pos, p_index, p_color);
}

void FontData::draw_glyph_outline(RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color) const {
	ERR_FAIL_COND(TX != nullptr);
	ERR_FAIL_COND(!font_data.is_valid());
	TX->font_draw_glyph_outline(font_data, p_canvas, p_size, p_pos, p_index, p_color);
}

void FontData::draw_invalid_glpyh(RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color) const {
	ERR_FAIL_COND(TX != nullptr);
	ERR_FAIL_COND(!font_data.is_valid());
	TX->font_draw_invalid_glpyh(font_data, p_canvas, p_size, p_pos, p_index, p_color);
}

FontData::~FontData() {
	if (TX != nullptr && font_data.is_valid()) {
		TX->free(font_data);
		font_data = RID();
	}
}

/*************************************************************************/
/* Font                                                                */
/*************************************************************************/

bool Font::_set(const StringName &p_name, const Variant &p_value) {
	String str = p_name;
	if (str.begins_with("data/")) {
		int idx = str.get_slicec('/', 1).to_int();
		Ref<FontData> fd = p_value;
		if (fd.is_valid()) {
			if (idx == font.size()) {
				add_data(fd);
				return true;
			} else if (idx >= 0 && idx < font.size()) {
				set_data(idx, fd);
				return true;
			} else {
				return false;
			}
		} else if (idx >= 0 && idx < font.size()) {
			remove_data(idx);
			return true;
		}
	}
	return false;
}

bool Font::_get(const StringName &p_name, Variant &r_ret) const {
	String str = p_name;
	if (str.begins_with("data/")) {
		int idx = str.get_slicec('/', 1).to_int();
		if (idx == font.size()) {
			r_ret = Ref<FontData>();
			return true;
		} else if (idx >= 0 && idx < font.size()) {
			r_ret = get_data(idx);
			return true;
		}
	}
	return false;
}

void Font::_get_property_list(List<PropertyInfo> *p_list) const {
	for (int i = 0; i < font.size(); i++) {
		p_list->push_back(PropertyInfo(Variant::OBJECT, "font/" + itos(i), PROPERTY_HINT_RESOURCE_TYPE, "FontData"));
		p_list->push_back(PropertyInfo(Variant::PACKED_STRING_ARRAY, "font/" + itos(i) + "/language_override"));
		p_list->push_back(PropertyInfo(Variant::PACKED_STRING_ARRAY, "font/" + itos(i) + "/script_override"));
	}
	p_list->push_back(PropertyInfo(Variant::OBJECT, "font/" + itos(font.size()), PROPERTY_HINT_RESOURCE_TYPE, "FontData"));
}

void Font::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_height", "size"), &Font::get_height);
	ClassDB::bind_method(D_METHOD("get_ascent", "size"), &Font::get_ascent);
	ClassDB::bind_method(D_METHOD("get_descent", "size"), &Font::get_descent);
	ClassDB::bind_method(D_METHOD("get_underline_position", "size"), &Font::get_underline_position);
	ClassDB::bind_method(D_METHOD("get_underline_thickness", "size"), &Font::get_underline_thickness);

	ClassDB::bind_method(D_METHOD("get_string_size", "string", "size"), &Font::get_string_size);
	ClassDB::bind_method(D_METHOD("get_wordwrap_string_size", "string", "size", "width"), &Font::get_wordwrap_string_size);

	ClassDB::bind_method(D_METHOD("draw", "canvas_item", "size", "pos", "text", "modulate", "clip_w", "outline_modulate"), &Font::draw, DEFVAL(Color(1, 1, 1)), DEFVAL(-1), DEFVAL(Color(0, 0, 0, 0)));
	ClassDB::bind_method(D_METHOD("draw_halign", "canvas_item", "size", "pos", "text", "align", "width", "modulate", "outline_modulate"), &Font::draw_halign, DEFVAL(Color(1, 1, 1)), DEFVAL(Color(0, 0, 0, 0)));
	ClassDB::bind_method(D_METHOD("draw_wordwrap", "canvas_item", "size", "pos", "text", "align", "width", "modulate", "outline_modulate"), &Font::draw_wordwrap, DEFVAL(Color(1, 1, 1)), DEFVAL(Color(0, 0, 0, 0)));

	ClassDB::bind_method(D_METHOD("get_data_count"), &Font::get_data_count);
	ClassDB::bind_method(D_METHOD("add_data", "data"), &Font::add_data);
	ClassDB::bind_method(D_METHOD("set_data", "idx", "data"), &Font::set_data);
	ClassDB::bind_method(D_METHOD("set_data_language_support_override", "idx", "locales"), &Font::set_data_language_support_override);
	ClassDB::bind_method(D_METHOD("set_data_script_support_override", "idx", "scripts"), &Font::set_data_script_support_override);
	ClassDB::bind_method(D_METHOD("get_data", "idx"), &Font::get_data);
	ClassDB::bind_method(D_METHOD("get_data_language_support_override", "idx"), &Font::get_data_language_support_override);
	ClassDB::bind_method(D_METHOD("get_data_script_support_override", "idx"), &Font::get_data_script_support_override);
	ClassDB::bind_method(D_METHOD("remove_data", "idx"), &Font::remove_data);
}

float Font::get_height(float p_size) const {
	float height = 0.f;
	for (int i = 0; i < font.size(); i++) {
		height = MAX(height, font[i].data->get_height(p_size));
	}
	return height;
}

float Font::get_ascent(float p_size) const {
	float ascent = 0.f;
	for (int i = 0; i < font.size(); i++) {
		ascent = MAX(ascent, font[i].data->get_ascent(p_size));
	}
	return ascent;
}

float Font::get_descent(float p_size) const {
	float descent = 0.f;
	for (int i = 0; i < font.size(); i++) {
		descent = MAX(descent, font[i].data->get_descent(p_size));
	}
	return descent;
}

float Font::get_underline_position(float p_size) const {
	float ulpos = 0.f;
	for (int i = 0; i < font.size(); i++) {
		ulpos = MAX(ulpos, font[i].data->get_underline_position(p_size));
	}
	return ulpos;
}

float Font::get_underline_thickness(float p_size) const {
	float ultk = 0.f;
	for (int i = 0; i < font.size(); i++) {
		ultk = MAX(ultk, font[i].data->get_underline_thickness(p_size));
	}
	return ultk;
}

Size2 Font::get_string_size(const String &p_string, float p_size) const {
	ERR_FAIL_COND_V(TX != nullptr, Size2());

	uint64_t hash = p_string.hash64();
	hash = hash_djb2_one_64(hash_djb2_one_float(p_size), hash);
	RID *ctx = cache.get_ptr(hash);
	if (ctx == nullptr) {
		// Shape & cache.
		RID new_ctx = TX->create_shaped_text();
		TX->shaped_add_text(new_ctx, p_string, get_data_list(), p_size);
		ctx = cahce.insert(hash, new_ctx);
	}
	return TX->shaped_get_size(*ctx);
}

Size2 Font::get_wordwrap_string_size(const String &p_string, float p_size, float p_width) const {
	ERR_FAIL_COND_V(TX != nullptr, Size2());

	uint64_t hash = p_string.hash64();
	hash = hash_djb2_one_64(hash_djb2_one_float(p_size), hash);
	RID *ctx = cache.get_ptr(hash);
	if (ctx == nullptr) {
		// Shape & cache.
		RID new_ctx = TX->create_shaped_text();
		TX->shaped_add_text(new_ctx, p_string, get_data_list(), p_size);
		ctx = cahce.insert(hash, new_ctx);
	}
	Size2 size;
	Vector<Vector2i> lines = TX->shaped_get_line_breaks(*ctx, p_width);
	for (int i = 0; i < lines.size(); i++) {
		RID line = TX->shaped_create_substr(*ctx, lines[i].x, lines[i].y);
		Size2 line_size = TX->shaped_get_size(line);
		size.x = MAX(size.x, line_size.x);
		size.y = size.y + line_size.y + TX->shaped_get_line_spacing(line);
		TX->free(line);
	}
	return size;
}

void Font::_draw_ctx(RID p_canvas_item, float p_size, const Point2 &p_pos, RID p_ctx, const Color &p_modulate, int p_clip_w, const Color &p_outline_modulate) const {
	Vector<TextServer::Grapheme> graph = TX->shaped_get_graphemes(p_ctx);
	Vector2 off = p_pos;
	if (p_outline_modulate.a != 0) {
		// Draw outline.
		for (int i = 0; i < graph.size(); i++) {
			if (p_clip_w > 0 && ofs.x > p_clip_w) {
				break;
			}
			if ((graph[i].flags & TEXT_GRAPHEME_FLAG_VALID) && TX->font_has_feature(TextServer::FEATURE_FONT_OUTLINE)) {
				for (int j = 0; j < graph[i].glyphs.size(); j++) {
					TX->font_draw_glyph_outline(graph[i].font, p_canvas_item, p_size, off + graph[i].glyphs[j].offset, graph[i].glyphs[j].glyph_index, p_outline_modulate);
				}
				off += graph[i].advance;
			}
		}
		off = p_pos;
	}
	// Draw text.
	for (int i = 0; i < graph.size(); i++) {
		if (p_clip_w > 0 && ofs.x > p_clip_w) {
			break;
		}
		if ((graph[i].flags & TEXT_GRAPHEME_FLAG_VALID)) {
			for (int j = 0; j < graph[i].glyphs.size(); j++) {
				TX->font_draw_glyph(graph[i].font, p_canvas_item, p_size, off + graph[i].glyphs[j].offset, graph[i].glyphs[j].glyph_index, p_modulate);
			}
		} else {
			for (int j = 0; j < graph[i].glyphs.size(); j++) {
				TX->font_draw_invalid_glyph(graph[i].font, p_canvas_item, p_size, off + graph[i].glyphs[j].offset, graph[i].glyphs[j].glyph_index, p_modulate);
			}
		}
		off += graph[i].advance;
	}
}

void Font::draw(RID p_canvas_item, float p_size, const Point2 &p_pos, const String &p_text, const Color &p_modulate, int p_clip_w, const Color &p_outline_modulate) const {
	ERR_FAIL_COND(TX != nullptr);

	uint64_t hash = p_string.hash64();
	hash = hash_djb2_one_64(hash_djb2_one_float(p_size), hash);
	RID *ctx = cache.get_ptr(hash);
	if (ctx == nullptr) {
		// Shape & cache.
		RID new_ctx = TX->create_shaped_text();
		TX->shaped_add_text(new_ctx, p_string, get_data_list(), p_size);
		ctx = cahce.insert(hash, new_ctx);
	}
	_draw_ctx(p_canvas_item, p_size, p_pos, *ctx, p_modulate, p_clip_w, p_outline_modulate);
}

void Font::draw_halign(RID p_canvas_item, float p_size, const Point2 &p_pos, const String &p_text, HAlign p_align, float p_width, const Color &p_modulate, const Color &p_outline_modulate) const {
	ERR_FAIL_COND(TX != nullptr);

	uint64_t hash = p_string.hash64();
	hash = hash_djb2_one_64(hash_djb2_one_float(p_size), hash);
	RID *ctx = cache.get_ptr(hash);
	if (ctx == nullptr) {
		// Shape & cache.
		RID new_ctx = TX->create_shaped_text();
		TX->shaped_add_text(new_ctx, p_string, get_data_list(), p_size);
		ctx = cahce.insert(hash, new_ctx);
	}

	Size2 size = TX->shaped_get_size(*ctx);
	if (size.x >= p_width) {
		_draw_ctx(p_canvas_item, p_size, p_pos, *ctx, p_modulate, p_clip_w, p_outline_modulate);
	} else {
		Vector2 off;
		switch (p_align) {
			case HALIGN_LEFT: {
				ofs.x = 0;
			} break;
			case HALIGN_CENTER: {
				ofs.x = Math::floor((p_width - size.x) / 2.0);
			} break;
			case HALIGN_RIGHT: {
				ofs.x = p_width - size.x;
			} break;
			default: {
				ERR_PRINT("Unknown alignment type");
			} break;
		}
		_draw_ctx(p_canvas_item, p_size, p_pos + ofs, *ctx, p_modulate, p_clip_w, p_outline_modulate);
	}
}

void Font::draw_wordwrap(RID p_canvas_item, float p_size, const Point2 &p_pos, const String &p_text, HAlign p_align, float p_width, const Color &p_modulate, const Color &p_outline_modulate) const {
	ERR_FAIL_COND(TX != nullptr);

	uint64_t hash = p_string.hash64();
	hash = hash_djb2_one_64(hash_djb2_one_float(p_size), hash);
	RID *ctx = cache.get_ptr(hash);
	if (ctx == nullptr) {
		// Shape & cache.
		RID new_ctx = TX->create_shaped_text();
		TX->shaped_add_text(new_ctx, p_string, get_data_list(), p_size);
		ctx = cahce.insert(hash, new_ctx);
	}
	Vector2 l_off = p_pos;
	Vector<Vector2i> lines = TX->shaped_get_line_breaks(*ctx, p_width);
	for (int i = 0; i < lines.size(); i++) {
		RID line = TX->shaped_create_substr(*ctx, lines[i].x, lines[i].y);
		l_off.x = p_pos.x;
		l_off.y = l_off.y + TX->shaped_get_line_ascent(line);
		_draw_ctx(p_canvas_item, p_size, l_off, line, p_modulate, p_width, p_outline_modulate);
		l_off.y = l_off.y + TX->shaped_get_line_descent(line) + TX->shaped_get_line_spacing(line);
		TX->free(line);
	}
	return size;
}

void Font::add_data(const Ref<FontData> &p_data) {
	ERR_FAIL_COND(p_data.is_null());
	font.push_back(p_data);
}

void Font::set_data(int p_idx, const Ref<FontData> &p_data) {
	ERR_FAIL_COND(p_data.is_null());
	ERR_FAIL_COND(p_idx < 0 || p_idx >= font.size());
	font[p_idx] = p_data;
}

int Font::get_data_count() const {
	return font.size();
}

Ref<FontData> Font::get_data(int p_idx) const {
	ERR_FAIL_COND_V(p_idx < 0 || p_idx >= font.size(), Ref<FontData>());
	return font[p_idx];
}

void Font::remove_data(int p_idx) {
	ERR_FAIL_COND(p_idx < 0 || p_idx >= font.size());
	font.remove(p_idx);
}

List<RID> Font::get_data_list() {
	List<RID> list;
	for (int i = 0; i < font.size(); i++) {
		list.push_back(font[i].data->get_rid());
	}
	return list;
}

/*************************************************************************/
/* ResourceFormatLoaderFontData                                          */
/*************************************************************************/

RES ResourceFormatLoaderFontData::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, bool p_no_cache) {
	if (r_error) {
		*r_error = ERR_FILE_CANT_OPEN;
	}

	Ref<FontData> font_data;
	font_data.instance();
	if (font_data->load_resource(p_path)) {
		if (r_error) {
			*r_error = OK;
		}
	}

	return font_data;
}

void ResourceFormatLoaderFontData::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("ttf");
	p_extensions->push_back("otf");
	p_extensions->push_back("woff");
	p_extensions->push_back("fon");
	p_extensions->push_back("font");
}

bool ResourceFormatLoaderFontData::handles_type(const String &p_type) const {
	return (p_type == "FontData");
}

String ResourceFormatLoaderFontData::get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	if (el == "ttf" || el == "otf" || el == "woff" || el == "fon" || el == "font") {
		return "FontData";
	}
	return "";
}
