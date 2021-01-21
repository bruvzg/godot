/*************************************************************************/
/*  dynamic_font.cpp                                                     */
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

#include "dynamic_font.h"
#include "core/os/file_access.h"
#include "core/os/os.h"

#include "scene/resources/text_line.h"
#include "scene/resources/text_paragraph.h"

void DynamicFontData::set_font_ptr(const uint8_t *p_font_mem, int p_font_mem_size) {
	if (rid != RID()) {
		TS->free(rid);
	}
	rid = TS->create_font_memory(p_font_mem, p_font_mem_size, "ttf", 16);
	font_path = TTR("(Memory: ttf @ 0x" + String::num_int64((uint64_t)p_font_mem, 16, true) + ")");
	emit_changed();
}

void DynamicFontData::set_font_path(const String &p_path) {
	if (rid != RID()) {
		TS->free(rid);
	}
	rid = TS->create_font_resource(p_path, 16);
	font_path = p_path;
	emit_changed();
}

String DynamicFontData::get_font_path() const {
	return font_path;
}

void DynamicFontData::set_force_autohinter(bool p_force) {
	ERR_FAIL_COND(rid == RID());
	TS->font_set_force_autohinter(rid, p_force);
	emit_changed();
}

DynamicFontData::Hinting DynamicFontData::get_hinting() const {
	if (rid == RID()) {
		return HINTING_NONE;
	}
	return (DynamicFontData::Hinting)TS->font_get_hinting(rid);
}

void DynamicFontData::set_hinting(Hinting p_hinting) {
	ERR_FAIL_COND(rid == RID());
	TS->font_set_hinting(rid, (TextServer::Hinting)p_hinting);
	emit_changed();
}

bool DynamicFontData::is_antialiased() const {
	if (rid == RID()) {
		return false;
	}
	return TS->font_get_antialiased(rid);
}

void DynamicFontData::set_antialiased(bool p_antialiased) {
	ERR_FAIL_COND(rid == RID());
	TS->font_set_antialiased(rid, p_antialiased);
	emit_changed();
}

Dictionary DynamicFontData::get_variation_list() const {
	if (rid == RID()) {
		return Dictionary();
	}
	return TS->font_get_variation_list(rid);
}

void DynamicFontData::set_variation(const String &p_name, double p_value) {
	ERR_FAIL_COND(rid == RID());
	TS->font_set_variation(rid, p_name, p_value);
	emit_changed();
}

double DynamicFontData::get_variation(const String &p_name) const {
	if (rid == RID()) {
		return 0;
	}
	return TS->font_get_variation(rid, p_name);
}

void DynamicFontData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_variation_list"), &DynamicFontData::get_variation_list);

	ClassDB::bind_method(D_METHOD("set_variation", "tag", "value"), &DynamicFontData::set_variation);
	ClassDB::bind_method(D_METHOD("get_variation", "tag"), &DynamicFontData::get_variation);

	ClassDB::bind_method(D_METHOD("set_antialiased", "antialiased"), &DynamicFontData::set_antialiased);
	ClassDB::bind_method(D_METHOD("is_antialiased"), &DynamicFontData::is_antialiased);
	ClassDB::bind_method(D_METHOD("set_font_path", "path"), &DynamicFontData::set_font_path);
	ClassDB::bind_method(D_METHOD("get_font_path"), &DynamicFontData::get_font_path);
	ClassDB::bind_method(D_METHOD("set_hinting", "mode"), &DynamicFontData::set_hinting);
	ClassDB::bind_method(D_METHOD("get_hinting"), &DynamicFontData::get_hinting);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "antialiased"), "set_antialiased", "is_antialiased");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "hinting", PROPERTY_HINT_ENUM, "None,Light,Normal"), "set_hinting", "get_hinting");

	BIND_ENUM_CONSTANT(HINTING_NONE);
	BIND_ENUM_CONSTANT(HINTING_LIGHT);
	BIND_ENUM_CONSTANT(HINTING_NORMAL);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "font_path", PROPERTY_HINT_FILE, "*.ttf,*.otf"), "set_font_path", "get_font_path");
}

DynamicFontData::DynamicFontData() {}

DynamicFontData::~DynamicFontData() {
	if (rid != RID()) {
		TS->free(rid);
	}
}

/////////////////////////

Vector<RID> DynamicFont::get_rids() const {
	Vector<RID> rids;
	if (data.is_valid()) {
		rids.push_back(data->get_rid());
	}
	for (int i = 0; i < fallbacks.size(); i++) {
		rids.push_back(fallbacks[i]->get_rid());
	}
	return rids;
}

