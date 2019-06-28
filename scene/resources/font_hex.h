/*************************************************************************/
/*  font_hex.h                                                           */
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

#include "core/reference.h"
#include "scene/resources/texture.h"

#ifndef FONT_HEX_H
#define FONT_HEX_H

class FontHexBox : public Reference {
	GDCLASS(FontHexBox, Reference);

protected:
	static Ref<ImageTexture> hex_box_img_tex;
	static int hcw, lcw, hch, lch, hca, lca;
	static bool is_hidpi;

	static void _ALWAYS_INLINE_ _draw_hexbox_element(RID p_canvas_item, const Point2 &p_pos, uint8_t p_x, uint8_t p_y, uint8_t p_id, const Color &p_modulate);

public:
	static int get_advance(uint32_t p_glyph);
	static Size2 get_glyph_size(uint32_t p_glyph);

	static int get_ascent();
	static int get_descent();
	static int get_line_gap();

	static void draw_glyph(RID p_canvas_item, const Point2 &p_pos, uint32_t p_glyph, const Color &p_modulate = Color(1, 1, 1));

	static void initialize_hex_font();
	static void finish_hex_font();
};

#endif
