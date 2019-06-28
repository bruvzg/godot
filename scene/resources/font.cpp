/*************************************************************************/
/*  font.cpp                                                             */
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

#include "font.h"

#include "servers/shaping/shaping_interface.h"
#include "servers/shaping_server.h"

#include "core/method_bind_ext.gen.inc"

/*************************************************************************/
/*  Font Data (Shaper independant resource)                              */
/*************************************************************************/

void FontData::_clear() {
	fid = "";
	data_path = String();
	data_mem = NULL;
	data_mem_size = 0;
	size_cache.clear();
}

void FontData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_font_data_path"), &FontData::get_font_data_path);
	ClassDB::bind_method(D_METHOD("set_font_data_path", "path"), &FontData::set_font_data_path);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "font_data_path", PROPERTY_HINT_FILE, "*.fon,*.font,*.ttf,*.otf"), "set_font_data_path", "get_font_data_path");

	ClassDB::bind_method(D_METHOD("get_force_priority_multiplier"), &FontData::get_force_priority_multiplier);
	ClassDB::bind_method(D_METHOD("set_force_priority_multiplier", "pmult"), &FontData::set_force_priority_multiplier);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "force_priority_multiplier"), "set_force_priority_multiplier", "get_force_priority_multiplier");

	ClassDB::bind_method(D_METHOD("get_force_supported_scripts"), &FontData::get_force_supported_scripts);
	ClassDB::bind_method(D_METHOD("set_force_supported_scripts", "supported_scripts"), &FontData::set_force_supported_scripts);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "force_supported_scripts"), "set_force_supported_scripts", "get_force_supported_scripts");

	ClassDB::bind_method(D_METHOD("get_force_supported_languages"), &FontData::get_force_supported_languages);
	ClassDB::bind_method(D_METHOD("set_force_supported_languages", "supported_languages"), &FontData::set_force_supported_languages);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "force_supported_languages"), "set_force_supported_languages", "get_force_supported_languages");

	BIND_ENUM_CONSTANT(HINTING_NONE);
	BIND_ENUM_CONSTANT(HINTING_LIGHT);
	BIND_ENUM_CONSTANT(HINTING_NORMAL);
}

String FontData::get_font_data_path() const {
	return data_path;
}

void FontData::set_font_data_path(const String &p_path) {
	_clear();
	data_path = p_path;
	if ((data_path.get_extension() == "fon") || (data_path.get_extension() == "font")) {
		fid = "bitmap";
	} else if ((data_path.get_extension() == "ttf") || (data_path.get_extension() == "otf")) {
		fid = "dynamic";
	} else {
		fid = data_path.get_extension();
	}
	emit_changed();
}

float FontData::get_force_priority_multiplier() const {
	return pmult;
}

void FontData::set_force_priority_multiplier(float p_mult) {
	pmult = p_mult;
}

String FontData::get_force_supported_scripts() const {
	return supported_scripts;
}

void FontData::set_force_supported_scripts(const String &p_supported_scripts) {
	if (supported_scripts != p_supported_scripts) {
		supported_scripts = p_supported_scripts;
		size_cache.clear();
		emit_changed();
	}
}

String FontData::get_force_supported_languages() const {
	return supported_langs;
}

void FontData::set_force_supported_languages(const String &p_supported_languages) {
	if (supported_langs != p_supported_languages) {
		supported_langs = p_supported_languages;
		size_cache.clear();
		emit_changed();
	}
}

void FontData::_get_static_font_data(const uint8_t **o_font_mem, size_t *o_font_mem_size) const {
	if (o_font_mem) *o_font_mem = data_mem;
	if (o_font_mem_size) *o_font_mem_size = data_mem_size;
}

void FontData::_set_static_font_data(const StringName &p_typeid, const uint8_t *p_font_mem, size_t p_font_mem_size) {
	fid = p_typeid;
	data_mem = p_font_mem;
	data_mem_size = p_font_mem_size;
	emit_changed();
}

StringName FontData::_get_font_id() const {
	return fid;
}

Ref<FontImplementation> FontData::_get_font_implementation_at_size(CacheID p_id) const {
	if (ShapingServer::get_singleton() == NULL) return NULL;
	if (ShapingServer::get_singleton()->get_primary_interface().is_null()) return NULL; //iface is not ready or headless build, do not disply error

	if (size_cache.has(p_id.key)) {
		Ref<FontImplementation> impl = size_cache[p_id.key];
		if (!impl->require_reload())
			return impl;
	}

	Ref<FontImplementation> impl = ShapingServer::get_singleton()->get_primary_interface()->create_font_implementation(this, p_id);
	if (impl.is_valid() && !impl->require_reload()) {
		size_cache[p_id.key] = impl;
		return impl;
	}

	return NULL; //TODO error message
}

FontData::FontData() {
	_clear();
	supported_scripts = "";
	supported_langs = "";
	pmult = 1.0f;
}

FontData::~FontData() {
	_clear();
}

/*************************************************************************/
/*  Font Implementation (Shaper and size specific resource)              */
/*************************************************************************/

Mutex *FontImplementation::font_imp_mutex = NULL;
SelfList<FontImplementation>::List *FontImplementation::font_imps = NULL;

void FontImplementation::_bind_methods() {

	ClassDB::bind_method(D_METHOD("require_reload"), &FontImplementation::require_reload);

	ClassDB::bind_method(D_METHOD("get_font_scale"), &FontImplementation::get_font_scale);
	ClassDB::bind_method(D_METHOD("get_glyph", "unicode", "variation_selector"), &FontImplementation::get_glyph, DEFVAL(0));

	ClassDB::bind_method(D_METHOD("get_advance", "glyph"), &FontImplementation::get_advance);
	ClassDB::bind_method(D_METHOD("get_kerning", "glyph_a", "glyph_b"), &FontImplementation::get_kerning);
	ClassDB::bind_method(D_METHOD("get_v_advance", "glyph"), &FontImplementation::get_v_advance);
	ClassDB::bind_method(D_METHOD("get_v_kerning", "glyph_a", "glyph_b"), &FontImplementation::get_v_kerning);

	ClassDB::bind_method(D_METHOD("get_glyph_size", "glyph"), &FontImplementation::get_glyph_size);

	ClassDB::bind_method(D_METHOD("get_ascent"), &FontImplementation::get_ascent);
	ClassDB::bind_method(D_METHOD("get_descent"), &FontImplementation::get_descent);
	ClassDB::bind_method(D_METHOD("get_line_gap"), &FontImplementation::get_line_gap);

	ClassDB::bind_method(D_METHOD("get_underline_position"), &FontImplementation::get_underline_position);
	ClassDB::bind_method(D_METHOD("get_underline_thickness"), &FontImplementation::get_underline_thickness);

	ClassDB::bind_method(D_METHOD("get_name"), &FontImplementation::get_name);
	ClassDB::bind_method(D_METHOD("get_script_support_priority", "script"), &FontImplementation::get_script_support_priority);
	ClassDB::bind_method(D_METHOD("get_language_support_priority", "language"), &FontImplementation::get_language_support_priority);

	ClassDB::bind_method(D_METHOD("draw_glyph", "canvas_item", "pos", "glyph", "modulate", "rotate_cw"), &FontImplementation::draw_glyph, DEFVAL(Color(1, 1, 1)), DEFVAL(false));
}

