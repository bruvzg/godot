/*************************************************************************/
/*  font.h                                                               */
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

#ifndef FONT_H
#define FONT_H

#include "core/lru.h"
#include "core/resource.h"
#include "scene/resources/texture.h"
#include "servers/text_server.h"

class FontData : public Resource {
	GDCLASS(FontData, Resource);

	mutable RID font_data;

protected:
	static void _bind_methods();

public:
	RID get_rid() const;

	bool load_system(const String &p_name);
	bool load_resource(const String &p_filename);
	bool load_memory(const Vector<uint8_t> &p_data);

	float get_height(float p_size) const;
	float get_ascent(float p_size) const;
	float get_descent(float p_size) const;
	float get_underline_position(float p_size) const;
	float get_underline_thickness(float p_size) const;

	bool has_feature(TextServer::FontFeature p_feature) const;

	bool get_language_supported(const String &p_locale) const;
	void set_language_supported(const String &p_locale, bool p_value);

	bool get_script_supported(const String &p_script) const;
	void set_script_supported(const String &p_script, bool p_value);

	void draw_glyph(RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color = Color(1, 1, 1)) const;
	void draw_glyph_outline(RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color = Color(1, 1, 1)) const;
	void draw_invalid_glpyh(RID p_canvas, float p_size, const Vector2 &p_pos, uint32_t p_index, const Color &p_color = Color(1, 1, 1)) const;

	~FontData();
};

/*************************************************************************/

class Font : public Resource {
	GDCLASS(Font, Resource);

	Vector<Ref<FontData>> font;
	LRU<uint64_t, RID> cache;

protected:
	void _draw_ctx(RID p_canvas_item, float p_size, const Point2 &p_pos, RID p_ctx, const Color &p_modulate, int p_clip_w, const Color &p_outline_modulate) const;

	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	static void _bind_methods();

public:
	float get_height(float p_size) const;
	float get_ascent(float p_size) const;
	float get_descent(float p_size) const;
	float get_underline_position(float p_size) const;
	float get_underline_thickness(float p_size) const;

	Size2 get_string_size(const String &p_string, float p_size) const;
	Size2 get_wordwrap_string_size(const String &p_string, float p_size, float p_width) const;

	void draw(RID p_canvas_item, float p_size, const Point2 &p_pos, const String &p_text, const Color &p_modulate = Color(1, 1, 1), int p_clip_w = -1, const Color &p_outline_modulate = Color(0, 0, 0, 0)) const;
	void draw_halign(RID p_canvas_item, float p_size, const Point2 &p_pos, const String &p_text, HAlign p_align, float p_width, const Color &p_modulate = Color(1, 1, 1), const Color &p_outline_modulate = Color(0, 0, 0, 0)) const;
	void draw_wordwrap(RID p_canvas_item, float p_size, const Point2 &p_pos, const String &p_text, HAlign p_align, float p_width, const Color &p_modulate = Color(1, 1, 1), const Color &p_outline_modulate = Color(0, 0, 0, 0)) const;

	void add_data(const Ref<FontData> &p_data);
	void set_data(int p_idx, const Ref<FontData> &p_data);
	int get_data_count() const;
	Ref<FontData> get_data(int p_idx) const;
	void remove_data(int p_idx);

	List<RID> get_data_list();
};

/*************************************************************************/

class ResourceFormatLoaderFontData : public ResourceFormatLoader {
public:
	virtual RES load(const String &p_path, const String &p_original_path = "", Error *r_error = nullptr, bool p_use_sub_threads = false, float *r_progress = nullptr, bool p_no_cache = false);
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual bool handles_type(const String &p_type) const;
	virtual String get_resource_type(const String &p_path) const;
};

#endif /* FONT_H */
