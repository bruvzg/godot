/*************************************************************************/
/*  godot_text_shaping.h                                                 */
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

#ifndef GODOT_NATIVE_TEXT_SHAPER_H
#define GODOT_NATIVE_TEXT_SHAPER_H

#include <gdnative/gdnative.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GODOT_TEXT_API_MAJOR 1
#define GODOT_TEXT_API_MINOR 0

typedef struct {
	//core api
	godot_gdnative_api_version version;
	void *(*constructor)(godot_object *);
	void (*destructor)(void *);
	godot_string (*get_name)(const void *);
	int32_t (*get_capabilities)(const void *);

	godot_bool (*set_property)(void *, const char *, const godot_variant *);

	godot_bool (*is_initialized)(const void *);
	godot_bool (*initialize)(void *);
	godot_bool (*uninitialize)(void *);

	void (*draw_string)(const void *, const godot_rid *, const godot_vector2 *, const godot_object *, const godot_string *, int32_t, const godot_color *, const godot_color *, const godot_vector2 *);
	void (*draw_string_justified)(const void *, const godot_rid *, const godot_vector2 *, const godot_object *, const godot_string *, int32_t, const godot_color *, const godot_color *, float);
	godot_vector2 (*get_string_size)(const void *, const godot_object *, const godot_string *, int32_t);
	godot_array (*get_string_cursor_shapes)(const void *, const godot_object *, const godot_string *, int64_t, int32_t);
	godot_array (*get_string_selection_shapes)(const void *, const godot_object *, const godot_string *, int64_t, int64_t, int32_t);
	godot_array (*get_string_line_breaks)(const void *, const godot_object *, const godot_string *, int32_t, float);
	int64_t (*string_hit_test)(const void *, const godot_object *, const godot_string *, const godot_vector2 *, int32_t);

	void (*draw_spannable)(const void *, const godot_rid *, const godot_vector2 *, const godot_object *, int32_t, const godot_vector2 *);
	void (*draw_spannable_justified)(const void *, const godot_rid *, const godot_vector2 *, const godot_object *, int32_t, float);
	godot_vector2 (*get_spannable_size)(const void *, const godot_object *, int32_t);
	godot_array (*get_spannable_cursor_shapes)(const void *, const godot_object *, int64_t, int32_t);
	godot_array (*get_spannable_selection_shapes)(const void *, const godot_object *, int64_t, int64_t, int32_t);
	godot_array (*get_spannable_line_breaks)(const void *, const godot_object *, int32_t, float);
	int64_t (*spannable_hit_test)(const void *, const godot_object *, const godot_vector2 *, int32_t);

} godot_text_shaper_interface_gdnative;

void GDAPI godot_text_shaper_register_interface(const godot_text_shaper_interface_gdnative *p_interface);

// helper functions for font direct access
int64_t GDAPI godot_text_shaper_font_get_fallback_count(godot_object *p_font);
uint32_t GDAPI godot_text_shaper_font_get_nominal_glyph_index(godot_object *p_font, int64_t p_fallback, uint32_t p_codepoint);
uint32_t GDAPI godot_text_shaper_font_get_variation_glyph_index(godot_object *p_font, int64_t p_fallback, uint32_t p_codepoint, uint32_t p_variation_selector);
godot_vector2 GDAPI godot_text_shaper_font_get_glyph_advance(godot_object *p_font, int64_t p_fallback, uint32_t p_index);
godot_vector2 GDAPI godot_text_shaper_font_get_glyph_origin(godot_object *p_font, int64_t p_fallback, uint32_t p_index);
godot_bool GDAPI godot_text_shaper_font_get_has_contour_points(godot_object *p_font, int64_t p_fallback, uint32_t p_index);
godot_vector2 GDAPI godot_text_shaper_font_get_glyph_contour_point(godot_object *p_font, int64_t p_fallback, uint32_t p_index, uint32_t p_point);
godot_vector2 GDAPI godot_text_shaper_font_get_glyph_kerning(godot_object *p_font, int64_t p_fallback, uint32_t p_index_a, uint32_t p_index_b);
godot_rect2 GDAPI godot_text_shaper_font_get_glyph_extents(godot_object *p_font, int64_t p_fallback, uint32_t p_index);
godot_vector3 GDAPI godot_text_shaper_font_get_font_extents(godot_object *p_font, int64_t p_fallback);
void *GDAPI godot_text_shaper_font_get_font_table(godot_object *p_font, int64_t p_fallback, uint32_t p_tag);
godot_bool GDAPI godot_text_shaper_font_get_script_supported(godot_object *p_font, int64_t p_fallback, uint32_t p_tag);
void GDAPI godot_text_shaper_font_draw_glyph(godot_object *p_font, const godot_rid *p_canvas_item, const godot_vector2 *p_pos, int64_t p_fallback, uint32_t p_index, const godot_color *p_modulate, godot_bool p_mirror, godot_bool p_rot_ccw, godot_bool p_rot_cw, godot_bool p_outline);

// helper functions for spannable direct access
int32_t GDAPI godot_text_shaper_spannable_hash(godot_object *p_spannable);
int64_t GDAPI godot_text_shaper_spannable_get_run_start(godot_object *p_spannable, uint32_t p_attribute, int64_t p_index);
int64_t GDAPI godot_text_shaper_spannable_get_run_end(godot_object *p_spannable, uint32_t p_attribute, int64_t p_index);
godot_bool GDAPI godot_text_shaper_spannable_has_attribute(godot_object *p_spannable, uint32_t p_attribute, int64_t p_index);
godot_variant GDAPI godot_text_shaper_spannable_get_attribute(godot_object *p_spannable, uint32_t p_attribute, int64_t p_index);

// helper function for translation server direct access
godot_string GDAPI godot_text_shaper_get_locale();

#ifdef __cplusplus
}
#endif

#endif /* !GODOT_NATIVE_TEXT_SHAPER_H */