void FontImplementation::invalidate_all() {
	if (font_imp_mutex) {
		font_imp_mutex->lock();
		SelfList<FontImplementation> *E = font_imps->first();
		while (E) {
			E->self()->invalidate();
			E = E->next();
		}

		font_imp_mutex->unlock();
	}
}

void FontImplementation::initialize_self_list() {
	font_imps = memnew(SelfList<FontImplementation>::List());
	font_imp_mutex = Mutex::create();
}

void FontImplementation::finish_self_list() {
	if (font_imp_mutex) {
		memdelete(font_imp_mutex);
		font_imp_mutex = NULL;
	}
	if (font_imps) {
		memdelete(font_imps);
		font_imps = NULL;
	}
}

FontImplementation::FontImplementation() :
		font_list(this) {

	if (font_imp_mutex) {
		font_imp_mutex->lock();
		font_imps->add(&font_list);
		font_imp_mutex->unlock();
	}
}

FontImplementation::~FontImplementation() {

	if (font_imp_mutex) {
		font_imp_mutex->lock();
		font_imps->remove(&font_list);
		font_imp_mutex->unlock();
	}
}

/*************************************************************************/
/*  Font                                                                 */
/*************************************************************************/
float Font::oversampling = 1.0f;

void Font::_reload_cache() {
	ERR_FAIL_COND(cache_id.size < 1);

	for (int i = 0; i < data.size(); i++) {
		data_at_size.write[i] = data[i]->_get_font_implementation_at_size(cache_id);
		if (outline_cache_id.outline_size > 0)
			outline_data_at_size.write[i] = data[i]->_get_font_implementation_at_size(outline_cache_id);
		else
			outline_data_at_size.write[i] = Ref<FontImplementation>();
	}

	emit_changed();
	_change_notify();
}

bool Font::_set(const StringName &p_name, const Variant &p_value) {
	String str = p_name;
	if (str.begins_with("data/")) {
		int idx = str.get_slicec('/', 1).to_int();
		Ref<FontData> fd = p_value;

		if (fd.is_valid()) {
			if (idx == data.size()) {
				add_font_data(fd);
				return true;
			} else if (idx >= 0 && idx < data.size()) {
				set_font_data(idx, fd);
				return true;
			} else {
				return false;
			}
		} else if (idx >= 0 && idx < data.size()) {
			remove_font_data(idx);
			return true;
		}
	}
	return false;
}

bool Font::_get(const StringName &p_name, Variant &r_ret) const {
	String str = p_name;
	if (str.begins_with("data/")) {
		int idx = str.get_slicec('/', 1).to_int();

		if (idx == data.size()) {
			r_ret = Ref<FontData>();
			return true;
		} else if (idx >= 0 && idx < data.size()) {
			r_ret = get_font_data(idx);
			return true;
		}
	}
	return false;
}

void Font::_get_property_list(List<PropertyInfo> *p_list) const {
	for (int i = 0; i < data.size(); i++) {
		p_list->push_back(PropertyInfo(Variant::OBJECT, "data/" + itos(i), PROPERTY_HINT_RESOURCE_TYPE, "FontData"));
	}
	p_list->push_back(PropertyInfo(Variant::OBJECT, "data/" + itos(data.size()), PROPERTY_HINT_RESOURCE_TYPE, "FontData"));
}

