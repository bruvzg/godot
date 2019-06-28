/*************************************************************************/
/*  font_hex.cpp                                                         */
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

#include "font_hex.h"

Ref<ImageTexture> FontHexBox::hex_box_img_tex = NULL;

bool FontHexBox::is_hidpi = false; //true;
int FontHexBox::hcw = 0;
int FontHexBox::lcw = 0;
int FontHexBox::hch = 0;
int FontHexBox::lch = 0;
int FontHexBox::hca = 0;
int FontHexBox::lca = 0;

struct _control_char {

	uint32_t unicode;
	CharString code;
};

static _control_char ctlmap[] = {
	{ 0x0000, "NUL" },
	{ 0x0001, "SOH" },
	{ 0x0002, "STX" },
	{ 0x0003, "ETX" },
	{ 0x0004, "EOT" },
	{ 0x0005, "ENQ" },
	{ 0x0006, "ACK" },
	{ 0x0007, "BEL" },
	{ 0x0008, "BS" },
	{ 0x0009, "HT" },
	{ 0x000A, "LF" },
	{ 0x000B, "VT" },
	{ 0x000C, "FF" },
	{ 0x000D, "CR" },
	{ 0x000E, "SO" },
	{ 0x000F, "SI" },
	{ 0x0010, "DLE" },
	{ 0x0011, "DC1" },
	{ 0x0012, "DC2" },
	{ 0x0013, "DC3" },
	{ 0x0014, "DC4" },
	{ 0x0015, "NAK" },
	{ 0x0016, "SYN" },
	{ 0x0017, "ETB" },
	{ 0x0018, "CAN" },
	{ 0x0019, "EM" },
	{ 0x001A, "SUB" },
	{ 0x001B, "ESC" },
	{ 0x001C, "FS" },
	{ 0x001D, "GS" },
	{ 0x001E, "RS" },
	{ 0x001F, "US" },
	{ 0x0020, "SP" },
	{ 0x007F, "DEL" },
	{ 0x0080, "PAD" },
	{ 0x0081, "HOP" },
	{ 0x0082, "BPH" },
	{ 0x0083, "NBH" },
	{ 0x0084, "IND" },
	{ 0x0085, "NEL" },
	{ 0x0086, "SSA" },
	{ 0x0087, "ESA" },
	{ 0x0088, "HTS" },
	{ 0x0089, "HTJ" },
	{ 0x008A, "VTS" },
	{ 0x008B, "PLD" },
	{ 0x008C, "PLU" },
	{ 0x008D, "RI" },
	{ 0x008E, "SS2" },
	{ 0x008F, "SS3" },
	{ 0x0090, "DCS" },
	{ 0x0091, "PU1" },
	{ 0x0092, "PU2" },
	{ 0x0093, "STS" },
	{ 0x0094, "CCH" },
	{ 0x0095, "MW" },
	{ 0x0096, "SPA" },
	{ 0x0097, "EPA" },
	{ 0x0098, "SOS" },
	{ 0x0099, "SGCI" },
	{ 0x009A, "SCI" },
	{ 0x009B, "CSI" },
	{ 0x009C, "ST" },
	{ 0x009D, "OSC" },
	{ 0x009E, "PM" },
	{ 0x009F, "APC" },
	{ 0x00A0, "NBSP" },
	{ 0x00AD, "SHY" },
	{ 0x034F, "CGJ" },
	{ 0x061C, "ALM" },
	{ 0x070F, "SAM" },
	{ 0x2000, "NQSP" },
	{ 0x2001, "MQSP" },
	{ 0x2002, "ENSP" },
	{ 0x2003, "EMSP" },
	{ 0x2004, "3MSP" },
	{ 0x2005, "4MSP" },
	{ 0x2006, "6MSP" },
	{ 0x2007, "FSP" },
	{ 0x2008, "PSP" },
	{ 0x2009, "THSP" },
	{ 0x200A, "HSP" },
	{ 0x200B, "ZWS" },
	{ 0x200C, "ZWNJ" },
	{ 0x200D, "ZWJ" },
	{ 0x200E, "LRM" },
	{ 0x200F, "RLM" },
	{ 0x2028, "LS" },
	{ 0x2029, "PS" },
	{ 0x202A, "LRE" },
	{ 0x202B, "RLE" },
	{ 0x202C, "PDF" },
	{ 0x202D, "LRO" },
	{ 0x202E, "RLO" },
	{ 0x202F, "NNBS" },
	{ 0x205F, "MMSP" },
	{ 0x2060, "WJ" },
	{ 0x2066, "LRI" },
	{ 0x2067, "RLI" },
	{ 0x2068, "FSI" },
	{ 0x2069, "PDI" },
	{ 0x206A, "ISS" },
	{ 0x206B, "ASS" },
	{ 0x206C, "IAFS" },
	{ 0x206D, "AAFS" },
	{ 0x206E, "NADS" },
	{ 0x206F, "NODS" },
	{ 0x2D7F, "TCJ" },
	{ 0x3000, "IDSP" },
	{ 0x303E, "IVI" },
	{ 0xFFF9, "IAA" },
	{ 0xFFFA, "IAB" },
	{ 0xFFFB, "IAT" },
	{ 0xFFFC, "OBJ" },
	{ 0x1BC9D, "DTLS" },
	{ 0x1D159, "NLNH" },
	{ 0x1FFFFF, "" }
};

