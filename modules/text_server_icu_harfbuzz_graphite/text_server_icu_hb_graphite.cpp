/*************************************************************************/
/*  text_server_icu_hb_graphite.cpp                                      */
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

#include "text_server_icu_hb_graphite.h"

bool TextServerICUHarfbuzzGraphite::has_feature(ServerFeature p_ftr) {
	switch (p_feature) {
		case FEATURE_SERVER_SHAPING:
		case FEATURE_SERVER_BIDI_LAYOUT:
		case FEATURE_SERVER_VERTICAL_LAYOUT:
		case FEATURE_SERVER_BITMAP_FONTS:
		case FEATURE_SERVER_DYNAMIC_FONTS:
		case FEATURE_SERVER_SYSTEM_FONTS:
			return true;
		default: {
		}
	}
	return false;
}

String TextServerICUHarfbuzzGraphite::get_name() const {
	return "ICU / Harfbuz / SIL Graphite text server";
}

bool TextServerICUHarfbuzzGraphite::load_data(const String &p_filename) {
	//TODO
}

bool TextServerICUHarfbuzzGraphite::is_data_loaded() const {
	//TODO
}

void TextServerICUHarfbuzzGraphite::free(RID p_rid) {
	_THREAD_SAFE_METHOD_

	if (font_owner.owns(p_rid)) {
		//TODO
		font_owner.free(p_rid);
	} else if (shaped_owner.owns(p_rid)) {
		//TODO
		shaped_owner.free(p_rid);
	} else {
		ERR_PRINT("Attempted to free invalid ID: " + itos(p_id.get_id()));
	}
}

// Font API
RID TextServerICUHarfbuzzGraphite::create_font_system(const String &p_name) {
	_FontData font;
	//TODO
	RID id = font_owner.make_rid(font);
	return id;
}

RID TextServerICUHarfbuzzGraphite::create_font_resource(const String &p_filename) {
	_FontData font;
	//TODO
	RID id = font_owner.make_rid(font);
	return id;
}

RID TextServerICUHarfbuzzGraphite::create_font_memory(const Vector<uint8_t> &p_data) {
	_FontData font;
	//TODO - copy existion bitmap/dynamic font code
	RID id = font_owner.make_rid(font);
	return id;
}

float TextServerICUHarfbuzzGraphite::font_get_height(RID p_font, float p_size) const {
	//TODO
}

float TextServerICUHarfbuzzGraphite::font_get_ascent(RID p_font, float p_size) const {
	//TODO
}

float TextServerICUHarfbuzzGraphite::font_get_descent(RID p_font, float p_size) const {
	//TODO
}

float TextServerICUHarfbuzzGraphite::font_get_underline_position(RID p_font, float p_size) const {
	//TODO
}

float TextServerICUHarfbuzzGraphite::font_get_underline_thickness(RID p_font, float p_size) const {
	//TODO
}

bool TextServerICUHarfbuzzGraphite::font_has_feature(RID p_font, FontFeature p_feature) const {
	//TODO
}

bool TextServerICUHarfbuzzGraphite::font_get_language_supported(RID p_font, const String &p_locale) const {
	//TODO
}

bool TextServerICUHarfbuzzGraphite::font_get_script_supported(RID p_font, const String &p_script) const {
	//TODO
}

void TextServerICUHarfbuzzGraphite::font_set_language_supported(RID p_font, const String &p_locale, bool p_value) {
	//TODO
}

void TextServerICUHarfbuzzGraphite::font_set_script_supported(RID p_font, const String &p_script, bool p_value) {
	//TODO
}

void TextServerICUHarfbuzzGraphite::font_draw_glyph(RID p_font, RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color) const {
	//TODO
}

void TextServerICUHarfbuzzGraphite::font_draw_glyph_outline(RID p_font, RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color) const {
	//TODO
}

void TextServerICUHarfbuzzGraphite::font_draw_invalid_glpyh(RID p_font, RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color) const {
	//TODO
}

/*************************************************************************/

void TextServerICUHarfbuzzGraphite::_shape(_ShapedCTX *p_ctxp) {
	if (p_ctxp->dirty) {
		p_ctxp->graphemes.clear();

		//TODO
		p_ctxp->dirty = false;
	}
}

RID TextServerICUHarfbuzzGraphite::create_shaped_text(TextDirection p_direction, TextOrientation p_orientation) {
	_ShapedCTX ctx;

	//TODO

	RID id = shaped_owner.make_rid(ctx);
	return id;
}

void TextServerICUHarfbuzzGraphite::shaped_set_direction(RID p_shaped, TextDirection p_direction) {
	//TODO
}

