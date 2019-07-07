
/*************************************************************************/
/*  text_shaping_gdnative.cpp                                            */
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

#include "text_shaping_gdnative.h"
#include "servers/text_shaping_server.h"

#include "core/os/os.h"

#include "scene/resources/font.h"
#include "scene/resources/spannable.h"

TextShapingInterfaceGDNative::TextShapingInterfaceGDNative() {
	print_verbose("Construct gdnative interface\n");

	data = NULL;
	interface = NULL;
}

TextShapingInterfaceGDNative::~TextShapingInterfaceGDNative() {
	print_verbose("Destruct gdnative interface\n");

	if (interface != NULL && is_initialized()) {
		interface->destructor(data);

		data = NULL;
		interface = NULL;
	};
}

void TextShapingInterfaceGDNative::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_property", "name", "value"), &TextShapingInterfaceGDNative::set_property);

	ADD_PROPERTY_DEFAULT("interface_is_initialized", false);
}

void TextShapingInterfaceGDNative::set_interface(const godot_text_shaper_interface_gdnative *p_interface) {

	if (interface != NULL) {
		interface->destructor(data);

		data = NULL;
		interface = NULL;
	};

	interface = p_interface;

	data = interface->constructor((godot_object *)this);
}

bool TextShapingInterfaceGDNative::set_property(const String &p_name, const Variant &p_value) {

	ERR_FAIL_COND_V(interface == NULL, false);

	return interface->set_property(data, p_name.ascii().get_data(), (const godot_variant *)&p_value);
}

StringName TextShapingInterfaceGDNative::get_name() const {

	ERR_FAIL_COND_V(interface == NULL, StringName("Invalid"));

	godot_string result = interface->get_name(data);
	StringName name = *(String *)&result;
	godot_string_destroy(&result);

	return name;
}

int32_t TextShapingInterfaceGDNative::get_capabilities() const {

	ERR_FAIL_COND_V(interface == NULL, 0);

	return interface->get_capabilities(data);
}

void TextShapingInterfaceGDNative::draw_string(RID p_canvas_item, const Point2 &p_pos, const Ref<Font> &p_font, const String &p_string, int32_t p_direction, const Color &p_modulate, const Color &p_outline_modulate, const Vector2 &p_clip) const {

	ERR_FAIL_COND(interface == NULL);

	const godot_rid *canvas_item = (const godot_rid *)&p_canvas_item;
	const godot_vector2 *pos = (const godot_vector2 *)&p_pos;
	const godot_object *font = (const godot_object *)p_font.ptr();
	const godot_string *string = (const godot_string *)&p_string;
	const godot_color *modulate = (const godot_color *)&p_modulate;
	const godot_color *outline_modulate = (const godot_color *)&p_outline_modulate;
	const godot_vector2 *clip = (const godot_vector2 *)&p_clip;

	interface->draw_string(data, canvas_item, pos, font, string, p_direction, modulate, outline_modulate, clip);
}

void TextShapingInterfaceGDNative::draw_string_justified(RID p_canvas_item, const Point2 &p_pos, const Ref<Font> &p_font, const String &p_string, int32_t p_direction, const Color &p_modulate, const Color &p_outline_modulate, float p_line_w) const {

	ERR_FAIL_COND(interface == NULL);

	const godot_rid *canvas_item = (const godot_rid *)&p_canvas_item;
	const godot_vector2 *pos = (const godot_vector2 *)&p_pos;
	const godot_object *font = (const godot_object *)p_font.ptr();
	const godot_string *string = (const godot_string *)&p_string;
	const godot_color *modulate = (const godot_color *)&p_modulate;
	const godot_color *outline_modulate = (const godot_color *)&p_outline_modulate;

	interface->draw_string_justified(data, canvas_item, pos, font, string, p_direction, modulate, outline_modulate, p_line_w);
}

Size2 TextShapingInterfaceGDNative::get_string_size(const Ref<Font> &p_font, const String &p_string, int32_t p_direction) const {

	ERR_FAIL_COND_V(interface == NULL, Size2());

	const godot_object *font = (const godot_object *)p_font.ptr();
	const godot_string *string = (const godot_string *)&p_string;

	godot_vector2 result = interface->get_string_size(data, font, string, p_direction);
	Size2 *ret = (Size2 *)&result;

	return *ret;
}