int FontHexBox::get_advance(uint32_t p_glyph) {
	for (int i = 0; ctlmap[i].unicode != 0x1FFFFF; i++) {
		if (ctlmap[i].unicode == p_glyph) {
			return 4 + ((ctlmap[i].code.length() > 2) ? 2 : 1) * ((is_hidpi ? hcw : lcw) + 1);
		}
	}
	return 4 + ((is_hidpi ? hcw : lcw) + 1) * ((p_glyph <= 0xFF) ? 1 : ((p_glyph <= 0xFFFF) ? 2 : 3));
}

Size2 FontHexBox::get_glyph_size(uint32_t p_glyph) {
	for (int i = 0; ctlmap[i].unicode != 0x1FFFFF; i++) {
		if (ctlmap[i].unicode == p_glyph) {
			return Size2(4 + ((ctlmap[i].code.length() > 2) ? 2 : 1) * ((is_hidpi ? hcw : lcw) + 1), 3 + 2 * (is_hidpi ? hch : lch));
		}
	}
	return Size2(4 + ((is_hidpi ? hcw : lcw) + 1) * ((p_glyph <= 0xFF) ? 1 : ((p_glyph <= 0xFFFF) ? 2 : 3)), 3 + 2 * (is_hidpi ? hch : lch));
}

int FontHexBox::get_ascent() {
	return 3 + 2 * (is_hidpi ? hca : lca);
}

int FontHexBox::get_descent() {
	return 2 * (is_hidpi ? (hch - hca) : (lch - lca));
}

int FontHexBox::get_line_gap() {
	return 0;
}

void _ALWAYS_INLINE_ FontHexBox::_draw_hexbox_element(RID p_canvas_item, const Point2 &p_pos, uint8_t p_x, uint8_t p_y, uint8_t p_id, const Color &p_modulate) {
	Rect2 dest = Rect2(p_pos + Point2(3 + p_x * ((is_hidpi ? hcw : lcw) + 1), 2 + p_y * ((is_hidpi ? hch : lch) + 1)), Size2((is_hidpi ? hcw : lcw), (is_hidpi ? hch : lch)));
	VisualServer::get_singleton()->canvas_item_add_texture_rect_region(p_canvas_item, dest, hex_box_img_tex->get_rid(), Rect2(Point2(p_id * (is_hidpi ? hcw : lcw), (is_hidpi ? 0 : hch)), dest.size), p_modulate, false, RID(), false);
}

