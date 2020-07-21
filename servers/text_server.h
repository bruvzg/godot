/*************************************************************************/
/*  text_server.h                                                        */
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

#ifndef TEXT_SERVER_H
#define TEXT_SERVER_H

#include "core/os/os.h"
#include "core/os/thread_safe.h"
#include "core/reference.h"
#include "core/rid.h"
#include "core/variant.h"

class TextServer : public Object {
	GDCLASS(TextServer, Object);
	_THREAD_SAFE_CLASS_

public:
	enum FontFeature {
		FEATURE_FONT_OUTLINE,
		FEATURE_FONT_RESIZEBLE,
		FEATURE_FONT_DISTANCE_FIELD,
		FEATURE_FONT_OPENPTYPE
	};

	enum ServerFeature {
		FEATURE_SERVER_SHAPING,
		FEATURE_SERVER_BIDI_LAYOUT,
		FEATURE_SERVER_VERTICAL_LAYOUT,
		FEATURE_SERVER_BITMAP_FONTS,
		FEATURE_SERVER_DYNAMIC_FONTS,
		FEATURE_SERVER_SYSTEM_FONTS
	};

	enum TextDirection {
		TEXT_DIRECTION_AUTO, // Detects text direction based on string content and specific locale
		TEXT_DIRECTION_LTR, // Left-to-right text.
		TEXT_DIRECTION_RTL // Right-to-left text.
	};

	enum TextOrientation {
		TEXT_ORIENTATION_HORIZONTAL_TB, // Text flows horizontally, next line to under
		TEXT_ORIENTATION_VERTICAL_RL, // For LTR text flows vertically top to bottom, next line is to the left. For RTL, text flows from bottom to top, next line to the right. Vertical scripts displayed upright.
		TEXT_ORIENTATION_VERTICAL_LR, // For LTR text flows vertically top to bottom, next line is to the right. For RTL, text flows from bottom to top, next line to the left. Vertical scripts displayed upright.
		TEXT_ORIENTATION_SIDEWAYS_RL, // ... Vertical scripts displayed sideways.
		TEXT_ORIENTATION_SIDEWAYS_LR
	};

	enum TextJustification {
		TEXT_JUSTIFICATION_NONE = 0,
		TEXT_JUSTIFICATION_KASHIDA = 1 << 1, // Change width or add/remove kashidas (ــــ).
		TEXT_JUSTIFICATION_WORD_BOUND = 1 << 2, // Adds/removes extra space between the words (for some languages, should add spaces even if there were non in the original string, using dictionary).
		TEXT_JUSTIFICATION_GRAPHEME_BOUND = 1 << 3, // Adds/removes extra space in between all non-joining graphemes.
		TEXT_JUSTIFICATION_GRAPHEME_WIDTH = 1 << 4 // Adjusts width of the graphemes visually (if supported by font), 10-15% of change should be OK in general.
	};

	enum TextBreak {
		TEXT_BREAK_NONE = 0,
		TEXT_BREAK_MANDATORY = 1 << 1, // Breaks line at the explicit line break characters ("\n" etc).
		TEXT_BREAK_WORD = 1 << 2, // Breaks line between the words.
		TEXT_BREAK_GRAPHEME = 1 << 3 // Breaks line between any graphemes (in general it's OK to break line anywhere, as long as it isn't reshaped after).
	};

	enum TextCaretMove {
		TEXT_CARET_MOVE_GRAPHEME,
		TEXT_CARET_MOVE_WORD,
		TEXT_CARET_MOVE_SENTENCE,
		TEXT_CARET_MOVE_PARAGRAPH
	};

	enum TextGraphemeFlags {
		TEXT_GRAPHEME_FLAG_VALID = 1 << 1,
		TEXT_GRAPHEME_FLAG_RTL = 1 << 2,
		TEXT_GRAPHEME_FLAG_ROTCW = 1 << 3, // For sideways vertical layout.
		TEXT_GRAPHEME_FLAG_ROTCCW = 1 << 4,
	};

