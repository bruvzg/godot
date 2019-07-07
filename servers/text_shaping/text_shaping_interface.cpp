/*************************************************************************/
/*  text_shaping_interface.cpp                                           */
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

#include "text_shaping_interface.h"
#include "servers/text_shaping_server.h"

#include "scene/resources/font.h"
#include "scene/resources/spannable.h"

#include "core/method_bind_ext.gen.inc"

TextShapingInterface::TextShapingInterface(){
	//NOP
};

TextShapingInterface::~TextShapingInterface(){
	//NOP
};

void TextShapingInterface::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_name"), &TextShapingInterface::get_name);
	ClassDB::bind_method(D_METHOD("get_capabilities"), &TextShapingInterface::get_capabilities);

	ClassDB::bind_method(D_METHOD("is_initialized"), &TextShapingInterface::is_initialized);
	ClassDB::bind_method(D_METHOD("set_is_initialized", "initialized"), &TextShapingInterface::set_is_initialized);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "interface_is_initialized"), "set_is_initialized", "is_initialized");

	ClassDB::bind_method(D_METHOD("initialize"), &TextShapingInterface::initialize);
	ClassDB::bind_method(D_METHOD("uninitialize"), &TextShapingInterface::uninitialize);

	ClassDB::bind_method(D_METHOD("set_is_primary", "enable"), &TextShapingInterface::set_is_primary);
	ClassDB::bind_method(D_METHOD("get_is_primary"), &TextShapingInterface::get_is_primary);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_primary"), "set_is_primary", "get_is_primary");

	ClassDB::bind_method(D_METHOD("draw_string", "canvas_item", "pos", "font", "string", "direction", "modulate", "outline_modulate", "clip"), &TextShapingInterface::draw_string, DEFVAL(TEXT_DIRECTION_AUTO), DEFVAL(Color(1, 1, 1)), DEFVAL(Color(1, 1, 1)), DEFVAL(Vector2()));
	ClassDB::bind_method(D_METHOD("draw_string_justified", "canvas_item", "pos", "font", "string", "direction", "modulate", "outline_modulate", "line_w"), &TextShapingInterface::draw_string_justified, DEFVAL(TEXT_DIRECTION_AUTO), DEFVAL(Color(1, 1, 1)), DEFVAL(Color(1, 1, 1)), DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("get_string_size", "font", "string", "direction"), &TextShapingInterface::get_string_size, DEFVAL(TEXT_DIRECTION_AUTO));
	ClassDB::bind_method(D_METHOD("get_string_cursor_shapes", "font", "string", "position", "direction"), &TextShapingInterface::get_string_cursor_shapes, DEFVAL(TEXT_DIRECTION_AUTO));
	ClassDB::bind_method(D_METHOD("get_string_selection_shapes", "font", "string", "start", "end", "direction"), &TextShapingInterface::get_string_selection_shapes, DEFVAL(TEXT_DIRECTION_AUTO));
	ClassDB::bind_method(D_METHOD("get_string_line_breaks", "font", "string", "direction", "line_w"), &TextShapingInterface::get_string_line_breaks, DEFVAL(TEXT_DIRECTION_AUTO), DEFVAL(-1.0f));
	ClassDB::bind_method(D_METHOD("string_hit_test", "font", "string", "point", "direction"), &TextShapingInterface::string_hit_test, DEFVAL(TEXT_DIRECTION_AUTO));

	ClassDB::bind_method(D_METHOD("draw_spannable", "canvas_item", "pos", "spannable", "direction", "clip"), &TextShapingInterface::draw_spannable, DEFVAL(TEXT_DIRECTION_AUTO), DEFVAL(Vector2()));
	ClassDB::bind_method(D_METHOD("draw_spannable_justified", "canvas_item", "pos", "spannable", "direction", "line_w"), &TextShapingInterface::draw_spannable_justified, DEFVAL(TEXT_DIRECTION_AUTO), DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("get_spannable_size", "spannable", "direction"), &TextShapingInterface::get_spannable_size, DEFVAL(TEXT_DIRECTION_AUTO));
	ClassDB::bind_method(D_METHOD("get_spannable_cursor_shapes", "spannable", "position", "direction"), &TextShapingInterface::get_spannable_cursor_shapes, DEFVAL(TEXT_DIRECTION_AUTO));
	ClassDB::bind_method(D_METHOD("get_spannable_selection_shapes", "spannable", "start", "end", "direction"), &TextShapingInterface::get_spannable_selection_shapes, DEFVAL(TEXT_DIRECTION_AUTO));
	ClassDB::bind_method(D_METHOD("get_spannable_line_breaks", "spannable", "direction", "line_w"), &TextShapingInterface::get_spannable_line_breaks, DEFVAL(TEXT_DIRECTION_AUTO), DEFVAL(-1.0f));
	ClassDB::bind_method(D_METHOD("spannable_hit_test", "spannable", "point", "direction"), &TextShapingInterface::spannable_hit_test, DEFVAL(TEXT_DIRECTION_AUTO));

	BIND_ENUM_CONSTANT(TEXT_DIRECTION_AUTO);
	BIND_ENUM_CONSTANT(TEXT_DIRECTION_LTR_TB);
	BIND_ENUM_CONSTANT(TEXT_DIRECTION_RTL_TB);
	BIND_ENUM_CONSTANT(TEXT_DIRECTION_TTB_LR);
	BIND_ENUM_CONSTANT(TEXT_DIRECTION_TTB_RL);

	BIND_ENUM_CONSTANT(TEXT_SHAPING_SIMPLE_LAYOUT);
	BIND_ENUM_CONSTANT(TEXT_SHAPING_COMPLEX_LAYOUT);
	BIND_ENUM_CONSTANT(TEXT_SHAPING_VERTICAL_LAYOUT);
}

