/*************************************************************************/
/*  shaped_text.cpp                                                      */
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

#include "shaped_text.h"

void ShapedText::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_rid"), &ShapedText::get_rid);

	ClassDB::bind_method(D_METHOD("set_direction", "direction"), &ShapedText::set_direction);
	ClassDB::bind_method(D_METHOD("get_direction"), &ShapedText::get_direction);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "direction", PROPERTY_HINT_ENUM, "Auto,Left-to-right,Right-to-left"), "set_direction", "get_direction");

	ClassDB::bind_method(D_METHOD("set_orientation", "orientation"), &ShapedText::set_orientation);
	ClassDB::bind_method(D_METHOD("get_orientation"), &ShapedText::get_orientation);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "orientation", PROPERTY_HINT_ENUM, "Horizontal top-to-bottom,Vertical right-to-left,Vertical left-to-right,Sideways right-to-left,Sideways left-to-right"), "set_orientation", "get_orientation");

	ClassDB::bind_method(D_METHOD("add_text", "text", "font", "size", "features", "locale"), &ShapedText::add_text, DEFVAL(""), DEFVAL(""));
	ClassDB::bind_method(D_METHOD("add_object", "id", "size", "inline_align"), &ShapedText::add_object);

	ClassDB::bind_method(D_METHOD("substr", "start", "length"), &ShapedText::substr);
	ClassDB::bind_method(D_METHOD("get_graphemes"), &ShapedText::get_graphemes);

	ClassDB::bind_method(D_METHOD("break_lines", "width", "break_mode"), &ShapedText::break_lines);

	ClassDB::bind_method(D_METHOD("get_object_rect", "id"), &ShapedText::get_object_rect);
	ClassDB::bind_method(D_METHOD("get_size"), &ShapedText::get_size);
	ClassDB::bind_method(D_METHOD("get_ascent"), &ShapedText::get_ascent);
	ClassDB::bind_method(D_METHOD("get_descent"), &ShapedText::get_descent);
	ClassDB::bind_method(D_METHOD("get_line_spacing"), &ShapedText::get_line_spacing);

	ClassDB::bind_method(D_METHOD("fit_to_width", "width", "justification_mode"), &ShapedText::fit_to_width);

	ClassDB::bind_method(D_METHOD("draw", "canvas_item", "pos", "modulate", "outline_modulate"), &ShapedText::draw, DEFVAL(Color(1, 1, 1)), DEFVAL(Color(1, 1, 1)));

	ClassDB::bind_method(D_METHOD("get_carets", "pos"), &ShapedText::get_carets);
	ClassDB::bind_method(D_METHOD("get_selection", "start", "end"), &ShapedText::get_selection);
	ClassDB::bind_method(D_METHOD("hit_test", "coords"), &ShapedText::hit_test);
}

RID ShapedText::get_rid() const {
	return ctx;
}

void ShapedText::set_direction(TextServer::TextDirection p_direction) {
	ERR_FAIL_COND(TX != nullptr);
	ERR_FAIL_COND(!ctx.is_valid());
	TX->shaped_set_direction(ctx, p_direction);
}

TextServer::TextDirection ShapedText::get_direction() const {
	ERR_FAIL_COND_V(TX != nullptr, TEXT_DIRECTION_AUTO);
	ERR_FAIL_COND_V(!ctx.is_valid(), TEXT_DIRECTION_AUTO);
	return TX->shaped_get_direction(ctx);
}

void ShapedText::set_orientation(TextServer::TextOrientation p_orientation) {
	ERR_FAIL_COND(TX != nullptr);
	ERR_FAIL_COND(!ctx.is_valid());
	TX->shaped_set_orientation(ctx, p_direction);
}

TextServer::TextOrientation ShapedText::get_orientation() const {
	ERR_FAIL_COND_V(TX != nullptr, TEXT_ORIENTATION_HORIZONTAL_TB);
	ERR_FAIL_COND_V(!ctx.is_valid(), TEXT_ORIENTATION_HORIZONTAL_TB);
	return TX->shaped_get_orientation(ctx);
}

