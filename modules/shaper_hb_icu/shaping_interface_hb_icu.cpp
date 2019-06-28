/*************************************************************************/
/*  shaping_interface_hb_icu.cpp                                         */
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

#include "shaping_interface_hb_icu.h"

#include "core/os/file_access.h"
#include "core/os/os.h"
#include "core/pair.h"
#include "core/translation.h"
#include "scene/resources/font.h"

#include <unicode/ubidi.h>
#include <unicode/ubrk.h>
#include <unicode/uchar.h>
#include <unicode/uclean.h>
#include <unicode/udata.h>
#include <unicode/uiter.h>
#include <unicode/uloc.h>
#include <unicode/uscript.h>
#include <unicode/ustring.h>
#include <unicode/utypes.h>

#include <hb-icu.h>
#include <hb.h>

#include <hb-ft.h>
#include <hb-ot.h>

#ifdef GRAPHITE_ENABLED
#include <graphite2/Font.h>
#endif

#include "font_implementation_hb_icu.h"

/*************************************************************************/

PoolByteArray ShapingInterfaceHBICU::icu_data;

_ALWAYS_INLINE_ bool is_rotated(uint32_t p_chr) { //Note: actually rotate upright glyph io opposite direction
	return (u_getIntPropertyValue(p_chr, UCHAR_VERTICAL_ORIENTATION) == U_VO_ROTATED) || (u_getIntPropertyValue(p_chr, UCHAR_VERTICAL_ORIENTATION) == U_VO_TRANSFORMED_ROTATED);
}

_ALWAYS_INLINE_ bool is_transformed(uint32_t p_chr) { //not required ?
	return (u_getIntPropertyValue(p_chr, UCHAR_VERTICAL_ORIENTATION) == U_VO_TRANSFORMED_UPRIGHT) || (u_getIntPropertyValue(p_chr, UCHAR_VERTICAL_ORIENTATION) == U_VO_TRANSFORMED_ROTATED);
}

_ALWAYS_INLINE_ bool is_ain(uint32_t p_chr) {
	return u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_AIN;
}

_ALWAYS_INLINE_ bool is_alef(uint32_t p_chr) {
	return u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_ALEF;
}

_ALWAYS_INLINE_ bool is_beh(uint32_t p_chr) {
	int32_t prop = u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP);
	return (prop == U_JG_BEH) || (prop == U_JG_NOON) || (prop == U_JG_AFRICAN_NOON) || (prop == U_JG_NYA) || (prop == U_JG_YEH) || (prop == U_JG_FARSI_YEH);
}

_ALWAYS_INLINE_ bool is_dal(uint32_t p_chr) {
	return u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_DAL;
}

_ALWAYS_INLINE_ bool is_feh(uint32_t p_chr) {
	return (u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_FEH) || (u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_AFRICAN_FEH);
}

_ALWAYS_INLINE_ bool is_gaf(uint32_t p_chr) {
	return u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_GAF;
}

_ALWAYS_INLINE_ bool is_heh(uint32_t p_chr) {
	return u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_HEH;
}

_ALWAYS_INLINE_ bool is_kaf(uint32_t p_chr) {
	return u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_KAF;
}

_ALWAYS_INLINE_ bool is_lam(uint32_t p_chr) {
	return u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_LAM;
}

_ALWAYS_INLINE_ bool is_qaf(uint32_t p_chr) {
	return (u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_QAF) || (u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_AFRICAN_QAF);
}

_ALWAYS_INLINE_ bool is_reh(uint32_t p_chr) {
	return u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_REH;
}

_ALWAYS_INLINE_ bool is_seen_sad(uint32_t p_chr) {
	return (u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_SAD) || (u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_SEEN);
}

_ALWAYS_INLINE_ bool is_tah(uint32_t p_chr) {
	return u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_TAH;
}

_ALWAYS_INLINE_ bool is_teh_marbuta(uint32_t p_chr) {
	return u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_TEH_MARBUTA;
}

_ALWAYS_INLINE_ bool is_yeh(uint32_t p_chr) {
	int32_t prop = u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP);
	return (prop == U_JG_YEH) || (prop == U_JG_FARSI_YEH) || (prop == U_JG_YEH_BARREE) || (prop == U_JG_BURUSHASKI_YEH_BARREE) || (prop == U_JG_YEH_WITH_TAIL);
}

_ALWAYS_INLINE_ bool is_waw(uint32_t p_chr) {
	return u_getIntPropertyValue(p_chr, UCHAR_JOINING_GROUP) == U_JG_WAW;
}

_ALWAYS_INLINE_ bool is_transparent(uint32_t p_chr) {
	return u_getIntPropertyValue(p_chr, UCHAR_JOINING_TYPE) == U_JT_TRANSPARENT;
}

_ALWAYS_INLINE_ bool is_ligature(uint32_t p_chr, uint32_t p_nchr) {
	return (is_lam(p_chr) && is_alef(p_nchr));
}

_ALWAYS_INLINE_ bool is_connected_to_prev(uint32_t p_chr, uint32_t p_pchr) {
	int32_t prop = u_getIntPropertyValue(p_pchr, UCHAR_JOINING_TYPE);
	return (prop != U_JT_RIGHT_JOINING) && (prop != U_JT_NON_JOINING) ? !is_ligature(p_pchr, p_chr) : false;
}

//S VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE
void ShapingInterfaceHBICU::generate_script_runs(const CharType *p_chars, int32_t p_length, int32_t p_start, int32_t p_end, Vector<ScriptRun> &o_runs) const { //REMOVE
	struct ParenStackEntry {
		int32_t pair_index;
		UScriptCode script_code;
	};

	ParenStackEntry paren_stack[128];

	int32_t script_start;
	int32_t script_end = 0;
	UScriptCode script_code;
	int32_t paren_sp = -1;
	int32_t start_sp = paren_sp;
	UErrorCode err = U_ZERO_ERROR;

	do {
		script_code = USCRIPT_COMMON;
		for (script_start = script_end; script_end < p_length; script_end++) {

			UChar32 ch;
			U16_GET(p_chars, 0, script_end, p_length, ch);

			UScriptCode sc = uscript_getScript(ch, &err);
			if (U_FAILURE(err)) {
				ERR_PRINTS(u_errorName(err));
				ERR_FAIL_COND(true);
			}
			if (u_getIntPropertyValue(ch, UCHAR_BIDI_PAIRED_BRACKET_TYPE) != U_BPT_NONE) {
				if (u_getIntPropertyValue(ch, UCHAR_BIDI_PAIRED_BRACKET_TYPE) == U_BPT_OPEN) {
					paren_stack[++paren_sp].pair_index = ch;
					paren_stack[paren_sp].script_code = script_code;
				} else if (paren_sp >= 0) {
					UChar32 paired_ch = u_getBidiPairedBracket(ch);
					while (paren_sp >= 0 && paren_stack[paren_sp].pair_index != paired_ch)
						paren_sp -= 1;
					if (paren_sp < start_sp) start_sp = paren_sp;
					if (paren_sp >= 0) sc = paren_stack[paren_sp].script_code;
				}
			}

			if (script_code <= USCRIPT_INHERITED || sc <= USCRIPT_INHERITED || script_code == sc) {
				if (script_code <= USCRIPT_INHERITED && sc > USCRIPT_INHERITED) {
					script_code = sc;
					while (start_sp < paren_sp)
						paren_stack[++start_sp].script_code = script_code;
				}
				if ((u_getIntPropertyValue(ch, UCHAR_BIDI_PAIRED_BRACKET_TYPE) == U_BPT_CLOSE) && paren_sp >= 0) {
					paren_sp -= 1;
					start_sp -= 1;
				}
			} else {
				break;
			}
		}

		if ((script_end > p_start) && (script_start < p_end)) {
			o_runs.push_back(ScriptRun(MAX(p_start, script_start), MIN(p_end, script_end), hb_icu_script_to_script(script_code)));
		}

	} while (script_end < p_length);
}
//E VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE

void ShapingInterfaceHBICU::generate_kashida_justification_opportunies(const CharType *p_chars, int32_t p_start, int32_t p_end, Vector<int32_t> &o_kashidas) const {
	int32_t kashida_pos = -1;
	int8_t priority = 100;
	int64_t i = p_start;

	uint32_t pc = 0;

	uint32_t lc;
	U16_GET(p_chars, 0, p_end - 1, p_end, lc);

	while ((p_end > p_start) && is_transparent(lc))
		p_end--;

	while (i < p_end) {
		uint32_t c, nc;
		U16_GET(p_chars, 0, i, p_end, c);
		U16_GET(p_chars, 0, i, p_end, nc);

		if (priority >= 1 && i < p_end - 1) {
			if (is_seen_sad(c) && (nc != 0x200C)) {
				kashida_pos = i;
				priority = 1;
			}
		}
		if (priority >= 2 && i > p_start) {
			if (is_teh_marbuta(c) || is_dal(c) || (is_heh(c) && i == p_end - 1)) {
				if (is_connected_to_prev(c, pc)) {
					kashida_pos = i - 1;
					priority = 2;
				}
			}
		}
		if (priority >= 3 && i > p_start) {
			if (is_alef(c) || ((is_lam(c) || is_tah(c) || is_kaf(c) || is_gaf(c)) && i == p_end - 1)) {
				if (is_connected_to_prev(c, pc)) {
					kashida_pos = i - 1;
					priority = 3;
				}
			}
		}
		if (priority >= 4 && i > p_start && i < p_end - 1) {
			if (is_beh(c)) {
				if (is_reh(nc) || is_yeh(nc)) {
					if (is_connected_to_prev(c, pc)) {
						kashida_pos = i - 1;
						priority = 4;
					}
				}
			}
		}
		if (priority >= 5 && i > p_start) {
			if (is_waw(c) || ((is_ain(c) || is_qaf(c) || is_feh(c)) && i == p_end - 1)) {
				if (is_connected_to_prev(c, pc)) {
					kashida_pos = i - 1;
					priority = 5;
				}
			}
		}
		if (priority >= 6 && i > p_start) {
			if (is_reh(c)) {
				if (is_connected_to_prev(c, pc)) {
					kashida_pos = i - 1;
					priority = 6;
				}
			}
		}
		if (!is_transparent(c)) pc = c;
		i++;
	}
	if (kashida_pos > -1) {
		o_kashidas.push_back(kashida_pos);
	}
}

ShapingInterfaceHBICU::ShapingInterfaceHBICU() {
#ifdef ICU_STATIC_DATA
	//Initalize ICU data (static)
	UErrorCode err = U_ZERO_ERROR;
	u_init(&err);
	if (U_FAILURE(err)) {
		ERR_PRINT(u_errorName(err));
		return;
	}
#else
	//Initialize ICU data from resource, should be done at most once in a process, before the first ICU operation (e.g., u_init())
	String datapath = OS::get_singleton()->get_executable_path().get_base_dir().plus_file("icudt64l.dat");
#ifdef OSX_ENABLED
	if (!FileAccess::exists(datapath)) {
		datapath = get_executable_path().get_base_dir().plus_file("../Resources").plus_file("icudt64l.dat");
	}
#endif
	if (!FileAccess::exists(datapath)) {
		ERR_PRINT("ICU data file not found!");
		return;
	} else {
		FileAccess *f = FileAccess::open(datapath, FileAccess::READ);
		if (f) {
			UErrorCode err = U_ZERO_ERROR;
			size_t len = f->get_len();
			icu_data.resize(len);
			f->get_buffer(icu_data.write().ptr(), len);
			f->close();

			udata_setCommonData(icu_data.read().ptr(), &err);
			if (U_FAILURE(err)) {
				ERR_PRINT(u_errorName(err));
				return;
			}

			err = U_ZERO_ERROR;
			u_init(&err);
			if (U_FAILURE(err)) {
				ERR_PRINT(u_errorName(err));
				return;
			}
		}
	}
#endif
}

ShapingInterfaceHBICU::~ShapingInterfaceHBICU() {
	u_cleanup();
}

void ShapingInterfaceHBICU::_bind_methods() {
	//NOP
}

StringName ShapingInterfaceHBICU::get_name() const {
#ifdef GRAPHITE_ENABLED
	return StringName("HarfBuzz/ICU/Graphite");
#else
	return StringName("HarfBuzz/ICU");
#endif
}

int32_t ShapingInterfaceHBICU::get_capabilities() const {
#ifdef GRAPHITE_ENABLED
	return SHAPING_INTERFACE_BIDI_LAYOUT | SHAPING_INTERFACE_GENERIC_SHAPING | SHAPING_INTERFACE_OPENTYPE_SHAPING | SHAPING_INTERFACE_GRAPHITE2_SHAPING;
#else
	return SHAPING_INTERFACE_BIDI_LAYOUT | SHAPING_INTERFACE_GENERIC_SHAPING | SHAPING_INTERFACE_OPENTYPE_SHAPING;
#endif
}

String ShapingInterfaceHBICU::get_info() const {
	UVersionInfo icu_ver;
	char icu_ver_str[U_MAX_VERSION_STRING_LENGTH] = "";

	u_getVersion(icu_ver);
	u_versionToString(icu_ver, icu_ver_str);

#ifdef ICU_STATIC_DATA
	String icd_data = " (with embedded data)";
#else
	String icd_data = " (with external data)";
#endif

#ifdef GRAPHITE_ENABLED
	int nMajor;
	int nMinor;
	int nBugFix;
	gr_engine_version(&nMajor, &nMinor, &nBugFix);
	return String("HarfBuzz version: ") + String(hb_version_string()) + String("; ICU version: ") + String(icu_ver_str) + icd_data + String("; Graphite version: ") + itos(nMajor) + String(".") + itos(nMinor) + String(".") + itos(nBugFix);
#else
	return String("HarfBuzz version: ") + String(hb_version_string()) + String("; ICU version: ") + String(icu_ver_str) + icd_data;
#endif
}

Ref<FontImplementation> ShapingInterfaceHBICU::create_font_implementation(const FontData *p_data, FontData::CacheID p_id) const {
	if (p_data->_get_font_id() == "bitmap") {
		//Bitmap font
		Ref<FontImplementationBitmapHBICU> ref;
		ref.instance();
		ref->create(p_data, p_id);
		if (!ref->require_reload()) return ref;
#ifdef MODULE_FREETYPE_ENABLED
	} else if (p_data->_get_font_id() == "dynamic") {
		//Dynamic font
		Ref<FontImplementationDynamicHBICU> ref;
		ref.instance();
		ref->create(p_data, p_id);
		if (!ref->require_reload()) return ref;
#endif
	}
	return NULL;
}

TextDirection ShapingInterfaceHBICU::get_paragraph_direction(TextDirection p_base_direction, const String &p_text, const String &p_language) const {
	if (p_base_direction != TEXT_DIRECTION_AUTO) return p_base_direction;

	UBiDiDirection direction = ubidi_getBaseDirection(p_text.c_str(), p_text.length());
	if (direction != UBIDI_NEUTRAL) {
		return (direction == UBIDI_RTL) ? TEXT_DIRECTION_RTL : TEXT_DIRECTION_LTR;
	} else {
		return uloc_isRightToLeft(p_language.ascii().get_data()) ? TEXT_DIRECTION_RTL : TEXT_DIRECTION_LTR;
	}
}