StringName TextShapingInterface::get_name() const {

	return "Fallback Shaper";
}

int32_t TextShapingInterface::get_capabilities() const {

	return TEXT_SHAPING_SIMPLE_LAYOUT;
}

void TextShapingInterface::draw_string(RID p_canvas_item, const Point2 &p_pos, const Ref<Font> &p_font, const String &p_string, int32_t p_direction, const Color &p_modulate, const Color &p_outline_modulate, const Vector2 &p_clip) const {

	ERR_FAIL_COND(p_font.is_null());

	Vector2 ofs;

	int64_t chars_start = 0;
	int64_t chars_end = 0;
	float width_start = 0.0f;

	bool with_outline = p_font->has_outline();
	for (int64_t i = 0; i < p_string.length(); i++) {

		float width = p_font->get_char_size(p_string[i]).width;

		if (p_clip.x >= 0.0f && (ofs.x + width) < p_clip.x) {
			chars_start = i;
			width_start = width;
			ofs.x += width;
			continue; //clip from start;
		}

		if (p_clip.y >= 0.0f && (ofs.x + width) > p_clip.y)
			break; //clip from end;

		ofs.x += p_font->draw_char(p_canvas_item, p_pos + ofs, p_string[i], p_string[i + 1], with_outline ? p_outline_modulate : p_modulate, with_outline);
		chars_end = i;
	}

	if (p_font->has_outline()) {
		ofs = Vector2(width_start, 0);
		for (int64_t i = chars_start; i <= chars_end; i++) {
			ofs.x += p_font->draw_char(p_canvas_item, p_pos + ofs, p_string[i], p_string[i + 1], p_modulate, false);
		}
	}
}

