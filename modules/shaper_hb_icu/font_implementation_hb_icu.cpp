/*************************************************************************/
/*  font_implementation_hb_icu.cpp                                       */
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

#include "font_implementation_hb_icu.h"

#include <unicode/uchar.h>

#include <hb-ft.h>
#include <hb-icu.h>

/*************************************************************************/
/*  Common                                                               */
/*************************************************************************/

struct _chars_per_script {

	uint32_t script;
	uint32_t chars;
};

static _chars_per_script charmap[] = {

	{ TAG('A', 'd', 'l', 'm'), 88 },
	{ TAG('A', 'g', 'h', 'b'), 53 },
	{ TAG('A', 'h', 'o', 'm'), 58 },
	{ TAG('A', 'r', 'a', 'b'), 1281 },
	{ TAG('A', 'r', 'm', 'i'), 31 },
	{ TAG('A', 'r', 'm', 'n'), 95 },
	{ TAG('A', 'v', 's', 't'), 61 },
	{ TAG('B', 'a', 'l', 'i'), 121 },
	{ TAG('B', 'a', 'm', 'u'), 657 },
	{ TAG('B', 'a', 's', 's'), 36 },
	{ TAG('B', 'a', 't', 'k'), 56 },
	{ TAG('B', 'e', 'n', 'g'), 96 },
	{ TAG('B', 'h', 'k', 's'), 97 },
	{ TAG('B', 'o', 'p', 'o'), 72 },
	{ TAG('B', 'r', 'a', 'h'), 109 },
	{ TAG('B', 'r', 'a', 'i'), 256 },
	{ TAG('B', 'u', 'g', 'i'), 30 },
	{ TAG('B', 'u', 'h', 'd'), 20 },
	{ TAG('C', 'a', 'k', 'm'), 70 },
	{ TAG('C', 'a', 'n', 's'), 710 },
	{ TAG('C', 'a', 'r', 'i'), 49 },
	{ TAG('C', 'h', 'a', 'm'), 83 },
	{ TAG('C', 'h', 'e', 'r'), 172 },
	{ TAG('C', 'o', 'p', 't'), 137 },
	{ TAG('C', 'p', 'r', 't'), 55 },
	{ TAG('C', 'y', 'r', 'l'), 443 },
	{ TAG('D', 'e', 'v', 'a'), 154 },
	{ TAG('D', 'o', 'g', 'r'), 60 },
	{ TAG('D', 's', 'r', 't'), 80 },
	{ TAG('D', 'u', 'p', 'l'), 143 },
	{ TAG('E', 'g', 'y', 'p'), 1080 },
	{ TAG('E', 'l', 'b', 'a'), 40 },
	{ TAG('E', 'l', 'y', 'm'), 23 },
	{ TAG('E', 't', 'h', 'i'), 495 },
	{ TAG('G', 'e', 'o', 'r'), 173 },
	{ TAG('G', 'l', 'a', 'g'), 132 },
	{ TAG('G', 'o', 'n', 'g'), 63 },
	{ TAG('G', 'o', 'n', 'm'), 75 },
	{ TAG('G', 'o', 't', 'h'), 27 },
	{ TAG('G', 'r', 'a', 'n'), 85 },
	{ TAG('G', 'r', 'e', 'k'), 518 },
	{ TAG('G', 'u', 'j', 'r'), 91 },
	{ TAG('G', 'u', 'r', 'u'), 80 },
	{ TAG('H', 'a', 'n', 'g'), 11739 },
	{ TAG('H', 'a', 'n', 'i'), 89233 },
	{ TAG('H', 'a', 'n', 'o'), 21 },
	{ TAG('H', 'a', 't', 'r'), 26 },
	{ TAG('H', 'e', 'b', 'r'), 134 },
	{ TAG('H', 'i', 'r', 'a'), 379 },
	{ TAG('H', 'l', 'u', 'w'), 583 },
	{ TAG('H', 'm', 'n', 'g'), 127 },
	{ TAG('H', 'm', 'n', 'p'), 71 },
	{ TAG('H', 'u', 'n', 'g'), 108 },
	{ TAG('I', 't', 'a', 'l'), 39 },
	{ TAG('J', 'a', 'v', 'a'), 90 },
	{ TAG('K', 'a', 'l', 'i'), 47 },
	{ TAG('K', 'a', 'n', 'a'), 304 },
	{ TAG('K', 'h', 'a', 'r'), 68 },
	{ TAG('K', 'h', 'm', 'r'), 146 },
	{ TAG('K', 'h', 'o', 'j'), 62 },
	{ TAG('K', 'n', 'd', 'a'), 89 },
	{ TAG('K', 't', 'h', 'i'), 67 },
	{ TAG('L', 'a', 'n', 'a'), 127 },
	{ TAG('L', 'a', 'o', 'o'), 82 },
	{ TAG('L', 'a', 't', 'n'), 366 },
	{ TAG('L', 'e', 'p', 'c'), 74 },
	{ TAG('L', 'i', 'm', 'b'), 68 },
	{ TAG('L', 'i', 'n', 'a'), 341 },
	{ TAG('L', 'i', 'n', 'b'), 211 },
	{ TAG('L', 'i', 's', 'u'), 48 },
	{ TAG('L', 'y', 'c', 'i'), 29 },
	{ TAG('L', 'y', 'd', 'i'), 27 },
	{ TAG('M', 'a', 'h', 'j'), 39 },
	{ TAG('M', 'a', 'k', 'a'), 25 },
	{ TAG('M', 'a', 'n', 'd'), 29 },
	{ TAG('M', 'a', 'n', 'i'), 51 },
	{ TAG('M', 'a', 'r', 'c'), 68 },
	{ TAG('M', 'e', 'd', 'f'), 91 },
	{ TAG('M', 'e', 'n', 'd'), 213 },
	{ TAG('M', 'e', 'r', 'c'), 90 },
	{ TAG('M', 'e', 'r', 'o'), 32 },
	{ TAG('M', 'l', 'y', 'm'), 117 },
	{ TAG('M', 'o', 'd', 'i'), 79 },
	{ TAG('M', 'o', 'n', 'g'), 167 },
	{ TAG('M', 'r', 'o', 'o'), 43 },
	{ TAG('M', 't', 'e', 'i'), 79 },
	{ TAG('M', 'u', 'l', 't'), 38 },
	{ TAG('M', 'y', 'm', 'r'), 223 },
	{ TAG('N', 'a', 'n', 'd'), 65 },
	{ TAG('N', 'a', 'r', 'b'), 32 },
	{ TAG('N', 'b', 'a', 't'), 40 },
	{ TAG('N', 'e', 'w', 'a'), 94 },
	{ TAG('N', 'k', 'o', 'o'), 62 },
	{ TAG('N', 's', 'h', 'u'), 397 },
	{ TAG('O', 'g', 'a', 'm'), 29 },
	{ TAG('O', 'l', 'c', 'k'), 48 },
	{ TAG('O', 'r', 'k', 'h'), 73 },
	{ TAG('O', 'r', 'y', 'a'), 90 },
	{ TAG('O', 's', 'g', 'e'), 72 },
	{ TAG('O', 's', 'm', 'a'), 40 },
	{ TAG('P', 'a', 'l', 'm'), 32 },
	{ TAG('P', 'a', 'u', 'c'), 57 },
	{ TAG('P', 'e', 'r', 'm'), 43 },
	{ TAG('P', 'h', 'a', 'g'), 56 },
	{ TAG('P', 'h', 'l', 'i'), 27 },
	{ TAG('P', 'h', 'l', 'p'), 29 },
	{ TAG('P', 'h', 'n', 'x'), 29 },
	{ TAG('P', 'l', 'r', 'd'), 149 },
	{ TAG('P', 'r', 't', 'i'), 30 },
	{ TAG('R', 'j', 'n', 'g'), 37 },
	{ TAG('R', 'o', 'h', 'g'), 50 },
	{ TAG('R', 'u', 'n', 'r'), 86 },
	{ TAG('S', 'a', 'm', 'r'), 61 },
	{ TAG('S', 'a', 'r', 'b'), 32 },
	{ TAG('S', 'a', 'u', 'r'), 82 },
	{ TAG('S', 'g', 'n', 'w'), 672 },
	{ TAG('S', 'h', 'a', 'w'), 48 },
	{ TAG('S', 'h', 'r', 'd'), 94 },
	{ TAG('S', 'i', 'd', 'd'), 92 },
	{ TAG('S', 'i', 'n', 'd'), 69 },
	{ TAG('S', 'i', 'n', 'h'), 110 },
	{ TAG('S', 'o', 'g', 'd'), 42 },
	{ TAG('S', 'o', 'g', 'o'), 40 },
	{ TAG('S', 'o', 'r', 'a'), 35 },
	{ TAG('S', 'o', 'y', 'o'), 83 },
	{ TAG('S', 'u', 'n', 'd'), 72 },
	{ TAG('S', 'y', 'l', 'o'), 44 },
	{ TAG('S', 'y', 'r', 'c'), 88 },
	{ TAG('T', 'a', 'g', 'b'), 18 },
	{ TAG('T', 'a', 'k', 'r'), 67 },
	{ TAG('T', 'a', 'l', 'e'), 35 },
	{ TAG('T', 'a', 'l', 'u'), 83 },
	{ TAG('T', 'a', 'm', 'l'), 123 },
	{ TAG('T', 'a', 'n', 'g'), 6892 },
	{ TAG('T', 'a', 'v', 't'), 72 },
	{ TAG('T', 'e', 'l', 'u'), 98 },
	{ TAG('T', 'f', 'n', 'g'), 59 },
	{ TAG('T', 'g', 'l', 'g'), 20 },
	{ TAG('T', 'h', 'a', 'a'), 50 },
	{ TAG('T', 'h', 'a', 'i'), 86 },
	{ TAG('T', 'i', 'b', 't'), 207 },
	{ TAG('T', 'i', 'r', 'h'), 82 },
	{ TAG('U', 'g', 'a', 'r'), 31 },
	{ TAG('V', 'a', 'i', 'i'), 300 },
	{ TAG('W', 'a', 'r', 'a'), 84 },
	{ TAG('W', 'c', 'h', 'o'), 59 },
	{ TAG('X', 'p', 'e', 'o'), 50 },
	{ TAG('X', 's', 'u', 'x'), 1234 },
	{ TAG('Y', 'i', 'i', 'i'), 1220 },
	{ TAG('Z', 'a', 'n', 'b'), 72 },
	{ 0, 0 }
};

