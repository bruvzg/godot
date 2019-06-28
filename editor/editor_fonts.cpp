/*************************************************************************/
/*  editor_fonts.cpp                                                     */
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

#include "editor_fonts.h"

#include "builtin_fonts.gen.h"
#include "core/os/dir_access.h"
#include "editor_scale.h"
#include "editor_settings.h"
#include "scene/resources/default_theme/default_theme.h"
#include "scene/resources/font.h"

#define MAKE_FALLBACKS(m_name)           \
	m_name->add_font_data(FontArabic);   \
	m_name->add_font_data(FontHebrew);   \
	m_name->add_font_data(FontThai);     \
	m_name->add_font_data(FontHindi);    \
	m_name->add_font_data(FontJapanese); \
	m_name->add_font_data(FontFallback); \
	m_name->add_font_data(FontUrdu);

// the custom spacings might only work with Noto Sans
#define MAKE_DEFAULT_FONT(m_name, m_size, m_hinting, m_aa)     \
	Ref<Font> m_name;                                          \
	m_name.instance();                                         \
	m_name->set_size(m_size);                                  \
	if (CustomFont.is_valid()) {                               \
		m_name->add_font_data(CustomFont);                     \
		m_name->add_font_data(DefaultFont);                    \
	} else {                                                   \
		m_name->add_font_data(DefaultFont);                    \
	}                                                          \
	m_name->set_hinting(m_hinting);                            \
	m_name->set_antialiased(m_aa);                             \
	m_name->set_extra_spacing(Font::SPACING_TOP, -EDSCALE);    \
	m_name->set_extra_spacing(Font::SPACING_BOTTOM, -EDSCALE); \
	MAKE_FALLBACKS(m_name);

#define MAKE_BOLD_FONT(m_name, m_size, m_hinting, m_aa)        \
	Ref<Font> m_name;                                          \
	m_name.instance();                                         \
	m_name->set_size(m_size);                                  \
	if (CustomFont.is_valid()) {                               \
		m_name->add_font_data(CustomFontBold);                 \
		m_name->add_font_data(DefaultFontBold);                \
	} else {                                                   \
		m_name->add_font_data(DefaultFontBold);                \
	}                                                          \
	m_name->set_hinting(m_hinting);                            \
	m_name->set_antialiased(m_aa);                             \
	m_name->set_extra_spacing(Font::SPACING_TOP, -EDSCALE);    \
	m_name->set_extra_spacing(Font::SPACING_BOTTOM, -EDSCALE); \
	MAKE_FALLBACKS(m_name);

#define MAKE_SOURCE_FONT(m_name, m_size, m_hinting, m_aa)      \
	Ref<Font> m_name;                                          \
	m_name.instance();                                         \
	m_name->set_size(m_size);                                  \
	if (CustomFontSource.is_valid()) {                         \
		m_name->add_font_data(CustomFontSource);               \
		m_name->add_font_data(dfmono);                         \
	} else {                                                   \
		m_name->add_font_data(dfmono);                         \
	}                                                          \
	m_name->set_hinting(m_hinting);                            \
	m_name->set_antialiased(m_aa);                             \
	m_name->set_extra_spacing(Font::SPACING_TOP, -EDSCALE);    \
	m_name->set_extra_spacing(Font::SPACING_BOTTOM, -EDSCALE); \
	MAKE_FALLBACKS(m_name);

