/*************************************************************************/
/*  font_implementation_gdn.h                                            */
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

#ifndef FONT_IMPLEMENTATION_GDN_H
#define FONT_IMPLEMENTATION_GDN_H

#include "core/pair.h"
#include "scene/resources/font.h"
#include "shaping_interface_gdn.h"

class FontImplementationGDN : public FontImplementation {
	GDCLASS(FontImplementationGDN, FontImplementation);

public:
	const godot_shaping_interface_gdnative *interface;
	const void *interface_data;
	void *font_data;
	bool valid;

public:
	virtual void invalidate();
	virtual bool require_reload() const;

	virtual void *get_native_handle() const;
	virtual float get_font_scale() const;

	virtual uint32_t get_glyph(uint32_t p_unicode, uint32_t p_variation_selector) const;

	virtual float get_advance(uint32_t p_glyph) const;
	virtual float get_kerning(uint32_t p_glyph_a, uint32_t p_glyph_b) const;

	virtual float get_v_advance(uint32_t p_glyph) const;
	virtual float get_v_kerning(uint32_t p_glyph_a, uint32_t p_glyph_b) const;

	virtual Size2 get_glyph_size(uint32_t p_glyph) const;

	virtual float get_ascent() const;
	virtual float get_descent() const;
	virtual float get_line_gap() const;

	virtual float get_underline_position() const;
	virtual float get_underline_thickness() const;

	virtual String get_name() const;

	virtual int32_t get_script_support_priority(uint32_t p_script) const;
	virtual int32_t get_language_support_priority(const String &p_lang) const;

	virtual void draw_glyph(RID p_canvas_item, const Point2 &p_pos, uint32_t p_glyph, const Color &p_modulate = Color(1, 1, 1), bool p_rotate_cw = false) const;

	virtual Error create(const FontData *p_data, FontData::CacheID p_cache_id);

	FontImplementationGDN();
	~FontImplementationGDN();
};

#endif