void TextShapingInterface::draw_string_justified(RID p_canvas_item, const Point2 &p_pos, const Ref<Font> &p_font, const String &p_string, int32_t p_direction, const Color &p_modulate, const Color &p_outline_modulate, float p_line_w) const {

	ERR_FAIL_COND(p_font.is_null());

	if (p_line_w <= 0) {
		draw_string(p_canvas_item, p_pos, p_font, p_string, p_direction, p_modulate, p_outline_modulate);
		return;
	}

	float str_width = get_string_size(p_font, p_string, p_direction).width;
	if (str_width >= p_line_w) {
		draw_string(p_canvas_item, p_pos, p_font, p_string, p_direction, p_modulate, p_outline_modulate);
		return;
	}

	Vector<String> words = p_string.split_spaces();
	float delta_width = (p_line_w - str_width) / (words.size() - 1);

	Vector2 ofs;

	bool inside = false;
	int64_t from = 0;

	bool with_outline = p_font->has_outline();
	for (int64_t i = 0; i < p_string.length(); i++) {

		bool empty = p_string[i] < 33;
		if (i == 0)
			inside = !empty;

		if (!empty && !inside) {
			inside = true;
			from = i;
		}

		float width = p_font->get_char_size(p_string[i]).width;

		if (empty && inside) {
			inside = false;
			width += delta_width;
		}

		p_font->draw_char(p_canvas_item, p_pos + ofs, p_string[i], p_string[i + 1], with_outline ? p_outline_modulate : p_modulate, with_outline);
		ofs.x += width;
	}

	if (p_font->has_outline()) {
		inside = false;
		from = 0;
		ofs = Vector2(0, 0);
		for (int64_t i = 0; i <= p_string.length(); i++) {

			bool empty = p_string[i] < 33;
			if (i == 0)
				inside = !empty;

			if (!empty && !inside) {
				inside = true;
				from = i;
			}

			float width = p_font->get_char_size(p_string[i]).width;

			if (empty && inside) {
				inside = false;
				width += delta_width;
			}

			p_font->draw_char(p_canvas_item, p_pos + ofs, p_string[i], p_string[i + 1], p_modulate, false);
			ofs.x += width;
		}
	}
}

Size2 TextShapingInterface::get_string_size(const Ref<Font> &p_font, const String &p_string, int32_t p_direction) const {

	ERR_FAIL_COND_V(p_font.is_null(), Size2());

	float w = 0.0f;

	int64_t l = p_string.length();
	if (l == 0)
		return Size2(0, p_font->get_height());
	const CharType *sptr = &p_string[0];

	for (int64_t i = 0; i < l; i++) {

		w += p_font->get_char_size(sptr[i], sptr[i + 1]).width;
	}

	return Size2(w, p_font->get_height());
}

Array TextShapingInterface::get_string_cursor_shapes(const Ref<Font> &p_font, const String &p_string, int64_t p_position, int32_t p_direction) const {

	ERR_FAIL_COND_V(p_font.is_null(), Array());

	Array ret;
	float w = 0.0f;

	int64_t l = p_string.length();
	if (l == 0) {
		//cursor before first char
		ret.push_back(Rect2(0.0f, 0.0f, 1.0f, p_font->get_height())); //LTR
		ret.push_back(Rect2()); //RTL - not supported
		return ret;
	}
	const CharType *sptr = &p_string[0];

	for (int64_t i = 0; i < l; i++) {

		if (i == p_position) {
			ret.push_back(Rect2(w, 0.0f, 1.0f, p_font->get_height())); //LTR
			ret.push_back(Rect2()); //RTL - not supported
			return ret;
		}

		w += p_font->get_char_size(sptr[i], sptr[i + 1]).width;
	}

	//cursor after last char
	ret.push_back(Rect2(w, 0.0f, 1.0f, p_font->get_height())); //LTR
	ret.push_back(Rect2()); //RTL - not supported
	return ret;
}

Array TextShapingInterface::get_string_selection_shapes(const Ref<Font> &p_font, const String &p_string, int64_t p_start, int64_t p_end, int32_t p_direction) const {

	ERR_FAIL_COND_V(p_font.is_null(), Array());

	Array ret;
	float w_start = 0.0f;
	float w_end = 0.0f;
	float w = 0.0f;

	int64_t l = p_string.length();
	if (l == 0) {
		ret.push_back(Rect2(0.0f, 0.0f, 2.0f, p_font->get_height()));
		return ret;
	}

	const CharType *sptr = &p_string[0];

	for (int64_t i = 0; i < l; i++) {

		if (i == p_start) {
			w_start = w;
		} else if (i == p_end) {
			w_end = w;
			ret.push_back(Rect2(w_start, 0.0f, w_end - w_start, p_font->get_height()));
			return ret;
		}

		w += p_font->get_char_size(sptr[i], sptr[i + 1]).width;
	}
	if (p_end >= l) {
		//after last char
		w_end = w;
	}

	ret.push_back(Rect2(w_start, 0.0f, w_end - w_start, p_font->get_height()));
	return ret;
}

