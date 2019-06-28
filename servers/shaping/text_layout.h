/*************************************************************************/
/*  text_layout.h                                                        */
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

#ifndef TEXT_LAYOUT_H
#define TEXT_LAYOUT_H

#include "core/os/thread_safe.h"
#include "core/pair.h"
#include "core/reference.h"
#include "scene/resources/texture.h"
#include "servers/shaping/shaping_defines.h"

typedef Map<int, HashMap<TextAttributes, Variant>> AMap;

class TextLayout : public Reference {

	struct Column {
		float offset; //indent before the run
		Vector<Run> visual;
	};

	struct Line {
		int start;
		int end;

		Vector<Column> columns;

		float ascent;
		float descent;
		Rect2 rect;
	};

	//init data
	String text;
	TextDirection base_direction;
	TextBreakFlags brk_flags;
	TextJustificationFlags jst_flags;
	TextTabluationFlags tab_flags;
	HAlign halign;
	VAlign valign;
	Size2 target_size;
	float tab_width;
	bool display_metrics;
	bool display_control;
	bool reverse_line_order;
	bool is_vertical;
	bool trim_edges;

	bool dispaly_spacing_chars;
	Ref<Texture> tab_icon;
	Ref<Texture> space_icon;
	Ref<Texture> break_icon;

	AMap text_attribs;
	AMap style_attribs; //and user defined

	float line_spacing;

	bool _compare_attributes(const HashMap<TextAttributes, Variant> &p_first, const HashMap<TextAttributes, Variant> &p_second) const;
	void _ensure_break(AMap &p_map, int64_t p_key);
	void _optimize_attributes(AMap &p_map);

	//out data
	mutable Size2 real_size;
	mutable TextDirection para_direction;
	mutable Vector<Run> visual;
	mutable Vector<Line> lines;

	//internal status
	mutable bool text_shaped; //text, font, ot features and language
	mutable bool lines_shaped; //breaks and width (if breaking on width)
	mutable bool layout_shaped; //alignment, tabs (may cause lines invalidation)

	void _draw_rect_border(RID p_canvas_item, const Rect2 &p_rect, const Color &p_color = Color(1, 1, 1), float p_width = 1.f, bool p_filled = false, bool p_antialiased = true) const;
	void _reshape() const;
	Point2 _translate_coordinates(const Point2 &p_point) const;

protected:
	static void _bind_methods();

public:
	Ref<TextLayout> copy() const;

	//base properties and deferred init
	void set_text(const String &p_text);
	String get_text() const;

	void set_display_metrics(bool p_enable);
	bool get_display_metrics() const;

	void set_display_control_chars(bool p_enable);
	bool get_display_control_chars() const;

	void set_dispaly_spacing_chars(bool p_enable);
	bool get_dispaly_spacing_chars() const;

	void set_tab_icon(const Ref<Texture> &p_icon);
	Ref<Texture> get_tab_icon() const;

	void set_space_icon(const Ref<Texture> &p_icon);
	Ref<Texture> get_space_icon() const;

	void set_break_icon(const Ref<Texture> &p_icon);
	Ref<Texture> get_break_icon() const;

	void set_base_direction(TextDirection p_dir);
	TextDirection get_base_direction() const;

	void set_break_mode(TextBreakFlags p_brk);
	TextBreakFlags get_break_mode() const;

	void set_h_align(HAlign p_align);
	HAlign get_h_align() const;

	void set_v_align(VAlign p_align);
	VAlign get_v_align() const;

	void set_justification_flags(TextJustificationFlags p_flags);
	TextJustificationFlags get_justification_flags() const;

	void set_justification_trim_edges(bool p_enable);
	bool get_justification_trim_edges() const;

	void set_target_size(const Size2 &p_size);
	Size2 get_target_size() const;

	void set_is_vertical(bool p_is_vertical);
	bool get_is_vertical() const;

	void set_reverse_line_order(bool p_order);
	bool get_reverse_line_order() const;

	void set_tab_width(float p_width);
	float get_tab_width() const;

	void set_tab_flags(TextTabluationFlags p_flags);
	TextTabluationFlags get_tab_flags() const;

	void set_line_spacing(float p_line_spacing);
	float get_line_spacing() const;

	void add_attribute(TextAttributes p_attribute, const Variant &p_value, int p_start, int p_end);
	void remove_attribute(TextAttributes p_attribute, int p_start, int p_end);
	void remove_all_attributes(int p_start, int p_end);
	void clear_attributes();

	bool has_attribute(int p_pos, TextAttributes p_attribute) const;
	Variant get_attribute(int p_pos, TextAttributes p_attribute) const;

	//output
	uint32_t hash() const;

	TextDirection get_paragraph_direction() const;
	TextDirection get_dominant_direciton_in_range(int p_start, int p_end) const;

	const Vector<Run> &get_runs() const;

	Size2 get_size() const;

	int get_line_count() const;
	Rect2 get_line_rect(int p_index) const;
	int get_line_start(int p_index) const;
	int get_line_end(int p_index) const;
	int get_line_column_count(int p_index) const;
	float get_line_ascent(int p_index) const;
	float get_line_descent(int p_index) const;
	const Vector<Run> &get_line_runs(int p_index, int p_col) const;

	Rect2 get_replacement_rect(int p_pos) const;

	int hit_test(const Point2 &p_pos) const;
	Rect2 get_ltr_caret(int p_pos) const;
	Rect2 get_rtl_caret(int p_pos) const;
	Vector<Rect2> get_selection_rects(int p_start, int p_end) const;

	Array _get_selection_rects(int p_start, int p_end) const;
	Dictionary _get_shaped_data() const;

	void shape_now();
	void draw(RID p_canvas_item, const Point2 &p_pos, bool p_clip = false) const;

	//self list api
	SelfList<TextLayout> layout_list;

	static Mutex *layout_mutex;
	static SelfList<TextLayout>::List *layouts;

	static void invalidate_all();

	static void initialize_self_list();
	static void finish_self_list();

	TextLayout();
	~TextLayout();
};

#endif /*TEXT_LAYOUT_H*/