Vector<ShapingInterface::SourceRun> ShapingInterfaceHBICU::get_bidi_runs(const String &p_text, int p_start, int p_end, TextDirection p_para_direction) const {
	Vector<SourceRun> runs;

	UErrorCode err = U_ZERO_ERROR;
	UBiDi *bidi_iter = ubidi_openSized(p_text.length(), 0, &err);
	if (U_FAILURE(err)) {
		ERR_PRINTS(u_errorName(err));
		return runs;
	}
	ubidi_setPara(bidi_iter, p_text.c_str(), p_text.length(), (p_para_direction == TEXT_DIRECTION_LTR) ? UBIDI_LTR : UBIDI_RTL, NULL, &err);
	if (U_FAILURE(err)) {
		ERR_PRINTS(u_errorName(err));
		return runs;
	}
	UBiDi *bidi_iter_base = bidi_iter;
	//line iter
	if ((p_start != 0) || (p_end != p_text.length())) {
		bidi_iter_base = ubidi_openSized((p_end - p_start), 0, &err);
		if (U_FAILURE(err)) {
			ubidi_close(bidi_iter);
			ERR_PRINTS(u_errorName(err));
			return runs;
		}
		ubidi_setLine(bidi_iter, p_start, p_end, bidi_iter_base, &err);
		if (U_FAILURE(err)) {
			ubidi_close(bidi_iter);
			ERR_PRINTS(u_errorName(err));
			return runs;
		}
	}
	int64_t bidi_run_count = ubidi_countRuns(bidi_iter_base, &err);
	if (U_FAILURE(err)) {
		if (bidi_iter_base != bidi_iter) ubidi_close(bidi_iter_base);
		ubidi_close(bidi_iter);
		ERR_PRINTS(u_errorName(err));
		return runs;
	}
	for (int64_t i = 0; i < bidi_run_count; i++) {
		int32_t bidi_run_start = 0;
		int32_t bidi_run_length = 0;
		uint32_t bidi_run_direction = ubidi_getVisualRun(bidi_iter_base, i, &bidi_run_start, &bidi_run_length);
		SourceRun sr;
		sr.start = bidi_run_start + p_start;
		sr.end = bidi_run_start + p_start + bidi_run_length;
		sr.value = bidi_run_direction;
		runs.push_back(sr);
	}
	return runs;
}

Vector<ShapingInterface::SourceRun> ShapingInterfaceHBICU::get_script_runs(const String &p_text, int p_start, int p_end, TextDirection p_para_direction) const {
	Vector<SourceRun> runs;

	struct ParenStackEntry {
		int pair_index;
		UScriptCode script_code;
	};

	ParenStackEntry paren_stack[128];

	int script_start;
	int script_end = 0;
	UScriptCode script_code;

	int paren_sp = -1;
	int start_sp = paren_sp;
	UErrorCode err = U_ZERO_ERROR;

	do {
		script_code = USCRIPT_COMMON;
		for (script_start = script_end; script_end < p_text.length(); script_end++) {

			UChar32 ch;
			U16_GET(p_text.c_str(), 0, script_end, p_text.length(), ch);

			UScriptCode sc = uscript_getScript(ch, &err);
			if (U_FAILURE(err)) {
				ERR_PRINTS(u_errorName(err));
				ERR_FAIL_COND_V(true, runs);
			}
			if (u_getIntPropertyValue(ch, UCHAR_BIDI_PAIRED_BRACKET_TYPE) != U_BPT_NONE) {
				if (u_getIntPropertyValue(ch, UCHAR_BIDI_PAIRED_BRACKET_TYPE) == U_BPT_OPEN) {
					paren_stack[++paren_sp].pair_index = ch;
					paren_stack[paren_sp].script_code = script_code;
				} else if (paren_sp >= 0) {
					UChar32 paired_ch = u_getBidiPairedBracket(ch);
					while (paren_sp >= 0 && paren_stack[paren_sp].pair_index != paired_ch)
						paren_sp -= 1;
					if (paren_sp < start_sp) start_sp = paren_sp;
					if (paren_sp >= 0) sc = paren_stack[paren_sp].script_code;
				}
			}

			if (script_code <= USCRIPT_INHERITED || sc <= USCRIPT_INHERITED || script_code == sc) {
				if (script_code <= USCRIPT_INHERITED && sc > USCRIPT_INHERITED) {
					script_code = sc;
					while (start_sp < paren_sp)
						paren_stack[++start_sp].script_code = script_code;
				}
				if ((u_getIntPropertyValue(ch, UCHAR_BIDI_PAIRED_BRACKET_TYPE) == U_BPT_CLOSE) && paren_sp >= 0) {
					paren_sp -= 1;
					start_sp -= 1;
				}
			} else {
				break;
			}
		}

		if ((script_end > p_start) && (script_start < p_end)) {
			runs.push_back(SourceRun(MAX(p_start, script_start), MIN(p_end, script_end), hb_icu_script_to_script(script_code)));
		}

	} while (script_end < p_text.length());

	return runs;
}

Vector<ShapingInterface::SourceRun> ShapingInterfaceHBICU::get_break_runs(const String &p_text, int p_start, int p_end, const String &p_language) const {
	Vector<SourceRun> runs;
	UErrorCode err = U_ZERO_ERROR;

	UBreakIterator *bi = ubrk_open(UBRK_LINE, p_language.ascii().get_data(), p_text.c_str() + p_start, p_end - p_start, &err);
	if (U_FAILURE(err)) {
		ERR_PRINTS(u_errorName(err));
		return runs;
	}
	Vector<BreakRun> break_runs;
	int32_t prev = p_start;
	while (ubrk_next(bi) != UBRK_DONE) {
		uint32_t ch;
		U16_GET(p_text.c_str(), 0, p_start + ubrk_current(bi) - 1, p_text.length(), ch);
		uint32_t nch;
		U16_GET(p_text.c_str(), 0, p_start + ubrk_current(bi), p_text.length(), nch);
		if ((nch < 0x1361) || (nch > 0x1368)) {
			uint32_t btype = RUN_BREAK_SOFT;
			if ((ch == 0x000A) || (ch == 0x000B) || (ch == 0x000C) || (ch == 0x000D) || (ch == 0x0085) || (ch == 0x2028) || (ch == 0x2029)) {
				btype = RUN_BREAK_HARD;
			}
			runs.push_back(SourceRun(prev, p_start + ubrk_current(bi), btype));
		}

		prev = p_start + ubrk_current(bi);
	}
	ubrk_close(bi);

	return runs;
}

