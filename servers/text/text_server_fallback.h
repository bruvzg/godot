/*************************************************************************/
/*  text_server_fallback.h                                               */
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

#ifndef TEXT_SERVER_FALLBACK_H
#define TEXT_SERVER_FALLBACK_H

#include "servers/text_server.h"

class TextServerFallback : public TextServer {
	GDCLASS(TextServerFallback, TextServer);
	_THREAD_SAFE_CLASS_

	struct _FontData {
		//TODO
	};
	RID_Owner<_FontData> font_owner;

	struct _ShapedCTX {
		struct _Span {
			String text;
			List<RID> font;
			Variant id;
			float size = 0.f;
		};
		Vector<_Span> spans;

		struct _Object {
			VAlign align;
			Rect2 rect;
		};
		Map<Variant, _Object> objects;

		float ascent = 0.f;
		float descent = 0.f;
		float width = 0.f;
		float line_spacing = 0.f;

		bool dirty = true;
		Vector<Grapheme> graphemes;
	};
	RID_Owner<_ShapedCTX> shaped_owner;

	void _shape(_ShapedCTX *p_ctxp);

public:
	// Common
	virtual bool has_feature(ServerFeature p_ftr);
	virtual String get_name() const;

	virtual bool load_data(const String &p_filename);
	virtual bool is_data_loaded() const;

	virtual void free(RID p_rid);

	// Font API
	virtual RID create_font_system(const String &p_name);
	virtual RID create_font_resource(const String &p_filename);
	virtual RID create_font_memory(const Vector<uint8_t> &p_data);

	virtual float font_get_height(RID p_font, float p_size) const;
	virtual float font_get_ascent(RID p_font, float p_size) const;
	virtual float font_get_descent(RID p_font, float p_size) const;
	virtual float font_get_underline_position(RID p_font, float p_size) const;
	virtual float font_get_underline_thickness(RID p_font, float p_size) const;

	virtual bool font_has_feature(RID p_font, FontFeature p_feature) const;
	virtual bool font_get_language_supported(RID p_font, const String &p_locale) const;
	virtual bool font_get_script_supported(RID p_font, const String &p_script) const;

	virtual void font_set_language_supported(RID p_font, const String &p_locale, bool p_value);
	virtual void font_set_script_supported(RID p_font, const String &p_script, bool p_value);

	virtual void font_draw_glyph(RID p_font, RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color) const;
	virtual void font_draw_glyph_outline(RID p_font, RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color) const;

	virtual void font_draw_invalid_glpyh(RID p_font, RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color) const;

	// Shaped Text Buffer API
	virtual RID create_shaped_text(TextDirection p_direction = TEXT_DIRECTION_AUTO, TextOrientation p_orientation = TEXT_ORIENTATION_HORIZONTAL_TB);
	virtual void shaped_set_direction(RID p_shaped, TextDirection p_direction = TEXT_DIRECTION_AUTO);
	virtual void shaped_set_orientation(RID p_shaped, TextOrientation p_orientation = TEXT_ORIENTATION_HORIZONTAL_TB);
	virtual TextDirection shaped_get_direction(RID p_shaped) const;
	virtual TextOrientation shaped_get_orientation(RID p_shaped) const;
	virtual bool shaped_add_text(RID p_shaped, const String &p_text, const List<RID> &p_font, float p_size, const String &p_features = "", const String &p_locale = "");
	virtual bool shaped_add_object(RID p_shaped, Variant p_id, const Size2 &p_size, VAlign p_inline_align);

	virtual RID shaped_create_substr(RID p_shaped, int p_start, int p_length) const;
	virtual Vector<Grapheme> shaped_get_graphemes(RID p_shaped) const;
	virtual TextDirection shaped_get_direction(RID p_shaped) const;

	virtual Vector<Vector2i> shaped_get_line_breaks(RID p_shaped, float p_width, /*TextBreak*/ uint8_t p_break_mode = TEXT_BREAK_MANDATORY | TEXT_BREAK_WORD) const;

	virtual Rect2 shaped_get_object_rect(RID p_shaped, Variant p_id) const;
	virtual Size2 shaped_get_size(RID p_shaped) const;
	virtual float shaped_get_ascent(RID p_shaped) const;
	virtual float shaped_get_descent(RID p_shaped) const;
	virtual float shaped_get_line_spacing(RID p_shaped) const;

	virtual float shaped_fit_to_width(RID p_shaped, float p_width, /*TextJustification*/ uint8_t p_justification_mode = TEXT_JUSTIFICATION_KASHIDA | TEXT_JUSTIFICATION_WORD_BOUND) const;

	// Shaped Text Buffer helpers for input controls
	virtual Vector<Caret> shaped_get_carets(RID p_shaped, int p_pos) const;
	virtual Vector<Rect2> shaped_get_selection(RID p_shaped, int p_start, int p_end) const;
	virtual int shaped_hit_test(RID p_shaped, const Vector2 &p_coords) const;

	// String API

	virtual bool string_get_word(const String &p_string, int p_offset, int &r_beg, int &r_end) const;
	virtual bool string_get_line(const String &p_string, int p_offset, int &r_beg, int &r_end) const;

	virtual int caret_advance(const String &p_string, int p_value, TextCaretMove p_type) const;

	virtual bool is_uppercase(char32_t p_char) const;
	virtual bool is_lowercase(char32_t p_char) const;
	virtual bool is_titlecase(char32_t p_char) const;
	virtual bool is_digit(char32_t p_char) const;
	virtual bool is_alphanumeric(char32_t p_char) const;
	virtual bool is_punctuation(char32_t p_char) const;
	virtual char32_t to_lowercase(char32_t p_char) const;
	virtual char32_t to_uppercase(char32_t p_char) const;
	virtual char32_t to_titlecase(char32_t p_char) const;

	virtual int32_t to_digit(char32_t p_char, int p_radix) const;

	TextServerFallback();
	~TextServerFallback();
};

#endif /* TEXT_SERVER_FALLBACK_H */
