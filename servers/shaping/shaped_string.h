/*************************************************************************/
/*  shaped_string.h                                                      */
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

#ifndef SHAPED_STRING_H
#define SHAPED_STRING_H

#include "servers/shaping/shaping_defines.h"

/*************************************************************************/
/*  AttributeMap                                               */
/*************************************************************************/

class AttributeMap : public Reference {
	GDCLASS(AttributeMap, Reference);

	Map<int32_t, HashMap<TextAttributes, Variant>> attribs;

	bool _compare_attributes(const HashMap<TextAttributes, Variant> &p_first, const HashMap<TextAttributes, Variant> &p_second) const;
	void _ensure_break(int64_t p_key);
	void _optimize_attributes();

protected:
	static void _bind_methods();

public:
	const Map<int32_t, HashMap<TextAttributes, Variant>>::Element *find_run(int32_t p_pos) const;

	bool has_attribute(int32_t p_pos, int p_attrib) const;
	Variant get_attribute(int32_t p_pos, int p_attrib) const;
	int32_t get_attribute_start(int32_t p_pos) const;
	int32_t get_attribute_end(int32_t p_pos) const;

	void add_attribute(int p_attribute, const Variant &p_value, int32_t p_start, int32_t p_end);
	void remove_attribute(int p_attribute, int32_t p_start, int32_t p_end);
	void remove_attributes(int32_t p_start, int32_t p_end);
	void clear_attributes();
};

/*************************************************************************/
/*  ShapedString                                                         */
/*************************************************************************/

class ShapedString : public Reference {
	GDCLASS(ShapedString, Reference);

	Vector<Run> visual;

	int ascent;
	int descent;
	int width;

	int spacing_top;
	int spacing_bottom;

	bool valid;
	bool display_controls;

	TextDirection base_dir;

protected:
	static void _bind_methods();

public:
	void update_metrics(int p_extra_top, int p_extra_bottom, TextDirection p_base_dir);
	Vector<Run> &get_runs();
	const Vector<Run> &get_runs() const;

	bool require_reload() const;

	int hit_test(float p_x) const;
	Vector2 hit_test_cluster(float p_x) const;

	Ref<ShapedString> copy() const;
	Ref<ShapedString> align_tabs(int p_tab_width) const;
	Ref<ShapedString> fit_to_width(int p_width, bool p_trim_edges = true) const;
	Vector<Pair<int32_t, int32_t>> break_lines(int p_width = -1) const;
	Array _break_lines(int p_width = -1) const;

	TextDirection get_base_direciton() const;
	TextDirection get_dominant_direciton_in_range(int32_t p_start, int32_t p_end) const;

	void draw(RID p_canvas_item, const Point2 &p_pos, int p_clip_l = 0, int p_clip_r = -1, const Color &p_modulate = Color(1, 1, 1), const Color &p_outline_modulate = Color(1, 1, 1, 0)) const;

	void draw_with_attributes(RID p_canvas_item, const Point2 &p_pos, int p_clip_l, int p_clip_r, const Ref<AttributeMap> &p_color_map) const;
	//void draw_with_callback(RID p_canvas_item, const Point2 &p_pos, int p_clip_l = 0, int p_clip_r = -1, CB) const;

	Vector<Rect2> get_selection_rects(int32_t p_start, int32_t p_end) const;
	Array _get_selection_rects(int32_t p_start, int32_t p_end) const;

	int get_missing_glyph_count() const;

	//void draw_tabs(RID p_canvas_item, const Point2 &p_pos, const Ref<Texture> &p_texture, int p_clip_l = 0, int p_clip_r = -1, const Color &p_modulate = Color(1, 1, 1)) const;
	//void draw_spaces(RID p_canvas_item, const Point2 &p_pos, const Ref<Texture> &p_texture, int p_clip_l = 0, int p_clip_r = -1, const Color &p_modulate = Color(1, 1, 1)) const;

	bool get_display_controls() const;
	void set_display_controls(bool p_enable);

	float get_ltr_caret_offset(int32_t p_caret_pos) const;
	float get_rtl_caret_offset(int32_t p_caret_pos) const;

	int32_t get_run_start(int32_t p_pos) const;
	int32_t get_run_end(int32_t p_pos) const;

	Size2 get_size() const;

	float get_ascent() const;
	float get_descent() const;

	Array get_data() const;

	static Ref<ShapedString> invalid();

	//self list api
	SelfList<ShapedString> shaped_list;

	static Mutex *shaped_mutex;
	static SelfList<ShapedString>::List *shapeds;

	static void invalidate_all();

	static void initialize_self_list();
	static void finish_self_list();

	ShapedString();
	~ShapedString();
};

#endif