bool ShapedText::add_text(const String &p_text, const Ref<FontTS> &p_font, float p_size, const String &p_features, const String &p_locale) {
	ERR_FAIL_COND_V(TX != nullptr, false);
	ERR_FAIL_COND_V(!ctx.is_valid(), false);
	ERR_FAIL_COND_V(!p_font.is_valid(), false);
	return TX->shaped_add_text(ctx, p_text, p_font->get_data_list(), p_size, p_features, p_locale);
}

bool ShapedText::add_object(Variant p_id, const Size2 &p_size, VAlign p_inline_align) {
	ERR_FAIL_COND_V(TX != nullptr, false);
	ERR_FAIL_COND_V(!ctx.is_valid(), false);
	return TX->add_object(ctx, p_id, p_size, p_inline_align);
}

Ref<ShapedText> ShapedText::substr(int p_start, int p_length) const {
	ERR_FAIL_COND_V(TX != nullptr, Ref<ShapedText>());
	ERR_FAIL_COND_V(!ctx.is_valid(), Ref<ShapedText>());

	Ref<ShapedText> res;
	res.instance();
	res->ctx = TX->shaped_create_substr(ctx, p_start, p_length);
	return res;
}

Array ShapedText::get_graphemes() const {
	ERR_FAIL_COND_V(TX != nullptr, Array());
	ERR_FAIL_COND_V(!ctx.is_valid(), Array());

	Vector<TextServer::Grapheme> grap = TX->shaped_get_graphemes(ctx);
	Array ret;
	for (int i = 0; i < grap.size(); i++) {
		Dictionary grapheme;
		grapheme["range"] = grap[i].range;
		grapheme["advance"] = grap[i].advance;
		grapheme["flags"] = grap[i].flags;
		grapheme["font"] = grap[i].font;

		Array glyphs;
		for (int j = 0; j < grap[i].glyph.size(); j++) {
			Dictionary glyph;
			glyph["index"] = grap[i].glyph[j].glyph_index;
			glyph["offset"] = grap[i].glyph[j].offset;
			glyphs.push_back(glyph);
		}
		grapheme["glyphs"] = glyphs;
		ret.push_back(grapheme);
	}
	return ret;
}

Array ShapedText::break_lines(float p_width, /*TextBreak*/ uint8_t p_break_mode) const {
	ERR_FAIL_COND_V(TX != nullptr, Array());
	ERR_FAIL_COND_V(!ctx.is_valid(), Array());

	Array ret;
	Vector<Vector2i> lines = TX->shaped_get_line_breaks(ctx, p_width, p_break_mode);
	for (int i = 0; i < lines.size(); i++) {
		Ref<ShapedText> line;
		line.instance();
		line->ctx = TX->shaped_create_substr(*ctx, lines[i].x, lines[i].y);
		ret.push_back(line);
	}
	return ret;
}

Rect2 ShapedText::get_object_rect(Variant p_id) const {
	ERR_FAIL_COND_V(TX != nullptr, Rect2());
	ERR_FAIL_COND_V(!ctx.is_valid(), Rect2());
	return TX->shaped_get_object_rect(ctx, p_id);
}

Size2 ShapedText::get_size() const {
	ERR_FAIL_COND_V(TX != nullptr, Size2());
	ERR_FAIL_COND_V(!ctx.is_valid(), Size2());
	return TX->shaped_get_size(ctx);
}

float ShapedText::get_ascent() const {
	ERR_FAIL_COND_V(TX != nullptr, 0.f);
	ERR_FAIL_COND_V(!ctx.is_valid(), 0.f);
	return TX->shaped_get_ascent(ctx);
}

float ShapedText::get_descent() const {
	ERR_FAIL_COND_V(TX != nullptr, 0.f);
	ERR_FAIL_COND_V(!ctx.is_valid(), 0.f);
	return TX->shaped_get_descent(ctx);
}