/*************************************************************************/
/*  Bitmap Font                                                          */
/*************************************************************************/

struct hb_bmp_font_t {
	const FontImplementationBitmapHBICU *bm_face;
};

static hb_bmp_font_t *_hb_bmp_font_create(const FontImplementationBitmapHBICU *bm_face) {
	hb_bmp_font_t *bm_font = reinterpret_cast<hb_bmp_font_t *>(memalloc(sizeof(hb_bmp_font_t)));

	if (!bm_font)
		return nullptr;

	bm_font->bm_face = bm_face;

	return bm_font;
}

static void _hb_bmp_font_destroy(void *data) {
	hb_bmp_font_t *bm_font = reinterpret_cast<hb_bmp_font_t *>(data);
	memfree(bm_font);
}

static hb_bool_t hb_bmp_get_nominal_glyph(hb_font_t *font, void *font_data, hb_codepoint_t unicode, hb_codepoint_t *glyph, void *user_data) {
	const hb_bmp_font_t *bm_font = reinterpret_cast<const hb_bmp_font_t *>(font_data);

	if (!bm_font->bm_face)
		return false;

	*glyph = bm_font->bm_face->get_glyph(unicode);
	return true;
}

static hb_position_t hb_bmp_get_glyph_h_advance(hb_font_t *font, void *font_data, hb_codepoint_t glyph, void *user_data) {
	const hb_bmp_font_t *bm_font = reinterpret_cast<const hb_bmp_font_t *>(font_data);

	if (!bm_font->bm_face)
		return 0;

	return bm_font->bm_face->get_advance(glyph) * 64;
}

