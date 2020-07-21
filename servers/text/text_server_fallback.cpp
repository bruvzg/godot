/*************************************************************************/
/*  text_server_fallback.cpp                                             */
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

#include "text_server_fallback.h"

bool TextServerFallback::has_feature(ServerFeature p_ftr) {
	switch (p_feature) {
		//case FEATURE_SERVER_SHAPING:
		//case FEATURE_SERVER_BIDI_LAYOUT:
		//case FEATURE_SERVER_VERTICAL_LAYOUT:
		case FEATURE_SERVER_BITMAP_FONTS:
		case FEATURE_SERVER_DYNAMIC_FONTS:
		//case FEATURE_SERVER_SYSTEM_FONTS:
			return true;
		default: {
		}
	}
	return false;
}

String TextServerFallback::get_name() const {
	return "Fallback text server";
}

bool TextServerFallback::load_data(const String &p_filename) { /* NOP */}
bool TextServerFallback::is_data_loaded() const { return false; }

void TextServerFallback::free(RID p_rid) {
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
RID TextServerFallback::create_font_system(const String &p_name) {
	ERR_FAIL_V(RID());
}

RID TextServerFallback::create_font_resource(const String &p_filename) {
	_FontData font;
	//TODO - copy existion bitmap/dynamic font code
	RID id = font_owner.make_rid(font);
	return id;
}

RID TextServerFallback::create_font_memory(const Vector<uint8_t> &p_data) {
	_FontData font;
	//TODO - copy existion bitmap/dynamic font code
	RID id = font_owner.make_rid(font);
	return id;
}

float TextServerFallback::font_get_height(RID p_font, float p_size) const {
	_FontData *fd = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!fd, 0.f);
	//TODO
}

float TextServerFallback::font_get_ascent(RID p_font, float p_size) const {
	_FontData *fd = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!fd, 0.f);
	//TODO
}

float TextServerFallback::font_get_descent(RID p_font, float p_size) const {
	_FontData *fd = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!fd, 0.f);
	//TODO
}

float TextServerFallback::font_get_underline_position(RID p_font, float p_size) const {
	_FontData *fd = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!fd, 0.f);
	//TODO
}

float TextServerFallback::font_get_underline_thickness(RID p_font, float p_size) const {
	_FontData *fd = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!fd, 0.f);
	//TODO
}

bool TextServerFallback::font_has_feature(RID p_font, FontFeature p_feature) const {
	_FontData *fd = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!fd, false);
	//TODO
}

bool TextServerFallback::font_get_language_supported(RID p_font, const String &p_locale) const {
	_FontData *fd = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!fd, false);
	//TODO
}

bool TextServerFallback::font_get_script_supported(RID p_font, const String &p_script) const {
	_FontData *fd = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!fd, false);
	//TODO
}

void TextServerFallback::font_set_language_supported(RID p_font, const String &p_locale, bool p_value) {
	_FontData *fd = font_owner.getornull(p_font);
	ERR_FAIL_COND(!fd);
	//TODO
}

void TextServerFallback::font_set_script_supported(RID p_font, const String &p_script, bool p_value) {
	_FontData *fd = font_owner.getornull(p_font);
	ERR_FAIL_COND(!fd);
	//TODO
}

void TextServerFallback::font_draw_glyph(RID p_font, RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color) const {
	_FontData *fd = font_owner.getornull(p_font);
	ERR_FAIL_COND(!fd);
	//TODO
}

void TextServerFallback::font_draw_glyph_outline(RID p_font, RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color) const {
	_FontData *fd = font_owner.getornull(p_font);
	ERR_FAIL_COND(!fd);
	//TODO
}

void TextServerFallback::font_draw_invalid_glpyh(RID p_font, RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color) const {
	_FontData *fd = font_owner.getornull(p_font);
	ERR_FAIL_COND(!fd);
	//TODO
}

/*************************************************************************/

void TextServerFallback::_shape(_ShapedCTX *p_ctxp) {
	if (p_ctxp->dirty) {
		p_ctxp->graphemes.clear();

		for (int i = 0; i < ctx->spans.size(); i++) {
			if (ctx->span[i].id != Variant()) {
				//TODO - object
			} else {
				for (int j = 0; j < ctx->span[i].text.length(); j++) {
					Grapheme grapheme;
					grapheme.range = Vector2i(prev_end + j, prev_end + j);
					char32_t chr = ctx->span[i].text[j];
					if (U16_IS_LEAD(chr) && j < ctx->span[i].text.length() - 1) {
						chr = U16_GET_SUPPLEMENTARY(chr, ctx->span[i].text[j + 1]); // Decode UTF-16 surrogates.
						grapheme.range.y = prev_end + j + 1;
						j++;
					}
					//TODO - "shape"
					ctx->graphemes.push_back(grapheme);
				}
			}
		}
		p_ctxp->dirty = false;
	}
}

