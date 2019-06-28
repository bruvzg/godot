/*************************************************************************/
/*  shaping_interface_fb.cpp                                             */
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

#include "shaping_interface_fb.h"
#include "font_implementation_fb.h"
#include "servers/shaping_server.h"

ShapingInterfaceFallback::ShapingInterfaceFallback() {
	//NOP
}

ShapingInterfaceFallback::~ShapingInterfaceFallback() {
	//NOP
}

void ShapingInterfaceFallback::_bind_methods() {
	//NOP
}

StringName ShapingInterfaceFallback::get_name() const {
	return "Fallback";
}

int32_t ShapingInterfaceFallback::get_capabilities() const {
	return SHAPING_INTERFACE_NONE;
}

String ShapingInterfaceFallback::get_info() const {
	return "";
}

Ref<FontImplementation> ShapingInterfaceFallback::create_font_implementation(const FontData *p_data, FontData::CacheID p_id) const {
	if (p_data->_get_font_id() == "bitmap") {
		//Bitmap font
		Ref<FontImplementationBitmap> ref;
		ref.instance();
		ref->create(p_data, p_id);
		if (!ref->require_reload()) return ref;
#ifdef MODULE_FREETYPE_ENABLED
	} else if (p_data->_get_font_id() == "dynamic") {
		//Dynamic font
		Ref<FontImplementationDynamic> ref;
		ref.instance();
		ref->create(p_data, p_id);
		if (!ref->require_reload()) return ref;
#endif
	}
	return NULL;
}

TextDirection ShapingInterfaceFallback::get_paragraph_direction(TextDirection p_base_direction, const String &p_text, const String &p_language) const {
	//BiDi is not supported
	return TEXT_DIRECTION_LTR;
}

Vector<ShapingInterface::SourceRun> ShapingInterfaceFallback::get_bidi_runs(const String &p_text, int p_start, int p_end, TextDirection p_para_direction) const {
	Vector<SourceRun> runs;
	runs.push_back(SourceRun(p_start, p_end, 0)); ///LTR
	return runs;
}

Vector<ShapingInterface::SourceRun> ShapingInterfaceFallback::get_script_runs(const String &p_text, int p_start, int p_end, TextDirection p_para_direction) const {
	Vector<SourceRun> runs;
	runs.push_back(SourceRun(p_start, p_end, TAG('Z', 'y', 'y', 'y'))); //Undetermined script
	return runs;
}

Vector<ShapingInterface::SourceRun> ShapingInterfaceFallback::get_break_runs(const String &p_text, int p_start, int p_end, const String &p_language) const {
	Vector<SourceRun> runs;
	int prev = p_start;
	const CharType *sptr = &p_text[0];
	for (int i = p_start; i < p_end; i++) {
		if (sptr[i] == '\n') {
			runs.push_back(SourceRun(prev, i + 1, RUN_BREAK_HARD));
			prev = i + 1;
		}
		if ((sptr[i] == ' ') || ((sptr[i] >= 0x2000) && (sptr[i] <= 0x200A)) || (sptr[i] == 0x3000) || (sptr[i] == '\t')) {
			runs.push_back(SourceRun(prev, i + 1, RUN_BREAK_SOFT));
			prev = i + 1;
		}
	}
	if (prev != p_end) runs.push_back(SourceRun(prev, p_end, RUN_BREAK_HARD));
	return runs;
}

