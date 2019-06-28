/*************************************************************************/
/*  label.cpp                                                            */
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

#include "label.h"
#include "core/print_string.h"
#include "core/project_settings.h"
#include "core/translation.h"

void Label::shape_text() {

	lines.clear();

	Ref<Font> font = get_font("font");
	if (font != NULL) {
		String &&display_text = (uppercase) ? xl_text.to_upper() : xl_text;

		total_char_cache = display_text.length();

		//shaped = font->shape_string(display_text, 0, display_text.length(), base_direction, language, ot_features);
		shaped = ShapedString::invalid();

		if (autowrap) {
			shape_lines();
		} else {
			lines.push_back(shaped);
		}
		minimum_size_changed();
	}
}

void Label::shape_lines() {

	if (!autowrap) {
		return;
	}

	lines.clear();

	Ref<StyleBox> style = get_stylebox("normal");
	Ref<Font> font = get_font("font");
	String &&display_text = (uppercase) ? xl_text.to_upper() : xl_text;

	float width = get_size().width - style->get_minimum_size().width;

	Vector<Pair<int32_t, int32_t>> line_ranges = shaped->break_lines(width);
	Point2 origin;
	for (int i = 0; i < line_ranges.size(); i++) {
		//Ref<ShapedString> shaped_line = font->shape_string(display_text, line_ranges[i].first, line_ranges[i].second, base_direction, language, ot_features);
		//ERR_FAIL_COND(shaped_line.is_null());
		//lines.push_back(shaped_line);
	}
}

void Label::set_autowrap(bool p_autowrap) {

	if (autowrap != p_autowrap) {
		autowrap = p_autowrap;
		shape_text();
		update();
	}
}

bool Label::has_autowrap() const {

	return autowrap;
}

void Label::set_uppercase(bool p_uppercase) {

	if (uppercase != p_uppercase) {
		uppercase = p_uppercase;
		shape_text();
		update();
	}
}

bool Label::is_uppercase() const {

	return uppercase;
}

int Label::get_line_height(int p_line) const {

	if (p_line < 0 || p_line >= lines.size()) {
		return get_font("font")->get_height();
	} else {
		return lines[p_line]->get_size().height;
	}
}