Array TextShapingInterfaceGDNative::get_string_cursor_shapes(const Ref<Font> &p_font, const String &p_string, int64_t p_position, int32_t p_direction) const {

	ERR_FAIL_COND_V(interface == NULL, Array());

	const godot_object *font = (const godot_object *)p_font.ptr();
	const godot_string *string = (const godot_string *)&p_string;

	godot_array result = interface->get_string_cursor_shapes(data, font, string, p_position, p_direction);
	Array *ret = (Array *)&result;

	return *ret;
}

Array TextShapingInterfaceGDNative::get_string_selection_shapes(const Ref<Font> &p_font, const String &p_string, int64_t p_start, int64_t p_end, int32_t p_direction) const {

	ERR_FAIL_COND_V(interface == NULL, Array());

	const godot_object *font = (const godot_object *)p_font.ptr();
	const godot_string *string = (const godot_string *)&p_string;

	godot_array result = interface->get_string_selection_shapes(data, font, string, p_start, p_end, p_direction);
	Array *ret = (Array *)&result;

	return *ret;
}

Array TextShapingInterfaceGDNative::get_string_line_breaks(const Ref<Font> &p_font, const String &p_string, int32_t p_direction, float p_line_w) const {

	ERR_FAIL_COND_V(interface == NULL, Array());

	const godot_object *font = (const godot_object *)p_font.ptr();
	const godot_string *string = (const godot_string *)&p_string;

	godot_array result = interface->get_string_line_breaks(data, font, string, p_direction, p_line_w);
	Array *ret = (Array *)&result;

	return *ret;
}

int64_t TextShapingInterfaceGDNative::string_hit_test(const Ref<Font> &p_font, const String &p_string, const Point2 &p_point, int32_t p_direction) const {

	ERR_FAIL_COND_V(interface == NULL, -1);

	const godot_object *font = (const godot_object *)p_font.ptr();
	const godot_string *string = (const godot_string *)&p_string;
	const godot_vector2 *point = (const godot_vector2 *)&p_point;

	return interface->string_hit_test(data, font, string, point, p_direction);
}

void TextShapingInterfaceGDNative::draw_spannable(RID p_canvas_item, const Point2 &p_pos, const Ref<Spannable> &p_spannable, int32_t p_direction, const Vector2 &p_clip) const {

	ERR_FAIL_COND(interface == NULL);

	const godot_rid *canvas_item = (const godot_rid *)&p_canvas_item;
	const godot_vector2 *pos = (const godot_vector2 *)&p_pos;
	const godot_object *spannable = (const godot_object *)p_spannable.ptr();
	const godot_vector2 *clip = (const godot_vector2 *)&p_clip;

	interface->draw_spannable(data, canvas_item, pos, spannable, p_direction, clip);
}

void TextShapingInterfaceGDNative::draw_spannable_justified(RID p_canvas_item, const Point2 &p_pos, const Ref<Spannable> &p_spannable, int32_t p_direction, float p_line_w) const {

	ERR_FAIL_COND(interface == NULL);

	const godot_rid *canvas_item = (const godot_rid *)&p_canvas_item;
	const godot_vector2 *pos = (const godot_vector2 *)&p_pos;
	const godot_object *spannable = (const godot_object *)p_spannable.ptr();

	interface->draw_spannable_justified(data, canvas_item, pos, spannable, p_direction, p_line_w);
}

Size2 TextShapingInterfaceGDNative::get_spannable_size(const Ref<Spannable> &p_spannable, int32_t p_direction) const {

	ERR_FAIL_COND_V(interface == NULL, Size2());

	const godot_object *spannable = (const godot_object *)p_spannable.ptr();

	godot_vector2 result = interface->get_spannable_size(data, spannable, p_direction);
	Size2 *ret = (Size2 *)&result;

	return *ret;
}

Array TextShapingInterfaceGDNative::get_spannable_cursor_shapes(const Ref<Spannable> &p_spannable, int64_t p_position, int32_t p_direction) const {

	ERR_FAIL_COND_V(interface == NULL, Array());

	const godot_object *spannable = (const godot_object *)p_spannable.ptr();

	godot_array result = interface->get_spannable_cursor_shapes(data, spannable, p_position, p_direction);
	Array *ret = (Array *)&result;

	return *ret;
}