void FontHexBox::draw_glyph(RID p_canvas_item, const Point2 &p_pos, uint32_t p_glyph, const Color &p_modulate) {
	if (hex_box_img_tex == NULL)
		return;

	//Draw control character abbreviation
	for (int i = 0; ctlmap[i].unicode != 0x1FFFFF; i++) {
		if (ctlmap[i].unicode == p_glyph) {
			int w = 3 + ((ctlmap[i].code.length() > 2) ? 2 : 1) * ((is_hidpi ? hcw : lcw) + 1);
			int h = 3 + 2 * (is_hidpi ? hch : lch);
			Point2 origin = Point2(p_pos.x, p_pos.y - (3 + 2 * (is_hidpi ? hca : lca)));

			VisualServer::get_singleton()->canvas_item_add_rect(p_canvas_item, Rect2(origin + Point2(1, 0), Size2(1, h)), p_modulate);
			VisualServer::get_singleton()->canvas_item_add_rect(p_canvas_item, Rect2(origin + Point2(w, 0), Size2(1, h)), p_modulate);
			VisualServer::get_singleton()->canvas_item_add_rect(p_canvas_item, Rect2(origin + Point2(1, 0), Size2(w, 1)), p_modulate);
			VisualServer::get_singleton()->canvas_item_add_rect(p_canvas_item, Rect2(origin + Point2(1, h), Size2(w, 1)), p_modulate);

			if (ctlmap[i].code.length() == 1) {
				uint8_t a = ctlmap[i].code[0] - ((ctlmap[i].code[0] >= 65) ? 55 : 48);
				_draw_hexbox_element(p_canvas_item, origin + Point2(0, (is_hidpi ? hch : lch) / 2), 0, 0, a, p_modulate);
			} else if (ctlmap[i].code.length() == 2) {
				uint8_t a = ctlmap[i].code[0] - ((ctlmap[i].code[0] >= 65) ? 55 : 48);
				uint8_t b = ctlmap[i].code[1] - ((ctlmap[i].code[1] >= 65) ? 55 : 48);
				_draw_hexbox_element(p_canvas_item, origin, 0, 0, a, p_modulate);
				_draw_hexbox_element(p_canvas_item, origin, 0, 1, b, p_modulate);
			} else if (ctlmap[i].code.length() == 3) {
				uint8_t a = ctlmap[i].code[0] - ((ctlmap[i].code[0] >= 65) ? 55 : 48);
				uint8_t b = ctlmap[i].code[1] - ((ctlmap[i].code[1] >= 65) ? 55 : 48);
				uint8_t c = ctlmap[i].code[2] - ((ctlmap[i].code[2] >= 65) ? 55 : 48);
				_draw_hexbox_element(p_canvas_item, origin, 0, 0, a, p_modulate);
				_draw_hexbox_element(p_canvas_item, origin, 1, 0, b, p_modulate);
				_draw_hexbox_element(p_canvas_item, origin + Point2((is_hidpi ? hcw : lcw) / 2, 0), 0, 1, c, p_modulate);
			} else if (ctlmap[i].code.length() == 4) {
				uint8_t a = ctlmap[i].code[0] - ((ctlmap[i].code[0] >= 65) ? 55 : 48);
				uint8_t b = ctlmap[i].code[1] - ((ctlmap[i].code[1] >= 65) ? 55 : 48);
				uint8_t c = ctlmap[i].code[2] - ((ctlmap[i].code[2] >= 65) ? 55 : 48);
				uint8_t d = ctlmap[i].code[3] - ((ctlmap[i].code[3] >= 65) ? 55 : 48);
				_draw_hexbox_element(p_canvas_item, origin, 0, 0, a, p_modulate);
				_draw_hexbox_element(p_canvas_item, origin, 1, 0, b, p_modulate);
				_draw_hexbox_element(p_canvas_item, origin, 0, 1, c, p_modulate);
				_draw_hexbox_element(p_canvas_item, origin, 1, 1, d, p_modulate);
			}
			return;
		}
	}

	//Draw hex code box
	uint8_t a = p_glyph & 0x0F;
	uint8_t b = (p_glyph >> 4) & 0x0F;
	uint8_t c = (p_glyph >> 8) & 0x0F;
	uint8_t d = (p_glyph >> 12) & 0x0F;
	uint8_t e = (p_glyph >> 16) & 0x0F;
	uint8_t f = (p_glyph >> 20) & 0x0F;

	int w = 3 + ((is_hidpi ? hcw : lcw) + 1) * ((p_glyph <= 0xFF) ? 1 : ((p_glyph <= 0xFFFF) ? 2 : 3));
	int h = 3 + 2 * (is_hidpi ? hch : lch);

	Point2 origin = Point2(p_pos.x, p_pos.y - (3 + 2 * (is_hidpi ? hca : lca)));

	VisualServer::get_singleton()->canvas_item_add_rect(p_canvas_item, Rect2(origin + Point2(1, 0), Size2(1, h)), p_modulate);
	VisualServer::get_singleton()->canvas_item_add_rect(p_canvas_item, Rect2(origin + Point2(w, 0), Size2(1, h)), p_modulate);
	VisualServer::get_singleton()->canvas_item_add_rect(p_canvas_item, Rect2(origin + Point2(1, 0), Size2(w, 1)), p_modulate);
	VisualServer::get_singleton()->canvas_item_add_rect(p_canvas_item, Rect2(origin + Point2(1, h), Size2(w, 1)), p_modulate);

	if (p_glyph <= 0xFF) {
		_draw_hexbox_element(p_canvas_item, origin, 0, 0, b, p_modulate);
		_draw_hexbox_element(p_canvas_item, origin, 0, 1, a, p_modulate);
	} else if (p_glyph <= 0xFFFF) {
		_draw_hexbox_element(p_canvas_item, origin, 0, 0, d, p_modulate);
		_draw_hexbox_element(p_canvas_item, origin, 1, 0, c, p_modulate);
		_draw_hexbox_element(p_canvas_item, origin, 0, 1, b, p_modulate);
		_draw_hexbox_element(p_canvas_item, origin, 1, 1, a, p_modulate);
	} else {
		_draw_hexbox_element(p_canvas_item, origin, 0, 0, f, p_modulate);
		_draw_hexbox_element(p_canvas_item, origin, 1, 0, e, p_modulate);
		_draw_hexbox_element(p_canvas_item, origin, 2, 0, d, p_modulate);
		_draw_hexbox_element(p_canvas_item, origin, 0, 1, c, p_modulate);
		_draw_hexbox_element(p_canvas_item, origin, 1, 1, b, p_modulate);
		_draw_hexbox_element(p_canvas_item, origin, 2, 1, a, p_modulate);
	}
}