void shape_subrun2(const String &p_text, int32_t p_start, int32_t p_end, const Vector<Ref<FontImplementation>> &p_fonts, const Vector<Ref<FontImplementation>> &p_fonts_outline, int p_fb_index, hb_language_t p_language, const Vector<int32_t> &p_spaces, const Vector<int32_t> &p_kashidas, const Vector<hb_feature_t> p_font_features, bool p_prefer_vertical, Run &o_run) {
	///*DEBUG*/ printf("      subrun shape(ICUHB) %d %d FBI%d/(%d, %d)\n", p_start, p_end, p_fb_index, p_fonts.size(), p_fonts_outline.size());
	Vector<Cluster> clusters;
	if (p_fb_index == p_fonts.size()) {
		const CharType *sptr = &p_text[0];

		int i = p_start;
		while (i < p_end) {
			Cluster cl;

			cl.start = i;
			cl.end = i;

			//decode surrogates
			uint32_t c = sptr[i];
			if (((c & 0xFFFFF800) == 0xD800) && ((c & 0x400) == 0) && (i + 1 < p_text.length())) {
				uint32_t c2 = sptr[i + 1];
				if ((c2 & 0x400) != 0) {
					c = (c << 10UL) + c2 - ((0xD800 << 10UL) + 0xDC00 - 0x10000);
					cl.end = i + 1;
					i++;
				}
			}

			Glyph gl;

			gl.codepoint = c;
			gl.offset = Vector2();
			gl.advance = FontHexBox::get_advance(c);
			cl.glyphs.push_back(gl);

			cl.font_impl = Ref<FontImplementation>();
			cl.font_outline_impl = Ref<FontImplementation>();
			cl.advance = gl.advance;
			cl.repeat = 1;
			cl.flags = GLYPH_VALID;

			if (u_isspace(c)) cl.flags |= GLYPH_SPACE;
			if (c == 0x0009) cl.flags |= GLYPH_TAB;
			if (c == 0x0640) cl.flags |= GLYPH_ELONGATION;
			if (u_iscntrl(c) && (c != 0x0009)) cl.flags |= GLYPH_CONTROL;
			if (c > 0xFFFF) cl.flags |= GLYPH_SURROGATE;

			if (o_run.level % 2 == 0)
				clusters.push_back(cl);
			else
				clusters.insert(0, cl);

			i++;
		}
	} else {
		hb_font_t *hb_font = NULL;
		if (p_fonts[p_fb_index].is_valid()) {
			hb_font = (hb_font_t *)p_fonts[p_fb_index]->get_native_handle();
		}
		if (!hb_font) {
			return shape_subrun2(p_text, p_start, p_end, p_fonts, p_fonts_outline, p_fb_index + 1, p_language, p_spaces, p_kashidas, p_font_features, p_prefer_vertical, o_run);
		}
		//Shape using HB
		hb_buffer_t *hb_buffer = hb_buffer_create();
		if (p_prefer_vertical) {
			hb_buffer_set_direction(hb_buffer, (o_run.level % 2 == 0) ? HB_DIRECTION_TTB : HB_DIRECTION_BTT);
		} else {
			hb_buffer_set_direction(hb_buffer, (o_run.level % 2 == 0) ? HB_DIRECTION_LTR : HB_DIRECTION_RTL);
		}
		hb_buffer_set_flags(hb_buffer, (hb_buffer_flags_t)(HB_BUFFER_FLAG_PRESERVE_DEFAULT_IGNORABLES | (p_start == 0 ? HB_BUFFER_FLAG_BOT : 0) | (p_end == p_text.length() ? HB_BUFFER_FLAG_EOT : 0)));
		hb_buffer_set_script(hb_buffer, (hb_script_t)o_run.script);
		hb_buffer_set_language(hb_buffer, p_language);
		hb_buffer_add_utf16(hb_buffer, (const uint16_t *)p_text.c_str(), p_text.length(), p_start, p_end - p_start);
		hb_shape(hb_font, hb_buffer, p_font_features.empty() ? NULL : &p_font_features[0], p_font_features.size());

		unsigned int glyph_count;
		hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(hb_buffer, &glyph_count);
		hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(hb_buffer, &glyph_count);

		if (glyph_count > 0) {
			uint32_t last_cluster_id = -1;
			int32_t fisrt_cluster_index = -1;
			int32_t last_cluster_index = -1;
			for (unsigned int i = 0; i < glyph_count; i++) {
				if (last_cluster_id != glyph_info[i].cluster) {
					Cluster cl;
					cl.start = glyph_info[i].cluster;
					cl.end = glyph_info[i].cluster;
					cl.repeat = 1;
					cl.font_impl = p_fonts[p_fb_index];
					cl.font_outline_impl = p_fonts_outline[p_fb_index];
					cl.flags = GLYPH_VALID;

					if ((i != 0) && (last_cluster_index != -1)) {
						if (o_run.level % 2 == 0) {
							clusters.write[last_cluster_index].end = glyph_info[i].cluster - 1;
						} else {
							cl.end = clusters[last_cluster_index].start - 1;
						}
					}
					if (o_run.level % 2 != 0) {
						for (int32_t j = 0; j < p_spaces.size(); j++) {
							if ((p_spaces[j] == cl.start) && ((cl.flags & GLYPH_SPACE) != GLYPH_SPACE)) {
								Cluster vcl;
								vcl.start = -1;
								vcl.end = -1;
								vcl.repeat = 1;
								vcl.font_impl = p_fonts[p_fb_index];
								vcl.font_outline_impl = p_fonts_outline[p_fb_index];
								vcl.flags = GLYPH_VALID | GLYPH_SPACE | GLYPH_VIRTUAL;

								Glyph vgl;
								vgl.codepoint = p_fonts[p_fb_index]->get_glyph(0x0020);
								vgl.offset = Point2();
								vgl.advance = 0;

								if (vgl.codepoint != 0) {
									vcl.glyphs.push_back(vgl);
									vcl.advance = vgl.advance;
									clusters.push_back(vcl);
								}
							}
						}
					}
					for (int32_t j = 0; j < p_kashidas.size(); j++) {
						if ((p_kashidas[j] == cl.start) && (i < glyph_count - 1)) {
							Cluster vcl;
							vcl.start = -1;
							vcl.end = -1;
							vcl.repeat = 0;
							vcl.font_impl = p_fonts[p_fb_index];
							vcl.font_outline_impl = p_fonts_outline[p_fb_index];
							vcl.flags = GLYPH_VALID | GLYPH_ELONGATION | GLYPH_VIRTUAL;

							Glyph vgl;
							vgl.codepoint = p_fonts[p_fb_index]->get_glyph(0x0640);
							if (p_prefer_vertical) {
								vgl.offset = Point2(0, -glyph_pos[i + 1].x_offset) * p_fonts[p_fb_index]->get_font_scale() / 64.0;
								vgl.advance = p_fonts[p_fb_index]->get_v_advance(vgl.codepoint);
							} else {
								vgl.offset = Point2(0, -glyph_pos[i + 1].y_offset) * p_fonts[p_fb_index]->get_font_scale() / 64.0;
								vgl.advance = p_fonts[p_fb_index]->get_advance(vgl.codepoint);
							}

							if (vgl.codepoint != 0) {
								vcl.glyphs.push_back(vgl);
								vcl.advance = vgl.advance;
								clusters.push_back(vcl);
							}
						}
					}
					clusters.push_back(cl);
					last_cluster_index = clusters.size() - 1;
					if (fisrt_cluster_index == -1) fisrt_cluster_index = clusters.size() - 1;
					if (o_run.level % 2 == 0) {
						for (int32_t j = 0; j < p_spaces.size(); j++) {
							if ((p_spaces[j] == cl.start) && ((cl.flags & GLYPH_SPACE) != GLYPH_SPACE)) {
								Cluster vcl;
								vcl.start = -1;
								vcl.end = -1;
								vcl.repeat = 1;
								vcl.font_impl = p_fonts[p_fb_index];
								vcl.font_outline_impl = p_fonts_outline[p_fb_index];
								vcl.flags = GLYPH_VALID | GLYPH_SPACE | GLYPH_VIRTUAL;

								Glyph vgl;
								vgl.codepoint = p_fonts[p_fb_index]->get_glyph(0x0020);
								vgl.offset = Point2();
								vgl.advance = 0;

								if (vgl.codepoint != 0) {
									vcl.glyphs.push_back(vgl);
									vcl.advance = vgl.advance;
									clusters.push_back(vcl);
								}
							}
						}
					}
				}
				last_cluster_id = glyph_info[i].cluster;
				if (last_cluster_index != -1) {
					uint32_t lc;
					U16_GET(p_text.c_str(), 0, glyph_info[i].cluster, (size_t)p_text.length(), lc);
					if (!u_iscntrl(lc) || (lc == 0x0009)) {
						Glyph gl;

						gl.codepoint = glyph_info[i].codepoint;
						if (p_prefer_vertical) {
							gl.offset = Point2(glyph_pos[i].y_offset, -glyph_pos[i].x_offset) * p_fonts[p_fb_index]->get_font_scale() / 64.0;
							gl.advance = (glyph_pos[i].y_advance * p_fonts[p_fb_index]->get_font_scale()) / 64.0;
						} else {
							gl.offset = Point2(glyph_pos[i].x_offset, -glyph_pos[i].y_offset) * p_fonts[p_fb_index]->get_font_scale() / 64.0;
							gl.advance = (glyph_pos[i].x_advance * p_fonts[p_fb_index]->get_font_scale()) / 64.0;
						}

						if (((glyph_info[i].codepoint == 0) && u_isgraph(lc)) || (lc == 0x0009)) {
							clusters.write[last_cluster_index].flags &= ~GLYPH_VALID;
						}
						if (u_isspace(lc)) clusters.write[last_cluster_index].flags |= GLYPH_SPACE;
						if (lc == 0x0009) { //replace tab with zero width space
							clusters.write[last_cluster_index].flags |= GLYPH_TAB;
							gl.codepoint = (clusters[last_cluster_index].font_impl.is_valid()) ? clusters[last_cluster_index].font_impl->get_glyph(0x0020) : 0;
							if (gl.codepoint != 0) clusters.write[last_cluster_index].flags |= GLYPH_VALID;
							gl.offset = Point2();
							gl.advance = 0;
						}
						if (lc == 0x0640) clusters.write[last_cluster_index].flags |= GLYPH_ELONGATION;
						if (lc > 0xFFFF) clusters.write[last_cluster_index].flags |= GLYPH_SURROGATE;

						if (is_rotated(lc) && p_prefer_vertical) clusters.write[last_cluster_index].flags |= GLYPH_ROTATE_CW;

						clusters.write[last_cluster_index].glyphs.push_back(gl);
						clusters.write[last_cluster_index].advance += gl.advance;
					} else {
						Glyph vgl;
						vgl.codepoint = lc;
						vgl.offset = Point2();
						vgl.advance = FontHexBox::get_advance(lc);
						clusters.write[last_cluster_index].font_impl = Ref<FontImplementation>();
						clusters.write[last_cluster_index].font_outline_impl = Ref<FontImplementation>();
						clusters.write[last_cluster_index].glyphs.push_back(vgl);
						clusters.write[last_cluster_index].advance += vgl.advance;
						clusters.write[last_cluster_index].repeat = 0;

						clusters.write[last_cluster_index].flags |= GLYPH_CONTROL;
					}
				}
			}
			if (clusters.size() > 0) {
				if ((o_run.level % 2 == 0) && (last_cluster_index != -1)) {
					clusters.write[last_cluster_index].end = p_end - 1;
				} else if (fisrt_cluster_index != -1) {
					clusters.write[fisrt_cluster_index].end = p_end - 1;
				}
			}
		}
		if (hb_buffer) hb_buffer_destroy(hb_buffer);
	}

	//process font fallbacks
	int32_t failed_subrun_start = p_end + 1;
	int32_t failed_subrun_end = p_start;
	for (int32_t i = 0; i < clusters.size(); i++) {
		if ((clusters[i].flags & GLYPH_VALID) == GLYPH_VALID) {
			if (failed_subrun_start != p_end + 1) {
				shape_subrun2(p_text, failed_subrun_start, failed_subrun_end + 1, p_fonts, p_fonts_outline, p_fb_index + 1, p_language, p_spaces, p_kashidas, p_font_features, p_prefer_vertical, o_run);
			}
			failed_subrun_start = p_end + 1;
			failed_subrun_end = p_start;

			o_run.clusters.push_back(clusters[i]);

			o_run.width += clusters[i].advance * clusters[i].repeat;
			if (clusters[i].repeat > 0) {
				for (int32_t j = 0; j < clusters[i].glyphs.size(); j++) {
					if (clusters[i].font_impl.is_valid()) {
						o_run.ascent = MAX(o_run.ascent, MAX(clusters[i].font_impl->get_ascent(), clusters[i].glyphs[j].offset.y));
						o_run.descent = MAX(o_run.descent, MAX(clusters[i].font_impl->get_descent(), -clusters[i].glyphs[j].offset.y));
					}
				}
			}
		} else {
			if (failed_subrun_start >= clusters[i].start) failed_subrun_start = clusters[i].start;
			if (failed_subrun_end <= clusters[i].end) failed_subrun_end = clusters[i].end;
		}
	}
	if (failed_subrun_start != p_end + 1) {
		shape_subrun2(p_text, failed_subrun_start, failed_subrun_end + 1, p_fonts, p_fonts_outline, p_fb_index + 1, p_language, p_spaces, p_kashidas, p_font_features, p_prefer_vertical, o_run);
	}
}