void Label::_notification(int p_what) {

	if (p_what == MainLoop::NOTIFICATION_SHAPING_INTERFACE_CHANGED) {
		shape_text();
		update();
	}

	if (p_what == NOTIFICATION_TRANSLATION_CHANGED) {

		String new_text = tr(text);
		if (new_text == xl_text)
			return; //nothing new
		xl_text = new_text;
		shape_text();
		update();
	}

	if (p_what == NOTIFICATION_DRAW) {

		if (clip) {
			VisualServer::get_singleton()->canvas_item_set_clip(get_canvas_item(), true);
		}

		if (shaped->require_reload()) shape_text();

		RID ci = get_canvas_item();

		Size2 string_size;
		Size2 size = get_size();
		Ref<StyleBox> style = get_stylebox("normal");
		Ref<Font> font = get_font("font");
		Color font_color = get_color("font_color");
		Color font_color_shadow = get_color("font_color_shadow");
		bool use_outline = get_constant("shadow_as_outline");
		Point2 shadow_ofs(get_constant("shadow_offset_x"), get_constant("shadow_offset_y"));
		int line_spacing = get_constant("line_spacing");
		Color font_outline_modulate = get_color("font_outline_modulate");

		style->draw(ci, Rect2(Point2(0, 0), get_size()));

		VisualServer::get_singleton()->canvas_item_set_distance_field_mode(get_canvas_item(), font.is_valid() && font->get_distance_field_hint());

		int lines_visible = get_visible_line_count();

		int lines_height = 0;
		for (int32_t i = lines_skipped; i < MIN(lines_skipped + lines_visible + 1, lines.size()); i++) {
			lines_height += (lines[i]->get_size().height + line_spacing);
		}

		int chars_total = 0;
		int vbegin = 0, vsep = 0;

		if (lines_visible > 0) {

			switch (valign) {

				case VALIGN_TOP: {
					//nothing
				} break;
				case VALIGN_CENTER: {
					vbegin = (size.y - lines_height) / 2;
					vsep = line_spacing;

				} break;
				case VALIGN_BOTTOM: {
					vbegin = size.y - lines_height;
					vsep = line_spacing;

				} break;
				case VALIGN_FILL: {
					vbegin = 0;
					if (lines_visible > 1) {
						vsep = (size.y - lines_height) / (lines_visible - 1);
					} else {
						vsep = line_spacing;
					}

				} break;
			}
		}

		float y_ofs = style->get_offset().y + vbegin;
		for (int32_t l = lines_skipped; l < MIN(lines_skipped + lines_visible + 1, lines.size()); l++) {

			y_ofs += lines[l]->get_ascent();
			float x_ofs = 0;

			switch (align) {

				case ALIGN_LEFT: {
					x_ofs = style->get_offset().x;
				} break;
				case ALIGN_CENTER: {
					x_ofs = int(size.width - (lines[l]->get_size().width)) / 2;
				} break;
				case ALIGN_RIGHT: {
					x_ofs = int(size.width - style->get_margin(MARGIN_RIGHT) - (lines[l]->get_size().width));
				} break;
				case ALIGN_FILL_RIGHT:
				case ALIGN_FILL_LEFT: {
					if (l == lines.size() - 1) {
						if (align == ALIGN_FILL_LEFT) x_ofs = style->get_offset().x;
						if (align == ALIGN_FILL_RIGHT) x_ofs = int(size.width - style->get_margin(MARGIN_RIGHT) - (lines[l]->get_size().width));
						break;
					}
					float width = size.width - style->get_margin(MARGIN_RIGHT) - style->get_margin(MARGIN_LEFT);
					lines.write[l] = lines[l]->fit_to_width(width);
				} break;
			}

			const Vector<Run> &v = lines[l]->get_runs();

			//draw shadow
			if (font_color_shadow.a > 0) {
				int chars_total_shadow = chars_total;
				float x_glyph_ofs = x_ofs;
				for (int32_t i = 0; i < v.size(); i++) {
					for (int32_t j = 0; j < v[i].clusters.size(); j++) {
						bool outline = (v[i].clusters[j].font_outline_impl != NULL);
						Ref<FontImplementation> fi = (outline) ? v[i].clusters[j].font_outline_impl : v[i].clusters[j].font_impl;
						for (int32_t k = 0; k < v[i].clusters[j].repeat; k++) {
							for (int32_t g = 0; g < v[i].clusters[j].glyphs.size(); g++) {
								if ((v[i].clusters[j].flags & GLYPH_VALID) == GLYPH_VALID && fi.is_valid()) {
									fi->draw_glyph(ci, Point2(x_glyph_ofs, y_ofs) + v[i].clusters[j].glyphs[g].offset + shadow_ofs, v[i].clusters[j].glyphs[g].codepoint, outline ? font_outline_modulate : font_color_shadow);
									if (use_outline) {
										fi->draw_glyph(ci, Point2(x_glyph_ofs, y_ofs) + v[i].clusters[j].glyphs[g].offset + Vector2(-shadow_ofs.x, shadow_ofs.y), v[i].clusters[j].glyphs[g].codepoint, outline ? font_outline_modulate : font_color_shadow);
										fi->draw_glyph(ci, Point2(x_glyph_ofs, y_ofs) + v[i].clusters[j].glyphs[g].offset + Vector2(shadow_ofs.x, -shadow_ofs.y), v[i].clusters[j].glyphs[g].codepoint, outline ? font_outline_modulate : font_color_shadow);
										fi->draw_glyph(ci, Point2(x_glyph_ofs, y_ofs) + v[i].clusters[j].glyphs[g].offset + Vector2(-shadow_ofs.x, -shadow_ofs.y), v[i].clusters[j].glyphs[g].codepoint, outline ? font_outline_modulate : font_color_shadow);
									}
								}
								x_glyph_ofs += v[i].clusters[j].glyphs[g].advance;
							}
						}
						chars_total_shadow += v[i].clusters[j].end - v[i].clusters[j].start + 1;
						if (visible_chars > 0 || chars_total_shadow <= visible_chars) break;
					}
				}
			}

			//draw outline
			float x_glyph_ofs = x_ofs;
			int chars_total_glpyh = chars_total;
			for (int32_t i = 0; i < v.size(); i++) {
				for (int32_t j = 0; j < v[i].clusters.size(); j++) {
					Ref<FontImplementation> fi = v[i].clusters[j].font_outline_impl;
					for (int32_t k = 0; k < v[i].clusters[j].repeat; k++) {
						for (int32_t g = 0; g < v[i].clusters[j].glyphs.size(); g++) {
							if ((v[i].clusters[j].flags & GLYPH_VALID) == GLYPH_VALID && fi.is_valid()) {
								fi->draw_glyph(ci, Point2(x_glyph_ofs, y_ofs) + v[i].clusters[j].glyphs[g].offset, v[i].clusters[j].glyphs[g].codepoint, font_outline_modulate);
							}
							x_glyph_ofs += v[i].clusters[j].glyphs[g].advance;
						}
					}
					chars_total_glpyh += v[i].clusters[j].end - v[i].clusters[j].start + 1;
					if (visible_chars > 0 || chars_total_glpyh <= visible_chars) break;
				}
			}

			//draw chars
			x_glyph_ofs = x_ofs;
			for (int32_t i = 0; i < v.size(); i++) {
				for (int32_t j = 0; j < v[i].clusters.size(); j++) {
					Ref<FontImplementation> fi = v[i].clusters[j].font_impl;
					for (int32_t k = 0; k < v[i].clusters[j].repeat; k++) {
						for (int32_t g = 0; g < v[i].clusters[j].glyphs.size(); g++) {
							if ((v[i].clusters[j].flags & GLYPH_VALID) == GLYPH_VALID && fi.is_valid()) {
								fi->draw_glyph(ci, Point2(x_glyph_ofs, y_ofs) + v[i].clusters[j].glyphs[g].offset, v[i].clusters[j].glyphs[g].codepoint, font_color);
							} else {
								FontHexBox::draw_glyph(ci, Point2(x_glyph_ofs, y_ofs) + v[i].clusters[j].glyphs[g].offset, v[i].clusters[j].glyphs[g].codepoint, font_color);
							}
							x_glyph_ofs += v[i].clusters[j].glyphs[g].advance;
						}
					}
					chars_total += v[i].clusters[j].end - v[i].clusters[j].start + 1;
					if (visible_chars > 0 || chars_total <= visible_chars) break;
				}
			}

			y_ofs += lines[l]->get_descent() + vsep;
		}
	}

	if (p_what == NOTIFICATION_THEME_CHANGED) {

		shape_text();
		update();
	}
	if (p_what == NOTIFICATION_RESIZED) {

		shape_lines();
	}
}