static hb_position_t hb_bmp_get_glyph_h_kerning(hb_font_t *font, void *font_data, hb_codepoint_t left_glyph, hb_codepoint_t right_glyph, void *user_data) {
	const hb_bmp_font_t *bm_font = reinterpret_cast<const hb_bmp_font_t *>(font_data);

	if (!bm_font->bm_face)
		return 0;

	return bm_font->bm_face->get_kerning(left_glyph, right_glyph) * 64;
}

static hb_bool_t hb_bmp_get_glyph_v_origin(hb_font_t *font, void *font_data, hb_codepoint_t glyph, hb_position_t *x, hb_position_t *y, void *user_data) {
	const hb_bmp_font_t *bm_font = reinterpret_cast<const hb_bmp_font_t *>(font_data);

	if (!bm_font->bm_face)
		return false;

	*x = 0;
	*y = 0;

	return true;
}

static hb_bool_t hb_bmp_get_glyph_extents(hb_font_t *font, void *font_data, hb_codepoint_t glyph, hb_glyph_extents_t *extents, void *user_data) {
	const hb_bmp_font_t *bm_font = reinterpret_cast<const hb_bmp_font_t *>(font_data);

	if (!bm_font->bm_face)
		return false;

	extents->x_bearing = 0;
	extents->y_bearing = 0;
	extents->width = bm_font->bm_face->get_glyph_size(glyph).x * 64;
	extents->height = bm_font->bm_face->get_glyph_size(glyph).y * 64;

	return true;
}

