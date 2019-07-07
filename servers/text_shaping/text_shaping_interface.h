/*************************************************************************/
/*  text_shaping_interface.h                                             */
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

#ifndef TEXT_SHAPING_INTERFACE_H
#define TEXT_SHAPING_INTERFACE_H

#include "core/os/thread_safe.h"
#include "core/reference.h"

class Font;
class Spannable;

class TextShapingInterface : public Reference {
	GDCLASS(TextShapingInterface, Reference);
	_THREAD_SAFE_CLASS_

public:
	enum TextShapingDirection {
		TEXT_DIRECTION_AUTO = 0,
		TEXT_DIRECTION_LTR_TB = 1, //glyphs left to right, lines top to bottom
		TEXT_DIRECTION_RTL_TB = 2, //glyphs right to left, lines top to bottom
		TEXT_DIRECTION_TTB_LR = 3, //glyphs top to bottom, lines left to right
		TEXT_DIRECTION_TTB_RL = 3, //glyphs top to bottom, lines right to left
	};

	enum TextShapingCapabilities {
		TEXT_SHAPING_SIMPLE_LAYOUT = 1L << 0, //simple LTR text
		TEXT_SHAPING_COMPLEX_LAYOUT = 1L << 1, //BiDi and contextual shaping
		TEXT_SHAPING_VERTICAL_LAYOUT = 1L << 2 //verticat (TTB) layouts (clip and width is height)
	};

protected:
	static void _bind_methods();

public:
	virtual StringName get_name() const;
	virtual int32_t get_capabilities() const;

	void set_is_primary(bool p_is_primary);
	bool get_is_primary() const;

	virtual bool is_initialized();
	virtual void set_is_initialized(bool p_initialized);

	virtual bool initialize();
	virtual bool uninitialize();

	virtual void draw_string(RID p_canvas_item, const Point2 &p_pos, const Ref<Font> &p_font, const String &p_string, int32_t p_direction = TEXT_DIRECTION_AUTO, const Color &p_modulate = Color(1, 1, 1), const Color &p_outline_modulate = Color(1, 1, 1), const Vector2 &p_clip = Vector2()) const;
	virtual void draw_string_justified(RID p_canvas_item, const Point2 &p_pos, const Ref<Font> &p_font, const String &p_string, int32_t p_direction = TEXT_DIRECTION_AUTO, const Color &p_modulate = Color(1, 1, 1), const Color &p_outline_modulate = Color(1, 1, 1), float p_line_w = -1.0f) const;
	virtual Size2 get_string_size(const Ref<Font> &p_font, const String &p_string, int32_t p_direction = TEXT_DIRECTION_AUTO) const;
	virtual Array get_string_cursor_shapes(const Ref<Font> &p_font, const String &p_string, int64_t p_position, int32_t p_direction = TEXT_DIRECTION_AUTO) const;
	virtual Array get_string_selection_shapes(const Ref<Font> &p_font, const String &p_string, int64_t p_start, int64_t p_end, int32_t p_direction = TEXT_DIRECTION_AUTO) const;
	virtual Array get_string_line_breaks(const Ref<Font> &p_font, const String &p_string, int32_t p_direction = TEXT_DIRECTION_AUTO, float p_line_w = -1.0f) const;
	virtual int64_t string_hit_test(const Ref<Font> &p_font, const String &p_string, const Point2 &p_point, int32_t p_direction = TEXT_DIRECTION_AUTO) const;

	virtual void draw_spannable(RID p_canvas_item, const Point2 &p_pos, const Ref<Spannable> &p_spannable, int32_t p_direction = TEXT_DIRECTION_AUTO, const Vector2 &p_clip = Vector2()) const;
	virtual void draw_spannable_justified(RID p_canvas_item, const Point2 &p_pos, const Ref<Spannable> &p_spannable, int32_t p_direction = TEXT_DIRECTION_AUTO, float p_line_w = -1.0f) const;
	virtual Size2 get_spannable_size(const Ref<Spannable> &p_spannable, int32_t p_direction = TEXT_DIRECTION_AUTO) const;
	virtual Array get_spannable_cursor_shapes(const Ref<Spannable> &p_spannable, int64_t p_position, int32_t p_direction = TEXT_DIRECTION_AUTO) const;
	virtual Array get_spannable_selection_shapes(const Ref<Spannable> &p_spannable, int64_t p_start, int64_t p_end, int32_t p_direction = TEXT_DIRECTION_AUTO) const;
	virtual Array get_spannable_line_breaks(const Ref<Spannable> &p_spannable, int32_t p_direction = TEXT_DIRECTION_AUTO, float p_line_w = -1.0f) const;
	virtual int64_t spannable_hit_test(const Ref<Spannable> &p_spannable, const Point2 &p_point, int32_t p_direction = TEXT_DIRECTION_AUTO) const;

	TextShapingInterface();
	~TextShapingInterface();
};

VARIANT_ENUM_CAST(TextShapingInterface::TextShapingDirection);
VARIANT_ENUM_CAST(TextShapingInterface::TextShapingCapabilities);

#endif