Run ShapingInterfaceFallback::shape_run2(const String &p_text, int p_start, int p_end, const Font *p_font, const String &p_features, const String &p_language, uint32_t p_level, uint32_t p_script, uint32_t p_break, bool p_prefer_vertical) const {
	Run r;

	Vector<Ref<FontImplementation>> font_impls;
	Vector<Ref<FontImplementation>> font_outline_impls;
	p_font->get_font_implementations(font_impls, font_outline_impls, p_script);

	const CharType *sptr = &p_text[0];

	r.width = 0.f;
	r.ascent = 0.f;
	r.descent = 0.f;

	r.clusters.resize(p_end - p_start);

	int32_t i = p_start;
	while (i < p_end) {

		Cluster cl;

		cl.start = i;
		cl.end = i;

		uint32_t c = sptr[i];

		//decode surrogates
		if (((c & 0xFFFFF800) == 0xD800) && ((c & 0x400) == 0) && (i + 1 < p_text.length())) {
			uint32_t c2 = sptr[i + 1];
			if ((c2 & 0x400) != 0) {
				c = (c << 10UL) + c2 - ((0xD800 << 10UL) + 0xDC00 - 0x10000);
				cl.end = i + 1;
				i++;
			}
		}

		Glyph gl;

		int fi = 0;
		while (gl.codepoint == 0 && fi < font_impls.size()) {
			gl.codepoint = font_impls[fi]->get_glyph(c);
			cl.font_impl = font_impls[fi];
			if (p_font->has_outline()) cl.font_outline_impl = font_outline_impls[fi];
			fi++;
		}

		if ((gl.codepoint == 0) && (c != 0x0009) && (c != 0x0020)) {
			gl.codepoint = c;
			gl.advance = FontHexBox::get_advance(c);
			cl.font_impl = Ref<FontImplementation>();
			cl.font_outline_impl = Ref<FontImplementation>();
		} else {
			gl.advance = cl.font_impl->get_advance(gl.codepoint);
			r.ascent = MAX(r.ascent, cl.font_impl->get_ascent());
			r.descent = MAX(r.descent, cl.font_impl->get_descent());
		}
		gl.offset = Vector2();

		if ((i < r.end - 1) && cl.font_impl.is_valid()) {
			uint32_t nc = sptr[i + 1];

			//decode surrogates
			if (((nc & 0xFFFFF800) == 0xD800) && ((nc & 0x400) == 0) && (i + 2 < p_text.length())) {
				uint32_t c2 = sptr[i + 2];
				if ((c2 & 0x400) != 0) {
					nc = (nc << 10UL) + c2 - ((0xD800 << 10UL) + 0xDC00 - 0x10000);
				}
			}
			gl.advance -= cl.font_impl->get_kerning(c, nc);
		}

		cl.repeat = 1;
		cl.flags = GLYPH_VALID;
		if ((c == 0x0020) || ((c >= 0x2000) && (c <= 0x200A)) || (c == 0x3000)) {
			cl.flags |= GLYPH_SPACE;
		}
		if (c == 0x0009) { //replace tab with zero width space
			cl.flags |= GLYPH_TAB;
			gl.codepoint = (cl.font_impl.is_valid()) ? cl.font_impl->get_glyph(0x0020) : 0;
			if (gl.codepoint != 0) cl.flags |= GLYPH_VALID;
			gl.offset = Point2();
			gl.advance = 0;
		}
		if (((c >= 0x0080 && c <= 0x009F) || (c < 0x0020) || (c == 0x007F)) && (c != 0x0009)) {
			cl.flags |= GLYPH_CONTROL;
		}
		if (c > 0xFFFF) cl.flags |= GLYPH_SURROGATE;

		cl.glyphs.push_back(gl);
		cl.advance = gl.advance;

		r.width += cl.advance;
		r.clusters.push_back(cl);

		i++;
	}

	return r;
}

Vector<float> ShapingInterfaceFallback::get_ligature_caret_offsets(const Ref<FontImplementation> &p_font_imp, TextDirection p_direction, uint32_t p_glyph) const {
	return Vector<float>(); //not supported
}

//S VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE
TextDirection ShapingInterfaceFallback::analyse_text(const String &p_text, int32_t p_start, int32_t p_end, TextDirection p_base_direction, const String &p_language, Vector<Run> &p_runs) const {
	int l = p_text.length();
	if (l == 0) return TEXT_DIRECTION_LTR;

	ERR_FAIL_COND_V(p_start > p_end, TEXT_DIRECTION_INVALID);
	ERR_FAIL_COND_V(p_end > l, TEXT_DIRECTION_INVALID);

	int32_t prev = p_start;
	const char16_t *sptr = &p_text[0];

	for (int32_t i = p_start; i < p_end; i++) {
		if (sptr[i] == '\n') {
			p_runs.push_back(Run(prev, i + 1, 0, 0, RUN_BREAK_HARD));
			prev = i + 1;
		}
		if ((sptr[i] == ' ') || ((sptr[i] >= 0x2000) && (sptr[i] <= 0x200A)) || (sptr[i] == 0x3000) || (sptr[i] == '\t')) {
			p_runs.push_back(Run(prev, i + 1, 0, 0, RUN_BREAK_SOFT));
			prev = i + 1;
		}
	}
	if (prev != p_end) p_runs.push_back(Run(prev, p_end, 0, 0, RUN_BREAK_HARD));

	return TEXT_DIRECTION_LTR;
}