void editor_register_fonts(Ref<Theme> p_theme) {
	DirAccess *dir = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);

	/* Custom font */

	bool font_antialiased = (bool)EditorSettings::get_singleton()->get("interface/editor/font_antialiased");
	int font_hinting_setting = (int)EditorSettings::get_singleton()->get("interface/editor/font_hinting");

	FontData::Hinting font_hinting;
	switch (font_hinting_setting) {
		case 0:
			// The "Auto" setting uses the setting that best matches the OS' font rendering:
			// - macOS doesn't use font hinting.
			// - Windows uses ClearType, which is in between "Light" and "Normal" hinting.
			// - Linux has configurable font hinting, but most distributions including Ubuntu default to "Light".
#ifdef OSX_ENABLED
			font_hinting = FontData::HINTING_NONE;
#else
			font_hinting = FontData::HINTING_LIGHT;
#endif
			break;
		case 1:
			font_hinting = FontData::HINTING_NONE;
			break;
		case 2:
			font_hinting = FontData::HINTING_LIGHT;
			break;
		default:
			font_hinting = FontData::HINTING_NORMAL;
			break;
	}

	String custom_font_path = EditorSettings::get_singleton()->get("interface/editor/main_font");
	Ref<FontData> CustomFont;
	if (custom_font_path.length() > 0 && dir->file_exists(custom_font_path)) {
		CustomFont.instance();
		CustomFont->set_font_data_path(custom_font_path);
		CustomFont->set_force_priority_multiplier(5.0f);
	} else {
		EditorSettings::get_singleton()->set_manually("interface/editor/main_font", "");
	}

	/* Custom Bold font */

	String custom_font_path_bold = EditorSettings::get_singleton()->get("interface/editor/main_font_bold");
	Ref<FontData> CustomFontBold;
	if (custom_font_path_bold.length() > 0 && dir->file_exists(custom_font_path_bold)) {
		CustomFontBold.instance();
		CustomFontBold->set_font_data_path(custom_font_path_bold);
		CustomFontBold->set_force_priority_multiplier(5.0f);
	} else {
		EditorSettings::get_singleton()->set_manually("interface/editor/main_font_bold", "");
	}

	/* Custom source code font */

	String custom_font_path_source = EditorSettings::get_singleton()->get("interface/editor/code_font");
	Ref<FontData> CustomFontSource;
	if (custom_font_path_source.length() > 0 && dir->file_exists(custom_font_path_source)) {
		CustomFontSource.instance();
		CustomFontSource->set_font_data_path(custom_font_path_source);
		CustomFontSource->set_force_priority_multiplier(5.0f);
	} else {
		EditorSettings::get_singleton()->set_manually("interface/editor/code_font", "");
	}

	memdelete(dir);

	/* Droid Sans */

	Ref<FontData> DefaultFont;
	DefaultFont.instance();
	DefaultFont->_set_static_font_data("dynamic", _font_NotoSansUI_Regular, _font_NotoSansUI_Regular_size);

	Ref<FontData> DefaultFontBold;
	DefaultFontBold.instance();
	DefaultFontBold->_set_static_font_data("dynamic", _font_NotoSansUI_Bold, _font_NotoSansUI_Bold_size);

	Ref<FontData> FontFallback;
	FontFallback.instance();
	FontFallback->_set_static_font_data("dynamic", _font_DroidSansFallback, _font_DroidSansFallback_size);

	Ref<FontData> FontJapanese;
	FontJapanese.instance();
	FontJapanese->_set_static_font_data("dynamic", _font_DroidSansJapanese, _font_DroidSansJapanese_size);

	Ref<FontData> FontArabic;
	FontArabic.instance();
	FontArabic->_set_static_font_data("dynamic", _font_NotoNaskhArabicUI_Regular, _font_NotoNaskhArabicUI_Regular_size);
	FontArabic->set_force_supported_languages("ar");

	Ref<FontData> FontHebrew;
	FontHebrew.instance();
	FontHebrew->_set_static_font_data("dynamic", _font_NotoSansHebrew_Regular, _font_NotoSansHebrew_Regular_size);

	Ref<FontData> FontThai;
	FontThai.instance();
	FontThai->_set_static_font_data("dynamic", _font_NotoSansThaiUI_Regular, _font_NotoSansThaiUI_Regular_size);

	Ref<FontData> FontHindi;
	FontHindi.instance();
	FontHindi->_set_static_font_data("dynamic", _font_NotoSansDevanagariUI_Regular, _font_NotoSansDevanagariUI_Regular_size);

	Ref<FontData> FontUrdu;
	FontUrdu.instance();
	FontUrdu->_set_static_font_data("dynamic", _font_NotoNastaliqUrdu_Regular, _font_NotoNastaliqUrdu_Regular_size);
	FontUrdu->set_force_supported_languages("ur_PK,ur");

	/* Hack */
	Ref<FontData> dfmono;
	dfmono.instance();
	dfmono->_set_static_font_data("dynamic", _font_Hack_Regular, _font_Hack_Regular_size);

	int default_font_size = int(EDITOR_GET("interface/editor/main_font_size")) * EDSCALE;

	// Default font
	MAKE_DEFAULT_FONT(df, default_font_size, font_hinting, font_antialiased);
	p_theme->set_default_theme_font(df);

	// Bold font
	MAKE_BOLD_FONT(df_bold, default_font_size, font_hinting, font_antialiased);
	p_theme->set_font("bold", "EditorFonts", df_bold);

	// Title font
	MAKE_BOLD_FONT(df_title, default_font_size + 2 * EDSCALE, font_hinting, font_antialiased);
	p_theme->set_font("title", "EditorFonts", df_title);

	// Documentation fonts
	MAKE_DEFAULT_FONT(df_doc, int(EDITOR_GET("text_editor/help/help_font_size")) * EDSCALE, font_hinting, font_antialiased);
	MAKE_BOLD_FONT(df_doc_bold, int(EDITOR_GET("text_editor/help/help_font_size")) * EDSCALE, font_hinting, font_antialiased);
	MAKE_BOLD_FONT(df_doc_title, int(EDITOR_GET("text_editor/help/help_title_font_size")) * EDSCALE, font_hinting, font_antialiased);
	MAKE_SOURCE_FONT(df_doc_code, int(EDITOR_GET("text_editor/help/help_source_font_size")) * EDSCALE, font_hinting, font_antialiased);

	p_theme->set_font("doc", "EditorFonts", df_doc);
	p_theme->set_font("doc_bold", "EditorFonts", df_doc_bold);
	p_theme->set_font("doc_title", "EditorFonts", df_doc_title);
	p_theme->set_font("doc_source", "EditorFonts", df_doc_code);

	// Ruler font
	MAKE_DEFAULT_FONT(df_rulers, 8 * EDSCALE, font_hinting, font_antialiased);
	p_theme->set_font("rulers", "EditorFonts", df_rulers);

	// Code font
	MAKE_SOURCE_FONT(df_code, int(EDITOR_GET("interface/editor/code_font_size")) * EDSCALE, font_hinting, font_antialiased);
	p_theme->set_font("source", "EditorFonts", df_code);

	MAKE_SOURCE_FONT(df_expression, (int(EDITOR_GET("interface/editor/code_font_size")) - 1) * EDSCALE, font_hinting, font_antialiased);
	p_theme->set_font("expression", "EditorFonts", df_expression);

	MAKE_SOURCE_FONT(df_output_code, int(EDITOR_GET("run/output/font_size")) * EDSCALE, font_hinting, font_antialiased);
	p_theme->set_font("output_source", "EditorFonts", df_output_code);

	MAKE_SOURCE_FONT(df_text_editor_status_code, default_font_size, font_hinting, font_antialiased);
	p_theme->set_font("status_source", "EditorFonts", df_text_editor_status_code);
}