Array TextShapingInterface::get_string_line_breaks(const Ref<Font> &p_font, const String &p_string, int32_t p_direction, float p_line_w) const {

	ERR_FAIL_COND_V(p_font.is_null(), Array());

	Array ret;

	int64_t l = p_string.length();
	if (l == 0)
		return ret;

	int64_t ofs = 0;
	float line_w = 0.0f;

	Vector<String> lines = p_string.split("\n");

	for (int64_t i = 0; i < lines.size(); i++) {
		String t = lines[i];

		if (p_line_w > 0) {
			int64_t wofs = 0;
			line_w = 0.0f;
			Vector<String> words = t.split_spaces();
			for (int j = 0; j < words.size(); j++) {
				line_w += get_string_size(p_font, words[j], p_direction).x;
				if (line_w > p_line_w) {
					ret.push_back(ofs + wofs);
					line_w = get_string_size(p_font, words[j], p_direction).x;
				}
				wofs += words[j].length();
			}
		}
		ofs += t.length();

		ret.push_back(ofs);
	}

	return ret;
}

int64_t TextShapingInterface::string_hit_test(const Ref<Font> &p_font, const String &p_string, const Point2 &p_point, int32_t p_direction) const {

	ERR_FAIL_COND_V(p_font.is_null(), -1);

	if ((p_point.y < 0) || (p_point.y > p_font->get_height())) {
		return -1;
	}

	float w = 0.0f;

	int64_t l = p_string.length();
	if (l == 0) {
		return 0;
	}

	const CharType *sptr = &p_string[0];

	for (int64_t i = 0; i < l; i++) {

		float char_w = p_font->get_char_size(sptr[i]).width;
		w += char_w;

		if (w > p_point.x) { //found what we look for
			return i;
		}
	}

	return l; //after last char
}

void TextShapingInterface::draw_spannable(RID p_canvas_item, const Point2 &p_pos, const Ref<Spannable> &p_spannable, int32_t p_direction, const Vector2 &p_clip) const {

	ERR_FAIL_COND(p_spannable.is_null());

	Vector2 ofs;

	int64_t chars_start = 0;
	int64_t chars_end = 0;
	float width_start = 0.0f;

	const String &s = p_spannable->get_string();

	bool with_outline = false;
	for (int64_t i = 0; i < s.length(); i++) {

		Ref<Font> font = p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_FONT, i);
		Color modulate = (font->has_outline()) ? p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_OUTLINE_COLOR, i) : p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_COLOR, i);

		ERR_FAIL_COND(font.is_null());

		with_outline |= font->has_outline();

		float width = font->get_char_size(s[i]).width;

		if (p_clip.x >= 0.0f && (ofs.x + width) < p_clip.x) {
			chars_start = i;
			width_start = width;
			ofs.x += width;
			continue; //clip from start;
		}

		if (p_clip.y >= 0.0f && (ofs.x + width) > p_clip.y)
			break; //clip from end;

		ofs.x += font->draw_char(p_canvas_item, p_pos + ofs, s[i], s[i + 1], modulate, font->has_outline());
		chars_end = i;
	}

	if (with_outline) {
		ofs = Vector2(width_start, 0);
		for (int64_t i = chars_start; i <= chars_end; i++) {
			Ref<Font> font = p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_FONT, i);
			Color modulate = p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_COLOR, i);

			ofs.x += font->draw_char(p_canvas_item, p_pos + ofs, s[i], s[i + 1], modulate, false);
		}
	}
}