Size2 Label::get_minimum_size() const {

	Size2 min_style = get_stylebox("normal")->get_minimum_size();
	int line_spacing = get_constant("line_spacing");

	Size2 minsize;

	if (shaped->require_reload()) const_cast<Label *>(this)->shape_text();

	for (int32_t i = 0; i < lines.size(); i++) {
		minsize += lines[i]->get_size();
		minsize.height += line_spacing;
	}

	if (autowrap)
		minsize = Size2(1, clip ? 1 : minsize.height);
	else {
		if (clip)
			minsize.width = 1;
	}
	return minsize + min_style;
}

int Label::get_line_count() const {

	if (!is_inside_tree())
		return 1;

	return lines.size();
}

int Label::get_visible_line_count() const {

	int line_spacing = get_constant("line_spacing");

	int lines_visible = lines.size();

	float lines_height = get_size().height - get_stylebox("normal")->get_minimum_size().height;

	if (shaped->require_reload()) const_cast<Label *>(this)->shape_text();

	for (int32_t i = lines_skipped; i < lines.size(); i++) {
		lines_height -= (lines[i]->get_ascent() + lines[i]->get_descent() + line_spacing);
		if (lines_height <= 0) {
			lines_visible = i - lines_skipped;
			break;
		}
	}

	if (lines_visible > lines.size())
		lines_visible = lines.size();

	if (max_lines_visible >= 0 && lines_visible > max_lines_visible)
		lines_visible = max_lines_visible;

	return lines_visible;
}