Array TextShapingInterfaceGDNative::get_spannable_selection_shapes(const Ref<Spannable> &p_spannable, int64_t p_start, int64_t p_end, int32_t p_direction) const {

	ERR_FAIL_COND_V(interface == NULL, Array());

	const godot_object *spannable = (const godot_object *)p_spannable.ptr();

	godot_array result = interface->get_spannable_selection_shapes(data, spannable, p_start, p_end, p_direction);
	Array *ret = (Array *)&result;

	return *ret;
}

Array TextShapingInterfaceGDNative::get_spannable_line_breaks(const Ref<Spannable> &p_spannable, int32_t p_direction, float p_line_w) const {

	ERR_FAIL_COND_V(interface == NULL, Array());

	const godot_object *spannable = (const godot_object *)p_spannable.ptr();

	godot_array result = interface->get_spannable_line_breaks(data, spannable, p_direction, p_line_w);
	Array *ret = (Array *)&result;

	return *ret;
}

int64_t TextShapingInterfaceGDNative::spannable_hit_test(const Ref<Spannable> &p_spannable, const Point2 &p_point, int32_t p_direction) const {

	ERR_FAIL_COND_V(interface == NULL, -1);

	const godot_object *spannable = (const godot_object *)p_spannable.ptr();
	const godot_vector2 *point = (const godot_vector2 *)&p_point;

	return interface->spannable_hit_test(data, spannable, point, p_direction);
}

bool TextShapingInterfaceGDNative::is_initialized() {

	ERR_FAIL_COND_V(interface == NULL, false);
	return interface->is_initialized(data);
}

bool TextShapingInterfaceGDNative::initialize() {

	ERR_FAIL_COND_V(interface == NULL, false);
	return interface->initialize(data);
}

bool TextShapingInterfaceGDNative::uninitialize() {

	ERR_FAIL_COND_V(interface == NULL, false);
	return interface->uninitialize(data);
}

/*************************************************************************/
/* GDNative helper callbacks                                             */
/*************************************************************************/