float ShapedText::get_line_spacing() const {
	ERR_FAIL_COND_V(TX != nullptr, 0.f);
	ERR_FAIL_COND_V(!ctx.is_valid(), 0.f);
	return TX->shaped_line_spacing(ctx);
}

float ShapedText::fit_to_width(float p_width, /*TextJustification*/ uint8_t p_justification_mode) const {
	ERR_FAIL_COND_V(TX != nullptr, 0.f);
	ERR_FAIL_COND_V(!ctx.is_valid(), 0.f);
	return TX->shaped_fit_to_width(ctx, p_width, p_justification_mode);
}

void ShapedText::draw(RID p_canvas_item, const Point2 &p_pos, const Color &p_modulate, const Color &p_outline_modulate) const {
	ERR_FAIL_COND(TX != nullptr);
	ERR_FAIL_COND(!ctx.is_valid());
	Vector<TextServer::Grapheme> graph = TX->shaped_get_graphemes(ctx);
	Vector2 off = p_pos;
	if (p_outline_modulate.a != 0) {
		// Draw outline.
		for (int i = 0; i < graph.size(); i++) {
			if ((graph[i].flags & TEXT_GRAPHEME_FLAG_VALID) && TX->font_has_feature(TextServer::FEATURE_FONT_OUTLINE)) {
				for (int j = 0; j < graph[i].glyphs.size(); j++) {
					TX->font_draw_glyph_outline(graph[i].font, p_canvas_item, p_size, off + graph[i].glyphs[j].offset, graph[i].glyphs[j].glyph_index, p_outline_modulate);
				}
				off += graph[i].advance;
			}
		}
		off = p_pos;
	}
	// Draw text.
	for (int i = 0; i < graph.size(); i++) {
		if ((graph[i].flags & TEXT_GRAPHEME_FLAG_VALID)) {
			for (int j = 0; j < graph[i].glyphs.size(); j++) {
				TX->font_draw_glyph(graph[i].font, p_canvas_item, p_size, off + graph[i].glyphs[j].offset, graph[i].glyphs[j].glyph_index, p_modulate);
			}
		} else {
			for (int j = 0; j < graph[i].glyphs.size(); j++) {
				TX->font_draw_invalid_glyph(graph[i].font, p_canvas_item, p_size, off + graph[i].glyphs[j].offset, graph[i].glyphs[j].glyph_index, p_modulate);
			}
		}
		off += graph[i].advance;
	}
}

Array ShapedText::get_carets(int p_pos) const {
	ERR_FAIL_COND_V(TX != nullptr, Array());
	ERR_FAIL_COND_V(!ctx.is_valid(), Array());
	Vector<TextServer::Caret> carets = TX->shaped_get_carets(ctx, p_pos);
	Array ret;
	for (int i = 0; i < carets.size(); i++) {
		Dictionary caret;
		caret["rect"] = carets[i].rect;
		caret["is_primary"] = carets[i].is_primary;
		ret.push_back(caret);
	}
	return ret;
}

Array ShapedText::get_selection(int p_start, int p_end) const {
	ERR_FAIL_COND_V(TX != nullptr, Array());
	ERR_FAIL_COND_V(!ctx.is_valid(), Array());
	Vector<Rect2> rects = TX->shaped_get_selection(ctx, p_start, p_end);
	Array ret;
	for (int i = 0; i < rects.size(); i++) {
		ret.push_back(rects[i]);
	}
	return ret;
}

int ShapedText::hit_test(const Vector2 &p_coords) const {
	ERR_FAIL_COND_V(TX != nullptr, -1);
	ERR_FAIL_COND_V(!ctx.is_valid(), -1);

	return TX->shaped_hit_test(ctx, p_coords);
}

ShapedText::ShapedText() {
	if (TX != nullptr) {
		ctx = TX->create_shaped_text();
	}
}

ShapedText::~ShapedText() {
	if (TX != nullptr && ctx.is_valid()) {
		TX->free(ctx);
		ctx = RID();
	}
}