RID TextServerFallback::create_shaped_text(TextDirection p_direction, TextOrientation p_orientation) {
	ERR_FAIL_COND_V_MSG(p_direction != TEXT_DIRECTION_RTL, RID(), "Not supported");
	ERR_FAIL_COND_V_MSG(p_orientation != TEXT_ORIENTATION_HORIZONTAL_TB, RID(), "Not supported");

	_ShapedCTX ctx;
	RID id = shaped_owner.make_rid(ctx);
	return id;
}

void TextServerFallback::shaped_set_direction(RID p_shaped, TextDirection p_direction) {
	ERR_FAIL_COND_MSG(p_direction != TEXT_DIRECTION_RTL, "Not supported");
}

void TextServerFallback::shaped_set_orientation(RID p_shaped, TextOrientation p_orientation) {
	ERR_FAIL_COND_MSG(p_orientation != TEXT_ORIENTATION_HORIZONTAL_TB, "Not supported");
}

TextServer::TextDirection TextServerFallback::shaped_get_direction(RID p_shaped) const {
	return TEXT_DIRECTION_LTR;
}

TextServer::TextOrientation TextServerFallback::shaped_get_orientation(RID p_shaped) const {
	return TEXT_ORIENTATION_HORIZONTAL_TB;
}

bool TextServerFallback::shaped_add_text(RID p_shaped, const String &p_text, const List<RID> &p_font, float p_size, const String &p_features, const String &p_locale) {
	_THREAD_SAFE_METHOD_

	_ShapedCTX *ctx = shaped_owner.getornull(p_shaped);
	ERR_FAIL_COND_V(!ctx, false);
	ERR_FAIL_COND_V(p_font.size() == 0, false);
	ERR_FAIL_COND_V(p_text.length() == 0, false);

	if (OS::get_singleton()->is_stdout_verbose() && (p_features != String())) {
		WARN_PRINT("OpenType features are not supported by this text shaping engine.");
	}

	_Span span;
	span.font = p_font;
	span.size = p_size;
	span.text = p_text;

	ctx->dirty = true;
	ctx->spans.push_back(span);
}

bool TextServerFallback::shaped_add_object(RID p_shaped, Variant p_id, const Size2 &p_size, VAlign p_inline_align) {
	_THREAD_SAFE_METHOD_

	_ShapedCTX *ctx = shaped_owner.getornull(p_shaped);
	ERR_FAIL_COND_V(!ctx, false);
	ERR_FAIL_COND_V(p_font.size() == 0, false);
	ERR_FAIL_COND_V(p_text.length() == 0, false);
	ERR_FAIL_COND_V(ctx->objects.has(p_id), false);

	_Span span;
	span.text = String::char(0xFFFC);
	span.id = p_id;

	_Object obj;
	obj.rect = Rect2(Vector2(), p_size);
	obj.align = p_inline_align;

	ctx->dirty = true;
	ctx->spans.push_back(span);
	ctx->objects[p_id] = obj;
}

RID TextServerFallback::shaped_create_substr(RID p_shaped, int p_start, int p_length) const {
	_THREAD_SAFE_METHOD_

	_ShapedCTX *ctx = shaped_owner.getornull(p_shaped);
	ERR_FAIL_COND_V(!ctx, RID());
	ERR_FAIL_COND_V(p_start >= ctx->text.length(), RID());
	ERR_FAIL_COND_V(p_start < 0, RID());
	if (ctx->dirty) {
		_shape(ctx);
	}

	_ShapedCTX sub_ctx;

	//TODO
	RID id = shaped_owner.make_rid(sub_ctx);
	return id;
}

Vector<TextServer::Grapheme> TextServerFallback::shaped_get_graphemes(RID p_shaped) const {
	_THREAD_SAFE_METHOD_

	_ShapedCTX *ctx = shaped_owner.getornull(p_shaped);
	ERR_FAIL_COND_V(!ctx, Vector<TextServer::Grapheme>());
	if (ctx->dirty) {
		_shape(ctx);
	}
	return ctx->graphemes;
}

TextServer::TextDirection TextServerFallback::shaped_get_direction(RID p_shaped) const {
	return TEXT_DIRECTION_LTR;
}