void TextServerICUHarfbuzzGraphite::shaped_set_orientation(RID p_shaped, TextOrientation p_orientation) {
	//TODO
}

TextServer::TextDirection TextServerICUHarfbuzzGraphite::shaped_get_direction(RID p_shaped) const {
	//TODO
}

TextServer::TextOrientation TextServerICUHarfbuzzGraphite::shaped_get_orientation(RID p_shaped) const {
	//TODO
}

bool TextServerICUHarfbuzzGraphite::shaped_add_text(RID p_shaped, const String &p_text, const List<RID> &p_font, float p_size, const String &p_features, const String &p_locale) {
	//TODO
}

bool TextServerICUHarfbuzzGraphite::shaped_add_object(RID p_shaped, Variant p_id, const Size2 &p_size, VAlign p_inline_align) {
	//TODO
}

RID TextServerICUHarfbuzzGraphite::shaped_create_substr(RID p_shaped, int p_start, int p_length) const {
	//TODO
}

Vector<TextServer::Grapheme> TextServerICUHarfbuzzGraphite::shaped_get_graphemes(RID p_shaped) const {
	//TODO
}

TextServer::TextDirection TextServerICUHarfbuzzGraphite::shaped_get_direction(RID p_shaped) const {
	//TODO
}

Vector<Vector2i> TextServerICUHarfbuzzGraphite::shaped_get_line_breaks(RID p_shaped, float p_width, /*TextBreak*/ uint8_t p_break_mode) const {
	//TODO
}

Rect2 TextServerICUHarfbuzzGraphite::shaped_get_object_rect(RID p_shaped, Variant p_id) const {
	//TODO
}

Size2 TextServerICUHarfbuzzGraphite::shaped_get_size(RID p_shaped) const {
	//TODO
}

float TextServerICUHarfbuzzGraphite::shaped_get_ascent(RID p_shaped) const {
	//TODO
}

float TextServerICUHarfbuzzGraphite::shaped_get_descent(RID p_shaped) const {
	//TODO
}

float TextServerICUHarfbuzzGraphite::shaped_get_line_spacing(RID p_shaped) const {
	//TODO
}

float TextServerICUHarfbuzzGraphite::shaped_fit_to_width(RID p_shaped, float p_width, /*TextJustification*/ uint8_t p_justification_mode) const {
	//TODO
}

/*************************************************************************/

Vector<TextServer::Caret> TextServerICUHarfbuzzGraphite::shaped_get_carets(RID p_shaped, int p_pos) const {
	//TODO
}

Vector<Rect2> TextServerICUHarfbuzzGraphite::shaped_get_selection(RID p_shaped, int p_start, int p_end) const {
	//TODO
}

int TextServerICUHarfbuzzGraphite::shaped_hit_test(RID p_shaped, const Vector2 &p_coords) const {
	//TODO
}

/*************************************************************************/

bool TextServerICUHarfbuzzGraphite::string_get_word(const String &p_string, int p_offset, int &r_beg, int &r_end) const {
	//TODO
}

bool TextServerICUHarfbuzzGraphite::string_get_line(const String &p_string, int p_offset, int &r_beg, int &r_end) const {
	//TODO
}

int TextServerICUHarfbuzzGraphite::caret_advance(const String &p_string, int p_value, TextCaretMove p_type) const {
	//TODO
}

bool TextServerICUHarfbuzzGraphite::is_uppercase(char32_t p_char) const {
	//TODO
}

bool TextServerICUHarfbuzzGraphite::is_lowercase(char32_t p_char) const {
	//TODO
}

bool TextServerICUHarfbuzzGraphite::is_titlecase(char32_t p_char) const {
	//TODO
}

bool TextServerICUHarfbuzzGraphite::is_digit(char32_t p_char) const {
	//TODO
}

bool TextServerICUHarfbuzzGraphite::is_alphanumeric(char32_t p_char) const {
	//TODO
}

bool TextServerICUHarfbuzzGraphite::is_punctuation(char32_t p_char) const {
	//TODO
}

char32_t TextServerICUHarfbuzzGraphite::to_lowercase(char32_t p_char) const {
	//TODO
}

char32_t TextServerICUHarfbuzzGraphite::to_uppercase(char32_t p_char) const {
	//TODO
}

char32_t TextServerICUHarfbuzzGraphite::to_titlecase(char32_t p_char) const {
	//TODO
}

int32_t TextServerICUHarfbuzzGraphite::to_digit(char32_t p_char, int p_radix) const {
	//TODO
}

TextServerICUHarfbuzzGraphite::TextServerICUHarfbuzzGraphite() {
	//TODO
}

TextServerICUHarfbuzzGraphite::~TextServerICUHarfbuzzGraphite() {
	//TODO
}