Run ShapingInterfaceHBICU::shape_run2(const String &p_text, int p_start, int p_end, const Font *p_font, const String &p_features, const String &p_language, uint32_t p_level, uint32_t p_script, uint32_t p_break, bool p_prefer_vertical) const {
	Run r;

	r.start = p_start;
	r.end = p_end;

	r.level = p_level;
	r.script = p_script;
	r.break_type = (RunBreakType)p_break;

	r.width = 0.f;
	r.ascent = 0.f;
	r.descent = 0.f;

	Vector<int32_t> spaces;
	Vector<int32_t> kashidas;

	hb_language_t language = hb_language_from_string(p_language.ascii().get_data(), -1);

	UErrorCode err = U_ZERO_ERROR;
	UBreakIterator *bi = ubrk_open(UBRK_WORD, p_language.ascii().get_data(), p_text.c_str() + p_start, p_end - p_start, &err);
	if (U_FAILURE(err)) {
		ERR_PRINTS(u_errorName(err));
		return r;
	}

	int64_t prev = 0;
	while (ubrk_next(bi) != UBRK_DONE) {
		if (ubrk_getRuleStatus(bi) != UBRK_WORD_NONE) {
			uint32_t pch, nch;
			U16_GET(p_text.c_str(), 0, p_start + ubrk_current(bi) - 1, p_text.length(), pch);
			U16_GET(p_text.c_str(), 0, p_start + ubrk_current(bi), p_text.length(), nch);
			if (!u_ispunct(pch) && !(u_charType(nch) == U_DASH_PUNCTUATION)) {
				generate_kashida_justification_opportunies(p_text.c_str(), p_start + prev, p_start + ubrk_current(bi), kashidas);
				spaces.push_back(p_start + ubrk_current(bi));
				prev = ubrk_current(bi);
			}
		}
	}
	ubrk_close(bi);

	Vector<hb_feature_t> font_features;
	Vector<String> v_features = p_features.split(",");
	for (int64_t i = 0; i < v_features.size(); i++) {
		hb_feature_t feature;
		if (hb_feature_from_string(v_features[i].ascii().get_data(), -1, &feature)) {
			font_features.push_back(feature);
		}
	}

	Vector<Ref<FontImplementation>> fonts;
	Vector<Ref<FontImplementation>> fonts_outline;
	if (p_font) p_font->get_font_implementations(fonts, fonts_outline, p_script);

	shape_subrun2(p_text, p_start, p_end, fonts, fonts_outline, 0, language, spaces, kashidas, font_features, p_prefer_vertical, r);

	r.indent = 0;
	for (int i = r.start; i < r.end; i++) {
		if (p_text[i] == '\t') r.indent++;
	}

	return r;
}

Vector<float> ShapingInterfaceHBICU::get_ligature_caret_offsets(const Ref<FontImplementation> &p_font_imp, TextDirection p_direction, uint32_t p_glyph) const {
	Vector<float> ret;

	unsigned int caret_count = 100;
	hb_position_t caret_array[100];

	hb_ot_layout_get_ligature_carets((hb_font_t *)p_font_imp->get_native_handle(), (p_direction == TEXT_DIRECTION_LTR) ? HB_DIRECTION_LTR : HB_DIRECTION_RTL, p_glyph, 0, &caret_count, &caret_array[0]);
	for (unsigned int i = 0; i < caret_count; i++) {
		ret.push_back(caret_array[i]);
	}

	return ret;
}

