/*************************************************************************/
/*  shaping_interface_fb.h                                               */
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

#ifndef SHAPING_INTERFACE_FB_H
#define SHAPING_INTERFACE_FB_H

#include "servers/shaping/shaping_interface.h"

class ShapingInterfaceFallback : public ShapingInterface {
	GDCLASS(ShapingInterfaceFallback, ShapingInterface);

protected:
	static void _bind_methods();

public:
	virtual StringName get_name() const;
	virtual int32_t get_capabilities() const;
	virtual String get_info() const;

	virtual Ref<FontImplementation> create_font_implementation(const FontData *p_data, FontData::CacheID p_id) const;

	//S VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE
	virtual TextDirection analyse_text(const String &p_text, int32_t p_start, int32_t p_end, TextDirection p_base_direction, const String &p_language, Vector<Run> &p_runs) const;
	virtual bool shape_run(const String &p_text, const Font *p_font, Run &p_run, const String &p_language, const String &p_features) const;
	virtual TextDirection get_string_direction(const String &p_text) const;
	//E VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE

	virtual TextDirection get_paragraph_direction(TextDirection p_base_direction, const String &p_text, const String &p_language) const;
	virtual Vector<SourceRun> get_bidi_runs(const String &p_text, int p_start, int p_end, TextDirection p_para_direction) const;
	virtual Vector<SourceRun> get_script_runs(const String &p_text, int p_start, int p_end, TextDirection p_para_direction) const;
	virtual Vector<SourceRun> get_break_runs(const String &p_text, int p_start, int p_end, const String &p_language) const;
	virtual Run shape_run2(const String &p_text, int p_start, int p_end, const Font *p_font, const String &p_features, const String &p_language, uint32_t p_level, uint32_t p_script, uint32_t p_break, bool p_prefer_vertical) const;

	virtual Vector<float> get_ligature_caret_offsets(const Ref<FontImplementation> &p_font_imp, TextDirection p_direction, uint32_t p_glyph) const;

	virtual TextDirection get_locale_direction(const String &p_lang) const;

	ShapingInterfaceFallback();
	~ShapingInterfaceFallback();
};

#endif