void TextShapingInterface::draw_spannable_justified(RID p_canvas_item, const Point2 &p_pos, const Ref<Spannable> &p_spannable, int32_t p_direction, float p_line_w) const {

	ERR_FAIL_COND(p_spannable.is_null());

	if (p_line_w <= 0) {
		draw_spannable(p_canvas_item, p_pos, p_spannable, p_direction);
		return;
	}

	float str_width = get_spannable_size(p_spannable, p_direction).width;
	if (str_width >= p_line_w) {
		draw_spannable(p_canvas_item, p_pos, p_spannable, p_direction);
		return;
	}

	const String &s = p_spannable->get_string();

	Vector<String> words = s.split_spaces();
	float delta_width = (p_line_w - str_width) / (words.size() - 1);

	Vector2 ofs;

	bool inside = false;
	int64_t from = 0;

	bool with_outline = false;
	for (int64_t i = 0; i < s.length(); i++) {

		Ref<Font> font = p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_FONT, i);
		Color modulate = (font->has_outline()) ? p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_OUTLINE_COLOR, i) : p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_COLOR, i);

		ERR_FAIL_COND(font.is_null());

		with_outline |= font->has_outline();

		bool empty = s[i] < 33;
		if (i == 0)
			inside = !empty;

		if (!empty && !inside) {
			inside = true;
			from = i;
		}

		float width = font->get_char_size(s[i]).width;

		if (empty && inside) {
			inside = false;
			width += delta_width;
		}

		font->draw_char(p_canvas_item, p_pos + ofs, s[i], s[i + 1], modulate, font->has_outline());
		ofs.x += width;
	}

	if (with_outline) {
		inside = false;
		from = 0;
		ofs = Vector2(0, 0);
		for (int64_t i = 0; i <= s.length(); i++) {

			Ref<Font> font = p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_FONT, i);
			Color modulate = p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_COLOR, i);

			ERR_FAIL_COND(font.is_null());

			bool empty = s[i] < 33;
			if (i == 0)
				inside = !empty;

			if (!empty && !inside) {
				inside = true;
				from = i;
			}

			float width = font->get_char_size(s[i]).width;

			if (empty && inside) {
				inside = false;
				width += delta_width;
			}

			font->draw_char(p_canvas_item, p_pos + ofs, s[i], s[i + 1], modulate, false);
			ofs.x += width;
		}
	}
}

Size2 TextShapingInterface::get_spannable_size(const Ref<Spannable> &p_spannable, int32_t p_direction) const {

	ERR_FAIL_COND_V(p_spannable.is_null(), Size2());

	float w = 0.0f;
	float h = 0.0f;
	const String &s = p_spannable->get_string();

	int64_t l = s.length();
	if (l == 0)
		return Size2(0, 0);
	const CharType *sptr = &s[0];

	for (int64_t i = 0; i < l; i++) {
		Ref<Font> font = p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_FONT, i);

		ERR_FAIL_COND_V(font.is_null(), Size2());

		w += font->get_char_size(sptr[i], sptr[i + 1]).width;
		h = MAX(h, font->get_height());
	}

	return Size2(w, h);
}

Array TextShapingInterface::get_spannable_cursor_shapes(const Ref<Spannable> &p_spannable, int64_t p_position, int32_t p_direction) const {

	ERR_FAIL_COND_V(p_spannable.is_null(), Array());

	Array ret;
	float w = 0.0f;
	const String &s = p_spannable->get_string();

	int64_t l = s.length();
	if (l == 0) {
		//cursor before first char
		Ref<Font> font = p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_FONT, 0);
		ERR_FAIL_COND_V(font.is_null(), ret);

		ret.push_back(Rect2(0.0f, 0.0f, 1.0f, font->get_height())); //LTR
		ret.push_back(Rect2()); //RTL - not supported
		return ret;
	}
	const CharType *sptr = &s[0];

	for (int64_t i = 0; i < l; i++) {

		Ref<Font> font = p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_FONT, i);
		ERR_FAIL_COND_V(font.is_null(), ret);

		if (i == p_position) {
			ret.push_back(Rect2(w, 0.0f, 1.0f, font->get_height())); //LTR
			ret.push_back(Rect2()); //RTL - not supported
			return ret;
		}

		w += font->get_char_size(sptr[i], sptr[i + 1]).width;
	}

	//cursor after last char
	Ref<Font> font = p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_FONT, l - 1);
	ERR_FAIL_COND_V(font.is_null(), ret);

	ret.push_back(Rect2(w, 0.0f, 1.0f, font->get_height())); //LTR
	ret.push_back(Rect2()); //RTL - not supported
	return ret;
}