void DynamicFont::_reload_cache() {
	ERR_FAIL_COND(base_size < 1);
	if (!data.is_valid()) {
		fallbacks.resize(0);
		return;
	}

	cache.clear();
	cache_wrap.clear();

	emit_changed();
	_change_notify();
}

void DynamicFont::set_font_data(const Ref<DynamicFontData> &p_data) {
	data = p_data;
	_reload_cache();

	emit_changed();
	_change_notify();
}

Ref<DynamicFontData> DynamicFont::get_font_data() const {
	return data;
}

int DynamicFont::get_size() const {
	return base_size;
}

void DynamicFont::set_size(int p_size) {
	if (base_size == p_size)
		return;
	base_size = p_size;
	_reload_cache();
}

void DynamicFont::set_outline_size(int p_size) {
	if (outline_size == p_size)
		return;
	ERR_FAIL_COND(p_size < 0 || p_size > UINT8_MAX);
	outline_size = p_size;
	_reload_cache();
}

void DynamicFont::set_outline_color(Color p_color) {
	if (p_color != outline_color) {
		outline_color = p_color;
		emit_changed();
		_change_notify();
	}
}

Color DynamicFont::get_outline_color() const {
	return outline_color;
}

bool DynamicFont::get_use_mipmaps() const {
	if (data.is_valid()) {
		return TS->font_legacy_dynamic_get_mipmaps(data->get_rid());
	} else {
		return false;
	}
}

void DynamicFont::set_use_mipmaps(bool p_enable) {
	if (data.is_valid()) {
		TS->font_legacy_dynamic_set_mipmaps(data->get_rid(), p_enable);
	}
	for (int i = 0; i < fallbacks.size(); i++) {
		TS->font_legacy_dynamic_set_mipmaps(fallbacks[i]->get_rid(), p_enable);
	}
}

bool DynamicFont::get_use_filter() const {
	if (data.is_valid()) {
		return TS->font_legacy_dynamic_get_filter(data->get_rid());
	} else {
		return false;
	}
}

void DynamicFont::set_use_filter(bool p_enable) {
	if (data.is_valid()) {
		TS->font_legacy_dynamic_set_filter(data->get_rid(), p_enable);
	}
	for (int i = 0; i < fallbacks.size(); i++) {
		TS->font_legacy_dynamic_set_filter(fallbacks[i]->get_rid(), p_enable);
	}
}

int DynamicFont::get_spacing(int p_type) const {
	if (p_type == SPACING_TOP) {
		return spacing_top;
	} else if (p_type == SPACING_BOTTOM) {
		return spacing_bottom;
	} else if (p_type == SPACING_CHAR) {
		return TS->font_get_spacing_glyph(data->get_rid());
	} else if (p_type == SPACING_SPACE) {
		return TS->font_get_spacing_space(data->get_rid());
	}

	return 0;
}

void DynamicFont::set_spacing(int p_type, int p_value) {
	if (p_type == SPACING_TOP) {
		spacing_top = p_value;
	} else if (p_type == SPACING_BOTTOM) {
		spacing_bottom = p_value;
	} else if (p_type == SPACING_CHAR) {
		TS->font_set_spacing_glyph(data->get_rid(), p_value);
		for (int i = 0; i < fallbacks.size(); i++) {
			TS->font_set_spacing_glyph(fallbacks[i]->get_rid(), p_value);
		}
	} else if (p_type == SPACING_SPACE) {
		TS->font_set_spacing_space(data->get_rid(), p_value);
		for (int i = 0; i < fallbacks.size(); i++) {
			TS->font_set_spacing_space(fallbacks[i]->get_rid(), p_value);
		}
	}

	emit_changed();
	_change_notify();
}

float DynamicFont::get_height() const {
	ERR_FAIL_COND_V(data.is_null(), 0.f);
	if (data->get_rid() == RID()) {
		return 0.f; // Do not raise errors in getters, to prevent editor from spamming errors on incomplete (without data_path set) fonts.
	}
	return TS->font_get_height(data->get_rid(), base_size);
}

float DynamicFont::get_ascent() const {
	ERR_FAIL_COND_V(data.is_null(), 0.f);
	if (data->get_rid() == RID()) {
		return 0.f;
	}
	return TS->font_get_ascent(data->get_rid(), base_size);
}

float DynamicFont::get_descent() const {
	ERR_FAIL_COND_V(data.is_null(), 0.f);
	if (data->get_rid() == RID()) {
		return 0.f;
	}
	return TS->font_get_descent(data->get_rid(), base_size);
}