void Font::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_ascent"), &Font::get_ascent);
	ClassDB::bind_method(D_METHOD("get_descent"), &Font::get_descent);
	ClassDB::bind_method(D_METHOD("get_height"), &Font::get_height);
	ClassDB::bind_method(D_METHOD("get_line_gap"), &Font::get_line_gap);

	ClassDB::bind_method(D_METHOD("get_mipmap"), &Font::get_mipmap);
	ClassDB::bind_method(D_METHOD("set_mipmap", "mipmap"), &Font::set_mipmap);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "mipmap"), "set_mipmap", "get_mipmap");

	ClassDB::bind_method(D_METHOD("get_filter"), &Font::get_filter);
	ClassDB::bind_method(D_METHOD("set_filter", "filter"), &Font::set_filter);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "filter"), "set_filter", "get_filter");

	ClassDB::bind_method(D_METHOD("get_antialiased"), &Font::get_antialiased);
	ClassDB::bind_method(D_METHOD("set_antialiased", "antialiased"), &Font::set_antialiased);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "antialiased"), "set_antialiased", "get_antialiased");

	ClassDB::bind_method(D_METHOD("get_hinting"), &Font::get_hinting);
	ClassDB::bind_method(D_METHOD("set_hinting", "hinting"), &Font::set_hinting);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "hinting", PROPERTY_HINT_ENUM, "None,Light,Normal"), "set_hinting", "get_hinting");

	ClassDB::bind_method(D_METHOD("get_force_autohinter"), &Font::get_force_autohinter);
	ClassDB::bind_method(D_METHOD("set_force_autohinter", "force"), &Font::set_force_autohinter);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "force_autohinter"), "set_force_autohinter", "get_force_autohinter");

	ClassDB::bind_method(D_METHOD("get_distance_field_hint"), &Font::get_distance_field_hint);
	ClassDB::bind_method(D_METHOD("set_distance_field_hint", "distance_field"), &Font::set_distance_field_hint);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "distance_field_hint"), "set_distance_field_hint", "get_distance_field_hint");

	ClassDB::bind_method(D_METHOD("get_extra_spacing", "type"), &Font::get_extra_spacing);
	ClassDB::bind_method(D_METHOD("set_extra_spacing", "type", "value"), &Font::set_extra_spacing);
	ADD_GROUP("Extra Spacing", "extra_spacing");
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_top"), "set_extra_spacing", "get_extra_spacing", SPACING_TOP);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_bottom"), "set_extra_spacing", "get_extra_spacing", SPACING_BOTTOM);

	ClassDB::bind_method(D_METHOD("get_size"), &Font::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &Font::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "size", PROPERTY_HINT_RANGE, "1,255,1"), "set_size", "get_size");

	ClassDB::bind_method(D_METHOD("has_outline"), &Font::has_outline);

	ClassDB::bind_method(D_METHOD("get_outline_size"), &Font::get_outline_size);
	ClassDB::bind_method(D_METHOD("set_outline_size", "size"), &Font::set_outline_size);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "outline_size", PROPERTY_HINT_RANGE, "0,255,1"), "set_outline_size", "get_outline_size");

	ClassDB::bind_method(D_METHOD("get_font_implementations", "script", "lang"), &Font::_get_font_implementations, DEFVAL(0), DEFVAL(""));

	//ClassDB::bind_method(D_METHOD("shape_string", "text", "start", "end", "base_dir", "lang", "ftr", "no_cahce"), &Font::shape_string, DEFVAL(TEXT_DIRECTION_AUTO), DEFVAL(""), DEFVAL(""), DEFVAL(false));

	ClassDB::bind_method(D_METHOD("draw", "canvas_item", "pos", "text", "width", "modulate", "outline_modulate", "base_dir", "lang,", "ftr"), &Font::draw, DEFVAL(-1), DEFVAL(Color(1, 1, 1)), DEFVAL(Color(1, 1, 1, 0)), DEFVAL(TEXT_DIRECTION_AUTO), DEFVAL(""), DEFVAL(""));
	ClassDB::bind_method(D_METHOD("draw_halign", "canvas_item", "pos", "text", "width", "align", "modulate", "outline_modulate", "base_dir", "lang", "ftr"), &Font::draw_halign, DEFVAL(Color(1, 1, 1)), DEFVAL(Color(1, 1, 1, 0)), DEFVAL(TEXT_DIRECTION_AUTO), DEFVAL(""), DEFVAL(""));

	ClassDB::bind_method(D_METHOD("draw_wordwrap", "canvas_item", "pos", "text", "width", "modulate", "outline_modulate", "base_dir", "lang", "ftr"), &Font::draw_wordwrap, DEFVAL(-1), DEFVAL(Color(1, 1, 1)), DEFVAL(Color(1, 1, 1, 0)), DEFVAL(TEXT_DIRECTION_AUTO), DEFVAL(""), DEFVAL(""));
	ClassDB::bind_method(D_METHOD("draw_wordwrap_halign", "canvas_item", "pos", "text", "width", "align", "modulate", "outline_modulate", "base_dir", "lang", "ftr"), &Font::draw_wordwrap_halign, DEFVAL(Color(1, 1, 1)), DEFVAL(Color(1, 1, 1, 0)), DEFVAL(TEXT_DIRECTION_AUTO), DEFVAL(""), DEFVAL(""));

	ClassDB::bind_method(D_METHOD("get_string_size", "text", "base_dir", "lang", "ftr"), &Font::get_string_size, DEFVAL(TEXT_DIRECTION_AUTO), DEFVAL(""), DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_wordwrap_string_size", "text", "width", "base_dir", "lang", "ftr"), &Font::get_wordwrap_string_size, DEFVAL(-1), DEFVAL(TEXT_DIRECTION_AUTO), DEFVAL(""), DEFVAL(""));

	ClassDB::bind_method(D_METHOD("draw_char", "canvas_item", "pos", "char", "next", "modulate", "outline"), &Font::draw_char, DEFVAL(0), DEFVAL(Color(1, 1, 1)), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_char_size", "char", "next"), &Font::get_char_size, 0);

	ClassDB::bind_method(D_METHOD("get_font_data_count"), &Font::get_font_data_count);
	ClassDB::bind_method(D_METHOD("add_font_data", "data"), &Font::add_font_data);
	ClassDB::bind_method(D_METHOD("set_font_data", "idx", "data"), &Font::set_font_data);
	ClassDB::bind_method(D_METHOD("get_font_data", "idx"), &Font::get_font_data);
	ClassDB::bind_method(D_METHOD("remove_font_data", "idx"), &Font::remove_font_data);

	ClassDB::bind_method(D_METHOD("update_changes"), &Font::update_changes);

	BIND_ENUM_CONSTANT(SPACING_TOP);
	BIND_ENUM_CONSTANT(SPACING_BOTTOM);
}

float Font::get_ascent() const {
	for (int i = 0; i < data_at_size.size(); i++) {
		if (!data_at_size[i].is_valid() || data_at_size[i]->require_reload())
			const_cast<Font *>(this)->data_at_size.write[i] = data[i]->_get_font_implementation_at_size(cache_id);
		if (data_at_size[i].is_valid()) {
			return data_at_size[i]->get_ascent();
		}
	}
	return 0.f;
}

float Font::get_descent() const {
	for (int i = 0; i < data_at_size.size(); i++) {
		if (!data_at_size[i].is_valid() || data_at_size[i]->require_reload())
			const_cast<Font *>(this)->data_at_size.write[i] = data[i]->_get_font_implementation_at_size(cache_id);
		if (data_at_size[i].is_valid()) {
			return data_at_size[i]->get_descent();
		}
	}
	return 0.f;
}

float Font::get_height() const {
	for (int i = 0; i < data_at_size.size(); i++) {
		if (!data_at_size[i].is_valid() || data_at_size[i]->require_reload())
			const_cast<Font *>(this)->data_at_size.write[i] = data[i]->_get_font_implementation_at_size(cache_id);
		if (data_at_size[i].is_valid()) {
			return data_at_size[i]->get_ascent() + data_at_size[i]->get_descent();
		}
	}
	return 0.f;
}

float Font::get_line_gap() const {
	float line_gap = 0.f;
	for (int i = 0; i < data_at_size.size(); i++) {
		if (!data_at_size[i].is_valid() || data_at_size[i]->require_reload())
			const_cast<Font *>(this)->data_at_size.write[i] = data[i]->_get_font_implementation_at_size(cache_id);
		if (data_at_size[i].is_valid()) {
			line_gap = MAX(line_gap, data_at_size[i]->get_line_gap());
		}
	}
	return line_gap;
}

bool Font::get_mipmap() const {
	return cache_id.mipmaps;
}

void Font::set_mipmap(bool p_mipmap) {
	if (cache_id.mipmaps == p_mipmap)
		return;
	cache_id.mipmaps = p_mipmap;
	outline_cache_id.mipmaps = p_mipmap;
	_reload_cache();
}

bool Font::get_filter() const {
	return cache_id.filter;
}

void Font::set_filter(bool p_filter) {
	if (cache_id.filter == p_filter)
		return;
	cache_id.filter = p_filter;
	outline_cache_id.filter = p_filter;
	_reload_cache();
}

bool Font::get_antialiased() const {
	return cache_id.antialiased;
}

void Font::set_antialiased(bool p_antialiased) {
	if (cache_id.antialiased == p_antialiased)
		return;
	cache_id.antialiased = p_antialiased;
	outline_cache_id.antialiased = p_antialiased;
	_reload_cache();
}

FontData::Hinting Font::get_hinting() const {
	return (FontData::Hinting)cache_id.hinting;
}

void Font::set_hinting(FontData::Hinting p_hinting) {
	if (cache_id.hinting == p_hinting)
		return;
	cache_id.hinting = p_hinting;
	outline_cache_id.hinting = p_hinting;
	_reload_cache();
}

bool Font::get_force_autohinter() const {
	return cache_id.force_autohinter;
}

void Font::set_force_autohinter(bool p_force) {
	if (cache_id.force_autohinter == p_force)
		return;
	cache_id.force_autohinter = p_force;
	outline_cache_id.force_autohinter = p_force;
	_reload_cache();
}

bool Font::get_distance_field_hint() const {
	return distance_field_hint;
}

void Font::set_distance_field_hint(bool p_distance_field) {
	distance_field_hint = p_distance_field;
}

int Font::get_extra_spacing(SpacingType p_type) const {
	if (p_type == SPACING_TOP) {
		return spacing_top;
	} else if (p_type == SPACING_BOTTOM) {
		return spacing_bottom;
	}
	return 0;
}

void Font::set_extra_spacing(SpacingType p_type, int p_value) {
	if (p_type == SPACING_TOP) {
		spacing_top = p_value;
	} else if (p_type == SPACING_BOTTOM) {
		spacing_bottom = p_value;
	}
	emit_changed();
	_change_notify();
}

int Font::get_size() const {
	return cache_id.size;
}

void Font::set_size(int p_size) {
	if (cache_id.size == p_size)
		return;
	ERR_FAIL_COND(p_size < 1 || p_size > UINT8_MAX);
	cache_id.size = p_size;
	outline_cache_id.size = p_size;
	_reload_cache();
}

bool Font::has_outline() const {
	return outline_cache_id.outline_size > 0;
}

int Font::get_outline_size() const {
	return outline_cache_id.outline_size;
}

void Font::set_outline_size(int p_size) {
	if (outline_cache_id.outline_size == p_size)
		return;
	ERR_FAIL_COND(p_size < 0 || p_size > UINT8_MAX);
	outline_cache_id.outline_size = p_size;
	_reload_cache();
}

struct FImpSupport {
	int32_t script_p;
	int32_t lang_p;
	int32_t order_p;
	Ref<FontImplementation> f;
	Ref<FontImplementation> fo;

	FImpSupport() {
		script_p = 0;
		lang_p = 0;
		order_p = 0;
	}

	FImpSupport(const Ref<FontImplementation> &p_ref, const Ref<FontImplementation> &p_refo, const Ref<FontData> &p_data, int32_t p_order, uint32_t p_script, const String &p_lang) {
		if (p_ref.is_valid()) {
			script_p = p_ref->get_script_support_priority(p_script) * p_data->get_force_priority_multiplier();
			lang_p = p_ref->get_language_support_priority(p_lang) * p_data->get_force_priority_multiplier();
		}
		order_p = p_order;
		f = p_ref;
		fo = p_refo;
	}
};

struct FImpSupportCompare {

	_FORCE_INLINE_ bool operator()(const FImpSupport &p_a, const FImpSupport &p_b) const {

		if ((p_a.lang_p < p_b.lang_p)) {
			return true;
		} else if ((p_a.script_p < p_b.script_p)) {
			return true;
		} else if ((p_a.order_p < p_b.order_p)) {
			return true;
		} else {
			return false;
		}
	}
};

//low level shaping
void Font::get_font_implementations(Vector<Ref<FontImplementation>> &o_fonts, Vector<Ref<FontImplementation>> &o_outlines, uint32_t p_script, const String &p_lang) const {

	//Vector<Pair<int32_t, int32_t>> data_list;
	Vector<FImpSupport> data_list;
	for (int32_t i = 0; i < data_at_size.size(); i++) {
		if (!data_at_size[i].is_valid() || data_at_size[i]->require_reload())
			const_cast<Font *>(this)->data_at_size.write[i] = data[i]->_get_font_implementation_at_size(cache_id);

		data_list.push_back(FImpSupport(data_at_size[i], outline_data_at_size[i], data[i], i, p_script, p_lang));
		//if (p_script != 0)
		//	data_list.push_back(Pair<int32_t, int32_t>(data_at_size[i].is_valid() ? data_at_size[i]->get_script_support_priority(p_script) * data[i]->get_force_priority_multiplier() : 0, i));
		//else
		//	data_list.push_back(Pair<int32_t, int32_t>(data[i]->get_force_priority_multiplier() * (data_at_size.size() - i), i));
	}

	//data_list.sort_custom<PairSort<int32_t, int32_t>>();
	data_list.sort_custom<FImpSupportCompare>();

	for (int i = data_list.size() - 1; i >= 0; i--) {
		o_fonts.push_back(data_list[i].f);
		o_outlines.push_back(data_list[i].fo);
		//o_fonts.push_back(data_at_size[data_list[i].second]);
		//o_outlines.push_back(outline_data_at_size[data_list[i].second]);
	}
}

Array Font::_get_font_implementations(uint32_t p_script, const String &p_lang) const {
	Array ret;

	//Vector<Pair<int32_t, int32_t>> data_list;
	Vector<FImpSupport> data_list;
	for (int32_t i = 0; i < data_at_size.size(); i++) {
		if (!data_at_size[i].is_valid() || data_at_size[i]->require_reload())
			const_cast<Font *>(this)->data_at_size.write[i] = data[i]->_get_font_implementation_at_size(cache_id);

		data_list.push_back(FImpSupport(data_at_size[i], outline_data_at_size[i], data[i], i, p_script, p_lang));
		//if (p_script != 0)
		//	data_list.push_back(Pair<int32_t, int32_t>(data_at_size[i].is_valid() ? data_at_size[i]->get_script_support_priority(p_script) * data[i]->get_force_priority_multiplier() : 0, i));
		//else
		//	data_list.push_back(Pair<int32_t, int32_t>(data[i]->get_force_priority_multiplier() * (data_at_size.size() - i), i));
	}

	//data_list.sort_custom<PairSort<int32_t, int32_t>>();
	data_list.sort_custom<FImpSupportCompare>();

	for (int i = data_list.size() - 1; i >= 0; i--) {
		Dictionary fontr;
		fontr["font"] = data_list[i].f;
		fontr["font_outline"] = data_list[i].fo;
		//fontr["font"] = data_at_size[data_list[i].second];
		//fontr["font_outline"] = outline_data_at_size[data_list[i].second];
		ret.push_back(fontr);
	}

	return ret;
}

// Ref<ShapedString> Font::shape_attributed_string(const String &p_text, int p_start, int p_end, const Ref<AttributeMap> &p_attributes, TextDirection p_base_dir, bool p_no_cache) const {
// 	ERR_FAIL_COND_V(ShapingServer::get_singleton() == NULL, ShapedString::invalid());
// 	ERR_FAIL_COND_V(ShapingServer::get_singleton()->get_primary_interface().is_null(), ShapedString::invalid());
// 	ERR_FAIL_COND_V(p_start > p_end, ShapedString::invalid());
// 	ERR_FAIL_COND_V(p_end > p_text.length(), ShapedString::invalid());

// 	//TEMP - use cache, map hash
// 	Ref<ShapedString> new_str;
// 	new_str.instance();
// 	Vector<Run> &vs = new_str->get_runs();
// 	Vector<Run> v;
// 	TextDirection direction = ShapingServer::get_singleton()->get_primary_interface()->analyse_text(p_text, p_start, p_end, p_base_dir, "", v); //todo analyse with lang attributes?
// 	ERR_FAIL_COND_V(direction == TEXT_DIRECTION_INVALID, ShapedString::invalid());
// 	bool valid = true;
// 	for (int32_t i = 0; i < v.size(); i++) {
// 		const Map<int32_t, HashMap<TextAttributes, Variant>>::Element *E = (v[i].level % 2 == 0) ? p_attributes->find_run(v[i].start) : p_attributes->find_run(v[i].end - 1);
// 		Run r;
// 		if (!E) {
// 			vs.push_back(v[i]);
// 			valid &= ShapingServer::get_singleton()->get_primary_interface()->shape_run(p_text, this, vs.write[vs.size() - 1], "", "");
// 		} else {
// 			//Iter Attrib runs and call next bidi_script_attrib run
// 			int32_t sh_start = (E) ? MAX(v[i].start, E->key()) : v[i].start;
// 			int32_t sh_end = (E->next()) ? MIN(v[i].end, E->next()->key()) : v[i].end;
// 			while (true) {
// 				vs.push_back(Run(sh_start, sh_end, v[i].level, v[i].script, v[i].break_type));
// 				if (E->get().has(TEXT_ATTRIB_REPLACEMENT)) {
// 					Size2 s = (Size2)E->get()[TEXT_ATTRIB_REPLACEMENT];
// 					vs.write[vs.size() - 1].width = s.width;
// 					vs.write[vs.size() - 1].ascent = s.height;
// 					vs.write[vs.size() - 1].descent = 0.f;
// 				} else {
// 					const Font *f = (E->get().has(TEXT_ATTRIB_FONT)) ? Object::cast_to<const Font>(E->get()[TEXT_ATTRIB_FONT]) : this;
// 					String lang = (E->get().has(TEXT_ATTRIB_LANGUAGE)) ? (String)E->get()[TEXT_ATTRIB_LANGUAGE] : "";
// 					String ftr = (E->get().has(TEXT_ATTRIB_OT_FEATURES)) ? (String)E->get()[TEXT_ATTRIB_OT_FEATURES] : "";
// 					valid &= ShapingServer::get_singleton()->get_primary_interface()->shape_run(p_text, f, vs.write[vs.size() - 1], lang, ftr);
// 				}
// 				if (v[i].level % 2 == 0) {
// 					if (E->next() && (E->next()->key() <= sh_end)) E = E->next();
// 					if (sh_end == v[i].end) break;
// 					sh_start = sh_end;
// 					sh_end = (E->next()) ? MIN(E->next()->key(), v[i].end) : v[i].end;
// 				} else {
// 					if (E->prev() && (E->key() >= sh_start)) E = E->prev();
// 					if (sh_start == v[i].start) break;
// 					sh_end = sh_start;
// 					sh_start = (E->prev()) ? MAX(E->key(), v[i].start) : v[i].start;
// 				}
// 			}
// 		}
// 	}
// 	ERR_FAIL_COND_V(!valid, ShapedString::invalid());
// 	new_str->update_metrics(spacing_top, spacing_bottom, direction);
// 	// if (!p_no_cache) {
// 	// 	cache_rec.shaped = new_str;
// 	// 	shaped_cache[hash] = cache_rec;
// 	// 	shaped_cache_last.push_back(hash);
// 	// 	while (shaped_cache_last.size() > shaped_cache_max_depth) {
// 	// 		uint32_t del_hash = shaped_cache_last.front()->get();
// 	// 		shaped_cache.erase(del_hash);
// 	// 		shaped_cache_last.pop_front();
// 	// 	}
// 	// }
// 	return new_str;
// }

// Ref<ShapedString> Font::shape_string(const String &p_text, int p_start, int p_end, TextDirection p_base_dir, const String &p_lang, const String &p_ftr, bool p_no_cache) const {
// 	ERR_FAIL_COND_V(ShapingServer::get_singleton() == NULL, ShapedString::invalid());
// 	ERR_FAIL_COND_V(ShapingServer::get_singleton()->get_primary_interface().is_null(), ShapedString::invalid());
// 	ERR_FAIL_COND_V(p_start > p_end, ShapedString::invalid());
// 	ERR_FAIL_COND_V(p_end > p_text.length(), ShapedString::invalid());

// 	if (p_start == p_end) {
// 		Ref<ShapedString> new_str;
// 		new_str.instance();
// 		new_str->update_metrics(get_ascent(), get_descent(), TEXT_DIRECTION_AUTO);
// 		return new_str;
// 	}

// 	uint32_t hash = 0;
// 	CacheRec *rec = NULL;

// 	if (!p_no_cache) {
// 		hash = p_text.hash();

// 		hash = hash_djb2_one_32(p_base_dir, hash);
// 		uint32_t c;
// 		const char *cstr = p_lang.ascii().get_data();
// 		while ((c = *cstr++))
// 			hash = ((hash << 5) + hash) + c;
// 		cstr = p_ftr.ascii().get_data();
// 		while ((c = *cstr++))
// 			hash = ((hash << 5) + hash) + c;

// 		rec = shaped_cache.getptr(hash);
// 	}
// 	if (rec && (!rec->shaped->require_reload())) {
// 		if ((p_start == 0) && (p_end == p_text.length())) {
// 			return rec->shaped;
// 		} else {
// 			Ref<ShapedString> new_str;
// 			new_str.instance();
// 			Vector<Run> &v = new_str->get_runs();
// 			const Vector<Run> &rv = rec->shaped->get_runs();
// 			TextDirection direction = ShapingServer::get_singleton()->get_primary_interface()->analyse_text(p_text, p_start, p_end, p_base_dir, p_lang, v);
// 			ERR_FAIL_COND_V(direction == TEXT_DIRECTION_INVALID, ShapedString::invalid());
// 			bool valid = true;
// 			for (int32_t i = 0; i < v.size(); i++) {
// 				if (likely(rec->runs.has(v[i].start) && (rv[rec->runs[v[i].start]].end == v[i].end))) {
// 					v.write[i] = rv[rec->runs[v[i].start]];
// 				} else {
// 					valid &= ShapingServer::get_singleton()->get_primary_interface()->shape_run(p_text, this, v.write[i], p_lang, p_ftr);
// 				}
// 			}
// 			ERR_FAIL_COND_V(!valid, ShapedString::invalid());
// 			new_str->update_metrics(spacing_top, spacing_bottom, direction);
// 			return new_str;
// 		}
// 	} else {
// 		CacheRec cache_rec;
// 		Ref<ShapedString> new_str;
// 		new_str.instance();
// 		Vector<Run> &v = new_str->get_runs();
// 		TextDirection direction = ShapingServer::get_singleton()->get_primary_interface()->analyse_text(p_text, p_start, p_end, p_base_dir, p_lang, v);
// 		ERR_FAIL_COND_V(direction == TEXT_DIRECTION_INVALID, ShapedString::invalid());
// 		bool valid = true;
// 		for (int32_t i = 0; i < v.size(); i++) {
// 			valid &= ShapingServer::get_singleton()->get_primary_interface()->shape_run(p_text, this, v.write[i], p_lang, p_ftr);
// 			if (!p_no_cache) cache_rec.runs[v[i].start] = i;
// 		}
// 		ERR_FAIL_COND_V(!valid, ShapedString::invalid());
// 		new_str->update_metrics(spacing_top, spacing_bottom, direction);
// 		if (!p_no_cache) {
// 			cache_rec.shaped = new_str;
// 			shaped_cache[hash] = cache_rec;
// 			shaped_cache_last.push_back(hash);
// 			while (shaped_cache_last.size() > shaped_cache_max_depth) {
// 				uint32_t del_hash = shaped_cache_last.front()->get();
// 				shaped_cache.erase(del_hash);
// 				shaped_cache_last.pop_front();
// 			}
// 		}
// 		return new_str;
// 	}
// }

Ref<TextLayout> Font::_shape_layout(const String &p_text, TextDirection p_base_dir, const String &p_lang, const String &p_ftr) const {
	// uint32_t hash = p_text.hash();
	// hash = hash_djb2_one_32(p_base_dir, hash);
	// uint32_t c;
	// const char *cstr = p_lang.ascii().get_data();
	// while ((c = *cstr++))
	// 	hash = ((hash << 5) + hash) + c;
	// cstr = p_ftr.ascii().get_data();
	// while ((c = *cstr++))
	// 	hash = ((hash << 5) + hash) + c;

	// CacheRec *rec = shaped_cache.getptr(hash);

	// printf("_sh_ly, %s (%x) %s\n", (rec) ? "HT" : "MS", hash, p_text.ascii().get_data());

	// if (rec) {
	// 	return rec->layout->copy();
	// } else {
	Ref<TextLayout> layout; // = memnew(TextLayout);
	layout.instance();
	layout->set_text(p_text);
	layout->set_base_direction(p_base_dir);
	layout->add_attribute(TEXT_ATTRIB_FONT, Ref<Font>(const_cast<Font *>(this)), 0, p_text.length());
	layout->add_attribute(TEXT_ATTRIB_LANGUAGE, p_lang, 0, p_text.length());
	layout->add_attribute(TEXT_ATTRIB_OT_FEATURES, p_ftr, 0, p_text.length());
	layout->set_break_mode(TEXT_BREAK_ON_NONE);
	layout->set_display_metrics(false);

	return layout;
	//layout->shape_now();

	// 	CacheRec cache_rec;
	// 	cache_rec.layout = layout;
	// 	shaped_cache[hash] = cache_rec;
	// 	shaped_cache_last.push_back(hash);
	// 	while (shaped_cache_last.size() > shaped_cache_max_depth) {
	// 		uint32_t del_hash = shaped_cache_last.front()->get();
	// 		memdelete(shaped_cache[del_hash].layout);
	// 		shaped_cache.erase(del_hash);
	// 		shaped_cache_last.pop_front();
	// 	}
	// 	return layout->copy();
	// }
}

//high level wrappers
void Font::draw(RID p_canvas_item, const Point2 &p_pos, const String &p_text, int p_width, const Color &p_modulate, const Color &p_outline_modulate, TextDirection p_base_dir, const String &p_lang, const String &p_ftr) const {
	Ref<TextLayout> layout = _shape_layout(p_text, p_base_dir, p_lang, p_ftr);

	layout->set_break_mode(TEXT_BREAK_ON_NONE);
	layout->set_target_size(Size2(p_width, -1.f));
	layout->add_attribute(TEXT_ATTRIB_COLOR, p_modulate, 0, p_text.length());
	layout->add_attribute(TEXT_ATTRIB_OUTLINE_COLOR, p_outline_modulate, 0, p_text.length());
	layout->shape_now();

	layout->draw(p_canvas_item, p_pos - Point2(0, get_ascent()), true);
	//Ref<ShapedString> shaped = shape_string(p_text, 0, p_text.length(), p_base_dir, p_lang, p_ftr);
	//ERR_FAIL_COND(shaped.is_null());
	//int r_clip = (p_width != -1) ? p_width + p_pos.x : -1;
	//shaped->draw(p_canvas_item, p_pos, p_pos.x, r_clip, p_modulate, p_outline_modulate);
}

void Font::draw_wordwrap(RID p_canvas_item, const Point2 &p_pos, const String &p_text, int p_width, const Color &p_modulate, const Color &p_outline_modulate, TextDirection p_base_dir, const String &p_lang, const String &p_ftr) const {
	Ref<TextLayout> layout = _shape_layout(p_text, p_base_dir, p_lang, p_ftr);

	layout->set_break_mode(TEXT_BREAK_ON_HARD_AND_SOFT);
	layout->set_target_size(Size2(p_width, -1.f));
	layout->add_attribute(TEXT_ATTRIB_COLOR, p_modulate, 0, p_text.length());
	layout->add_attribute(TEXT_ATTRIB_OUTLINE_COLOR, p_outline_modulate, 0, p_text.length());
	layout->shape_now();

	layout->draw(p_canvas_item, p_pos - Point2(0, get_ascent()), true);
	// Ref<ShapedString> shaped = shape_string(p_text, 0, p_text.length(), p_base_dir, p_lang, p_ftr);
	// ERR_FAIL_COND(shaped.is_null());

	// Vector<Pair<int32_t, int32_t>> lines = shaped->break_lines(p_width);

	// Point2 origin;
	// for (int i = 0; i < lines.size(); i++) {
	// 	Ref<ShapedString> line = shape_string(p_text, lines[i].first, lines[i].second, p_base_dir, p_lang, p_ftr);
	// 	ERR_FAIL_COND(line.is_null());

	// 	origin.x = 0.f;
	// 	if (i > 0) origin.y += line->get_ascent();

	// 	int r_clip = (p_width != -1) ? p_width + p_pos.x : -1;
	// 	line->draw(p_canvas_item, origin + p_pos, p_pos.x, r_clip, p_modulate, p_outline_modulate);

	// 	origin.y += line->get_descent();
	// }
}

void Font::draw_halign(RID p_canvas_item, const Point2 &p_pos, const String &p_text, int p_width, HAlign p_align, const Color &p_modulate, const Color &p_outline_modulate, TextDirection p_base_dir, const String &p_lang, const String &p_ftr) const {
	Ref<TextLayout> layout = _shape_layout(p_text, p_base_dir, p_lang, p_ftr);

	layout->set_break_mode(TEXT_BREAK_ON_NONE);
	layout->set_h_align(p_align);
	layout->set_target_size(Size2(p_width, -1.f));
	layout->add_attribute(TEXT_ATTRIB_COLOR, p_modulate, 0, p_text.length());
	layout->add_attribute(TEXT_ATTRIB_OUTLINE_COLOR, p_outline_modulate, 0, p_text.length());
	layout->shape_now();

	layout->draw(p_canvas_item, p_pos - Point2(0, get_ascent()), true);
	// Ref<ShapedString> shaped = shape_string(p_text, 0, p_text.length(), p_base_dir, p_lang, p_ftr);
	// ERR_FAIL_COND(shaped.is_null());

	// float length = shaped->get_size().width;
	// float d_width = (p_width - length);

	// float ofs = 0.f;
	// switch (p_align) {
	// 	case HALIGN_LEFT: {
	// 		ofs = 0.f;
	// 	} break;
	// 	case HALIGN_CENTER: {
	// 		ofs = Math::floor(d_width / 2.0);
	// 	} break;
	// 	case HALIGN_RIGHT: {
	// 		ofs = d_width;
	// 	} break;
	// 	case HALIGN_FILL_LEFT:
	// 	case HALIGN_FILL_RIGHT: {
	// 		ofs = 0.f;

	// 		shaped = shaped->fit_to_width(d_width);
	// 	} break;
	// 	default: {
	// 		ERR_PRINT("Unknown halignment type");
	// 	} break;
	// }

	// int r_clip = (p_width == -1) ? -1 : p_width + ofs + p_pos.x;
	// shaped->draw(p_canvas_item, Point2(ofs, 0) + p_pos, ofs + p_pos.x, r_clip, p_modulate, p_outline_modulate);
}

void Font::draw_wordwrap_halign(RID p_canvas_item, const Point2 &p_pos, const String &p_text, int p_width, HAlign p_align, const Color &p_modulate, const Color &p_outline_modulate, TextDirection p_base_dir, const String &p_lang, const String &p_ftr) const {
	Ref<TextLayout> layout = _shape_layout(p_text, p_base_dir, p_lang, p_ftr);

	layout->set_break_mode(TEXT_BREAK_ON_HARD_AND_SOFT);
	layout->set_h_align(p_align);
	layout->set_target_size(Size2(p_width, -1.f));
	layout->add_attribute(TEXT_ATTRIB_COLOR, p_modulate, 0, p_text.length());
	layout->add_attribute(TEXT_ATTRIB_OUTLINE_COLOR, p_outline_modulate, 0, p_text.length());
	layout->shape_now();

	layout->draw(p_canvas_item, p_pos - Point2(0, get_ascent()), true);
	// Ref<ShapedString> shaped = shape_string(p_text, 0, p_text.length(), p_base_dir, p_lang, p_ftr);
	// ERR_FAIL_COND(shaped.is_null());

	// Vector<Pair<int32_t, int32_t>> lines = shaped->break_lines(p_width);

	// Point2 origin;
	// for (int i = 0; i < lines.size(); i++) {
	// 	Ref<ShapedString> line = shape_string(p_text, lines[i].first, lines[i].second, p_base_dir, p_lang, p_ftr);
	// 	ERR_FAIL_COND(line.is_null());

	// 	float length = line->get_size().width;
	// 	origin.x = 0.f;
	// 	if (i > 0) origin.y += line->get_ascent();

	// 	float d_width = (p_width - length);

	// 	switch (p_align) {
	// 		case HALIGN_LEFT: {
	// 			origin.x = 0.f;
	// 		} break;
	// 		case HALIGN_CENTER: {
	// 			origin.x = Math::floor(d_width / 2.0);
	// 		} break;
	// 		case HALIGN_RIGHT: {
	// 			origin.x = d_width;
	// 		} break;
	// 		case HALIGN_FILL_LEFT:
	// 		case HALIGN_FILL_RIGHT: {
	// 			origin.x = 0.f;
	// 			if (i == lines.size() - 1) { //do not fill last line
	// 				if (p_align == HALIGN_FILL_LEFT) origin.x = 0.f;
	// 				if (p_align == HALIGN_FILL_RIGHT) origin.x = d_width;
	// 				break;
	// 			}
	// 			line = line->fit_to_width(p_width);
	// 		} break;
	// 		default: {
	// 			ERR_PRINT("Unknown halignment type");
	// 		} break;
	// 	}

	// 	int r_clip = (p_width == -1) ? -1 : (p_width + origin.x + p_pos.x);
	// 	line->draw(p_canvas_item, origin + p_pos, origin.x + p_pos.x, r_clip, p_modulate, p_outline_modulate);

	// 	origin.y += line->get_descent();
	// }
}

Size2 Font::get_string_size(const String &p_text, TextDirection p_base_dir, const String &p_lang, const String &p_ftr) const {
	// Ref<ShapedString> shaped = shape_string(p_text, 0, p_text.length(), p_base_dir, p_lang, p_ftr);
	// ERR_FAIL_COND_V(shaped.is_null(), Size2());
	// return Size2(shaped->get_size().width, MAX(shaped->get_size().height, get_height()));
	Ref<TextLayout> layout = _shape_layout(p_text, p_base_dir, p_lang, p_ftr);

	layout->set_break_mode(TEXT_BREAK_ON_NONE);
	layout->shape_now();

	return layout->get_size();
}

Size2 Font::get_wordwrap_string_size(const String &p_text, float p_width, TextDirection p_base_dir, const String &p_lang, const String &p_ftr) const {
	Ref<TextLayout> layout = _shape_layout(p_text, p_base_dir, p_lang, p_ftr);

	layout->set_break_mode(TEXT_BREAK_ON_HARD_AND_SOFT);
	layout->set_target_size(Size2(p_width, -1.f));
	layout->shape_now();

	return layout->get_size();
	// Ref<ShapedString> shaped = shape_string(p_text, 0, p_text.length(), p_base_dir, p_lang, p_ftr);
	// ERR_FAIL_COND_V(shaped.is_null(), Size2());

	// Vector<Pair<int32_t, int32_t>> lines = shaped->break_lines(p_width);
	// float height = 0;
	// float width = 0;
	// for (int i = 0; i < lines.size(); i++) {
	// 	Ref<ShapedString> line = shape_string(p_text, lines[i].first, lines[i].second, p_base_dir, p_lang, p_ftr);
	// 	ERR_FAIL_COND_V(line.is_null(), Size2());
	// 	Size2 line_size = line->get_size();
	// 	height += line_size.height;
	// 	width = MAX(width, line_size.width);
	// }
	// return Size2(width, MAX(height, get_height()));
}

float Font::draw_char(RID p_canvas_item, const Point2 &p_pos, CharType p_char, CharType p_next, const Color &p_modulate, bool p_outline) const {
	CharType ucodestr[3] = {
		static_cast<CharType>(p_char),
		static_cast<CharType>(p_next),
		0
	};
	draw(p_canvas_item, p_pos, ucodestr, -1, p_modulate, p_outline ? p_modulate : Color(1, 1, 1, 0));
	return get_string_size(ucodestr).width;
}

Size2 Font::get_char_size(CharType p_char, CharType p_next) const {
	CharType ucodestr[3] = {
		static_cast<CharType>(p_char),
		static_cast<CharType>(p_next),
		0
	};
	return get_string_size(ucodestr);
}

//data control
int Font::get_font_data_count() const {
	return data.size();
}

void Font::add_font_data(const Ref<FontData> &p_data) {
	ERR_FAIL_COND(p_data.is_null());
	data.push_back(p_data);
	data_at_size.push_back(data[data.size() - 1]->_get_font_implementation_at_size(cache_id));
	if (outline_cache_id.outline_size > 0)
		outline_data_at_size.push_back(data[data.size() - 1]->_get_font_implementation_at_size(outline_cache_id));
	else
		outline_data_at_size.push_back(Ref<FontImplementation>());

	emit_changed();
	_change_notify();
}

void Font::set_font_data(int p_idx, const Ref<FontData> &p_data) {
	ERR_FAIL_COND(p_data.is_null());
	ERR_FAIL_INDEX(p_idx, data.size());
	data.write[p_idx] = p_data;
	data_at_size.write[p_idx] = data[p_idx]->_get_font_implementation_at_size(cache_id);
	if (outline_cache_id.outline_size > 0)
		outline_data_at_size.write[p_idx] = data[p_idx]->_get_font_implementation_at_size(outline_cache_id);
	else
		outline_data_at_size.write[p_idx] = Ref<FontImplementation>();

	emit_changed();
}

Ref<FontData> Font::get_font_data(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, data.size(), Ref<FontData>());
	return data[p_idx];
}