//S VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE
TextDirection ShapingInterfaceHBICU::analyse_text(const String &p_text, int32_t p_start, int32_t p_end, TextDirection p_base_direction, const String &p_language, Vector<Run> &p_runs) const {
	UErrorCode err = U_ZERO_ERROR;
	UBiDi *bidi_iter = ubidi_openSized(p_text.length(), 0, &err);
	if (U_FAILURE(err)) {
		ERR_PRINTS(u_errorName(err));
		return TEXT_DIRECTION_INVALID;
	}

	String lang = (p_language == "auto") ? TranslationServer::get_singleton()->get_locale() : p_language;

	TextDirection para_direction = TEXT_DIRECTION_INVALID;
	switch (p_base_direction) {
		case TEXT_DIRECTION_LTR: {
			ubidi_setPara(bidi_iter, p_text.c_str(), p_text.length(), UBIDI_LTR, NULL, &err);
			para_direction = TEXT_DIRECTION_LTR;
		} break;
		case TEXT_DIRECTION_RTL: {
			ubidi_setPara(bidi_iter, p_text.c_str(), p_text.length(), UBIDI_RTL, NULL, &err);
			para_direction = TEXT_DIRECTION_RTL;
		} break;
		case TEXT_DIRECTION_AUTO: {
			UBiDiDirection direction = ubidi_getBaseDirection(p_text.c_str(), p_text.length());
			if (direction != UBIDI_NEUTRAL) {
				ubidi_setPara(bidi_iter, p_text.c_str(), p_text.length(), direction, NULL, &err);
				para_direction = (direction == UBIDI_RTL) ? TEXT_DIRECTION_RTL : TEXT_DIRECTION_LTR;
			} else {
				bool is_rtl = uloc_isRightToLeft(lang.ascii().get_data());
				ubidi_setPara(bidi_iter, p_text.c_str(), p_text.length(), is_rtl ? UBIDI_RTL : UBIDI_LTR, NULL, &err);
				para_direction = is_rtl ? TEXT_DIRECTION_RTL : TEXT_DIRECTION_LTR;
			}
		} break;
		case TEXT_DIRECTION_INVALID: {
			ERR_PRINTS("Invalid base direction!");
			return TEXT_DIRECTION_INVALID;
		}
	}
	if (U_FAILURE(err)) {
		ERR_PRINTS(u_errorName(err));
		return TEXT_DIRECTION_INVALID;
	}
	UBiDi *bidi_iter_base = bidi_iter;
	//line iter
	if ((p_start != 0) || (p_end != p_text.length())) {
		bidi_iter_base = ubidi_openSized((p_end - p_start), 0, &err);
		if (U_FAILURE(err)) {
			ubidi_close(bidi_iter);
			ERR_PRINTS(u_errorName(err));
			return TEXT_DIRECTION_INVALID;
		}
		ubidi_setLine(bidi_iter, p_start, p_end, bidi_iter_base, &err);
		if (U_FAILURE(err)) {
			ubidi_close(bidi_iter);
			ERR_PRINTS(u_errorName(err));
			return TEXT_DIRECTION_INVALID;
		}
	}
	int64_t bidi_run_count = ubidi_countRuns(bidi_iter_base, &err);
	if (U_FAILURE(err)) {
		if (bidi_iter_base != bidi_iter) ubidi_close(bidi_iter_base);
		ubidi_close(bidi_iter);
		ERR_PRINTS(u_errorName(err));
		return TEXT_DIRECTION_INVALID;
	}

	//break iter
	err = U_ZERO_ERROR;
	UBreakIterator *bi = ubrk_open(UBRK_LINE, lang.ascii().get_data(), p_text.c_str() + p_start, p_end - p_start, &err);
	if (U_FAILURE(err)) {
		if (bidi_iter_base != bidi_iter) ubidi_close(bidi_iter_base);
		ubidi_close(bidi_iter);
		ERR_PRINTS(u_errorName(err));
		return TEXT_DIRECTION_INVALID;
	}
	Vector<BreakRun> break_runs;
	int32_t prev = p_start;
	while (ubrk_next(bi) != UBRK_DONE) {
		uint32_t ch;
		U16_GET(p_text.c_str(), 0, p_start + ubrk_current(bi) - 1, p_text.length(), ch);
		uint32_t nch;
		U16_GET(p_text.c_str(), 0, p_start + ubrk_current(bi), p_text.length(), nch);
		if ((nch < 0x1361) || (nch > 0x1368)) {
			break_runs.push_back(BreakRun(prev, p_start + ubrk_current(bi), ch == '\n'));
		}

		prev = p_start + ubrk_current(bi);
	}
	ubrk_close(bi);

	//script iter
	Vector<ScriptRun> script_runs;
	generate_script_runs(p_text.c_str(), p_text.length(), p_start, p_end, script_runs);

	p_runs.clear();

	for (int64_t i = 0; i < bidi_run_count; i++) {
		int32_t bidi_run_start = 0;
		int32_t bidi_run_length = 0;
		uint32_t bidi_run_direction = ubidi_getVisualRun(bidi_iter_base, i, &bidi_run_start, &bidi_run_length);
		bidi_run_start += p_start;

		if (bidi_run_direction % 2 == 0) {
			int32_t j = 0;
			while (j < break_runs.size() && break_runs[j].end < bidi_run_start)
				j++;

			int32_t k = 0;
			while (k < script_runs.size() && script_runs[k].end < bidi_run_start)
				k++;

			int32_t run_start = bidi_run_start;
			int32_t run_end = MIN(MIN(bidi_run_start + bidi_run_length, script_runs[k].end), break_runs[j].end);
			while (run_start < bidi_run_start + bidi_run_length) {

				RunBreakType bt = (run_end != break_runs[j].end) ? RUN_BREAK_NO : (break_runs[j].hard) ? RUN_BREAK_HARD : RUN_BREAK_SOFT;
				p_runs.push_back(Run(run_start, run_end, bidi_run_direction, script_runs[k].script, bt));

				if ((script_runs[k].end <= run_end) && (k < script_runs.size() - 1)) k++;
				if ((break_runs[j].end <= run_end) && (j < break_runs.size() - 1)) j++;

				run_start = run_end;
				run_end = MIN(MIN(bidi_run_start + bidi_run_length, script_runs[k].end), break_runs[j].end);
			}
		} else {
			int32_t j = break_runs.size() - 1;
			while (j >= 0 && break_runs[j].start > bidi_run_start + bidi_run_length)
				j--;

			int32_t k = script_runs.size() - 1;
			while (k >= 0 && script_runs[k].start > bidi_run_start + bidi_run_length)
				k--;

			int32_t run_end = bidi_run_start + bidi_run_length;
			int32_t run_start = MAX(MAX(bidi_run_start, script_runs[k].start), break_runs[j].start);
			while (run_end > bidi_run_start) {

				RunBreakType bt = (run_end != break_runs[j].end) ? RUN_BREAK_NO : (break_runs[j].hard) ? RUN_BREAK_HARD : RUN_BREAK_SOFT;
				p_runs.push_back(Run(run_start, run_end, bidi_run_direction, script_runs[k].script, bt));

				if ((script_runs[k].start >= run_start) && (k > 0)) k--;
				if ((break_runs[j].start >= run_start) && (j > 0)) j--;

				run_end = run_start;
				run_start = MAX(MAX(bidi_run_start, script_runs[k].start), break_runs[j].start);
			}
		}
	}

	//close bidi
	if (bidi_iter_base != bidi_iter) ubidi_close(bidi_iter_base);
	ubidi_close(bidi_iter);

	return para_direction;
}