	struct Grapheme {
		struct Glyph {
			uint32_t glyph_index = 0; // Glyph index is internal value of the font and can't be reused with other fonts, or store UTF-32 codepoint for invalid glyphs (for faster invalid char hex code box display).
			Vector2 offset; // Offset from the origin of the glyph.
		};

		Vector<Glyph> glyphs;
		Vector2i range; // Range in the original string this grapheme corresponds to.
		Vector2 advance; // Advance to the next glyph.
		/*TextGraphemeFlags*/ uint8_t flags; // Used for caret drawing.
		RID font;
	};

	struct Caret {
		Rect2 rect; // Caret rectangle
		bool is_primary;
	};

private:
	static TextServer *singleton;
	static void _bind_methods();

public:
	_FORCE_INLINE_ static TextServer *get_singleton() {
		return singleton;
	};

	// Common
	virtual bool has_feature(ServerFeature p_ftr) = 0;
	virtual String get_name() const = 0;

	virtual bool load_data(const String &p_filename) = 0; // Load custom ICU data file.
	virtual bool is_data_loaded() const = 0;

	virtual void free(RID p_rid) = 0; // Free Font / Shaped Text Buffer resources.

	// Font API
	virtual RID create_font_system(const String &p_name) = 0; // Loads OS default font by name (if supported).
	virtual RID create_font_resource(const String &p_filename) = 0; // Loads custom font from "res://" or filesystem.
	virtual RID create_font_memory(const Vector<uint8_t> &p_data) = 0; // Loads custom font from memory (for built-in fonts).

	virtual float font_get_height(RID p_font, float p_size) const = 0;
	virtual float font_get_ascent(RID p_font, float p_size) const = 0;
	virtual float font_get_descent(RID p_font, float p_size) const = 0;
	virtual float font_get_underline_position(RID p_font, float p_size) const = 0;
	virtual float font_get_underline_thickness(RID p_font, float p_size) const = 0;

	virtual bool font_has_feature(RID p_font, FontFeature p_feature) const = 0; // Outline, Resizable, Distance field
	virtual bool font_get_language_supported(RID p_font, const String &p_locale) const = 0;
	virtual bool font_get_script_supported(RID p_font, const String &p_script) const = 0;

	virtual void font_set_language_supported(RID p_font, const String &p_locale, bool p_value) = 0; // User override.
	virtual void font_set_script_supported(RID p_font, const String &p_script, bool p_value) = 0;

	virtual void font_draw_glyph(RID p_font, RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color) const = 0;
	virtual void font_draw_glyph_outline(RID p_font, RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color) const = 0;

	virtual void font_draw_invalid_glpyh(RID p_font, RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color) const = 0; // Draws box with hex code, scaled to match font size.

	// Shaped Text Buffer API
	virtual RID create_shaped_text(TextDirection p_direction = TEXT_DIRECTION_AUTO, TextOrientation p_orientation = TEXT_ORIENTATION_HORIZONTAL_TB) = 0;
	virtual void shaped_set_direction(RID p_shaped, TextDirection p_direction = TEXT_DIRECTION_AUTO) = 0;
	virtual void shaped_set_orientation(RID p_shaped, TextOrientation p_orientation = TEXT_ORIENTATION_HORIZONTAL_TB) = 0;
	virtual TextDirection shaped_get_direction(RID p_shaped) const = 0;
	virtual TextOrientation shaped_get_orientation(RID p_shaped) const = 0;
	virtual bool shaped_add_text(RID p_shaped, const String &p_text, const List<RID> &p_font, float p_size, const String &p_features = "", const String &p_locale = "") = 0; // Add text and object to span stack, lazy
	virtual bool shaped_add_object(RID p_shaped, Variant p_id, const Size2 &p_size, VAlign p_inline_align) = 0; // Add inline object

	virtual RID shaped_create_substr(RID p_shaped, int p_start, int p_length) const = 0; // Get shaped substring (e.g for line breaking)
	virtual Vector<Grapheme> shaped_get_graphemes(RID p_shaped) const = 0; // Returns graphemes as is or BiDi reorders them for the line if range is specified. Graphemes returned in visual (LTR) order. Returned graphems should be usable in the place of characters for the most UI use cases, without massive code changes.
	virtual TextDirection shaped_get_direction(RID p_shaped) const = 0; // Returns detected base direction of the string if it was shaped with AUTO direction.