bool ShapingInterfaceFallback::shape_run(const String &p_text, const Font *p_font, Run &p_run, const String &p_language, const String &p_features) const {
	ERR_FAIL_COND_V(p_font == NULL, false);

	Vector<Ref<FontImplementation>> font_impls;
	Vector<Ref<FontImplementation>> font_outline_impls;
	p_font->get_font_implementations(font_impls, font_outline_impls, p_run.script);

	ERR_FAIL_COND_V(font_impls.size() == 0, false);

	const CharType *sptr = &p_text[0];

	p_run.width = 0.f;
	p_run.ascent = 0.f;
	p_run.descent = 0.f;

	p_run.clusters.clear();
	p_run.clusters.resize(p_run.end - p_run.start);

	int32_t i = p_run.start;
	while (i < p_run.end) {

		Cluster cl;

		cl.start = i;
		cl.end = i;

		uint32_t c = sptr[i];

		//decode surrogates
		if (((c & 0xFFFFF800) == 0xD800) && ((c & 0x400) == 0) && (i + 1 < p_text.length())) {
			uint32_t c2 = sptr[i + 1];
			if ((c2 & 0x400) != 0) {
				c = (c << 10UL) + c2 - ((0xD800 << 10UL) + 0xDC00 - 0x10000);
				cl.end = i + 1;
				i++;
			}
		}

		Glyph gl;

		int fi = 0;
		while (gl.codepoint == 0 && fi < font_impls.size()) {
			gl.codepoint = font_impls[fi]->get_glyph(c);
			cl.font_impl = font_impls[fi];
			if (p_font->has_outline()) cl.font_outline_impl = font_outline_impls[fi];
			fi++;
		}

		if ((gl.codepoint == 0) && (c != 0x0009) && (c != 0x0020)) {
			gl.codepoint = c;
			gl.advance = FontHexBox::get_advance(c);
			cl.font_impl = Ref<FontImplementation>();
			cl.font_outline_impl = Ref<FontImplementation>();
		} else {
			gl.advance = cl.font_impl->get_advance(gl.codepoint);
			p_run.ascent = MAX(p_run.ascent, cl.font_impl->get_ascent());
			p_run.descent = MAX(p_run.descent, cl.font_impl->get_descent());
		}
		gl.offset = Vector2();

		if ((i < p_run.end - 1) && cl.font_impl.is_valid()) {
			uint32_t nc = sptr[i + 1];

			//decode surrogates
			if (((nc & 0xFFFFF800) == 0xD800) && ((nc & 0x400) == 0) && (i + 2 < p_text.length())) {
				uint32_t c2 = sptr[i + 2];
				if ((c2 & 0x400) != 0) {
					nc = (nc << 10UL) + c2 - ((0xD800 << 10UL) + 0xDC00 - 0x10000);
				}
			}
			gl.advance -= cl.font_impl->get_kerning(c, nc);
		}

		cl.repeat = 1;
		cl.flags = GLYPH_VALID;
		if ((c == 0x0020) || ((c >= 0x2000) && (c <= 0x200A)) || (c == 0x3000)) {
			cl.flags |= GLYPH_SPACE;
		}
		if (c == 0x0009) { //replace tab with zero width space
			cl.flags |= GLYPH_TAB;
			gl.codepoint = (cl.font_impl.is_valid()) ? cl.font_impl->get_glyph(0x0020) : 0;
			if (gl.codepoint != 0) cl.flags |= GLYPH_VALID;
			gl.offset = Point2();
			gl.advance = 0;
		}
		if (((c >= 0x0080 && c <= 0x009F) || (c < 0x0020) || (c == 0x007F)) && (c != 0x0009)) {
			cl.flags |= GLYPH_CONTROL;
		}
		if (c > 0xFFFF) cl.flags |= GLYPH_SURROGATE;

		cl.glyphs.push_back(gl);
		cl.advance = gl.advance;

		p_run.width += cl.advance;
		p_run.clusters.push_back(cl);

		i++;
	}

	return true;
}

TextDirection ShapingInterfaceFallback::get_string_direction(const String &p_text) const {
	return TEXT_DIRECTION_LTR;
}
//E VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE

TextDirection ShapingInterfaceFallback::get_locale_direction(const String &p_lang) const {
	return TEXT_DIRECTION_LTR;
}