bool shape_subrun(const String &p_text, int32_t p_start, int32_t p_end, const Vector<Ref<FontImplementation>> &p_fonts, const Vector<Ref<FontImplementation>> &p_fonts_outline, int p_fb_index, Run &p_run, hb_language_t p_language, const Vector<int32_t> &p_spaces, const Vector<int32_t> &p_kashidas, const Vector<hb_feature_t> p_font_features) {
	bool valid = true;
	Vector<Cluster> clusters;
	if (p_fb_index == p_fonts.size()) {
		//Fallback shape
		const CharType *sptr = &p_text[0];

		int32_t i = p_start;
		while (i < p_end) {

			Cluster cl;

			cl.start = i;
			cl.end = i;

			uint32_t c = sptr[i];

			//decode surrogates
			if (((c & 0xFFFFF800) == 0xD800) && ((c & 0x400) == 0) && (i + 1 < p_text.length())) {
				uint32_t c2 = sptr[i + 1];
				if ((c2 & 0x400) != 0) {
					c = (c << 10UL) + c2 - ((0xD800 << 10UL) + 0xDC00 - 0x10000);
					cl.end = i + 1;
					i++;
				}
			}

			Glyph gl;

			gl.codepoint = c;
			gl.offset = Vector2();
			gl.advance = FontHexBox::get_advance(c);
			cl.glyphs.push_back(gl);

			cl.font_impl = Ref<FontImplementation>();
			cl.font_outline_impl = Ref<FontImplementation>();
			cl.advance = gl.advance;
			cl.repeat = 1;
			cl.flags = GLYPH_VALID;

			if (u_isspace(c)) cl.flags |= GLYPH_SPACE;
			if (c == 0x0009) cl.flags |= GLYPH_TAB;
			if (c == 0x0640) cl.flags |= GLYPH_ELONGATION;
			if (u_iscntrl(c) && (c != 0x0009)) cl.flags |= GLYPH_CONTROL;
			if (c > 0xFFFF) cl.flags |= GLYPH_SURROGATE;

			if (p_run.level % 2 == 0)
				clusters.push_back(cl);
			else
				clusters.insert(0, cl);

			i++;
		}
	} else {
		hb_font_t *hb_font = NULL;
		if (p_fonts[p_fb_index].is_valid()) {
			hb_font = (hb_font_t *)p_fonts[p_fb_index]->get_native_handle();
		}
		if (!hb_font) {
			return shape_subrun(p_text, p_start, p_end, p_fonts, p_fonts_outline, p_fb_index + 1, p_run, p_language, p_spaces, p_kashidas, p_font_features);
		}
		//Shape using HB
		hb_buffer_t *hb_buffer = hb_buffer_create();
		hb_buffer_set_direction(hb_buffer, (p_run.level % 2 == 0) ? HB_DIRECTION_LTR : HB_DIRECTION_RTL);
		hb_buffer_set_flags(hb_buffer, (hb_buffer_flags_t)(HB_BUFFER_FLAG_PRESERVE_DEFAULT_IGNORABLES | (p_start == 0 ? HB_BUFFER_FLAG_BOT : 0) | (p_end == p_text.length() ? HB_BUFFER_FLAG_EOT : 0)));
		hb_buffer_set_script(hb_buffer, (hb_script_t)p_run.script);
		hb_buffer_add_utf16(hb_buffer, (const uint16_t *)p_text.c_str(), p_text.length(), p_start, p_end - p_start);
		hb_shape(hb_font, hb_buffer, p_font_features.empty() ? NULL : &p_font_features[0], p_font_features.size());

		unsigned int glyph_count;
		hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(hb_buffer, &glyph_count);
		hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(hb_buffer, &glyph_count);

		if (glyph_count > 0) {
			uint32_t last_cluster_id = -1;
			int32_t fisrt_cluster_index = -1;
			int32_t last_cluster_index = -1;
			for (unsigned int i = 0; i < glyph_count; i++) {
				if (last_cluster_id != glyph_info[i].cluster) {
					Cluster cl;
					cl.start = glyph_info[i].cluster;
					cl.end = glyph_info[i].cluster;
					cl.repeat = 1;
					cl.font_impl = p_fonts[p_fb_index];
					cl.font_outline_impl = p_fonts_outline[p_fb_index];
					cl.flags = GLYPH_VALID;

					if ((i != 0) && (last_cluster_index != -1)) {
						if (p_run.level % 2 == 0) {
							clusters.write[last_cluster_index].end = glyph_info[i].cluster - 1;
						} else {
							cl.end = clusters[last_cluster_index].start - 1;
						}
					}
					if (p_run.level % 2 != 0) {
						for (int32_t j = 0; j < p_spaces.size(); j++) {
							if ((p_spaces[j] == cl.start) && ((cl.flags & GLYPH_SPACE) != GLYPH_SPACE)) {
								Cluster vcl;
								vcl.start = -1;
								vcl.end = -1;
								vcl.repeat = 1;
								vcl.font_impl = p_fonts[p_fb_index];
								vcl.font_outline_impl = p_fonts_outline[p_fb_index];
								vcl.flags = GLYPH_VALID | GLYPH_SPACE | GLYPH_VIRTUAL;

								Glyph vgl;
								vgl.codepoint = p_fonts[p_fb_index]->get_glyph(0x0020);
								vgl.offset = Point2();
								vgl.advance = 0;

								if (vgl.codepoint != 0) {
									vcl.glyphs.push_back(vgl);
									vcl.advance = vgl.advance;
									clusters.push_back(vcl);
								}
							}
						}
					}
					for (int32_t j = 0; j < p_kashidas.size(); j++) {
						if ((p_kashidas[j] == cl.start) && (i < glyph_count - 1)) {
							Cluster vcl;
							vcl.start = -1;
							vcl.end = -1;
							vcl.repeat = 0;
							vcl.font_impl = p_fonts[p_fb_index];
							vcl.font_outline_impl = p_fonts_outline[p_fb_index];
							vcl.flags = GLYPH_VALID | GLYPH_ELONGATION | GLYPH_VIRTUAL;

							Glyph vgl;
							vgl.codepoint = p_fonts[p_fb_index]->get_glyph(0x0640);
							vgl.offset = Point2(0, -glyph_pos[i + 1].y_offset) * p_fonts[p_fb_index]->get_font_scale() / 64.0;
							vgl.advance = p_fonts[p_fb_index]->get_advance(vgl.codepoint);

							if (vgl.codepoint != 0) {
								vcl.glyphs.push_back(vgl);
								vcl.advance = vgl.advance;
								clusters.push_back(vcl);
							}
						}
					}
					clusters.push_back(cl);
					last_cluster_index = clusters.size() - 1;
					if (fisrt_cluster_index == -1) fisrt_cluster_index = clusters.size() - 1;
					if (p_run.level % 2 == 0) {
						for (int32_t j = 0; j < p_spaces.size(); j++) {
							if ((p_spaces[j] == cl.start) && ((cl.flags & GLYPH_SPACE) != GLYPH_SPACE)) {
								Cluster vcl;
								vcl.start = -1;
								vcl.end = -1;
								vcl.repeat = 1;
								vcl.font_impl = p_fonts[p_fb_index];
								vcl.font_outline_impl = p_fonts_outline[p_fb_index];
								vcl.flags = GLYPH_VALID | GLYPH_SPACE | GLYPH_VIRTUAL;

								Glyph vgl;
								vgl.codepoint = p_fonts[p_fb_index]->get_glyph(0x0020);
								vgl.offset = Point2();
								vgl.advance = 0;

								if (vgl.codepoint != 0) {
									vcl.glyphs.push_back(vgl);
									vcl.advance = vgl.advance;
									clusters.push_back(vcl);
								}
							}
						}
					}
				}
				last_cluster_id = glyph_info[i].cluster;
				if (last_cluster_index != -1) {
					uint32_t lc;
					U16_GET(p_text.c_str(), 0, glyph_info[i].cluster, (size_t)p_text.length(), lc);
					if (!u_iscntrl(lc) || (lc == 0x0009)) {
						Glyph gl;

						gl.codepoint = glyph_info[i].codepoint;
						gl.offset = Point2(glyph_pos[i].x_offset, -glyph_pos[i].y_offset) * p_fonts[p_fb_index]->get_font_scale() / 64.0;
						gl.advance = (glyph_pos[i].x_advance * p_fonts[p_fb_index]->get_font_scale()) / 64.0;

						if (((glyph_info[i].codepoint == 0) && u_isgraph(lc)) || (lc == 0x0009)) {
							clusters.write[last_cluster_index].flags &= ~GLYPH_VALID;
						}
						if (u_isspace(lc)) clusters.write[last_cluster_index].flags |= GLYPH_SPACE;
						if (lc == 0x0009) { //replace tab with zero width space
							clusters.write[last_cluster_index].flags |= GLYPH_TAB;
							gl.codepoint = (clusters[last_cluster_index].font_impl.is_valid()) ? clusters[last_cluster_index].font_impl->get_glyph(0x0020) : 0;
							if (gl.codepoint != 0) clusters.write[last_cluster_index].flags |= GLYPH_VALID;
							gl.offset = Point2();
							gl.advance = 0;
						}
						if (lc == 0x0640) clusters.write[last_cluster_index].flags |= GLYPH_ELONGATION;
						if (lc > 0xFFFF) clusters.write[last_cluster_index].flags |= GLYPH_SURROGATE;

						clusters.write[last_cluster_index].glyphs.push_back(gl);
						clusters.write[last_cluster_index].advance += gl.advance;
					} else {
						Glyph vgl;
						vgl.codepoint = lc;
						vgl.offset = Point2();
						vgl.advance = FontHexBox::get_advance(lc);
						clusters.write[last_cluster_index].font_impl = Ref<FontImplementation>();
						clusters.write[last_cluster_index].font_outline_impl = Ref<FontImplementation>();
						clusters.write[last_cluster_index].glyphs.push_back(vgl);
						clusters.write[last_cluster_index].advance += vgl.advance;
						clusters.write[last_cluster_index].repeat = 0;

						clusters.write[last_cluster_index].flags |= GLYPH_CONTROL;
					}
				}
			}
			if (clusters.size() > 0) {
				if ((p_run.level % 2 == 0) && (last_cluster_index != -1)) {
					clusters.write[last_cluster_index].end = p_end - 1;
				} else if (fisrt_cluster_index != -1) {
					clusters.write[fisrt_cluster_index].end = p_end - 1;
				}
			}
		}
		if (hb_buffer) hb_buffer_destroy(hb_buffer);
	}

	//process font fallbacks
	int32_t failed_subrun_start = p_end + 1;
	int32_t failed_subrun_end = p_start;
	for (int32_t i = 0; i < clusters.size(); i++) {
		if ((clusters[i].flags & GLYPH_VALID) == GLYPH_VALID) {
			if (failed_subrun_start != p_end + 1) {
				valid &= shape_subrun(p_text, failed_subrun_start, failed_subrun_end + 1, p_fonts, p_fonts_outline, p_fb_index + 1, p_run, p_language, p_spaces, p_kashidas, p_font_features);
			}
			failed_subrun_start = p_end + 1;
			failed_subrun_end = p_start;

			p_run.clusters.push_back(clusters[i]);

			p_run.width += clusters[i].advance * clusters[i].repeat;
			if (clusters[i].repeat > 0) {
				for (int32_t j = 0; j < clusters[i].glyphs.size(); j++) {
					if (clusters[i].font_impl.is_valid()) {
						p_run.ascent = MAX(p_run.ascent, MAX(clusters[i].font_impl->get_ascent(), clusters[i].glyphs[j].offset.y));
						p_run.descent = MAX(p_run.descent, MAX(clusters[i].font_impl->get_descent(), -clusters[i].glyphs[j].offset.y));
					}
				}
			}
		} else {
			if (failed_subrun_start >= clusters[i].start) failed_subrun_start = clusters[i].start;
			if (failed_subrun_end <= clusters[i].end) failed_subrun_end = clusters[i].end;
		}
	}
	if (failed_subrun_start != p_end + 1) {
		valid &= shape_subrun(p_text, failed_subrun_start, failed_subrun_end + 1, p_fonts, p_fonts_outline, p_fb_index + 1, p_run, p_language, p_spaces, p_kashidas, p_font_features);
	}

	return valid;
}