static hb_bool_t hb_bmp_get_font_h_extents(hb_font_t *font, void *font_data, hb_font_extents_t *metrics, void *user_data) {
	const hb_bmp_font_t *bm_font = reinterpret_cast<const hb_bmp_font_t *>(font_data);

	if (!bm_font->bm_face)
		return false;

	metrics->ascender = bm_font->bm_face->get_ascent();
	metrics->descender = bm_font->bm_face->get_descent();
	metrics->line_gap = bm_font->bm_face->get_line_gap();

	return true;
}

static hb_font_funcs_t *_hb_bmp_get_font_funcs(void) {

	hb_font_funcs_t *funcs = hb_font_funcs_create();

	hb_font_funcs_set_font_h_extents_func(funcs, hb_bmp_get_font_h_extents, nullptr, nullptr);
	//hb_font_funcs_set_font_v_extents_func (funcs, hb_bmp_get_font_v_extents, nullptr, nullptr);
	hb_font_funcs_set_nominal_glyph_func(funcs, hb_bmp_get_nominal_glyph, nullptr, nullptr);
	//hb_font_funcs_set_variation_glyph_func (funcs, hb_bmp_get_variation_glyph, nullptr, nullptr);
	hb_font_funcs_set_glyph_h_advance_func(funcs, hb_bmp_get_glyph_h_advance, nullptr, nullptr);
	//hb_font_funcs_set_glyph_v_advance_func (funcs, hb_bmp_get_glyph_v_advance, nullptr, nullptr);
	//hb_font_funcs_set_glyph_h_origin_func (funcs, hb_bmp_get_glyph_h_origin, nullptr, nullptr);
	hb_font_funcs_set_glyph_v_origin_func(funcs, hb_bmp_get_glyph_v_origin, nullptr, nullptr);
	hb_font_funcs_set_glyph_h_kerning_func(funcs, hb_bmp_get_glyph_h_kerning, nullptr, nullptr);
	//hb_font_funcs_set_glyph_v_kerning_func (funcs, hb_bmp_get_glyph_v_kerning, nullptr, nullptr);
	hb_font_funcs_set_glyph_extents_func(funcs, hb_bmp_get_glyph_extents, nullptr, nullptr);
	//hb_font_funcs_set_glyph_contour_point_func (funcs, hb_bmp_get_glyph_contour_point, nullptr, nullptr);
	//hb_font_funcs_set_glyph_name_func (funcs, hb_bmp_get_glyph_name, nullptr, nullptr);
	//hb_font_funcs_set_glyph_from_name_func (funcs, hb_bmp_get_glyph_from_name, nullptr, nullptr);

	hb_font_funcs_make_immutable(funcs);

	return funcs;
}

