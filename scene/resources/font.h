/*************************************************************************/
/*  font.h                                                               */
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

#ifndef FONT_H
#define FONT_H

#include "font_hex.h"

#include "core/hash_map.h"
#include "core/map.h"
#include "core/pair.h"
#include "core/resource.h"
#include "scene/resources/texture.h"

#include "servers/shaping/shaped_string.h"
#include "servers/shaping/shaping_defines.h"
#include "servers/shaping/text_layout.h"

/*************************************************************************/
/*  Font Data (Shaper independant resource)                              */
/*************************************************************************/

class FontImplementation;
class FontData : public Resource {
	GDCLASS(FontData, Resource);

public:
	struct CacheID {
		union {
			struct {
				uint32_t size : 8;
				uint32_t outline_size : 8;
				uint32_t mipmaps : 1;
				uint32_t filter : 1;
				uint32_t antialiased : 1;
				uint32_t force_autohinter : 1;
				uint32_t hinting : 2;
				uint32_t unused : 10;
			};
			uint32_t key;
		};
		bool operator<(CacheID right) const;
		CacheID() {
			key = 0;
		}
	};

	enum Hinting {
		HINTING_NONE,
		HINTING_LIGHT,
		HINTING_NORMAL
	};

private:
	String data_path;

	//raw dynamic font data
	const uint8_t *data_mem;
	size_t data_mem_size;
	StringName fid;

	mutable HashMap<uint32_t, Ref<FontImplementation>> size_cache;

	String supported_scripts;
	String supported_langs;
	float pmult;

	void _clear();
	void _clear_cache();

protected:
	static void _bind_methods();

public:
	String get_font_data_path() const;
	void set_font_data_path(const String &p_path);

	float get_force_priority_multiplier() const;
	void set_force_priority_multiplier(float p_mult);

	String get_force_supported_scripts() const;
	void set_force_supported_scripts(const String &p_supported_scripts); // , separated

	String get_force_supported_languages() const;
	void set_force_supported_languages(const String &p_supported_languages); // , separated

	//static data
	void _get_static_font_data(const uint8_t **o_font_mem, size_t *o_font_mem_size) const;
	void _set_static_font_data(const StringName &p_typeid, const uint8_t *p_font_mem, size_t p_font_mem_size);
	StringName _get_font_id() const;

	Ref<FontImplementation> _get_font_implementation_at_size(CacheID p_id) const;

	FontData();
	~FontData();
};

VARIANT_ENUM_CAST(FontData::Hinting);

/*************************************************************************/
/*  Font Implementation (Shaper and size specific resource)              */
/*************************************************************************/

class FontImplementation : public Reference {
	GDCLASS(FontImplementation, Reference);

protected:
	static void _bind_methods();

public:
	virtual void invalidate() = 0;
	virtual bool require_reload() const = 0;

	virtual void *get_native_handle() const = 0;
	virtual float get_font_scale() const = 0;

	virtual uint32_t get_glyph(uint32_t p_unicode, uint32_t p_variation_selector = 0) const = 0;
	virtual float get_advance(uint32_t p_glyph) const = 0;
	virtual float get_kerning(uint32_t p_glyph_a, uint32_t p_glyph_b) const = 0;

	virtual float get_v_advance(uint32_t p_glyph) const = 0;
	virtual float get_v_kerning(uint32_t p_glyph_a, uint32_t p_glyph_b) const = 0;

	virtual Size2 get_glyph_size(uint32_t p_glyph) const = 0;

	virtual float get_ascent() const = 0;
	virtual float get_descent() const = 0;
	virtual float get_line_gap() const = 0;

	virtual float get_underline_position() const = 0;
	virtual float get_underline_thickness() const = 0;

	virtual String get_name() const = 0;

	virtual int32_t get_script_support_priority(uint32_t p_script) const = 0;
	virtual int32_t get_language_support_priority(const String &p_lang) const = 0;

	virtual void draw_glyph(RID p_canvas_item, const Point2 &p_pos, uint32_t p_glyph, const Color &p_modulate = Color(1, 1, 1), bool p_rotate_cw = false) const = 0;

	virtual Error create(const FontData *p_data, FontData::CacheID p_cache_id) = 0;

	//self list api
	SelfList<FontImplementation> font_list;

	static Mutex *font_imp_mutex;
	static SelfList<FontImplementation>::List *font_imps;

	static void invalidate_all();

	static void initialize_self_list();
	static void finish_self_list();

	FontImplementation();
	virtual ~FontImplementation();
};

/*************************************************************************/
/*  Font                                                                 */
/*************************************************************************/

class Font : public Resource {
	GDCLASS(Font, Resource);

	void _reload_cache();

public:
	enum SpacingType {
		SPACING_TOP,
		SPACING_BOTTOM
	};

private:
	static float oversampling;

	Vector<Ref<FontData>> data;
	Vector<Ref<FontImplementation>> data_at_size;
	Vector<Ref<FontImplementation>> outline_data_at_size;

	FontData::CacheID cache_id;
	FontData::CacheID outline_cache_id;

	bool distance_field_hint;

	int spacing_top;
	int spacing_bottom;

	struct CacheRec {
		Ref<TextLayout> layout;
	};

