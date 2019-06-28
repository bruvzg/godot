/*************************************************************************/
/*  godot_shaping.h                                                      */
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

#ifndef GODOT_SHAPING_H
#define GODOT_SHAPING_H

#include <gdnative/gdnative.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GODOT_SHAPING_API_MAJOR 1
#define GODOT_SHAPING_API_MINOR 0

#define GODOT_SHAPING_INTERFACE_NONE 0
#define GODOT_SHAPING_INTERFACE_BIDI_LAYOUT 1
#define GODOT_SHAPING_INTERFACE_VERTICAL_LAYOUT 2
#define GODOT_SHAPING_INTERFACE_GENERIC_SHAPING 4
#define GODOT_SHAPING_INTERFACE_OPENTYPE_SHAPING 8
#define GODOT_SHAPING_INTERFACE_GRAPHITE2_SHAPING 16

#define GODOT_TEXT_DIRECTION_INVALID -2
#define GODOT_TEXT_DIRECTION_AUTO -1
#define GODOT_TEXT_DIRECTION_LTR 0
#define GODOT_TEXT_DIRECTION_RTL 1

#define GODOT_RUN_BREAK_NO 0
#define GODOT_RUN_BREAK_SOFT 1
#define GODOT_RUN_BREAK_HARD 2

#define GODOT_GLYPH_INVALD 0
#define GODOT_GLYPH_VALID 1
#define GODOT_GLYPH_SPACE 2
#define GODOT_GLYPH_ELONGATION 4
#define GODOT_GLYPH_TAB 8
#define GODOT_GLYPH_VIRTUAL 16
#define GODOT_GLYPH_CONTROL 32

typedef struct {
	//core api
	godot_gdnative_api_version version;
	void *(*constructor)(godot_object *);
	void (*destructor)(void *);
	godot_string (*get_name)(const void *);
	godot_string (*get_info)(const void *);
	int32_t (*get_capabilities)(const void *);

	//S VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE
	int32_t (*analyse_text)(void *, const godot_string *, int32_t, int32_t, int32_t, const godot_string *, godot_object *);
	godot_bool (*shape_run)(void *, const godot_string *, godot_object *, godot_object *, godot_object *, const godot_string *, const godot_string *);
	int32_t (*get_string_direction)(void *, const godot_string *);
	//E VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE

	//TODO ADD VERSION 2

	int32_t (*get_locale_direction)(void *, const godot_string *);

	void *(*create_font_impl)(const void *, const godot_object *);
	void (*destroy_font_impl)(const void *, void *);

	float (*font_impl_font_scale)(const void *, const void *);
	uint32_t (*font_impl_get_glyph)(const void *, const void *, uint32_t, uint32_t);
	float (*font_impl_get_advance)(const void *, const void *, uint32_t);
	float (*font_impl_get_kerning)(const void *, const void *, uint32_t, uint32_t);
	godot_vector2 (*font_impl_get_size)(const void *, const void *, uint32_t);

	float (*font_impl_get_ascent)(const void *, const void *);
	float (*font_impl_get_descent)(const void *, const void *);
	float (*font_impl_get_line_gap)(const void *, const void *);
	float (*font_impl_get_underline_position)(const void *, const void *);
	float (*font_impl_get_underline_thickness)(const void *, const void *);

	godot_string (*font_impl_get_name)(const void *, const void *);
	int32_t (*font_impl_get_script_support_priority)(const void *, const void *, uint32_t);
	int32_t (*font_impl_get_language_support_priority)(const void *, const void *, const godot_string *);
	void (*font_impl_draw_glpyh)(const void *, const void *, godot_rid *, godot_vector2 *, uint32_t, godot_color *);
} godot_shaping_interface_gdnative;

//ShapingInterface registration
void GDAPI godot_shaping_register_interface(const godot_shaping_interface_gdnative *p_interface);

//Font direct access helper functions
float GDAPI godot_shaping_font_oversampling();

int GDAPI godot_shaping_hexbox_advance(uint32_t p_unicode);
int GDAPI godot_shaping_hexbox_ascent();
int GDAPI godot_shaping_hexbox_descent();

//FontData direct access helper functions
godot_string GDAPI godot_shaping_font_data_get_type(godot_object *p_fontdata);
godot_string GDAPI godot_shaping_font_data_get_filename(godot_object *p_fontdata);
void godot_shaping_font_data_get_static_font_data(godot_object *p_fontdata, const uint8_t **o_font_mem, size_t *o_font_mem_size);

//Vector<Runs> direct access helper functions
int32_t GDAPI godot_shaping_runs_size(const godot_object *p_vector);
void GDAPI godot_shaping_runs_clear(godot_object *p_vector);
void GDAPI godot_shaping_runs_push_back(godot_object *p_vector, int32_t p_start, int32_t p_end, int32_t p_level, uint32_t p_script, uint8_t p_break_type);
godot_object *GDAPI godot_shaping_runs_get(const godot_object *p_vector, int32_t p_index);

//Run direct access helper functions
void GDAPI godot_shaping_run_set_bounds(godot_object *p_run, int32_t p_start, int32_t p_end, int32_t p_level, uint32_t p_script);
void GDAPI godot_shaping_run_get_bounds(const godot_object *p_run, int32_t *o_start, int32_t *o_end, int32_t *o_level, uint32_t *o_script);
void GDAPI godot_shaping_run_set_metrics(godot_object *p_run, int p_ascent, int p_descent, int p_width);
void GDAPI godot_shaping_run_get_metrics(const godot_object *p_run, int *o_ascent, int *o_descent, int *o_width);

//Cluster/Glyph direct access helper functions (write only)
void GDAPI godot_shaping_run_cluster_clear(godot_object *p_run);
void GDAPI godot_shaping_run_cluster_push_back(godot_object *p_run, int32_t p_start, int32_t p_end, int p_advance, int32_t p_repeat, void *p_font, void *p_outl_font, uint8_t p_flags);
void GDAPI godot_shaping_run_glyph_push_back(godot_object *p_run, uint32_t p_codepoint, float p_offset_x, float p_offset_y, int p_advance);

//Vector<FontImplementation> direct access helper functions
int32_t GDAPI godot_shaping_font_imp_vector_size(godot_object *p_vector);
godot_object *GDAPI godot_shaping_font_imp_vector_item(godot_object *p_vector, int32_t p_index);

//FontImplementation direct access helper functions
void *GDAPI godot_shaping_font_imp_native_data(godot_object *p_font);

//TranslationServer direct access helper functions
godot_string GDAPI godot_shaping_get_locale();

#ifdef __cplusplus
}
#endif

#endif