Array TextShapingInterface::get_spannable_selection_shapes(const Ref<Spannable> &p_spannable, int64_t p_start, int64_t p_end, int32_t p_direction) const {

	ERR_FAIL_COND_V(p_spannable.is_null(), Array());

	Array ret;
	float w_start = 0.0f;
	float w_end = 0.0f;
	float w = 0.0f;
	const String &s = p_spannable->get_string();

	int64_t l = s.length();
	if (l == 0) {
		Ref<Font> font = p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_FONT, 0);
		ERR_FAIL_COND_V(font.is_null(), ret);

		ret.push_back(Rect2(0.0f, 0.0f, 2.0f, font->get_height()));
		return ret;
	}

	const CharType *sptr = &s[0];

	for (int64_t i = 0; i < l; i++) {

		Ref<Font> font = p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_FONT, i);
		ERR_FAIL_COND_V(font.is_null(), ret);

		if (i == p_start) {
			w_start = w;
		} else if (i == p_end) {
			w_end = w;
			ret.push_back(Rect2(w_start, 0.0f, w_end - w_start, font->get_height()));
			return ret;
		}

		w += font->get_char_size(sptr[i], sptr[i + 1]).width;
	}
	if (p_end >= l) {
		//after last char
		w_end = w;
	}

	Ref<Font> font = p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_FONT, l - 1);
	ERR_FAIL_COND_V(font.is_null(), ret);

	ret.push_back(Rect2(w_start, 0.0f, w_end - w_start, font->get_height()));
	return ret;
}

Array TextShapingInterface::get_spannable_line_breaks(const Ref<Spannable> &p_spannable, int32_t p_direction, float p_line_w) const {

	ERR_FAIL_COND_V(p_spannable.is_null(), Array());

	Array ret;

	const String &s = p_spannable->get_string();

	int64_t l = s.length();
	if (l == 0)
		return ret;

	int64_t ofs = 0;
	float line_w = 0.0f;

	Vector<String> lines = s.split("\n");

	for (int64_t i = 0; i < lines.size(); i++) {
		String t = lines[i];

		if (p_line_w > 0) {
			int64_t wofs = 0;
			line_w = 0.0f;
			Vector<String> words = t.split_spaces();
			for (int j = 0; j < words.size(); j++) {
				for (int k = 0; k < words[j].length(); k++) {
					Ref<Font> font = p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_FONT, ofs + wofs + k);
					ERR_FAIL_COND_V(font.is_null(), ret);

					line_w += font->get_char_size(words[j][k], words[j][k + 1]).width;
					if (line_w > p_line_w) {
						ret.push_back(ofs + wofs);
						line_w = 0.0f;
						for (int l = 0; l < words[j].length(); l++) {
							line_w += font->get_char_size(words[j][l], words[j][l + 1]).width;
						}
						break;
					}
				}
				wofs += words[j].length();
			}
		}
		ofs += t.length();

		ret.push_back(ofs);
	}

	return ret;
}

int64_t TextShapingInterface::spannable_hit_test(const Ref<Spannable> &p_spannable, const Point2 &p_point, int32_t p_direction) const {

	ERR_FAIL_COND_V(p_spannable.is_null(), -1);

	const String &s = p_spannable->get_string();

	float w = 0.0f;

	int64_t l = s.length();
	if (l == 0) {
		return 0;
	}

	const CharType *sptr = &s[0];

	for (int64_t i = 0; i < l; i++) {

		Ref<Font> font = p_spannable->get_attribute(Spannable::TEXT_ATTRIBUTE_FONT, i);
		ERR_FAIL_COND_V(font.is_null(), -1);

		float char_w = font->get_char_size(sptr[i]).width;
		w += char_w;

		if ((p_point.y >= 0) && (p_point.y <= font->get_height())) {
			if (w > p_point.x) { //found what we look for
				return i;
			}
		}
	}

	return l; //after last char
}

bool TextShapingInterface::is_initialized() {
	return true;
}

void TextShapingInterface::set_is_initialized(bool p_initialized) {
	if (p_initialized) {
		if (!is_initialized()) {
			initialize();
		};
	} else {
		if (is_initialized()) {
			uninitialize();
		};
	};
}

bool TextShapingInterface::initialize() {
	return true;
}

bool TextShapingInterface::uninitialize() {
	return true;
}

bool TextShapingInterface::get_is_primary() const {

	TextShapingServer *text_server = TextShapingServer::get_singleton();
	ERR_FAIL_NULL_V(text_server, false);

	return text_server->get_primary_interface() == this;
}

void TextShapingInterface::set_is_primary(bool p_is_primary) {

	TextShapingServer *text_server = TextShapingServer::get_singleton();
	ERR_FAIL_NULL(text_server);

	if (p_is_primary) {
		text_server->set_primary_interface(this);
	} else {
		text_server->clear_primary_interface_if(this);
	};
}