void Label::set_align(Align p_align) {

	ERR_FAIL_INDEX((int)p_align, 5);
	align = p_align;
	update();
}

Label::Align Label::get_align() const {

	return align;
}

void Label::set_valign(VAlign p_align) {

	ERR_FAIL_INDEX((int)p_align, 4);
	valign = p_align;
	update();
}

Label::VAlign Label::get_valign() const {

	return valign;
}

void Label::set_text_direction(TextDirection p_text_direction) {

	ERR_FAIL_INDEX((int)p_text_direction, 3);
	if (base_direction != p_text_direction) {
		base_direction = p_text_direction;
		shape_text();
		update();
	}
}

TextDirection Label::get_text_direction() const {

	return base_direction;
}

void Label::set_ot_features(const String &p_features) {

	if (ot_features != p_features) {
		ot_features = p_features;
		shape_text();
		update();
	}
}

String Label::get_ot_features() const {

	return ot_features;
}

void Label::set_language(const String &p_language) {

	if (language != p_language) {
		language = p_language;
		shape_text();
		update();
	}
}

String Label::get_language() const {

	return language;
}

void Label::set_text(const String &p_string) {

	if (text == p_string)
		return;
	text = p_string;
	xl_text = tr(p_string);
	shape_text();
	if (percent_visible < 1)
		visible_chars = get_total_character_count() * percent_visible;
	update();
}

void Label::set_clip_text(bool p_clip) {

	clip = p_clip;
	update();
	minimum_size_changed();
}

bool Label::is_clipping_text() const {

	return clip;
}

String Label::get_text() const {

	return text;
}

void Label::set_visible_characters(int p_amount) {

	visible_chars = p_amount;
	if (get_total_character_count() > 0) {
		percent_visible = (float)p_amount / (float)total_char_cache;
	}
	_change_notify("percent_visible");
	update();
}

int Label::get_visible_characters() const {

	return visible_chars;
}

void Label::set_percent_visible(float p_percent) {

	if (p_percent < 0 || p_percent >= 1) {

		visible_chars = -1;
		percent_visible = 1;

	} else {

		visible_chars = get_total_character_count() * p_percent;
		percent_visible = p_percent;
	}
	_change_notify("visible_chars");
	update();
}

float Label::get_percent_visible() const {

	return percent_visible;
}

void Label::set_lines_skipped(int p_lines) {

	lines_skipped = p_lines;
	update();
}

int Label::get_lines_skipped() const {

	return lines_skipped;
}

void Label::set_max_lines_visible(int p_lines) {

	max_lines_visible = p_lines;
	update();
}

int Label::get_max_lines_visible() const {

	return max_lines_visible;
}

int Label::get_total_character_count() const {

	return total_char_cache;
}

