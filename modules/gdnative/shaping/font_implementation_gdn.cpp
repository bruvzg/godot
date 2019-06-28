/*************************************************************************/
/*  font_implementation_gdn.cpp                                          */
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

#include "font_implementation_gdn.h"
#include "modules/gdnative/gdnative.h"
#include "servers/shaping/shaping_interface.h"

void FontImplementationGDN::invalidate() {

	if (interface) {
		interface->destroy_font_impl(interface_data, font_data);
	}
	interface = NULL;
	interface_data = NULL;
	font_data = NULL;

	valid = false;
}

bool FontImplementationGDN::require_reload() const {
	return !valid;
}

void *FontImplementationGDN::get_native_handle() const {
	return font_data;
}

float FontImplementationGDN::get_font_scale() const {
	ERR_FAIL_COND_V(interface == NULL, 0);
	return interface->font_impl_font_scale(interface_data, font_data);
}

uint32_t FontImplementationGDN::get_glyph(uint32_t p_unicode, uint32_t p_variation_selector) const {
	ERR_FAIL_COND_V(interface == NULL, 0);
	return interface->font_impl_get_glyph(interface_data, font_data, p_unicode, p_variation_selector);
}

float FontImplementationGDN::get_advance(uint32_t p_glyph) const {
	ERR_FAIL_COND_V(interface == NULL, 0);
	return interface->font_impl_get_advance(interface_data, font_data, p_glyph);
}

float FontImplementationGDN::get_kerning(uint32_t p_glyph_a, uint32_t p_glyph_b) const {
	ERR_FAIL_COND_V(interface == NULL, 0);
	return interface->font_impl_get_kerning(interface_data, font_data, p_glyph_a, p_glyph_b);
}

float FontImplementationGDN::get_v_advance(uint32_t p_glyph) const {
	ERR_FAIL_COND_V(interface == NULL, 0);
	return interface->font_impl_get_advance(interface_data, font_data, p_glyph); ///TODO
}

float FontImplementationGDN::get_v_kerning(uint32_t p_glyph_a, uint32_t p_glyph_b) const {
	ERR_FAIL_COND_V(interface == NULL, 0);
	return interface->font_impl_get_kerning(interface_data, font_data, p_glyph_a, p_glyph_b); ///TODO
}

Size2 FontImplementationGDN::get_glyph_size(uint32_t p_glyph) const {
	ERR_FAIL_COND_V(interface == NULL, Size2());
	godot_vector2 result = interface->font_impl_get_size(interface_data, font_data, p_glyph);
	Vector2 *vec = (Vector2 *)&result;
	return *vec;
}

float FontImplementationGDN::get_ascent() const {
	ERR_FAIL_COND_V(interface == NULL, 0);
	return interface->font_impl_get_ascent(interface_data, font_data);
}

float FontImplementationGDN::get_descent() const {
	ERR_FAIL_COND_V(interface == NULL, 0);
	return interface->font_impl_get_descent(interface_data, font_data);
}

float FontImplementationGDN::get_line_gap() const {
	ERR_FAIL_COND_V(interface == NULL, 0);
	return interface->font_impl_get_line_gap(interface_data, font_data);
}

float FontImplementationGDN::get_underline_position() const {
	ERR_FAIL_COND_V(interface == NULL, 0);
	return interface->font_impl_get_underline_position(interface_data, font_data);
}

float FontImplementationGDN::get_underline_thickness() const {
	ERR_FAIL_COND_V(interface == NULL, 0);
	return interface->font_impl_get_underline_thickness(interface_data, font_data);
}

String FontImplementationGDN::get_name() const {
	ERR_FAIL_COND_V(interface == NULL, StringName(""));

	godot_string result = interface->font_impl_get_name(interface_data, font_data);
	String name = *(String *)&result;
	godot_string_destroy(&result);

	return name;
}

int32_t FontImplementationGDN::get_script_support_priority(uint32_t p_script) const {
	ERR_FAIL_COND_V(interface == NULL, 0);
	return interface->font_impl_get_script_support_priority(interface_data, font_data, p_script);
}

int32_t FontImplementationGDN::get_language_support_priority(const String &p_lang) const {
	ERR_FAIL_COND_V(interface == NULL, 0);
	const godot_string *lang = (const godot_string *)&p_lang;
	return interface->font_impl_get_language_support_priority(interface_data, font_data, lang);
}

void FontImplementationGDN::draw_glyph(RID p_canvas_item, const Point2 &p_pos, uint32_t p_glyph, const Color &p_modulate, bool p_rotate_cw) const {
	ERR_FAIL_COND(interface == NULL);
	//TODO use rotate_cw
	interface->font_impl_draw_glpyh(interface_data, font_data, (godot_rid *)&p_canvas_item, (godot_vector2 *)&p_pos, p_glyph, (godot_color *)&p_modulate);
}

Error FontImplementationGDN::create(const FontData *p_data, FontData::CacheID p_cache_id) {
	return ERR_CANT_CREATE; //not used directly
}

FontImplementationGDN::FontImplementationGDN() {
	interface = NULL;
	interface_data = NULL;
	font_data = NULL;
	valid = false;
}

FontImplementationGDN::~FontImplementationGDN() {
	invalidate();
}
