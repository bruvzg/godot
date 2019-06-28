/*************************************************************************/
/*  font_implementation_hb_icu.h                                         */
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

#ifndef FONT_IMPLEMENTATION_HB_ICU_H
#define FONT_IMPLEMENTATION_HB_ICU_H

#include <hb.h>

#include "modules/shaper_fallback/font_implementation_fb.h"

/*************************************************************************/
/*  Bitmap Font                                                          */
/*************************************************************************/

class FontImplementationBitmapHBICU : public FontImplementationBitmap {
	GDCLASS(FontImplementationBitmapHBICU, FontImplementationBitmap);

	hb_font_t *hb_font;

	void _FORCE_INLINE_ scr_sup_up(uint32_t p_tag, int32_t p_prior = 1) {
		if (supported_scripts.has(p_tag))
			supported_scripts[p_tag] += p_prior;
		else
			supported_scripts[p_tag] = p_prior;
	}

public:
	virtual void *get_native_handle() const;

	//construct
	virtual Error create(const FontData *p_data, FontData::CacheID p_cache_id);

	FontImplementationBitmapHBICU();
	virtual ~FontImplementationBitmapHBICU();
};

/*************************************************************************/
/*  Dynamic Font                                                         */
/*************************************************************************/

#ifdef MODULE_FREETYPE_ENABLED

class FontImplementationDynamicHBICU : public FontImplementationDynamic {
	GDCLASS(FontImplementationDynamicHBICU, FontImplementationDynamic);

	hb_font_t *hb_font;

	void _FORCE_INLINE_ scr_sup_up(uint32_t p_tag, int32_t p_prior = 1) {
		if (supported_scripts.has(p_tag))
			supported_scripts[p_tag] += p_prior;
		else
			supported_scripts[p_tag] = p_prior;
	}

public:
	virtual void *get_native_handle() const;

	//construct
	virtual Error create(const FontData *p_data, FontData::CacheID p_cache_id);

	FontImplementationDynamicHBICU();
	virtual ~FontImplementationDynamicHBICU();
};

#endif /*MODULE_FREETYPE_ENABLED*/

#endif
