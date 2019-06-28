/*************************************************************************/
/*  shaping_defines.h                                                    */
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

#ifndef SHAPING_DEFINES_H
#define SHAPING_DEFINES_H

#include "core/os/thread_safe.h"
#include "core/pair.h"
#include "core/reference.h"
#include "scene/resources/texture.h"

#define TAG(c1, c2, c3, c4) ((uint32_t)((((uint32_t)(c1)&0xFF) << 24) | (((uint32_t)(c2)&0xFF) << 16) | (((uint32_t)(c3)&0xFF) << 8) | ((uint32_t)(c4)&0xFF)))

static uint32_t _FORCE_INLINE_ tag_from_string(const String &p_str) {

	char tag[4];

	int32_t i;
	for (i = 0; i < MIN(4, p_str.ascii().length()); i++) {
		tag[i] = p_str.ascii()[i];
	}

	for (; i < 4; i++) {
		tag[i] = ' ';
	}

	return TAG(tag[0], tag[1], tag[2], tag[3]);
}

class FontImplementation;

/*************************************************************************/
/*  Low level shaped string data                                         */
/*************************************************************************/

struct Glyph {
	uint32_t codepoint;

	Point2 offset;
	float advance;

	Glyph();
};

struct Cluster {

	int32_t start;
	int32_t end;

	Vector<Glyph> glyphs;

	float advance;
	int16_t repeat;

	Ref<FontImplementation> font_impl;
	Ref<FontImplementation> font_outline_impl;
	int8_t flags;

	Cluster();
};

struct Run {

	int32_t start;
	int32_t end;

	int32_t level;
	uint32_t script;
	RunBreakType break_type;
	int indent;

	Vector<Cluster> clusters;

	float width;
	float ascent;
	float descent;

	Run();
	Run(int32_t p_start, int32_t p_end, int32_t p_level, uint32_t p_script, RunBreakType p_break_type);
};

struct RunCompare {

	_FORCE_INLINE_ bool operator()(const Run &p_a, const Run &p_b) const {

		return p_a.start < p_b.start; //runs should never intersect, check only start offset
	}
};

#endif
