/*************************************************************************/
/*  spannable.h                                                          */
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

#ifndef SPANNABLE_H
#define SPANNABLE_H

#include "core/reference.h"
#include "core/variant.h"

#include "scene/resources/font.h"

class Spannable : public Reference {
	GDCLASS(Spannable, Reference);

public:
	enum TextAttribute {
		TEXT_ATTRIBUTE_FONT,
		TEXT_ATTRIBUTE_OT_FEATURES,
		TEXT_ATTRIBUTE_LANGUAGE,
		TEXT_ATTRIBUTE_COLOR,
		TEXT_ATTRIBUTE_OUTLINE_COLOR,

		TEXT_ATTRIBUTE_USER = 100
	};

private:
	String string;

	Map<int64_t, Map<TextAttribute, Variant> > attributes;

	void ensure_break(int64_t p_index);
	bool compare_attributes(const Map<TextAttribute, Variant> &p_first, const Map<TextAttribute, Variant> &p_second) const;
	void optimize_attributes();

protected:
	static void _bind_methods();

public:
	uint32_t hash() const;

	void set_string(const String &p_string);
	String get_string() const;

	void add_attribute(TextAttribute p_attribute, Variant p_value, int64_t p_start, int64_t p_end);
	void remove_attribute(TextAttribute p_attribute, int64_t p_start, int64_t p_end);
	void remove_attributes(int64_t p_start, int64_t p_end);
	void clear_attributes();

	bool has_attribute(TextAttribute p_attribute, int64_t p_index) const;
	Variant get_attribute(TextAttribute p_attribute, int64_t p_index) const;
	int64_t get_run_start(TextAttribute p_attribute, int64_t p_index) const;
	int64_t get_run_end(TextAttribute p_attribute, int64_t p_index) const;

	Ref<Spannable> substr(int64_t p_start, int64_t p_end) const;
};

VARIANT_ENUM_CAST(Spannable::TextAttribute);

#endif