void *FontImplementationBitmapHBICU::get_native_handle() const {

	if (hb_font) return hb_font;

	hb_face_t *face = hb_face_create(NULL, 0);

	const_cast<FontImplementationBitmapHBICU *>(this)->hb_font = hb_font_create(face);
	hb_face_destroy(face);

	hb_font_set_funcs(hb_font, _hb_bmp_get_font_funcs(), _hb_bmp_font_create(this), _hb_bmp_font_destroy);

	return hb_font;
}

Error FontImplementationBitmapHBICU::create(const FontData *p_data, FontData::CacheID p_cache_id) {
	Error status = FontImplementationBitmap::create(p_data, p_cache_id);

	if (status == OK) {
		//get script support data
		if (p_data->get_force_supported_scripts() == "") {
			const uint32_t *k = NULL;
			Map<uint32_t, int32_t> blockranges;
			while ((k = glyph_map.next(k))) {
				UErrorCode err = U_ZERO_ERROR;
				uint32_t scr = hb_icu_script_to_script(uscript_getScript(*k, &err));
				if (blockranges.has(scr)) {
					blockranges[scr]++;
				} else {
					blockranges[scr] = 1;
				}
			}
			for (int i = 0; charmap[i].script != 0; i++) {
				if (blockranges.has(charmap[i].script)) {
					scr_sup_up(charmap[i].script, (blockranges[charmap[i].script] * 10000) / (charmap[i].chars));
				}
			}
		} else {
			Vector<String> script_list = p_data->get_force_supported_scripts().split(",");
			for (int32_t k = 0; k < script_list.size(); k++) {
				scr_sup_up(tag_from_string(script_list[k].strip_edges()), 10000);
			}
		}
	}

	// char buf[6] = "     ";
	// printf("BMP: %s: ", p_data->get_font_data_path().ascii().get_data());
	// const uint32_t *k = NULL;
	// while ((k = supported_scripts.next(k))) {
	// 	buf[0] = (char)(uint8_t)(*k >> 24);
	// 	buf[1] = (char)(uint8_t)(*k >> 16);
	// 	buf[2] = (char)(uint8_t)(*k >> 8);
	// 	buf[3] = (char)(uint8_t)(*k >> 0);
	// 	buf[4] = 0;
	// 	printf("%s(%d), ", buf, supported_scripts[*k]);
	// }
	// printf("\n");

	return status;
}

FontImplementationBitmapHBICU::FontImplementationBitmapHBICU() {
	hb_font = NULL;
}

FontImplementationBitmapHBICU::~FontImplementationBitmapHBICU() {
	if (hb_font) {
		hb_font_destroy(hb_font);
		hb_font = NULL;
	}
}

/*************************************************************************/
/*  Dynamic Font                                                         */
/*************************************************************************/

#ifdef MODULE_FREETYPE_ENABLED

void *FontImplementationDynamicHBICU::get_native_handle() const {

	if (hb_font) return hb_font;
	const_cast<FontImplementationDynamicHBICU *>(this)->hb_font = hb_ft_font_create(face, NULL);

	int ft_hinting;

	switch (id.hinting) {
		case FontData::HINTING_NONE:
			ft_hinting = FT_LOAD_NO_HINTING;
			break;
		case FontData::HINTING_LIGHT:
			ft_hinting = FT_LOAD_TARGET_LIGHT;
			break;
		default:
			ft_hinting = FT_LOAD_TARGET_NORMAL;
			break;
	}
	hb_ft_font_set_load_flags(hb_font, FT_LOAD_DEFAULT | (id.force_autohinter ? FT_LOAD_FORCE_AUTOHINT : 0) | ft_hinting);

	return hb_font;
}