	int32_t shaped_cache_max_depth;
	mutable HashMap<uint32_t, CacheRec> shaped_cache;
	mutable List<uint32_t> shaped_cache_last;

	Ref<TextLayout> _shape_layout(const String &p_text, TextDirection p_base_dir, const String &p_lang, const String &p_ftr) const;

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	static void _bind_methods();

public:
	//max props
	float get_ascent() const;
	float get_descent() const;
	float get_height() const;
	float get_line_gap() const;

	//global properties
	bool get_mipmap() const;
	void set_mipmap(bool p_mipmap);

	bool get_filter() const;
	void set_filter(bool p_filter);

	bool get_antialiased() const;
	void set_antialiased(bool p_antialiased);

	FontData::Hinting get_hinting() const;
	void set_hinting(FontData::Hinting p_hinting);

	bool get_force_autohinter() const;
	void set_force_autohinter(bool p_force);

	bool get_distance_field_hint() const;
	void set_distance_field_hint(bool p_distance_field);

	int get_extra_spacing(SpacingType p_type) const;
	void set_extra_spacing(SpacingType p_type, int p_value);

	static float get_oversampling();
	static void set_oversampling(float p_oversampling);

	int get_size() const;
	void set_size(int p_size);

	bool has_outline() const;

	int get_outline_size() const;
	void set_outline_size(int p_size);

	//low level shaping
	void get_font_implementations(Vector<Ref<FontImplementation>> &o_fonts, Vector<Ref<FontImplementation>> &o_outlines, uint32_t p_script = 0, const String &p_lang = "") const;
	Array _get_font_implementations(uint32_t p_script = 0, const String &p_lang = "") const;

	//Ref<ShapedString> shape_string(const String &p_text, int p_start, int p_end, TextDirection p_base_dir = TEXT_DIRECTION_AUTO, const String &p_lang = "", const String &p_ftr = "", bool p_no_cache = false) const;
	//Ref<ShapedString> shape_attributed_string(const String &p_text, int p_start, int p_end, const Ref<AttributeMap> &p_attributes, TextDirection p_base_dir = TEXT_DIRECTION_AUTO, bool p_no_cache = false) const;

	//high level wrappers
	void draw(RID p_canvas_item, const Point2 &p_pos, const String &p_text, int p_width = -1, const Color &p_modulate = Color(1, 1, 1), const Color &p_outline_modulate = Color(1, 1, 1, 0), TextDirection p_base_dir = TEXT_DIRECTION_AUTO, const String &p_lang = "", const String &p_ftr = "") const;
	void draw_wordwrap(RID p_canvas_item, const Point2 &p_pos, const String &p_text, int p_width = -1, const Color &p_modulate = Color(1, 1, 1), const Color &p_outline_modulate = Color(1, 1, 1, 0), TextDirection p_base_dir = TEXT_DIRECTION_AUTO, const String &p_lang = "", const String &p_ftr = "") const;
	void draw_halign(RID p_canvas_item, const Point2 &p_pos, const String &p_text, int p_width, HAlign p_align, const Color &p_modulate = Color(1, 1, 1), const Color &p_outline_modulate = Color(1, 1, 1, 0), TextDirection p_base_dir = TEXT_DIRECTION_AUTO, const String &p_lang = "", const String &p_ftr = "") const;
	void draw_wordwrap_halign(RID p_canvas_item, const Point2 &p_pos, const String &p_text, int p_width, HAlign p_align, const Color &p_modulate = Color(1, 1, 1), const Color &p_outline_modulate = Color(1, 1, 1, 0), TextDirection p_base_dir = TEXT_DIRECTION_AUTO, const String &p_lang = "", const String &p_ftr = "") const;

	Size2 get_string_size(const String &p_text, TextDirection p_base_dir = TEXT_DIRECTION_AUTO, const String &p_lang = "", const String &p_ftr = "") const;
	Size2 get_wordwrap_string_size(const String &p_text, float p_width = -1, TextDirection p_base_dir = TEXT_DIRECTION_AUTO, const String &p_lang = "", const String &p_ftr = "") const;

	//compatiblility wrappers
	float draw_char(RID p_canvas_item, const Point2 &p_pos, CharType p_char, CharType p_next = 0, const Color &p_modulate = Color(1, 1, 1), bool p_outline = false) const;
	Size2 get_char_size(CharType p_char, CharType p_next = 0) const;

	//data control
	int get_font_data_count() const;
	void add_font_data(const Ref<FontData> &p_data);
	void set_font_data(int p_idx, const Ref<FontData> &p_data);
	Ref<FontData> get_font_data(int p_idx) const;
	void remove_font_data(int p_idx);

	void update_changes(); //call manually after data change

	Font();
	~Font();
};

VARIANT_ENUM_CAST(Font::SpacingType);

/*************************************************************************/
/*  Font Data (Resource loader)                                          */
/*************************************************************************/

class ResourceFormatLoaderFontData : public ResourceFormatLoader {
public:
	virtual RES load(const String &p_path, const String &p_original_path = "", Error *r_error = NULL);
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual bool handles_type(const String &p_type) const;
	virtual String get_resource_type(const String &p_path) const;
};

#endif