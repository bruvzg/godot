/*************************************************************************/
/*  font_implementation_fb.h                                             */
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

#ifndef FONT_IMPLEMENTATION_FB_H
#define FONT_IMPLEMENTATION_FB_H

#include "core/pair.h"
#include "scene/resources/font.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ADVANCES_H
#include FT_MULTIPLE_MASTERS_H
#include FT_TRUETYPE_TABLES_H

/*************************************************************************/
/*  Bitmap Font                                                          */
/*************************************************************************/

class FontImplementationBitmap : public FontImplementation {
	GDCLASS(FontImplementationBitmap, FontImplementation);

protected:
	struct Glyph {

		int texture_idx;
		Rect2 rect;
		float v_align;
		float h_align;
		float advance;

		Glyph() {
			texture_idx = 0;
			v_align = 0;
		}
	};

	struct KerningPairKey {

		union {
			struct {
				uint32_t A, B;
			};

			uint64_t pair;
		};

		_FORCE_INLINE_ bool operator<(const KerningPairKey &p_r) const { return pair < p_r.pair; }
	};

	bool valid;

	float ascent;
	float height;

	String name;

	HashMap<uint32_t, int32_t> supported_scripts;
	Vector<String> supported_langs;

	Vector<Ref<Texture>> textures;
	HashMap<uint32_t, Glyph> glyph_map;
	Map<KerningPairKey, int> kerning_map;

	void clear();

public:
	virtual void invalidate();
	virtual bool require_reload() const;

	virtual void *get_native_handle() const;
	virtual float get_font_scale() const;

	virtual uint32_t get_glyph(uint32_t p_unicode, uint32_t p_variation_selector = 0) const;

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

	//construct
	virtual Error create(const FontData *p_data, FontData::CacheID p_cache_id);

	virtual void set_name(const String &p_name);
	virtual void set_ascent(float p_ascent);
	virtual void set_height(float p_height);
	virtual void add_texture(const Ref<Texture> &p_texture);
	virtual void add_glyph(uint32_t p_glyph, int p_texture_idx, const Rect2 &p_rect, const Size2 &p_align, int p_advance = -1);
	virtual void add_kerning_pair(uint32_t p_A, uint32_t p_B, int p_kerning);

	FontImplementationBitmap();
	virtual ~FontImplementationBitmap();
};

/*************************************************************************/
/*  Dynamic Font                                                         */
/*************************************************************************/

#ifdef MODULE_FREETYPE_ENABLED

class FontImplementationDynamic : public FontImplementation {
	GDCLASS(FontImplementationDynamic, FontImplementation);

protected:
	FT_Library library;
	FT_Face face;
	FT_StreamRec stream;

	HashMap<uint32_t, int32_t> supported_scripts;
	Vector<String> supported_langs;

	String name;

	float ascent;
	float descent;
	float linegap;

	float underline_position;
	float underline_thickness;

	int rect_margin;

	float scale_color_font;
	float oversampling;

	uint32_t texture_flags;

	bool valid;

	struct GlyphTexture {
		PoolVector<uint8_t> imgdata;
		int texture_size;
		Vector<int> offsets;
		Ref<ImageTexture> texture;
	};

	Vector<GlyphTexture> textures;

	struct Glyph {
		bool found;
		int texture_idx;

		Rect2 rect;
		Rect2 rect_uv;

		Point2 align;

		float advance;

		Glyph() {
			texture_idx = 0;
			advance = 0.f;
		}

		static Glyph not_found();
	};

	struct TexturePosition {
		int index;
		int x;
		int y;
	};

	Glyph _make_outline_glyph(uint32_t p_index);
	TexturePosition _find_texture_pos_for_glyph(int p_color_size, Image::Format p_image_format, int p_width, int p_height);
	Glyph _bitmap_to_glyph(FT_Bitmap bitmap, int yofs, int xofs, float h_advance);

	_FORCE_INLINE_ void _update_glyph(uint32_t p_index);

	static unsigned long _ft_stream_io(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count);
	static void _ft_stream_close(FT_Stream stream);

	HashMap<uint32_t, Glyph> glyph_map;

	FontData::CacheID id;

	static HashMap<String, Vector<uint8_t>> _fontdata;

	void clear();

public:
	virtual void invalidate();
	virtual bool require_reload() const;

	virtual void *get_native_handle() const;
	virtual float get_font_scale() const;

	virtual uint32_t get_glyph(uint32_t p_unicode, uint32_t p_variation_selector = 0) const;

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

	//construct
	virtual Error create(const FontData *p_data, FontData::CacheID p_cache_id);

	FontImplementationDynamic();
	virtual ~FontImplementationDynamic();
};

#endif

#endif