	virtual Vector<Vector2i> shaped_get_line_breaks(RID p_shaped, float p_width, /*TextBreak*/ uint8_t p_break_mode = TEXT_BREAK_MANDATORY | TEXT_BREAK_WORD) const = 0; // Returns line ranges, ranges can be directly used with get_graphemes function to render multiline text.

	virtual Rect2 shaped_get_object_rect(RID p_shaped, Variant p_id) const = 0;
	virtual Size2 shaped_get_size(RID p_shaped) const = 0;
	virtual float shaped_get_ascent(RID p_shaped) const = 0; // For some languages, graphemes can be offset from the base line significantly, these functions should return maximum ascent and descent, though for most cases using font ascent/descent is OK.
	virtual float shaped_get_descent(RID p_shaped) const = 0; // Also, can include size of inline objects.
	virtual float shaped_get_line_spacing(RID p_shaped) const = 0; // Offset to the next line (in the direction specified by text orientation)

	virtual float shaped_fit_to_width(RID p_shaped, float p_width, /*TextJustification*/ uint8_t p_justification_mode = TEXT_JUSTIFICATION_KASHIDA | TEXT_JUSTIFICATION_WORD_BOUND) const = 0; // Adjusts spaces and elongations in the line to fit it to the specified width, returns line width after adjustment.

	// Shaped Text Buffer helpers for input controls
	virtual Vector<Caret> shaped_get_carets(RID p_shaped, int p_pos) const = 0;
	virtual Vector<Rect2> shaped_get_selection(RID p_shaped, int p_start, int p_end) const = 0;
	virtual int shaped_hit_test(RID p_shaped, const Vector2 &p_coords) const = 0;

	// String API

	virtual bool string_get_word(const String &p_string, int p_offset, int &r_beg, int &r_end) const = 0;
	virtual bool string_get_line(const String &p_string, int p_offset, int &r_beg, int &r_end) const = 0;

	virtual int caret_advance(const String &p_string, int p_value, TextCaretMove p_type) const = 0;

	virtual bool is_uppercase(char32_t p_char) const = 0;
	virtual bool is_lowercase(char32_t p_char) const = 0;
	virtual bool is_titlecase(char32_t p_char) const = 0;
	virtual bool is_digit(char32_t p_char) const = 0;
	virtual bool is_alphanumeric(char32_t p_char) const = 0;
	virtual bool is_punctuation(char32_t p_char) const = 0;
	virtual char32_t to_lowercase(char32_t p_char) const = 0;
	virtual char32_t to_uppercase(char32_t p_char) const = 0;
	virtual char32_t to_titlecase(char32_t p_char) const = 0;

	virtual int32_t to_digit(char32_t p_char, int p_radix) const = 0;

	TextServer();
	~TextServer();
};

#define TX TextServer::get_singleton()

#define U16_IS_LEAD(c) (((c) & 0xFFFFFC00) == 0xD800)
#define U16_IS_TRAIL(c) (((c) & 0xFFFFFC00) == 0xDC00)
#define U16_SURROGATE_OFFSET ((0xD800 << 10UL) + 0xDC00 - 0x10000)
#define U16_GET_SUPPLEMENTARY(lead, trail) (((char32_t)(lead) << 10UL) + (char32_t)(trail) - U16_SURROGATE_OFFSET)

VARIANT_ENUM_CAST(TextServer::ServerFeature);
VARIANT_ENUM_CAST(TextServer::FontFeature);
VARIANT_ENUM_CAST(TextServer::TextDirection);
VARIANT_ENUM_CAST(TextServer::TextOrientation);
VARIANT_ENUM_CAST(TextServer::TextJustification);
VARIANT_ENUM_CAST(TextServer::TextBreak);
VARIANT_ENUM_CAST(TextServer::TextCaretMove);
VARIANT_ENUM_CAST(TextServer::TextGraphemeFlags);

#endif /* TEXT_SERVER_H */