Vector<Vector2i> TextServerFallback::shaped_get_line_breaks(RID p_shaped, float p_width, /*TextBreak*/ uint8_t p_break_mode) const {
	_THREAD_SAFE_METHOD_

	_ShapedCTX *ctx = shaped_owner.getornull(p_shaped);
	ERR_FAIL_COND_V(!ctx, Vector<Vector2i>());
	if (ctx->dirty) {
		_shape(ctx);
	}

	Vector<Vector2i> ret;
	//TODO

	return ret;
}

Rect2 TextServerFallback::shaped_get_object_rect(RID p_shaped, Variant p_id) const {
	_THREAD_SAFE_METHOD_

	_ShapedCTX *ctx = shaped_owner.getornull(p_shaped);
	ERR_FAIL_COND_V(!ctx, Rect2());
	if (ctx->dirty) {
		_shape(ctx);
	}
	if (ctx->objects.has(p_id)) {
		return ctx->objects[p_id].rect;
	} else {
		return Rect2();
	}
}

Size2 TextServerFallback::shaped_get_size(RID p_shaped) const {
	_THREAD_SAFE_METHOD_

	_ShapedCTX *ctx = shaped_owner.getornull(p_shaped);
	ERR_FAIL_COND_V(!ctx, Size2());
	if (ctx->dirty) {
		_shape(ctx);
	}
	return Size2(ctx->width, ctx->ascent + ctx->descent);
}

float TextServerFallback::shaped_get_ascent(RID p_shaped) const {
	_THREAD_SAFE_METHOD_

	_ShapedCTX *ctx = shaped_owner.getornull(p_shaped);
	ERR_FAIL_COND_V(!ctx, 0.f);
	if (ctx->dirty) {
		_shape(ctx);
	}
	return ctx->ascent;
}

float TextServerFallback::shaped_get_descent(RID p_shaped) const {
	_THREAD_SAFE_METHOD_

	_ShapedCTX *ctx = shaped_owner.getornull(p_shaped);
	ERR_FAIL_COND_V(!ctx, 0.f);
	if (ctx->dirty) {
		_shape(ctx);
	}
	return ctx->descent;
}

float TextServerFallback::shaped_get_line_spacing(RID p_shaped) const {
	_THREAD_SAFE_METHOD_

	_ShapedCTX *ctx = shaped_owner.getornull(p_shaped);
	ERR_FAIL_COND_V(!ctx, 0.f);
	if (ctx->dirty) {
		_shape(ctx);
	}
	return ctx->line_spacing;
}

float TextServerFallback::shaped_fit_to_width(RID p_shaped, float p_width, /*TextJustification*/ uint8_t p_justification_mode) const {
	_THREAD_SAFE_METHOD_

	_ShapedCTX *ctx = shaped_owner.getornull(p_shaped);
	ERR_FAIL_COND_V(!ctx, 0.f);
	if (ctx->dirty) {
		_shape(ctx);
	}
	//TODO - align
	return ctx->width;
}

/*************************************************************************/

Vector<TextServer::Caret> TextServerFallback::shaped_get_carets(RID p_shaped, int p_pos) const {
	//TODO
}

Vector<Rect2> TextServerFallback::shaped_get_selection(RID p_shaped, int p_start, int p_end) const {
	//TODO
}

int TextServerFallback::shaped_hit_test(RID p_shaped, const Vector2 &p_coords) const {
	//TODO
}

/*************************************************************************/

bool TextServerFallback::string_get_word(const String &p_string, int p_offset, int &r_beg, int &r_end) const {
	//TODO
}

bool TextServerFallback::string_get_line(const String &p_string, int p_offset, int &r_beg, int &r_end) const {
	//TODO
}

int TextServerFallback::caret_advance(const String &p_string, int p_value, TextCaretMove p_type) const {
	//TODO
}

bool TextServerFallback::is_uppercase(char32_t p_char) const {
	//TODO
}

bool TextServerFallback::is_lowercase(char32_t p_char) const {
	//TODO
}

bool TextServerFallback::is_titlecase(char32_t p_char) const {
	//TODO
}

bool TextServerFallback::is_digit(char32_t p_char) const {
	//TODO
}

bool TextServerFallback::is_alphanumeric(char32_t p_char) const {
	//TODO
}

bool TextServerFallback::is_punctuation(char32_t p_char) const {
	//TODO
}

char32_t TextServerFallback::to_lowercase(char32_t p_char) const {
	//TODO
}

char32_t TextServerFallback::to_uppercase(char32_t p_char) const {
	//TODO
}

char32_t TextServerFallback::to_titlecase(char32_t p_char) const {
	//TODO
}

int32_t TextServerFallback::to_digit(char32_t p_char, int p_radix) const {
	//TODO
}

TextServerFallback::TextServerFallback() {
	//TODO
}

TextServerFallback::~TextServerFallback() {
	//TODO
}