void Label::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_align", "align"), &Label::set_align);
	ClassDB::bind_method(D_METHOD("get_align"), &Label::get_align);
	ClassDB::bind_method(D_METHOD("set_valign", "valign"), &Label::set_valign);
	ClassDB::bind_method(D_METHOD("get_valign"), &Label::get_valign);
	ClassDB::bind_method(D_METHOD("set_text_direction", "direction"), &Label::set_text_direction);
	ClassDB::bind_method(D_METHOD("get_text_direction"), &Label::get_text_direction);
	ClassDB::bind_method(D_METHOD("set_ot_features", "features"), &Label::set_ot_features);
	ClassDB::bind_method(D_METHOD("get_ot_features"), &Label::get_ot_features);
	ClassDB::bind_method(D_METHOD("set_language", "language"), &Label::set_language);
	ClassDB::bind_method(D_METHOD("get_language"), &Label::get_language);
	ClassDB::bind_method(D_METHOD("set_text", "text"), &Label::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &Label::get_text);
	ClassDB::bind_method(D_METHOD("set_autowrap", "enable"), &Label::set_autowrap);
	ClassDB::bind_method(D_METHOD("has_autowrap"), &Label::has_autowrap);
	ClassDB::bind_method(D_METHOD("set_clip_text", "enable"), &Label::set_clip_text);
	ClassDB::bind_method(D_METHOD("is_clipping_text"), &Label::is_clipping_text);
	ClassDB::bind_method(D_METHOD("set_uppercase", "enable"), &Label::set_uppercase);
	ClassDB::bind_method(D_METHOD("is_uppercase"), &Label::is_uppercase);
	ClassDB::bind_method(D_METHOD("get_line_height", "line"), &Label::get_line_height, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("get_line_count"), &Label::get_line_count);
	ClassDB::bind_method(D_METHOD("get_visible_line_count"), &Label::get_visible_line_count);
	ClassDB::bind_method(D_METHOD("get_total_character_count"), &Label::get_total_character_count);
	ClassDB::bind_method(D_METHOD("set_visible_characters", "amount"), &Label::set_visible_characters);
	ClassDB::bind_method(D_METHOD("get_visible_characters"), &Label::get_visible_characters);
	ClassDB::bind_method(D_METHOD("set_percent_visible", "percent_visible"), &Label::set_percent_visible);
	ClassDB::bind_method(D_METHOD("get_percent_visible"), &Label::get_percent_visible);
	ClassDB::bind_method(D_METHOD("set_lines_skipped", "lines_skipped"), &Label::set_lines_skipped);
	ClassDB::bind_method(D_METHOD("get_lines_skipped"), &Label::get_lines_skipped);
	ClassDB::bind_method(D_METHOD("set_max_lines_visible", "lines_visible"), &Label::set_max_lines_visible);
	ClassDB::bind_method(D_METHOD("get_max_lines_visible"), &Label::get_max_lines_visible);

	BIND_ENUM_CONSTANT(ALIGN_LEFT);
	BIND_ENUM_CONSTANT(ALIGN_CENTER);
	BIND_ENUM_CONSTANT(ALIGN_RIGHT);
	BIND_ENUM_CONSTANT(ALIGN_FILL_LEFT);
	BIND_ENUM_CONSTANT(ALIGN_FILL_RIGHT);

	BIND_ENUM_CONSTANT(VALIGN_TOP);
	BIND_ENUM_CONSTANT(VALIGN_CENTER);
	BIND_ENUM_CONSTANT(VALIGN_BOTTOM);
	BIND_ENUM_CONSTANT(VALIGN_FILL);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_MULTILINE_TEXT, "", PROPERTY_USAGE_DEFAULT_INTL), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "align", PROPERTY_HINT_ENUM, "Left,Center,Right,Fill Left, Fill Right"), "set_align", "get_align");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "valign", PROPERTY_HINT_ENUM, "Top,Center,Bottom,Fill"), "set_valign", "get_valign");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "text_direction", PROPERTY_HINT_ENUM, "Auto,LTR,RTL"), "set_text_direction", "get_text_direction");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "ot_features"), "set_ot_features", "get_ot_features");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "language"), "set_language", "get_language");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autowrap"), "set_autowrap", "has_autowrap");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clip_text"), "set_clip_text", "is_clipping_text");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "uppercase"), "set_uppercase", "is_uppercase");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "visible_characters", PROPERTY_HINT_RANGE, "-1,128000,1", PROPERTY_USAGE_EDITOR), "set_visible_characters", "get_visible_characters");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "percent_visible", PROPERTY_HINT_RANGE, "0,1,0.001"), "set_percent_visible", "get_percent_visible");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "lines_skipped", PROPERTY_HINT_RANGE, "0,999,1"), "set_lines_skipped", "get_lines_skipped");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_lines_visible", PROPERTY_HINT_RANGE, "-1,999,1"), "set_max_lines_visible", "get_max_lines_visible");
}

Label::Label(const String &p_text) {

	base_direction = TEXT_DIRECTION_AUTO;
	align = ALIGN_LEFT;
	valign = VALIGN_TOP;
	xl_text = "";
	autowrap = false;
	set_v_size_flags(0);
	clip = false;
	set_mouse_filter(MOUSE_FILTER_IGNORE);
	total_char_cache = 0;
	visible_chars = -1;
	percent_visible = 1;
	lines_skipped = 0;
	max_lines_visible = -1;
	set_text(p_text);
	uppercase = false;
	set_v_size_flags(SIZE_SHRINK_CENTER);
	shaped = ShapedString::invalid();
}

Label::~Label() {
}
