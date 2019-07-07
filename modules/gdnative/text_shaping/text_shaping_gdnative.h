/*************************************************************************/
/*  text_shaping_gdnative.h                                              */
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

#ifndef TEXT_SHAPING_GDNATIVE_H
#define TEXT_SHAPING_GDNATIVE_H

#include "modules/gdnative/gdnative.h"
#include "servers/text_shaping/text_shaping_interface.h"

class TextShapingInterfaceGDNative : public TextShapingInterface {
	GDCLASS(TextShapingInterfaceGDNative, TextShapingInterface);

	const godot_text_shaper_interface_gdnative *interface;
	void *data;

protected:
	static void _bind_methods();

public:
	void set_interface(const godot_text_shaper_interface_gdnative *p_interface);

	bool set_property(const String &p_name, const Variant &p_value);

	virtual StringName get_name() const;
	virtual int32_t get_capabilities() const;

	virtual bool is_initialized();

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

	TextShapingInterfaceGDNative();
	~TextShapingInterfaceGDNative();
};

#endif
