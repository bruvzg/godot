/*************************************************************************/
/*  shaped_text.h                                                        */
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

#ifndef SHAPED_TEXT_H
#define SHAPED_TEXT_H

#include "font.h"

class ShapedText : public Reference {
	GDCLASS(ShapedText, Reference);

	RID ctx;

protected:
	static void _bind_methods();

public:
	RID get_rid() const;

	void set_direction(TextServer::TextDirection p_direction);
	TextServer::TextDirection get_direction() const;

	void set_orientation(TextServer::TextOrientation p_orientation);
	TextServer::TextOrientation get_orientation() const;

	bool add_text(const String &p_text, const Ref<Font> &p_font, float p_size, const String &p_features = "", const String &p_locale = "");
	bool add_object(Variant p_id, const Size2 &p_size, VAlign p_inline_align);

	Ref<ShapedText> substr(int p_start, int p_length) const;
	Array get_graphemes() const;

	Array break_lines(float p_width, /*TextBreak*/ uint8_t p_break_mode) const;

	Rect2 get_object_rect(Variant p_id) const;
	Size2 get_size() const;
	float get_ascent() const;
	float get_descent() const;
	float get_line_spacing() const;

	float fit_to_width(float p_width, /*TextJustification*/ uint8_t p_justification_mode) const;

	void draw(RID p_canvas_item, const Point2 &p_pos, const Color &p_modulate = Color(1, 1, 1), const Color &p_outline_modulate = Color(1, 1, 1)) const;

	Array get_carets(int p_pos) const;
	Array get_selection(int p_start, int p_end) const;
	int hit_test(const Vector2 &p_coords) const;

	ShapedText();
	~ShapedText();
};

#endif /* SHAPED_TEXT_H */