Error FontImplementationDynamicHBICU::create(const FontData *p_data, FontData::CacheID p_cache_id) {
	Error status = FontImplementationDynamic::create(p_data, p_cache_id);

	if (status == OK) {
		supported_scripts.clear();
		if (p_data->get_force_supported_scripts() == "") {
#pragma pack(push, 1)
			struct OT_Meta_DataMap {
				uint32_t tag;
				uint32_t dataOffset;
				uint32_t dataLength;
			};

			struct OT_Meta_Table {
				uint32_t version;
				uint32_t flags;
				uint32_t reserved;
				uint32_t dataMapsCount;
				OT_Meta_DataMap dataMaps[0];
			};
#pragma pack(pop)

			//Load "meta", "GPOS" and "GSUB" tables to get supported scripts
			FT_ULong tcnt;
			FT_Sfnt_Table_Info(face, 0, NULL, &tcnt);
			for (FT_ULong i = 0; i < tcnt; i++) {
				FT_ULong tag;
				FT_ULong length;
				FT_Sfnt_Table_Info(face, i, &tag, &length);
				if ((tag == TAG('M', 'E', 'T', 'A')) || (tag == TAG('m', 'e', 't', 'a'))) {
					OT_Meta_Table *meta = (OT_Meta_Table *)memalloc(length);
					FT_ULong error = FT_Load_Sfnt_Table(face, tag, 0, (FT_Byte *)meta, &length);
					if (!error) {
						for (int32_t j = 0; j < BSWAP32(meta->dataMapsCount); j++) {
							if ((BSWAP32(meta->dataMaps[j].tag) == TAG('s', 'l', 'n', 'g')) || (BSWAP32(meta->dataMaps[j].tag) == TAG('d', 'l', 'n', 'g'))) {
								String scripts;
								scripts.parse_utf8((const char *)((FT_Byte *)(meta) + BSWAP32(meta->dataMaps[j].dataOffset)), BSWAP32(meta->dataMaps[j].dataLength));
								Vector<String> script_list = scripts.split(",");
								for (int32_t k = 0; k < script_list.size(); k++) {
									scr_sup_up(tag_from_string(script_list[k].strip_edges()), 5000);
								}
							}
						}
						memfree(meta);
					}
				}
			}

			//Scan font for actually supported ranges
			Map<uint32_t, int32_t> blockranges;

			FT_ULong charcode;
			FT_UInt gindex;
			charcode = FT_Get_First_Char(face, &gindex);
			while (gindex != 0) {
				UErrorCode err = U_ZERO_ERROR;
				uint32_t scr = hb_icu_script_to_script(uscript_getScript(charcode, &err));
				if (blockranges.has(scr)) {
					blockranges[scr]++;
				} else {
					blockranges[scr] = 1;
				}
				charcode = FT_Get_Next_Char(face, charcode, &gindex);
			}
			for (int i = 0; charmap[i].script != 0; i++) {
				if (blockranges.has(charmap[i].script)) {
					scr_sup_up(charmap[i].script, (blockranges[charmap[i].script] * 10000) / (charmap[i].chars));
				}
			}
		} else {
			Vector<String> script_list = p_data->get_force_supported_scripts().split(",");
			for (int32_t k = 0; k < script_list.size(); k++) {
				scr_sup_up(tag_from_string(script_list[k].strip_edges()), 10000);
			}
		}

		// char buf[6] = "     ";
		// printf("DYN: %s: ", p_data->get_font_data_path().ascii().get_data());
		// const uint32_t *k = NULL;
		// while ((k = supported_scripts.next(k))) {
		// 	buf[0] = (char)(uint8_t)(*k >> 24);
		// 	buf[1] = (char)(uint8_t)(*k >> 16);
		// 	buf[2] = (char)(uint8_t)(*k >> 8);
		// 	buf[3] = (char)(uint8_t)(*k >> 0);
		// 	buf[4] = 0;
		// 	printf("%s(%d), ", buf, supported_scripts[*k]);
		// }
		// printf("\n");
	}

	return status;
}

FontImplementationDynamicHBICU::FontImplementationDynamicHBICU() {
	hb_font = NULL;
}

FontImplementationDynamicHBICU::~FontImplementationDynamicHBICU() {
	if (hb_font) {
		hb_font_destroy(hb_font);
		hb_font = NULL;
	}
}

#endif
