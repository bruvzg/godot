/*************************************************************************/
/*  dynamic_font.h                                                       */
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

#ifndef DYNAMIC_FONT_H
#define DYNAMIC_FONT_H

#include "core/io/resource_loader.h"
#include "core/pair.h"
#include "scene/resources/font.h"

class DynamicFontData : public Resource {

	GDCLASS(DynamicFontData, Resource);

public:
	enum Hinting {
		HINTING_NONE = TextServer::HINTING_NONE,
		HINTING_LIGHT = TextServer::HINTING_LIGHT,
		HINTING_NORMAL = TextServer::HINTING_NORMAL,
	};

	bool is_antialiased() const;
	void set_antialiased(bool p_antialiased);
	Hinting get_hinting() const;
	void set_hinting(Hinting p_hinting);

private:
	String font_path;
	RID rid;

protected:
	static void _bind_methods();

public:
	RID get_rid() const { return rid; };

	Dictionary get_variation_list() const;
	void set_variation(const String &p_name, double p_value);
	double get_variation(const String &p_name) const;

	void set_font_ptr(const uint8_t *p_font_mem, int p_font_mem_size);
	void set_font_path(const String &p_path);
	String get_font_path() const;
	void set_force_autohinter(bool p_force);

	DynamicFontData();
	~DynamicFontData();
};

VARIANT_ENUM_CAST(DynamicFontData::Hinting);

///////////////

class DynamicFont : public Font {

	GDCLASS(DynamicFont, Font);

public:
	enum SpacingType {
		SPACING_TOP = Font::SPACING_TOP,
		SPACING_BOTTOM = Font::SPACING_BOTTOM,
		SPACING_CHAR = Font::SPACING_CHAR,
		SPACING_SPACE = Font::SPACING_SPACE
	};

private:
	Ref<DynamicFontData> data;
	Vector<Ref<DynamicFontData> > fallbacks;

	Color outline_color;

	void _reload_cache();

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	static void _bind_methods();

public:
	void set_font_data(const Ref<DynamicFontData> &p_data);
	Ref<DynamicFontData> get_font_data() const;

	int get_size() const;
	void set_size(int p_size);
	int get_size() const;

	void set_outline_size(int p_size);
	int get_outline_size() const;

	void set_outline_color(Color p_color);
	Color get_outline_color() const;

	bool get_use_mipmaps() const;
	void set_use_mipmaps(bool p_enable);

	bool get_use_filter() const;
	void set_use_filter(bool p_enable);

	int get_spacing(int p_type) const;
	void set_spacing(int p_type, int p_value);

	void add_fallback(const Ref<DynamicFontData> &p_data);
	void set_fallback(int p_idx, const Ref<DynamicFontData> &p_data);
	int get_fallback_count() const;
	Ref<DynamicFontData> get_fallback(int p_idx) const;
	void remove_fallback(int p_idx);

	virtual float get_height() const;

	virtual float get_ascent() const;
	virtual float get_descent() const;

	virtual Size2 get_char_size(char32_t p_char, char32_t p_next = 0) const;
	String get_available_chars() const;

	virtual bool is_distance_field_hint() const;

	virtual bool has_outline() const;

	virtual float draw_char(RID p_canvas_item, const Point2 &p_pos, char32_t p_char, char32_t p_next = 0, const Color &p_modulate = Color(1, 1, 1), bool p_outline = false) const;

	virtual Vector<RID> get_rids() const;

	DynamicFont();
	~DynamicFont();
};

VARIANT_ENUM_CAST(DynamicFont::SpacingType);

/////////////

class ResourceFormatLoaderDynamicFont : public ResourceFormatLoader {
public:
	virtual RES load(const String &p_path, const String &p_original_path = "", Error *r_error = NULL);
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual bool handles_type(const String &p_type) const;
	virtual String get_resource_type(const String &p_path) const;
};

#endif