extern "C" {

void GDAPI godot_text_shaper_register_interface(const godot_text_shaper_interface_gdnative *p_interface) {

	Ref<TextShapingInterfaceGDNative> new_interface;
	new_interface.instance();
	new_interface->set_interface(p_interface);
	TextShapingServer::get_singleton()->add_interface(new_interface);
}

// helper functions for font direct access
int64_t GDAPI godot_text_shaper_font_get_fallback_count(godot_object *p_font) {

#ifdef DEBUG_ENABLED
	Font *font = Object::cast_to<Font>((Object *)p_font);
#else
	Font *font = (Font *)p_font;
#endif
	ERR_FAIL_COND_V(font == NULL, -1);

	return font->get_fallback_count();
}

uint32_t GDAPI godot_text_shaper_font_get_nominal_glyph_index(godot_object *p_font, int64_t p_fallback, uint32_t p_codepoint) {

#ifdef DEBUG_ENABLED
	Font *font = Object::cast_to<Font>((Object *)p_font);
#else
	Font *font = (Font *)p_font;
#endif
	ERR_FAIL_COND_V(font == NULL, -1);

	return font->get_nominal_glyph_index(p_fallback, p_codepoint);
}

uint32_t GDAPI godot_text_shaper_font_get_variation_glyph_index(godot_object *p_font, int64_t p_fallback, uint32_t p_codepoint, uint32_t p_variation_selector) {

#ifdef DEBUG_ENABLED
	Font *font = Object::cast_to<Font>((Object *)p_font);
#else
	Font *font = (Font *)p_font;
#endif
	ERR_FAIL_COND_V(font == NULL, -1);

	return font->get_variation_glyph_index(p_fallback, p_codepoint, p_variation_selector);
}

godot_vector2 GDAPI godot_text_shaper_font_get_glyph_advance(godot_object *p_font, int64_t p_fallback, uint32_t p_index) {

	godot_vector2 point;
	Point2 *point_ptr = (Point2 *)&point;

#ifdef DEBUG_ENABLED
	Font *font = Object::cast_to<Font>((Object *)p_font);
#else
	Font *font = (Font *)p_font;
#endif
	if (font == NULL) {
		*point_ptr = Point2();
		ERR_FAIL_V(point);
	}

	*point_ptr = font->get_glyph_advance(p_fallback, p_index);
	return point;
}

godot_vector2 GDAPI godot_text_shaper_font_get_glyph_origin(godot_object *p_font, int64_t p_fallback, uint32_t p_index) {

	godot_vector2 point;
	Point2 *point_ptr = (Point2 *)&point;

#ifdef DEBUG_ENABLED
	Font *font = Object::cast_to<Font>((Object *)p_font);
#else
	Font *font = (Font *)p_font;
#endif
	if (font == NULL) {
		*point_ptr = Point2();
		ERR_FAIL_V(point);
	}

	*point_ptr = font->get_glyph_origin(p_fallback, p_index);
	return point;
}

godot_bool GDAPI godot_text_shaper_font_get_has_contour_points(godot_object *p_font, int64_t p_fallback, uint32_t p_index) {

#ifdef DEBUG_ENABLED
	Font *font = Object::cast_to<Font>((Object *)p_font);
#else
	Font *font = (Font *)p_font;
#endif
	ERR_FAIL_COND_V(font == NULL, false);

	return font->get_has_contour_points(p_fallback, p_index);
}

godot_vector2 GDAPI godot_text_shaper_font_get_glyph_contour_point(godot_object *p_font, int64_t p_fallback, uint32_t p_index, uint32_t p_point) {

	godot_vector2 point;
	Point2 *point_ptr = (Point2 *)&point;

#ifdef DEBUG_ENABLED
	Font *font = Object::cast_to<Font>((Object *)p_font);
#else
	Font *font = (Font *)p_font;
#endif
	if (font == NULL) {
		*point_ptr = Point2();
		ERR_FAIL_V(point);
	}

	*point_ptr = font->get_glyph_contour_point(p_fallback, p_index, p_point);
	return point;
}

godot_vector2 GDAPI godot_text_shaper_font_get_glyph_kerning(godot_object *p_font, int64_t p_fallback, uint32_t p_index_a, uint32_t p_index_b) {

	godot_vector2 point;
	Point2 *point_ptr = (Point2 *)&point;

#ifdef DEBUG_ENABLED
	Font *font = Object::cast_to<Font>((Object *)p_font);
#else
	Font *font = (Font *)p_font;
#endif
	if (font == NULL) {
		*point_ptr = Point2();
		ERR_FAIL_V(point);
	}

	*point_ptr = font->get_glyph_kerning(p_fallback, p_index_a, p_index_b);
	return point;
}

godot_rect2 GDAPI godot_text_shaper_font_get_glyph_extents(godot_object *p_font, int64_t p_fallback, uint32_t p_index) {

	godot_rect2 rect;
	Rect2 *rect_ptr = (Rect2 *)&rect;

#ifdef DEBUG_ENABLED
	Font *font = Object::cast_to<Font>((Object *)p_font);
#else
	Font *font = (Font *)p_font;
#endif
	if (font == NULL) {
		*rect_ptr = Rect2();
		ERR_FAIL_V(rect);
	}

	*rect_ptr = font->get_glyph_extents(p_fallback, p_index);
	return rect;
}

godot_vector3 GDAPI godot_text_shaper_font_get_font_extents(godot_object *p_font, int64_t p_fallback) {

	godot_vector3 point;
	Vector3 *point_ptr = (Vector3 *)&point;

#ifdef DEBUG_ENABLED
	Font *font = Object::cast_to<Font>((Object *)p_font);
#else
	Font *font = (Font *)p_font;
#endif
	if (font == NULL) {
		*point_ptr = Vector3();
		ERR_FAIL_V(point);
	}

	*point_ptr = font->get_font_extents(p_fallback);
	return point;
}

void *GDAPI godot_text_shaper_font_get_font_table(godot_object *p_font, int64_t p_fallback, uint32_t p_tag) {

#ifdef DEBUG_ENABLED
	Font *font = Object::cast_to<Font>((Object *)p_font);
#else
	Font *font = (Font *)p_font;
#endif
	ERR_FAIL_COND_V(font == NULL, NULL);

	return font->get_font_table(p_fallback, p_tag);
}

godot_bool GDAPI godot_text_shaper_font_get_script_supported(godot_object *p_font, int64_t p_fallback, uint32_t p_tag) {

#ifdef DEBUG_ENABLED
	Font *font = Object::cast_to<Font>((Object *)p_font);
#else
	Font *font = (Font *)p_font;
#endif
	ERR_FAIL_COND_V(font == NULL, false);

	return font->get_script_supported(p_fallback, p_tag);
}

void GDAPI godot_text_shaper_font_draw_glyph(godot_object *p_font, const godot_rid *p_canvas_item, const godot_vector2 *p_pos, int64_t p_fallback, uint32_t p_index, const godot_color *p_modulate, godot_bool p_mirror, godot_bool p_rot_ccw, godot_bool p_rot_cw, godot_bool p_outline) {

#ifdef DEBUG_ENABLED
	Font *font = Object::cast_to<Font>((Object *)p_font);
#else
	Font *font = (Font *)p_font;
#endif
	ERR_FAIL_COND(font == NULL);

	RID canvas_item = *(const RID *)p_canvas_item;
	Vector2 pos = *(const Vector2 *)p_pos;
	Color modulate = *(const Color *)p_modulate;

	font->draw_glyph(canvas_item, pos, p_fallback, p_index, modulate, p_mirror, p_rot_ccw, p_rot_cw, p_outline);
}

// helper function for spannable direct access

int32_t GDAPI godot_text_shaper_spannable_hash(godot_object *p_spannable) {

#ifdef DEBUG_ENABLED
	Spannable *span = Object::cast_to<Spannable>((Object *)p_spannable);
#else
	Spannable *span = (Spannable *)p_spannable;
#endif
	ERR_FAIL_COND_V(span == NULL, -1);

	return span->hash();
}

int64_t GDAPI godot_text_shaper_spannable_get_run_start(godot_object *p_spannable, uint32_t p_attribute, int64_t p_index) {

#ifdef DEBUG_ENABLED
	Spannable *span = Object::cast_to<Spannable>((Object *)p_spannable);
#else
	Spannable *span = (Spannable *)p_spannable;
#endif
	ERR_FAIL_COND_V(span == NULL, -1);

	return span->get_run_start((Spannable::TextAttribute)p_attribute, p_index);
}

int64_t GDAPI godot_text_shaper_spannable_get_run_end(godot_object *p_spannable, uint32_t p_attribute, int64_t p_index) {

#ifdef DEBUG_ENABLED
	Spannable *span = Object::cast_to<Spannable>((Object *)p_spannable);
#else
	Spannable *span = (Spannable *)p_spannable;
#endif
	ERR_FAIL_COND_V(span == NULL, -1);

	return span->get_run_end((Spannable::TextAttribute)p_attribute, p_index);
}

godot_bool GDAPI godot_text_shaper_spannable_has_attribute(godot_object *p_spannable, uint32_t p_attribute, int64_t p_index) {

#ifdef DEBUG_ENABLED
	Spannable *span = Object::cast_to<Spannable>((Object *)p_spannable);
#else
	Spannable *span = (Spannable *)p_spannable;
#endif
	ERR_FAIL_COND_V(span == NULL, false);

	return span->has_attribute((Spannable::TextAttribute)p_attribute, p_index);
}

godot_variant GDAPI godot_text_shaper_spannable_get_attribute(godot_object *p_spannable, uint32_t p_attribute, int64_t p_index) {

	godot_variant variant;
	Variant *variant_ptr = (Variant *)&variant;

#ifdef DEBUG_ENABLED
	Spannable *span = Object::cast_to<Spannable>((Object *)p_spannable);
#else
	Spannable *span = (Spannable *)p_spannable;
#endif
	if (span == NULL) {
		memnew_placement(variant_ptr, Variant);
		ERR_FAIL_V(variant);
	}

	memnew_placement(variant_ptr, Variant(span->get_attribute((Spannable::TextAttribute)p_attribute, p_index)));
	return variant;
}

godot_string GDAPI godot_text_shaper_get_locale() {

	godot_string raw_dest;
	String *dest = (String *)&raw_dest;
	memnew_placement(dest, String(OS::get_singleton()->get_locale()));
	return raw_dest;
}
};