Size2 DynamicFont::get_char_size(char32_t p_char, char32_t p_next) const {
	ERR_FAIL_COND_V(data.is_null(), Size2());
	ERR_FAIL_COND_V(data->get_rid() == RID(), Size2());
	float adv = 0.f;
	uint32_t index_a = TS->font_get_glyph_index(data->get_rid(), p_char, 0x0000);
	if (p_next != 0) {
		uint32_t index_b = TS->font_get_glyph_index(data->get_rid(), p_next, 0x0000);
		adv += TS->font_get_glyph_kerning(data->get_rid(), index_a, index_b, base_size).x;
	}
	return TS->font_get_glyph_advance(data->get_rid(), index_a, base_size) + Vector2(adv, 0);
}

String DynamicFont::get_available_chars() const {
	ERR_FAIL_COND_V(data.is_null(), "");
	return TS->font_get_supported_chars(data->get_rid());
}

bool DynamicFont::is_distance_field_hint() const {
	return false;
}

bool DynamicFont::has_outline() const {
	return outline_size > 0;
}

float DynamicFont::draw_char(RID p_canvas_item, const Point2 &p_pos, char32_t p_char, char32_t p_next, const Color &p_modulate, bool p_outline) const {
	ERR_FAIL_COND_V(data.is_null(), 0);
	ERR_FAIL_COND_V(data->get_rid() == RID(), 0);
	float adv = 0.f;
	uint32_t index_a = TS->font_get_glyph_index(data->get_rid(), p_char, 0x0000);
	if (p_next != 0) {
		uint32_t index_b = TS->font_get_glyph_index(data->get_rid(), p_next, 0x0000);
		adv += TS->font_get_glyph_kerning(data->get_rid(), index_a, index_b, base_size).x;
	}
	if (p_outline) {
		return TS->font_draw_glyph_outline(data->get_rid(), p_canvas_item, base_size, outline_size, p_pos, index_a, outline_color * p_modulate).x + adv;
	} else {
		return TS->font_draw_glyph(data->get_rid(), p_canvas_item, base_size, p_pos, index_a, p_modulate).x + adv;
	}
}

void DynamicFont::set_fallback(int p_idx, const Ref<DynamicFontData> &p_data) {
	ERR_FAIL_COND(p_data.is_null());
	ERR_FAIL_INDEX(p_idx, fallbacks.size());
	fallbacks.write[p_idx] = p_data;
	cache.clear();
	cache_wrap.clear();
}

void DynamicFont::add_fallback(const Ref<DynamicFontData> &p_data) {
	ERR_FAIL_COND(p_data.is_null());
	TS->font_set_spacing_glyph(p_data->get_rid(), TS->font_get_spacing_glyph(data->get_rid()));
	TS->font_set_spacing_space(p_data->get_rid(), TS->font_get_spacing_space(data->get_rid()));
	fallbacks.push_back(p_data);

	cache.clear();
	cache_wrap.clear();

	_change_notify();
	emit_changed();
	_change_notify();
}

int DynamicFont::get_fallback_count() const {
	return fallbacks.size();
}

Ref<DynamicFontData> DynamicFont::get_fallback(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, fallbacks.size(), Ref<DynamicFontData>());
	return fallbacks[p_idx];
}

void DynamicFont::remove_fallback(int p_idx) {
	ERR_FAIL_INDEX(p_idx, fallbacks.size());
	fallbacks.remove(p_idx);

	cache.clear();
	cache_wrap.clear();
	emit_changed();
	_change_notify();
}

bool DynamicFont::_set(const StringName &p_name, const Variant &p_value) {
	String str = p_name;
	if (str.begins_with("fallback/")) {
		int idx = str.get_slicec('/', 1).to_int();
		Ref<DynamicFontData> fd = p_value;

		if (fd.is_valid()) {
			if (idx == fallbacks.size()) {
				add_fallback(fd);
				return true;
			} else if (idx >= 0 && idx < fallbacks.size()) {
				set_fallback(idx, fd);
				return true;
			} else {
				return false;
			}
		} else if (idx >= 0 && idx < fallbacks.size()) {
			remove_fallback(idx);
			return true;
		}
	}

	return false;
}

bool DynamicFont::_get(const StringName &p_name, Variant &r_ret) const {
	String str = p_name;
	if (str.begins_with("fallback/")) {
		int idx = str.get_slicec('/', 1).to_int();

		if (idx == fallbacks.size()) {
			r_ret = Ref<DynamicFontData>();
			return true;
		} else if (idx >= 0 && idx < fallbacks.size()) {
			r_ret = get_fallback(idx);
			return true;
		}
	}

	return false;
}

