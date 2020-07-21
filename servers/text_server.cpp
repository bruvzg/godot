/*************************************************************************/
/*  text_server.cpp                                                      */
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

#include "text_server.h"

TextServer *TextServer::singleton = nullptr;

void TextServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("has_feature", "feature"), &TextServer::has_feature);
	ClassDB::bind_method(D_METHOD("get_name"), &TextServer::get_name);

	ClassDB::bind_method(D_METHOD("load_data", "filename"), &TextServer::load_data);
	ClassDB::bind_method(D_METHOD("is_data_loaded"), &TextServer::is_data_loaded);

	ClassDB::bind_method(D_METHOD("free", "rid"), &TextServer::free);

	ClassDB::bind_method(D_METHOD("create_font_system", "name"), &TextServer::create_font_system);
	ClassDB::bind_method(D_METHOD("create_font_resource", "filename"), &TextServer::create_font_resource);
	ClassDB::bind_method(D_METHOD("create_font_memory", "data"), &TextServer::create_font_memory);

	ClassDB::bind_method(D_METHOD("font_get_height", "font", "size"), &TextServer::font_get_height);
	ClassDB::bind_method(D_METHOD("font_get_ascent", "font", "size"), &TextServer::font_get_ascent);
	ClassDB::bind_method(D_METHOD("font_get_descent", "font", "size"), &TextServer::font_get_descent);
	ClassDB::bind_method(D_METHOD("font_get_underline_position", "font", "size"), &TextServer::font_get_underline_position);
	ClassDB::bind_method(D_METHOD("font_get_underline_thickness", "font", "size"), &TextServer::font_get_underline_thickness);

	ClassDB::bind_method(D_METHOD("font_has_feature", "font", "feature"), &TextServer::font_has_feature);
	ClassDB::bind_method(D_METHOD("font_get_language_supported", "font", "locale"), &TextServer::font_get_language_supported);
	ClassDB::bind_method(D_METHOD("font_get_script_supported", "font", "script"), &TextServer::font_get_script_supported);
	ClassDB::bind_method(D_METHOD("font_set_language_supported", "font", "locale", "supported"), &TextServer::font_set_language_supported);
	ClassDB::bind_method(D_METHOD("font_set_script_supported", "font", "script", "supported"), &TextServer::font_set_script_supported);

	ClassDB::bind_method(D_METHOD("font_draw_glyph", "font", "canvas", "size", "pos", "index", "color"), &TextServer::font_draw_glyph);
	ClassDB::bind_method(D_METHOD("font_draw_glyph_outline", "font", "canvas", "size", "pos", "index", "color"), &TextServer::font_draw_glyph_outline);

	ClassDB::bind_method(D_METHOD("font_draw_invalid_glpyh", "font", "canvas", "size", "pos", "index", "color"), &TextServer::font_draw_invalid_glpyh);

	ClassDB::bind_method(D_METHOD("create_shaped_text", "direction", "orientation"), &TextServer::create_shaped_text, DEFVAL(TEXT_DIRECTION_AUTO), DEFVAL(TEXT_ORIENTATION_HORIZONTAL_TB));
	ClassDB::bind_method(D_METHOD("shaped_set_direction", "shaped", "direction"), &TextServer::shaped_set_direction, DEFVAL(TEXT_DIRECTION_AUTO));
	ClassDB::bind_method(D_METHOD("shaped_set_orientation", "shaped", "rientation"), &TextServer::shaped_set_orientation, DEFVAL(TEXT_ORIENTATION_HORIZONTAL_TB));
	ClassDB::bind_method(D_METHOD("shaped_add_text", "shaped", "text", "font", "size", "features", "locale"), &TextServer::shaped_add_text, DEFVAL(""), DEFVAL(""));
	ClassDB::bind_method(D_METHOD("shaped_add_object", "shaped", "id", "size", "inline_align"), &TextServer::shaped_add_object);

	ClassDB::bind_method(D_METHOD("shaped_create_substr", "shaped", "start", "length"), &TextServer::shaped_create_substr);
	ClassDB::bind_method(D_METHOD("shaped_get_graphemes", "shaped"), &TextServer::shaped_get_graphemes);
	ClassDB::bind_method(D_METHOD("shaped_get_direction", "shaped"), &TextServer::shaped_get_direction);

	ClassDB::bind_method(D_METHOD("shaped_get_line_breaks", "shaped", "width", "break_mode"), &TextServer::shaped_get_line_breaks, DEFVAL(TEXT_BREAK_MANDATORY | TEXT_BREAK_WORD));

	ClassDB::bind_method(D_METHOD("shaped_get_object_rect", "shaped", "id"), &TextServer::shaped_get_object_rect);
	ClassDB::bind_method(D_METHOD("shaped_get_size", "shaped"), &TextServer::shaped_get_size);
	ClassDB::bind_method(D_METHOD("shaped_get_ascent", "shaped"), &TextServer::shaped_get_ascent);
	ClassDB::bind_method(D_METHOD("shaped_get_descent", "shaped"), &TextServer::shaped_get_descent);
	ClassDB::bind_method(D_METHOD("shaped_get_line_spacing", "shaped"), &TextServer::shaped_get_line_spacing);

	ClassDB::bind_method(D_METHOD("shaped_fit_to_width", "shaped", "width", "justification_mode"), &TextServer::shaped_fit_to_width, DEFVAL(TEXT_JUSTIFICATION_KASHIDA | TEXT_JUSTIFICATION_WORD_BOUND));

	ClassDB::bind_method(D_METHOD("shaped_get_carets", "shaped", "pos"), &TextServer::shaped_get_carets);
	ClassDB::bind_method(D_METHOD("shaped_get_selection", "shaped", "start", "end"), &TextServer::shaped_get_selection);
	ClassDB::bind_method(D_METHOD("shaped_hit_test", "shaped", "coords"), &TextServer::shaped_hit_test);

	ClassDB::bind_method(D_METHOD("string_get_word", "string", "offset", "beg", "end"), &TextServer::string_get_word);
	ClassDB::bind_method(D_METHOD("string_get_line", "string", "offset", "beg", "end"), &TextServer::string_get_line);

	ClassDB::bind_method(D_METHOD("caret_advance", "string", "value", "type"), &TextServer::caret_advance);

	ClassDB::bind_method(D_METHOD("is_uppercase", "char"), &TextServer::is_uppercase);
	ClassDB::bind_method(D_METHOD("is_lowercase", "char"), &TextServer::is_lowercase);
	ClassDB::bind_method(D_METHOD("is_titlecase", "char"), &TextServer::is_titlecase);
	ClassDB::bind_method(D_METHOD("is_digit", "char"), &TextServer::is_digit);
	ClassDB::bind_method(D_METHOD("is_alphanumeric", "char"), &TextServer::is_alphanumeric);
	ClassDB::bind_method(D_METHOD("is_punctuation", "char"), &TextServer::is_punctuation);
	ClassDB::bind_method(D_METHOD("to_lowercase", "char"), &TextServer::to_lowercase);
	ClassDB::bind_method(D_METHOD("to_uppercase", "char"), &TextServer::to_uppercase);
	ClassDB::bind_method(D_METHOD("to_titlecase", "char"), &TextServer::to_titlecase);

	ClassDB::bind_method(D_METHOD("to_digit", "char", "radix"), &TextServer::to_digit);

	BIND_ENUM_CONSTANT(FEATURE_FONT_OUTLINE);
	BIND_ENUM_CONSTANT(FEATURE_FONT_RESIZEBLE);
	BIND_ENUM_CONSTANT(FEATURE_FONT_DISTANCE_FIELD);
	BIND_ENUM_CONSTANT(FEATURE_FONT_OPENPTYPE);

	BIND_ENUM_CONSTANT(FEATURE_SERVER_SHAPING);
	BIND_ENUM_CONSTANT(FEATURE_SERVER_BIDI_LAYOUT);
	BIND_ENUM_CONSTANT(FEATURE_SERVER_VERTICAL_LAYOUT);
	BIND_ENUM_CONSTANT(FEATURE_SERVER_BITMAP_FONTS);
	BIND_ENUM_CONSTANT(FEATURE_SERVER_DYNAMIC_FONTS);
	BIND_ENUM_CONSTANT(FEATURE_SERVER_SYSTEM_FONTS);

	BIND_ENUM_CONSTANT(TEXT_DIRECTION_AUTO);
	BIND_ENUM_CONSTANT(TEXT_DIRECTION_LTR);
	BIND_ENUM_CONSTANT(TEXT_DIRECTION_RTL);

	BIND_ENUM_CONSTANT(TEXT_ORIENTATION_HORIZONTAL_TB);
	BIND_ENUM_CONSTANT(TEXT_ORIENTATION_VERTICAL_RL);
	BIND_ENUM_CONSTANT(TEXT_ORIENTATION_VERTICAL_LR);
	BIND_ENUM_CONSTANT(TEXT_ORIENTATION_SIDEWAYS_RL);
	BIND_ENUM_CONSTANT(TEXT_ORIENTATION_SIDEWAYS_LR);

	BIND_ENUM_CONSTANT(TEXT_JUSTIFICATION_NONE);
	BIND_ENUM_CONSTANT(TEXT_JUSTIFICATION_KASHIDA);
	BIND_ENUM_CONSTANT(TEXT_JUSTIFICATION_WORD_BOUND);
	BIND_ENUM_CONSTANT(TEXT_JUSTIFICATION_GRAPHEME_BOUND);
	BIND_ENUM_CONSTANT(TEXT_JUSTIFICATION_GRAPHEME_WIDTH);

	BIND_ENUM_CONSTANT(TEXT_BREAK_NONE);
	BIND_ENUM_CONSTANT(TEXT_BREAK_MANDATORY);
	BIND_ENUM_CONSTANT(TEXT_BREAK_WORD);
	BIND_ENUM_CONSTANT(TEXT_BREAK_GRAPHEME);

	BIND_ENUM_CONSTANT(TEXT_CARET_MOVE_GRAPHEME);
	BIND_ENUM_CONSTANT(TEXT_CARET_MOVE_WORD);
	BIND_ENUM_CONSTANT(TEXT_CARET_MOVE_SENTENCE);
	BIND_ENUM_CONSTANT(TEXT_CARET_MOVE_PARAGRAPH);

	BIND_ENUM_CONSTANT(TEXT_GRAPHEME_FLAG_VALID);
	BIND_ENUM_CONSTANT(TEXT_GRAPHEME_FLAG_RTL);
	BIND_ENUM_CONSTANT(TEXT_GRAPHEME_FLAG_ROTCW);
	BIND_ENUM_CONSTANT(TEXT_GRAPHEME_FLAG_ROTCCW);
}

TextServer::TextServer() {
	singleton = this;
}

TextServer::~TextServer() {
	singleton = nullptr;
}