void Font::remove_font_data(int p_idx) {
	ERR_FAIL_INDEX(p_idx, data.size());
	data.remove(p_idx);
	data_at_size.remove(p_idx);
	outline_data_at_size.remove(p_idx);

	emit_changed();
	_change_notify();
}

void Font::update_changes() {
	shaped_cache.clear();
	_reload_cache();
}

float Font::get_oversampling() {
	return oversampling;
}

void Font::set_oversampling(float p_oversampling) {
	if (oversampling != p_oversampling) {
		oversampling = p_oversampling;
		ShapedString::invalidate_all();
		TextLayout::invalidate_all();
		FontImplementation::invalidate_all();
	}
}

Font::Font() {
	shaped_cache_max_depth = 2048;

	distance_field_hint = false;

	cache_id.size = 16;
	cache_id.outline_size = 0;
	cache_id.antialiased = true;
	cache_id.force_autohinter = false;
	cache_id.hinting = FontData::HINTING_NORMAL;

	outline_cache_id = cache_id;

	spacing_top = 0;
	spacing_bottom = 0;
}

Font::~Font() {
	//NOP
}

/*************************************************************************/
/*  Font Data (Resource loader)                                          */
/*************************************************************************/

RES ResourceFormatLoaderFontData::load(const String &p_path, const String &p_original_path, Error *r_error) {

	if (r_error)
		*r_error = ERR_FILE_CANT_OPEN;

	Ref<FontData> font_data;
	font_data.instance();
	font_data->set_font_data_path(p_path);

	if (r_error)
		*r_error = OK;

	return font_data;
}

void ResourceFormatLoaderFontData::get_recognized_extensions(List<String> *p_extensions) const {

	p_extensions->push_back("fnt");
	p_extensions->push_back("font");
	p_extensions->push_back("ttf");
	p_extensions->push_back("otf");
}

bool ResourceFormatLoaderFontData::handles_type(const String &p_type) const {

	return (p_type == "FontData");
}

String ResourceFormatLoaderFontData::get_resource_type(const String &p_path) const {

	String el = p_path.get_extension().to_lower();
	if (el == "fnt" || el == "font" || el == "ttf" || el == "otf")
		return "FontData";
	return "";
}