void DynamicFont::_get_property_list(List<PropertyInfo> *p_list) const {
	for (int i = 0; i < fallbacks.size(); i++) {
		p_list->push_back(PropertyInfo(Variant::OBJECT, "fallback/" + itos(i), PROPERTY_HINT_RESOURCE_TYPE, "DynamicFontData"));
	}

	p_list->push_back(PropertyInfo(Variant::OBJECT, "fallback/" + itos(fallbacks.size()), PROPERTY_HINT_RESOURCE_TYPE, "DynamicFontData"));
}

void DynamicFont::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_font_data", "data"), &DynamicFont::set_font_data);
	ClassDB::bind_method(D_METHOD("get_font_data"), &DynamicFont::get_font_data);

	ClassDB::bind_method(D_METHOD("get_available_chars"), &DynamicFont::get_available_chars);

	ClassDB::bind_method(D_METHOD("set_size", "data"), &DynamicFont::set_size);

	ClassDB::bind_method(D_METHOD("set_outline_size", "size"), &DynamicFont::set_outline_size);

	ClassDB::bind_method(D_METHOD("set_outline_color", "color"), &DynamicFont::set_outline_color);
	ClassDB::bind_method(D_METHOD("get_outline_color"), &DynamicFont::get_outline_color);

	ClassDB::bind_method(D_METHOD("set_use_mipmaps", "enable"), &DynamicFont::set_use_mipmaps);
	ClassDB::bind_method(D_METHOD("get_use_mipmaps"), &DynamicFont::get_use_mipmaps);
	ClassDB::bind_method(D_METHOD("set_use_filter", "enable"), &DynamicFont::set_use_filter);
	ClassDB::bind_method(D_METHOD("get_use_filter"), &DynamicFont::get_use_filter);
	ClassDB::bind_method(D_METHOD("set_spacing", "type", "value"), &DynamicFont::set_spacing);
	ClassDB::bind_method(D_METHOD("get_spacing", "type"), &DynamicFont::get_spacing);

	ClassDB::bind_method(D_METHOD("add_fallback", "data"), &DynamicFont::add_fallback);
	ClassDB::bind_method(D_METHOD("set_fallback", "idx", "data"), &DynamicFont::set_fallback);
	ClassDB::bind_method(D_METHOD("get_fallback", "idx"), &DynamicFont::get_fallback);
	ClassDB::bind_method(D_METHOD("remove_fallback", "idx"), &DynamicFont::remove_fallback);
	ClassDB::bind_method(D_METHOD("get_fallback_count"), &DynamicFont::get_fallback_count);

	ADD_GROUP("Settings", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "size", PROPERTY_HINT_RANGE, "1,1024,1"), "set_size", "get_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "outline_size", PROPERTY_HINT_RANGE, "0,1024,1"), "set_outline_size", "get_outline_size");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "outline_color"), "set_outline_color", "get_outline_color");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_mipmaps"), "set_use_mipmaps", "get_use_mipmaps");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_filter"), "set_use_filter", "get_use_filter");
	ADD_GROUP("Extra Spacing", "extra_spacing");
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_top"), "set_spacing", "get_spacing", SPACING_TOP);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_bottom"), "set_spacing", "get_spacing", SPACING_BOTTOM);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_char"), "set_spacing", "get_spacing", SPACING_CHAR);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_space"), "set_spacing", "get_spacing", SPACING_SPACE);
	ADD_GROUP("Font", "");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "font_data", PROPERTY_HINT_RESOURCE_TYPE, "DynamicFontData"), "set_font_data", "get_font_data");

	BIND_ENUM_CONSTANT(SPACING_TOP);
	BIND_ENUM_CONSTANT(SPACING_BOTTOM);
	BIND_ENUM_CONSTANT(SPACING_CHAR);
	BIND_ENUM_CONSTANT(SPACING_SPACE);
}

DynamicFont::DynamicFont() {}

DynamicFont::~DynamicFont() {}

/////////////////////////

RES ResourceFormatLoaderDynamicFont::load(const String &p_path, const String &p_original_path, Error *r_error) {

	if (r_error)
		*r_error = ERR_FILE_CANT_OPEN;

	Ref<DynamicFontData> dfont;
	dfont.instance();
	dfont->set_font_path(p_path);

	if (r_error)
		*r_error = OK;

	return dfont;
}

void ResourceFormatLoaderDynamicFont::get_recognized_extensions(List<String> *p_extensions) const {

	p_extensions->push_back("ttf");
	p_extensions->push_back("otf");
}

bool ResourceFormatLoaderDynamicFont::handles_type(const String &p_type) const {

	return (p_type == "DynamicFontData");
}

String ResourceFormatLoaderDynamicFont::get_resource_type(const String &p_path) const {

	String el = p_path.get_extension().to_lower();
	if (el == "ttf" || el == "otf")
		return "DynamicFontData";
	return "";
}