bool ShapingInterfaceHBICU::shape_run(const String &p_text, const Font *p_font, Run &p_run, const String &p_language, const String &p_features) const {
	ERR_FAIL_COND_V(p_font == NULL, false);

	Vector<int32_t> spaces;
	Vector<int32_t> kashidas;

	String lang = (p_language == "auto") ? TranslationServer::get_singleton()->get_locale() : p_language;
	hb_language_t language = hb_language_from_string(lang.ascii().get_data(), -1);

	UErrorCode err = U_ZERO_ERROR;
	UBreakIterator *bi = ubrk_open(UBRK_WORD, lang.ascii().get_data(), p_text.c_str() + p_run.start, p_run.end - p_run.start, &err);
	if (U_FAILURE(err)) {
		ERR_PRINTS(u_errorName(err));
		return false;
	}

	int64_t prev = 0;
	while (ubrk_next(bi) != UBRK_DONE) {
		if (ubrk_getRuleStatus(bi) != UBRK_WORD_NONE) {
			uint32_t pch, nch;
			U16_GET(p_text.c_str(), 0, p_run.start + ubrk_current(bi) - 1, p_text.length(), pch);
			U16_GET(p_text.c_str(), 0, p_run.start + ubrk_current(bi), p_text.length(), nch);
			if (!u_ispunct(pch) && !(u_charType(nch) == U_DASH_PUNCTUATION)) {
				generate_kashida_justification_opportunies(p_text.c_str(), p_run.start + prev, p_run.start + ubrk_current(bi), kashidas);
				spaces.push_back(p_run.start + ubrk_current(bi));
				prev = ubrk_current(bi);
			}
		}
	}
	ubrk_close(bi);

	Vector<hb_feature_t> font_features;
	Vector<String> v_features = p_features.split(",");
	for (int64_t i = 0; i < v_features.size(); i++) {
		hb_feature_t feature;
		if (hb_feature_from_string(v_features[i].ascii().get_data(), -1, &feature)) {
			font_features.push_back(feature);
		}
	}

	p_run.ascent = 0;
	p_run.descent = 0;
	p_run.width = 0;

	Vector<Ref<FontImplementation>> fonts;
	Vector<Ref<FontImplementation>> fonts_outline;
	p_font->get_font_implementations(fonts, fonts_outline, p_run.script);

	return shape_subrun(p_text, p_run.start, p_run.end, fonts, fonts_outline, 0, p_run, language, spaces, kashidas, font_features);
}

TextDirection ShapingInterfaceHBICU::get_string_direction(const String &p_text) const {
	UBiDiDirection direction = ubidi_getBaseDirection(p_text.c_str(), p_text.length());
	if (direction == UBIDI_NEUTRAL) {
		return TEXT_DIRECTION_AUTO;
	} else if (direction == UBIDI_RTL) {
		return TEXT_DIRECTION_RTL;
	} else {
		return TEXT_DIRECTION_LTR;
	}
}
//E VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE

TextDirection ShapingInterfaceHBICU::get_locale_direction(const String &p_lang) const {
	if (uloc_isRightToLeft(p_lang.ascii().get_data())) {
		return TEXT_DIRECTION_RTL;
	} else {
		return TEXT_DIRECTION_LTR;
	}
}
