/*************************************************************************/
/*  font.h                                                               */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef FONT_H
#define FONT_H

#include "core/lru.h"
#include "core/map.h"
#include "core/resource.h"
#include "scene/resources/texture.h"
#include "servers/text_server.h"

class TextLine;
class TextParagraph;

class Font : public Resource {
	GDCLASS(Font, Resource);

protected:
	static void _bind_methods();

public:
	enum SpacingType {
		SPACING_TOP,
		SPACING_BOTTOM,
		SPACING_CHAR,
		SPACING_SPACE
	};

	mutable LRUCache<uint64_t, Ref<TextLine> > cache;
	mutable LRUCache<uint64_t, Ref<TextParagraph> > cache_wrap;

	int base_size = 16;
	int outline_size = 0;
	int spacing_top = 0;
	int spacing_bottom = 0;

	virtual float get_height() const = 0;

	virtual float get_ascent() const = 0;
	virtual float get_descent() const = 0;

	int get_spacing(int p_type) const;

	virtual Size2 get_char_size(char32_t p_char, char32_t p_next = 0) const = 0;
	Size2 get_string_size(const String &p_string) const;
	Size2 get_wordwrap_string_size(const String &p_string, float p_width) const;

	virtual bool is_distance_field_hint() const = 0;

	void draw(RID p_canvas_item, const Point2 &p_pos, const String &p_text, const Color &p_modulate = Color(1, 1, 1), int p_clip_w = -1, const Color &p_outline_modulate = Color(1, 1, 1)) const;
	void draw_halign(RID p_canvas_item, const Point2 &p_pos, HAlign p_align, float p_width, const String &p_text, const Color &p_modulate = Color(1, 1, 1), const Color &p_outline_modulate = Color(1, 1, 1)) const;
	void draw_wordwrap_string(RID p_canvas_item, const Point2 &p_pos, HAlign p_align, float p_width, const String &p_text, const Color &p_modulate = Color(1, 1, 1), const Color &p_outline_modulate = Color(1, 1, 1)) const;

	virtual bool has_outline() const { return false; }
	virtual float draw_char(RID p_canvas_item, const Point2 &p_pos, char32_t p_char, char32_t p_next = 0, const Color &p_modulate = Color(1, 1, 1), bool p_outline = false) const = 0;

	virtual Vector<RID> get_rids() const = 0;

	void update_changes();

	Font();
	~Font();
};

VARIANT_ENUM_CAST(Font::SpacingType);

// Helper class to that draws outlines immediately and draws characters in its destructor. //REMOVE
class FontDrawer {
	const Ref<Font> &font;
	Color outline_color;
	bool has_outline;

	struct PendingDraw {
		RID canvas_item;
		Point2 pos;
		char32_t chr;
		char32_t next;
		Color modulate;
	};

	Vector<PendingDraw> pending_draws;

public:
	FontDrawer(const Ref<Font> &p_font, const Color &p_outline_color) :
			font(p_font),
			outline_color(p_outline_color) {
		has_outline = p_font->has_outline();
	}

	float draw_char(RID p_canvas_item, const Point2 &p_pos, char32_t p_char, char32_t p_next = 0, const Color &p_modulate = Color(1, 1, 1)) {
		if (has_outline) {
			PendingDraw draw = { p_canvas_item, p_pos, p_char, p_next, p_modulate };
			pending_draws.push_back(draw);
		}
		return font->draw_char(p_canvas_item, p_pos, p_char, p_next, has_outline ? outline_color : p_modulate, has_outline);
	}

	~FontDrawer() {
		for (int i = 0; i < pending_draws.size(); ++i) {
			const PendingDraw &draw = pending_draws[i];
			font->draw_char(draw.canvas_item, draw.pos, draw.chr, draw.next, draw.modulate, false);
		}
	}
};

class BitmapFont : public Font {
	GDCLASS(BitmapFont, Font);
	RES_BASE_EXTENSION("font");

	RID rid;
	Ref<BitmapFont> fallback;

	void _set_chars(const PoolVector<int> &p_chars);
	PoolVector<int> _get_chars() const;
	void _set_kernings(const PoolVector<int> &p_kernings);
	PoolVector<int> _get_kernings() const;
	void _set_textures(const Vector<Variant> &p_textures);
	Vector<Variant> _get_textures() const;

protected:
	static void _bind_methods();

public:
	Error create_from_memory(const uint8_t *p_data, size_t p_size);
	Error create_from_fnt(const String &p_file);

	Dictionary get_variation_list() const;
	void set_variation(const String &p_name, double p_value);
	double get_variation(const String &p_name) const;

	void set_height(float p_height);
	void set_ascent(float p_ascent);
	void add_texture(const Ref<Texture> &p_texture);
	void add_char(char32_t p_char, int p_texture_idx, const Rect2 &p_rect, const Size2 &p_align, float p_advance = -1);
	int get_texture_count() const;
	Ref<Texture> get_texture(int p_idx) const;

	void add_kerning_pair(char32_t p_A, char32_t p_B, int p_kerning);
	int get_kerning_pair(char32_t p_A, char32_t p_B) const;

	void clear();

	float get_height() const;
	float get_ascent() const;
	float get_descent() const;

	Size2 get_char_size(char32_t p_char, char32_t p_next = 0) const;

	void set_fallback(const Ref<BitmapFont> &p_fallback);
	Ref<BitmapFont> get_fallback() const;

	void set_distance_field_hint(bool p_distance_field);
	bool is_distance_field_hint() const;

	float draw_char(RID p_canvas_item, const Point2 &p_pos, char32_t p_char, char32_t p_next = 0, const Color &p_modulate = Color(1, 1, 1), bool p_outline = false) const;

	virtual Vector<RID> get_rids() const;

	BitmapFont();
	~BitmapFont();
};

class ResourceFormatLoaderBMFont : public ResourceFormatLoader {
public:
	virtual RES load(const String &p_path, const String &p_original_path = "", Error *r_error = NULL);
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual bool handles_type(const String &p_type) const;
	virtual String get_resource_type(const String &p_path) const;
};

#endif