void FontHexBox::initialize_hex_font() {
	const unsigned char hex_box_raw_data[480] = {
		/*
			Font subset (0-9 and A-Z characters only) base on Tamsyn font in 10x20 and 5x9 sizes.

			Source: http://www.fial.com/~scott/tamsyn-font/

			License:

			Tamsyn font is free.  You are hereby granted permission to use, copy, modify,
			and distribute it as you see fit.

			Tamsyn font is provided "as is" without any express or implied warranty.

			The author makes no representations about the suitability of this font for
			a particular purpose.

			In no event will the author be held liable for damages arising from the use
			of this font.
		*/
		0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
		0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x13, 0x01, 0x03, 0x00, 0x00, 0x00, 0x8E, 0x77, 0xF2,
		0x73, 0x00, 0x00, 0x00, 0x06, 0x50, 0x4C, 0x54, 0x45, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xA5,
		0xD9, 0x9F, 0xDD, 0x00, 0x00, 0x00, 0x01, 0x74, 0x52, 0x4E, 0x53, 0x00, 0x40, 0xE6, 0xD8, 0x66,
		0x00, 0x00, 0x01, 0x87, 0x49, 0x44, 0x41, 0x54, 0x18, 0x19, 0x7D, 0xC1, 0x3D, 0x6B, 0x13, 0x61,
		0x00, 0xC0, 0xF1, 0xFF, 0x3D, 0xF7, 0x5A, 0x6A, 0xCD, 0x89, 0x0E, 0x15, 0xC5, 0x5E, 0x75, 0x10,
		0xCC, 0x72, 0x60, 0x11, 0x1D, 0x6C, 0x2E, 0x08, 0xE2, 0xE2, 0x67, 0xB0, 0xD6, 0x17, 0x1C, 0x9D,
		0x25, 0xE2, 0x63, 0xED, 0xE2, 0x20, 0x74, 0x56, 0xB0, 0xD7, 0x6F, 0x10, 0x5C, 0x9C, 0x84, 0xAB,
		0x0E, 0x4E, 0xB6, 0xB7, 0x64, 0x71, 0x90, 0x8B, 0x0E, 0x15, 0x41, 0x9A, 0x96, 0xC8, 0x5D, 0xAE,
		0x4F, 0xF2, 0xD8, 0x12, 0xC1, 0x06, 0x89, 0xBF, 0x1F, 0x8D, 0xD9, 0xBC, 0x73, 0x6B, 0xE3, 0x49,
		0xA3, 0xD8, 0xFA, 0xA6, 0x6B, 0xBA, 0xE6, 0x16, 0xD3, 0x61, 0xB0, 0xFE, 0xB8, 0x97, 0x6F, 0xE9,
		0x6C, 0xFD, 0x99, 0x53, 0xE9, 0x58, 0x4E, 0xFD, 0xD8, 0x74, 0x33, 0xDA, 0x59, 0x9A, 0xAA, 0xA0,
		0x56, 0xEA, 0xA2, 0x7F, 0x83, 0xA0, 0x1D, 0x38, 0x53, 0xCE, 0x91, 0x1F, 0x67, 0xD6, 0xD0, 0x46,
		0x28, 0xAC, 0x79, 0x66, 0xA5, 0x6F, 0x60, 0x14, 0x16, 0x09, 0x11, 0x10, 0xF5, 0x3A, 0x0C, 0xE9,
		0x65, 0x02, 0x61, 0x4B, 0xEA, 0x1C, 0x66, 0xC8, 0xB2, 0x00, 0x14, 0xFB, 0x3E, 0xAE, 0x20, 0x5C,
		0x26, 0x16, 0x2B, 0xAB, 0xA2, 0xC4, 0x4C, 0x99, 0x99, 0xA7, 0xC0, 0x06, 0x05, 0x68, 0x68, 0x93,
		0x12, 0x09, 0x9F, 0xCA, 0x5C, 0x7B, 0x4D, 0xDA, 0xBD, 0x9F, 0x21, 0xED, 0xF7, 0x47, 0x3D, 0x40,
		0x0D, 0xE8, 0x9A, 0x9F, 0x24, 0xF8, 0xC4, 0x88, 0x80, 0xC0, 0x8D, 0xB1, 0x70, 0x7D, 0xF6, 0x75,
		0x61, 0x0F, 0x06, 0x78, 0xFD, 0x38, 0x61, 0x97, 0xD0, 0x90, 0x22, 0x21, 0xDA, 0x59, 0xE0, 0x2F,
		0x4B, 0x4B, 0x0E, 0x24, 0x19, 0x70, 0x19, 0x04, 0xC6, 0x32, 0x3E, 0x1E, 0x16, 0x8A, 0x88, 0x04,
		0x48, 0x38, 0x90, 0x41, 0xF7, 0x64, 0xC3, 0x40, 0xEC, 0x7D, 0xC9, 0x53, 0x5D, 0x93, 0xE5, 0x03,
		0x73, 0x73, 0x7B, 0x55, 0x2A, 0xF3, 0x6D, 0xBA, 0x9D, 0x50, 0xEA, 0xEF, 0x3A, 0xD6, 0xF2, 0x34,
		0x76, 0x87, 0x31, 0x02, 0xFE, 0x30, 0x19, 0xA3, 0x60, 0xC8, 0x64, 0x0C, 0xC5, 0x90, 0xB1, 0xF8,
		0x22, 0x8A, 0x72, 0xCF, 0x7F, 0xFA, 0xA8, 0x38, 0x6F, 0xB4, 0x98, 0x38, 0x8B, 0x73, 0x9B, 0xC3,
		0xC4, 0xDC, 0xB9, 0x2B, 0xC7, 0x2F, 0x56, 0x27, 0x9F, 0xFB, 0x4D, 0xFF, 0x6E, 0xB5, 0x3A, 0x09,
		0x1F, 0x18, 0x21, 0xD4, 0x89, 0xEB, 0x0B, 0x1B, 0xF7, 0xB8, 0x7F, 0x89, 0xE0, 0xB5, 0x6D, 0x67,
		0xF4, 0xBF, 0x32, 0x42, 0x70, 0xEA, 0xDA, 0x9B, 0x0B, 0xAF, 0x78, 0xF9, 0xD9, 0xCB, 0x06, 0xA5,
		0x6A, 0xF6, 0xEF, 0x58, 0x8C, 0x10, 0xAD, 0x77, 0x57, 0xD3, 0x96, 0xFD, 0xEB, 0x61, 0x76, 0x33,
		0x27, 0xDD, 0x4C, 0xB3, 0xD8, 0xE5, 0x3F, 0x42, 0xFE, 0xF1, 0x1B, 0xE3, 0x53, 0x8D, 0x26, 0xD8,
		0x83, 0x3A, 0x59, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82, 0x00
	};
	hcw = 7;
	hch = 13;
	hca = 10;

	lcw = 4;
	lch = 6;
	lca = 5;

	PoolByteArray hex_box_data;
	hex_box_data.resize(480);
	memcpy(hex_box_data.write().ptr(), hex_box_raw_data, 480);

	Ref<Image> image;
	image.instance();
	image->load_png_from_buffer(hex_box_data);

	hex_box_img_tex.instance();
	hex_box_img_tex->create_from_image(image, Texture::FLAG_VIDEO_SURFACE);

	//is_hidpi = GLOBAL_DEF("gui/theme/use_hidpi", false);
}

void FontHexBox::finish_hex_font() {
	hex_box_img_tex.unref();
}