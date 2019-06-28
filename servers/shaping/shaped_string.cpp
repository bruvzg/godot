/*************************************************************************/
/*  shaped_string.cpp                                                    */
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

#include "servers/shaping/shaped_string.h"
#include "scene/resources/font.h"
#include "servers/shaping_server.h"

#include "core/method_bind_ext.gen.inc"
#include "core/os/os.h"

/*************************************************************************/
/*  AttributeMap                                               */
/*************************************************************************/

bool AttributeMap::_compare_attributes(const HashMap<TextAttributes, Variant> &p_first, const HashMap<TextAttributes, Variant> &p_second) const {
	if (p_first.size() != p_second.size()) return false;
	const TextAttributes *K = NULL;
	while ((K = p_first.next(K))) {
		const Variant *E = p_first.getptr(*K);
		const Variant *F = p_second.getptr(*K);
		if ((!F) || (!E) || (*E != *F)) return false;
	}
	return true;
}

void AttributeMap::_ensure_break(int64_t p_key) {
	//Ensures there is a run break at offset.
	const Map<int32_t, HashMap<TextAttributes, Variant>>::Element *E = attribs.find_closest(p_key);
	attribs[p_key] = (E) ? E->get() : HashMap<TextAttributes, Variant>();
}

void AttributeMap::_optimize_attributes() {
	Vector<int> erase_list;
	for (const Map<int32_t, HashMap<TextAttributes, Variant>>::Element *E = attribs.front(); E; E = E->next()) {

		if (E->prev() && (_compare_attributes(E->get(), E->prev()->get()))) {
			erase_list.push_back(E->key());
		}
	}

	for (int64_t i = 0; i < erase_list.size(); i++) {
		attribs.erase(erase_list[i]);
	}
}

void AttributeMap::_bind_methods() {
	ClassDB::bind_method(D_METHOD("has_attribute", "pos", "attrib"), &AttributeMap::has_attribute);

	ClassDB::bind_method(D_METHOD("get_attribute", "pos", "attrib"), &AttributeMap::get_attribute);
	ClassDB::bind_method(D_METHOD("get_attribute_start", "pos"), &AttributeMap::get_attribute_start);
	ClassDB::bind_method(D_METHOD("get_attribute_end", "pos"), &AttributeMap::get_attribute_end);

	ClassDB::bind_method(D_METHOD("add_attribute", "attribute", "value", "start", "end"), &AttributeMap::add_attribute);
	ClassDB::bind_method(D_METHOD("remove_attribute", "attribute", "start", "end"), &AttributeMap::remove_attribute);
	ClassDB::bind_method(D_METHOD("remove_attributes", "start", "end"), &AttributeMap::remove_attributes);

	ClassDB::bind_method(D_METHOD("clear_attributes"), &AttributeMap::clear_attributes);
}

const Map<int32_t, HashMap<TextAttributes, Variant>>::Element *AttributeMap::find_run(int32_t p_pos) const {
	const Map<int32_t, HashMap<TextAttributes, Variant>>::Element *E = attribs.find_closest(p_pos);
	return E;
}

bool AttributeMap::has_attribute(int32_t p_pos, int p_attrib) const {
	const Map<int32_t, HashMap<TextAttributes, Variant>>::Element *E = attribs.find_closest(p_pos);
	if (!E) return false;
	return E->get().has((TextAttributes)p_attrib);
}

Variant AttributeMap::get_attribute(int32_t p_pos, int p_attrib) const {
	const Map<int32_t, HashMap<TextAttributes, Variant>>::Element *E = attribs.find_closest(p_pos);
	if (!E) return Variant();
	if (E->get().has((TextAttributes)p_attrib)) {
		return E->get()[(TextAttributes)p_attrib];
	} else {
		return Variant();
	}
}

int32_t AttributeMap::get_attribute_start(int32_t p_pos) const {
	const Map<int32_t, HashMap<TextAttributes, Variant>>::Element *E = attribs.find_closest(p_pos);
	if (!E) return -1;
	return E->key();
}

int32_t AttributeMap::get_attribute_end(int32_t p_pos) const {
	const Map<int32_t, HashMap<TextAttributes, Variant>>::Element *E = attribs.find_closest(p_pos);
	if (!E) return -1;
	E = E->next();
	if (!E) return -1;
	return E->key();
}

void AttributeMap::add_attribute(int p_attribute, const Variant &p_value, int32_t p_start, int32_t p_end) {
	_ensure_break(0);
	_ensure_break(p_start);
	_ensure_break(p_end);

	Map<int32_t, HashMap<TextAttributes, Variant>>::Element *E = attribs.find(p_start);
	while (E && ((E->key() < p_end))) {
		E->get()[(TextAttributes)p_attribute] = p_value;
		E = E->next();
	}
	_optimize_attributes();
}

void AttributeMap::remove_attribute(int p_attribute, int32_t p_start, int32_t p_end) {
	_ensure_break(p_start);
	_ensure_break(p_end);

	Map<int32_t, HashMap<TextAttributes, Variant>>::Element *E = attribs.find(p_start);
	while (E && (E->key() < p_end)) {
		E->get().erase((TextAttributes)p_attribute);
		E = E->next();
	}
	_optimize_attributes();
}

void AttributeMap::remove_attributes(int32_t p_start, int32_t p_end) {
	_ensure_break(p_start);
	_ensure_break(p_end);
	Map<int32_t, HashMap<TextAttributes, Variant>>::Element *E = attribs.find(p_start);
	while (E && (E->key() < p_end)) {
		E->get().clear();
		E = E->next();
	}
	_optimize_attributes();
}

void AttributeMap::clear_attributes() {
	attribs.clear();
}

/*************************************************************************/
/*  ShapedString                                                         */
/*************************************************************************/

void ShapedString::_bind_methods() {

	ClassDB::bind_method(D_METHOD("require_reload"), &ShapedString::require_reload);

	ClassDB::bind_method(D_METHOD("hit_test", "x"), &ShapedString::hit_test);
	ClassDB::bind_method(D_METHOD("hit_test_cluster", "x"), &ShapedString::hit_test_cluster);

	ClassDB::bind_method(D_METHOD("copy"), &ShapedString::copy);
	ClassDB::bind_method(D_METHOD("align_tabs", "tab_width"), &ShapedString::align_tabs);
	ClassDB::bind_method(D_METHOD("fit_to_width", "width", "trim_edges"), &ShapedString::fit_to_width);
	ClassDB::bind_method(D_METHOD("break_lines", "width"), &ShapedString::_break_lines, DEFVAL(-1));

	ClassDB::bind_method(D_METHOD("draw", "canvas_item", "pos", "clip_l", "clip_r", "modulate", "outline_modulate"), &ShapedString::draw, DEFVAL(0), DEFVAL(-1), DEFVAL(Color(1, 1, 1)), DEFVAL(Color(1, 1, 1, 0)));
	ClassDB::bind_method(D_METHOD("draw_with_attributes", "canvas_item", "pos", "clip_l", "clip_r", "attribs"), &ShapedString::draw_with_attributes);

	ClassDB::bind_method(D_METHOD("get_selection_rects", "start", "end"), &ShapedString::_get_selection_rects);

	ClassDB::bind_method(D_METHOD("get_missing_glyph_count"), &ShapedString::get_missing_glyph_count);

	ClassDB::bind_method(D_METHOD("get_base_direciton"), &ShapedString::get_base_direciton);
	ClassDB::bind_method(D_METHOD("get_dominant_direciton_in_range", "start", "end"), &ShapedString::get_dominant_direciton_in_range);

	ClassDB::bind_method(D_METHOD("get_display_controls"), &ShapedString::get_display_controls);
	ClassDB::bind_method(D_METHOD("set_display_controls", "enable"), &ShapedString::set_display_controls);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "display_controls"), "set_display_controls", "get_display_controls");

	ClassDB::bind_method(D_METHOD("get_ltr_caret_offset", "caret_pos"), &ShapedString::get_ltr_caret_offset);
	ClassDB::bind_method(D_METHOD("get_rtl_caret_offset", "caret_pos"), &ShapedString::get_rtl_caret_offset);

	ClassDB::bind_method(D_METHOD("get_run_start", "pos"), &ShapedString::get_run_start);
	ClassDB::bind_method(D_METHOD("get_run_end", "pos"), &ShapedString::get_run_end);

	ClassDB::bind_method(D_METHOD("get_size"), &ShapedString::get_size);

	ClassDB::bind_method(D_METHOD("get_ascent"), &ShapedString::get_ascent);
	ClassDB::bind_method(D_METHOD("get_descent"), &ShapedString::get_descent);

	ClassDB::bind_method(D_METHOD("get_data"), &ShapedString::get_data);
}

int ShapedString::get_missing_glyph_count() const {
	ERR_FAIL_COND_V(!valid, 0);
	int cnt = 0;
	for (int32_t i = 0; i < visual.size(); i++) {
		for (int32_t j = 0; j < visual[i].clusters.size(); j++) {
			if (((visual[i].clusters[j].flags & GLYPH_VALID) == GLYPH_VALID) || visual[i].clusters[j].font_impl.is_null()) cnt++;
		}
	}
	return cnt;
}

TextDirection ShapedString::get_base_direciton() const {
	ERR_FAIL_COND_V(!valid, TEXT_DIRECTION_INVALID);
	return base_dir;
}

TextDirection ShapedString::get_dominant_direciton_in_range(int32_t p_start, int32_t p_end) const {
	ERR_FAIL_COND_V(!valid, TEXT_DIRECTION_INVALID);

	if (visual.size() == 0) return TEXT_DIRECTION_AUTO; //neutral

	int rtl = 0;
	int ltr = 0;

	for (int32_t i = 0; i < visual.size(); i++) {
		if ((visual[i].end > p_start) && (visual[i].start < p_end)) {
			int mag = MIN(p_end, visual[i].end) - MAX(p_start, visual[i].start);
			if (visual[i].level % 2 == 0)
				ltr += mag;
			else
				rtl += mag;
		}
	}
	if (ltr == rtl) return TEXT_DIRECTION_AUTO; //neutral
	if (ltr > rtl)
		return TEXT_DIRECTION_LTR;
	else
		return TEXT_DIRECTION_RTL;
}

void ShapedString::update_metrics(int p_extra_top, int p_extra_bottom, TextDirection p_base_dir) {
	ascent = 0;
	descent = 0;
	width = 0;

	base_dir = p_base_dir;

	spacing_top = p_extra_top;
	spacing_bottom = p_extra_bottom;

	for (int32_t i = 0; i < visual.size(); i++) {
		ascent = MAX(ascent, visual[i].ascent);
		descent = MAX(descent, visual[i].descent);
		width += visual[i].width;
	}
	ascent += spacing_top;
	descent += spacing_bottom;
	valid = true;
}

const Vector<Run> &ShapedString::get_runs() const {
	return visual;
}

Vector<Run> &ShapedString::get_runs() {
	return visual;
}

bool ShapedString::require_reload() const {
	return !valid;
}

Vector<Pair<int32_t, int32_t>> ShapedString::break_lines(int p_width) const {
	Vector<Pair<int32_t, int32_t>> ret;

	ERR_FAIL_COND_V(!valid, ret);

	if (visual.size() == 0) {
		ret.push_back(Pair<int32_t, int32_t>(0, 0));
		return ret;
	}

	Vector<Run> logical = visual;
	logical.sort_custom<RunCompare>();

	float line_width = 0.f;
	int32_t line_start_run = 0;
	int32_t line_end_run = -1;
	int32_t i = 0;

	while (i < logical.size()) {
		bool break_line = false;
		line_width += logical[i].width;
		if ((p_width > 0) && (line_width > p_width) && (line_end_run != -1)) {
			break_line = true;
		}
		if (!break_line && (logical[i].break_type == RUN_BREAK_SOFT)) {
			line_end_run = i;
		}
		if (!break_line && ((logical[i].break_type == RUN_BREAK_HARD) || (i == logical.size() - 1))) {
			line_end_run = i;
			break_line = true;
		}
		if (break_line) {
			ret.push_back(Pair<int32_t, int32_t>(logical[line_start_run].start, logical[line_end_run].end));

			line_width = 0.f;
			i = line_end_run;

			line_start_run = line_end_run + 1;
			line_end_run = -1;
		}
		i++;
	}
	return ret;
}

Array ShapedString::_break_lines(int p_width) const {
	Array ret;
	ERR_FAIL_COND_V(!valid, ret);

	Vector<Pair<int32_t, int32_t>> lines = break_lines(p_width);

	for (int i = 0; i < lines.size(); i++) {
		Dictionary line;
		line["start"] = lines[i].first;
		line["end"] = lines[i].second;
		ret.push_back(line);
	}

	return ret;
}

int ShapedString::hit_test(float p_x) const {
	ERR_FAIL_COND_V(!valid, -1);

	if (visual.size() == 0) return 0;

	if (p_x < 0) {
		//Hit before first cluster
		if (visual[0].level % 2 != 0) {
			return visual[0].end + 1;
		} else {
			return visual[0].start;
		}
	}

	float pixel_ofs = 0.f;
	for (int32_t i = 0; i < visual.size(); i++) {
		for (int32_t j = 0; j < visual[i].clusters.size(); j++) {
			if ((visual[i].clusters[j].flags & GLYPH_CONTROL) == GLYPH_CONTROL && !display_controls) continue;
			if ((visual[i].clusters[j].flags & GLYPH_VALID) == GLYPH_VALID) {
				if ((visual[i].clusters[j].flags & GLYPH_VIRTUAL) != GLYPH_VIRTUAL) {
					if ((p_x >= pixel_ofs) && (p_x <= pixel_ofs + visual[i].clusters[j].advance * visual[i].clusters[j].repeat)) {
						int count = (visual[i].clusters[j].end + 1 - visual[i].clusters[j].start);
						float char_width = float(visual[i].clusters[j].advance * visual[i].clusters[j].repeat) / float(count);
						int32_t pos_ofs = round(float(p_x - pixel_ofs) / char_width);
						if ((visual[i].clusters[j].flags & GLYPH_SURROGATE) == GLYPH_SURROGATE) {
							//do not allow mid cluster hit for surrogates
							pos_ofs = (pos_ofs < count / 2) ? 0 : count;
						}
						if (visual[i].level % 2 != 0) {
							return visual[i].clusters[j].end + 1 - pos_ofs;
						} else {
							return visual[i].clusters[j].start + pos_ofs;
						}
					} else {
					}
				}
			}
			pixel_ofs += visual[i].clusters[j].advance * visual[i].clusters[j].repeat;
		}
	}

	//Hit after last cluster
	if ((visual.size() > 0) && (p_x > pixel_ofs)) {
		if (visual[visual.size() - 1].level % 2 != 0) {
			return visual[visual.size() - 1].start;
		} else {
			return visual[visual.size() - 1].end + 1;
		}
	}

	return -1;
}

Vector2 ShapedString::hit_test_cluster(float p_x) const {
	ERR_FAIL_COND_V(!valid, Vector2(-1, -1));

	if (visual.size() == 0) return Vector2(-1, -1);

	if (p_x < 0) {
		return Vector2(-1, -1);
	}

	float pixel_ofs = 0.f;
	for (int32_t i = 0; i < visual.size(); i++) {
		for (int32_t j = 0; j < visual[i].clusters.size(); j++) {
			if ((visual[i].clusters[j].flags & GLYPH_CONTROL) == GLYPH_CONTROL && !display_controls) continue;
			if ((p_x >= pixel_ofs) && (p_x <= pixel_ofs + visual[i].clusters[j].advance * visual[i].clusters[j].repeat)) {
				return Vector2(i, j);
			}
			pixel_ofs += visual[i].clusters[j].advance * visual[i].clusters[j].repeat;
		}
	}

	return Vector2(-1, -1);
}

Ref<ShapedString> ShapedString::copy() const {
	Ref<ShapedString> ret;
	ERR_FAIL_COND_V(!valid, ret);

	ret.instance();
	ret->visual = visual;
	ret->display_controls = display_controls;

	ret->update_metrics(spacing_top, spacing_bottom, base_dir);
	return ret;
}

Ref<ShapedString> ShapedString::align_tabs(int p_tab_width) const {
	Ref<ShapedString> ret;
	ERR_FAIL_COND_V(!valid, ret);

	ret.instance();
	ret->visual = visual;
	ret->display_controls = display_controls;

	if (p_tab_width > 0) {
		float width = 0.f;
		for (int32_t i = 0; i < ret->visual.size(); i++) {
			for (int32_t j = 0; j < ret->visual[i].clusters.size(); j++) {
				if ((ret->visual[i].clusters[j].flags & GLYPH_TAB) == GLYPH_TAB) {
					if (ret->visual[i].clusters[j].glyphs.size() == 1) {
						float align = (Math::round(width / p_tab_width) * p_tab_width) - width;
						if (align < 0) align += p_tab_width;

						ret->visual.write[i].width -= ret->visual[i].clusters[j].advance * ret->visual[i].clusters[j].repeat;
						ret->visual.write[i].clusters.write[j].advance = align;
						ret->visual.write[i].clusters.write[j].glyphs.write[0].advance = align;
						ret->visual.write[i].width += ret->visual[i].clusters[j].advance * ret->visual[i].clusters[j].repeat;
					}
				}
				width += ret->visual[i].clusters[j].advance * ret->visual[i].clusters[j].repeat;
			}
		}
	}
	ret->update_metrics(spacing_top, spacing_bottom, base_dir);
	return ret;
}

Ref<ShapedString> ShapedString::fit_to_width(int p_width, bool p_trim_edges) const {
	Ref<ShapedString> ret;
	ERR_FAIL_COND_V(!valid, ret);

	ret.instance();
	ret->visual = visual;
	ret->display_controls = display_controls;
	ret->base_dir = base_dir;

	int32_t elongations = 0;
	int32_t spaces = 0;

	int32_t skip_left_run = 0;
	int32_t skip_left_gl = 0;
	for (int32_t i = 0; i < ret->visual.size(); i++) {
		for (int32_t j = 0; j < ret->visual[i].clusters.size(); j++) {
			if ((ret->visual[i].clusters[j].flags & GLYPH_SPACE) != GLYPH_SPACE) {
				skip_left_run = i;
				skip_left_gl = j;
				goto end_lskip;
			} else if (p_trim_edges) {
				for (int32_t g = 0; g < ret->visual[i].clusters[j].glyphs.size(); g++) {
					ret->visual.write[i].clusters.write[j].glyphs.write[g].advance = 0;
				}
				ret->visual.write[i].width -= ret->visual.write[i].clusters.write[j].advance * ret->visual[i].clusters[j].repeat;
				ret->visual.write[i].clusters.write[j].advance = 0;
			}
		}
	}
end_lskip:

	int32_t skip_right_run = ret->visual.size() - 1;
	int32_t skip_right_gl = 0;
	for (int32_t i = ret->visual.size() - 1; i >= 0; i--) {
		skip_right_gl = ret->visual[i].clusters.size() - 1;
		for (int32_t j = ret->visual[i].clusters.size() - 1; j >= 0; j--) {
			if ((ret->visual[i].clusters[j].flags & GLYPH_SPACE) != GLYPH_SPACE) {
				skip_right_run = i;
				skip_right_gl = j;
				goto end_rskip;
			} else if (p_trim_edges) {
				for (int32_t g = 0; g < ret->visual[i].clusters[j].glyphs.size(); g++) {
					ret->visual.write[i].clusters.write[j].glyphs.write[g].advance = 0;
				}
				ret->visual.write[i].width -= ret->visual.write[i].clusters.write[j].advance * ret->visual[i].clusters[j].repeat;
				ret->visual.write[i].clusters.write[j].advance = 0;
			}
		}
	}
end_rskip:

	float old_width = 0;
	for (int32_t i = 0; i < ret->visual.size(); i++) {
		old_width += ret->visual[i].width;
	}

	for (int32_t i = skip_left_run; i <= skip_right_run; i++) {
		int32_t gl_s = (i == skip_left_run) ? skip_left_gl : 0;
		int32_t gl_e = (i == skip_right_run) ? skip_right_gl : ret->visual[i].clusters.size() - 1;
		for (int32_t j = gl_s; j <= gl_e; j++) {
			if ((ret->visual[i].clusters[j].flags & GLYPH_VALID) == GLYPH_VALID) {
				if (ret->visual.write[i].clusters.write[j].glyphs.size() > 0) {
					if ((ret->visual[i].clusters[j].flags & GLYPH_ELONGATION) == GLYPH_ELONGATION) elongations++;
					if ((ret->visual[i].clusters[j].flags & GLYPH_SPACE) == GLYPH_SPACE) spaces++;
				}
			}
		}
	}

	float d_width = (p_width - old_width);

	if (elongations > 0) {
		float d_elong = d_width / float(elongations);
		for (int32_t i = skip_left_run; i <= skip_right_run; i++) {
			int32_t gl_s = (i == skip_left_run) ? skip_left_gl : 0;
			int32_t gl_e = (i == skip_right_run) ? skip_right_gl : ret->visual[i].clusters.size() - 1;
			for (int32_t j = gl_s; j <= gl_e; j++) {
				if ((ret->visual[i].clusters[j].flags & GLYPH_VALID) == GLYPH_VALID) {
					if ((ret->visual[i].clusters[j].flags & GLYPH_ELONGATION) == GLYPH_ELONGATION) {
						int count = MAX(0, ret->visual[i].clusters[j].repeat + Math::floor(d_elong / ret->visual[i].clusters[j].advance));
						float delta = (count - ret->visual[i].clusters[j].repeat) * ret->visual[i].clusters[j].advance;
						if (delta < d_width) {
							ret->visual.write[i].clusters.write[j].repeat = count;
							ret->visual.write[i].width += delta;
							d_width -= delta;
						}
					}
				}
			}
		}
	}

	if (spaces > 0) {
		float d_space = d_width / float(spaces);
		for (int32_t i = skip_left_run; i <= skip_right_run; i++) {
			int32_t gl_s = (i == skip_left_run) ? skip_left_gl : 0;
			int32_t gl_e = (i == skip_right_run) ? skip_right_gl : ret->visual[i].clusters.size() - 1;
			for (int32_t j = gl_s; j <= gl_e; j++) {
				float min_space = 2.f;
				if (ret->visual[i].clusters[j].font_impl.is_valid()) {
					min_space = ret->visual[i].clusters[j].font_impl->get_advance(ret->visual[i].clusters[j].font_impl->get_glyph(' ')) / 5.f; //20% of a space
				}
				if ((ret->visual[i].clusters[j].flags & GLYPH_VALID) == GLYPH_VALID) {
					if ((ret->visual[i].clusters[j].flags & GLYPH_SPACE) == GLYPH_SPACE) {
						if (ret->visual[i].clusters[j].glyphs.size() == 1) {
							bool virt = ((ret->visual[i].clusters[j].flags & GLYPH_VIRTUAL) == GLYPH_VIRTUAL);
							float delta = Math::ceil((d_space / float(ret->visual[i].clusters[j].repeat)));
							if ((ret->visual.write[i].clusters.write[j].glyphs[0].advance + delta) <= ((virt) ? 0 : min_space)) {
								delta = ((virt) ? 0 : min_space) - ret->visual.write[i].clusters.write[j].glyphs[0].advance;
							}
							if (delta > d_width) {
								if (d_width > 0)
									delta = d_width;
								else
									delta = 0;
							}
							ret->visual.write[i].clusters.write[j].glyphs.write[0].advance += delta;
							ret->visual.write[i].clusters.write[j].advance += delta;
							ret->visual.write[i].width += delta * ret->visual[i].clusters[j].repeat;
							d_width -= delta * ret->visual[i].clusters[j].repeat;
						}
					}
				}
			}
		}
	}
	ret->update_metrics(spacing_top, spacing_bottom, base_dir);
	return ret;
}

void ShapedString::draw(RID p_canvas_item, const Point2 &p_pos, int p_clip_l, int p_clip_r, const Color &p_modulate, const Color &p_outline_modulate) const {
	ERR_FAIL_COND(!valid);

	Vector2 origin = p_pos;
	if (p_outline_modulate.a == 0) goto finish_outline;
	for (int32_t i = 0; i < visual.size(); i++) {
		for (int32_t j = 0; j < visual[i].clusters.size(); j++) {
			if ((visual[i].clusters[j].flags & GLYPH_CONTROL) == GLYPH_CONTROL && !display_controls) continue;
			for (int32_t k = 0; k < visual[i].clusters[j].repeat; k++) {
				for (int32_t g = 0; g < visual[i].clusters[j].glyphs.size(); g++) {
					if ((p_clip_r > 0) && (origin.x + visual[i].clusters[j].glyphs[g].advance >= p_clip_r)) goto finish_outline;
					if ((p_clip_l <= 0) || (origin.x >= p_clip_l)) {
						if (((visual[i].clusters[j].flags & GLYPH_VALID) == GLYPH_VALID) && visual[i].clusters[j].font_outline_impl.is_valid()) {
							visual[i].clusters[j].font_outline_impl->draw_glyph(p_canvas_item, origin + visual[i].clusters[j].glyphs[g].offset, visual[i].clusters[j].glyphs[g].codepoint, p_outline_modulate);
						}
					}
					origin.x += visual[i].clusters[j].glyphs[g].advance;
				}
			}
		}
	}
finish_outline:
	origin = p_pos;
	for (int32_t i = 0; i < visual.size(); i++) {
		for (int32_t j = 0; j < visual[i].clusters.size(); j++) {
			if ((visual[i].clusters[j].flags & GLYPH_CONTROL) == GLYPH_CONTROL && !display_controls) continue;
			for (int32_t k = 0; k < visual[i].clusters[j].repeat; k++) {
				for (int32_t g = 0; g < visual[i].clusters[j].glyphs.size(); g++) {
					if ((p_clip_r > 0) && (origin.x + visual[i].clusters[j].glyphs[g].advance >= p_clip_r)) return;
					if ((p_clip_l <= 0) || (origin.x >= p_clip_l)) {
						if (((visual[i].clusters[j].flags & GLYPH_VALID) == GLYPH_VALID) && visual[i].clusters[j].font_impl.is_valid()) {
							visual[i].clusters[j].font_impl->draw_glyph(p_canvas_item, origin + visual[i].clusters[j].glyphs[g].offset, visual[i].clusters[j].glyphs[g].codepoint, p_modulate);
						} else {
							FontHexBox::draw_glyph(p_canvas_item, origin + visual[i].clusters[j].glyphs[g].offset, visual[i].clusters[j].glyphs[g].codepoint, p_modulate);
						}
					}
					origin.x += visual[i].clusters[j].glyphs[g].advance;
				}
			}
		}
	}
}

void ShapedString::draw_with_attributes(RID p_canvas_item, const Point2 &p_pos, int p_clip_l, int p_clip_r, const Ref<AttributeMap> &p_color_map) const {
	ERR_FAIL_COND(!valid);

	Vector2 origin = p_pos;
	for (int32_t i = 0; i < visual.size(); i++) {
		for (int32_t j = 0; j < visual[i].clusters.size(); j++) {
			if ((visual[i].clusters[j].flags & GLYPH_CONTROL) == GLYPH_CONTROL && !display_controls) continue;
			bool draw = p_color_map->has_attribute(visual[i].clusters[j].start, TEXT_ATTRIB_OUTLINE_COLOR);
			Color color = (draw) ? (Color)p_color_map->get_attribute(visual[i].clusters[j].start, TEXT_ATTRIB_OUTLINE_COLOR) : Color();
			for (int32_t k = 0; k < visual[i].clusters[j].repeat; k++) {
				for (int32_t g = 0; g < visual[i].clusters[j].glyphs.size(); g++) {
					if ((p_clip_r > 0) && (origin.x + visual[i].clusters[j].glyphs[g].advance >= p_clip_r)) goto finish_outline;
					if (((p_clip_l <= 0) || (origin.x >= p_clip_l)) && draw) {
						if (((visual[i].clusters[j].flags & GLYPH_VALID) == GLYPH_VALID) && visual[i].clusters[j].font_outline_impl.is_valid()) {
							visual[i].clusters[j].font_outline_impl->draw_glyph(p_canvas_item, origin + visual[i].clusters[j].glyphs[g].offset, visual[i].clusters[j].glyphs[g].codepoint, color);
						}
					}
					origin.x += visual[i].clusters[j].glyphs[g].advance;
				}
			}
		}
	}
finish_outline:
	origin = p_pos;
	for (int32_t i = 0; i < visual.size(); i++) {
		for (int32_t j = 0; j < visual[i].clusters.size(); j++) {
			if ((visual[i].clusters[j].flags & GLYPH_CONTROL) == GLYPH_CONTROL && !display_controls) continue;
			for (int32_t k = 0; k < visual[i].clusters[j].repeat; k++) {
				bool ul_draw = p_color_map->has_attribute(visual[i].clusters[j].start, TEXT_ATTRIB_UNDERLINE_COLOR);
				Color color = (p_color_map->has_attribute(visual[i].clusters[j].start, TEXT_ATTRIB_COLOR)) ? (Color)p_color_map->get_attribute(visual[i].clusters[j].start, TEXT_ATTRIB_COLOR) : Color(1, 1, 1);
				Color ul_color = (ul_draw) ? (Color)p_color_map->get_attribute(visual[i].clusters[j].start, TEXT_ATTRIB_UNDERLINE_COLOR) : Color();
				for (int32_t g = 0; g < visual[i].clusters[j].glyphs.size(); g++) {
					if ((p_clip_r > 0) && (origin.x + visual[i].clusters[j].glyphs[g].advance >= p_clip_r)) return;
					if ((p_clip_l <= 0) || (origin.x >= p_clip_l)) {
						if (((visual[i].clusters[j].flags & GLYPH_VALID) == GLYPH_VALID) && visual[i].clusters[j].font_impl.is_valid()) {
							visual[i].clusters[j].font_impl->draw_glyph(p_canvas_item, origin + visual[i].clusters[j].glyphs[g].offset, visual[i].clusters[j].glyphs[g].codepoint, color);
							if (ul_draw) {
								float y = visual[i].clusters[j].font_impl->get_underline_position();
								VisualServer::get_singleton()->canvas_item_add_line(p_canvas_item, origin + Point2(0, y), origin + Point2(visual[i].clusters[j].glyphs[g].advance, y), ul_color, visual[i].clusters[j].font_impl->get_underline_thickness());
							}
						} else {
							FontHexBox::draw_glyph(p_canvas_item, origin + visual[i].clusters[j].glyphs[g].offset, visual[i].clusters[j].glyphs[g].codepoint, color);
						}
					}
					origin.x += visual[i].clusters[j].glyphs[g].advance;
				}
			}
		}
	}
}

// void ShapedString::draw_tabs(RID p_canvas_item, const Point2 &p_pos, const Ref<Texture> &p_texture, int p_clip_l, int p_clip_r, const Color &p_modulate) const {
// 	ERR_FAIL_COND(!valid);
// 	Vector2 origin = p_pos;
// 	for (int32_t i = 0; i < visual.size(); i++) {
// 		for (int32_t j = 0; j < visual[i].clusters.size(); j++) {
// 			if ((p_clip_l <= 0) || (origin.x - p_pos.x >= p_clip_l)) {
// 				if ((visual[i].clusters[j].flags & GLYPH_TAB) == GLYPH_TAB) {
// 					int y_ofs = ((ascent + descent) - p_texture->get_height()) / 2;
// 					p_texture->draw(p_canvas_item, Point2(origin.x, origin.y + y_ofs), p_modulate);
// 				}
// 			}
// 			origin.x += visual[i].clusters[j].advance * visual[i].clusters[j].repeat;
// 			if ((p_clip_r > 0) && (origin.x - p_pos.x > p_clip_r)) return;
// 		}
// 	}
// }

// void ShapedString::draw_spaces(RID p_canvas_item, const Point2 &p_pos, const Ref<Texture> &p_texture, int p_clip_l, int p_clip_r, const Color &p_modulate) const {
// 	ERR_FAIL_COND(!valid);
// 	Vector2 origin = p_pos;
// 	for (int32_t i = 0; i < visual.size(); i++) {
// 		for (int32_t j = 0; j < visual[i].clusters.size(); j++) {
// 			if ((p_clip_l <= 0) || (origin.x - p_pos.x >= p_clip_l)) {
// 				if ((visual[i].clusters[j].flags & GLYPH_SPACE) == GLYPH_SPACE) {
// 					int y_ofs = ((ascent + descent) - p_texture->get_height()) / 2;
// 					p_texture->draw(p_canvas_item, Point2(origin.x, origin.y + y_ofs), p_modulate);
// 				}
// 			}
// 			origin.x += visual[i].clusters[j].advance * visual[i].clusters[j].repeat;
// 			if ((p_clip_r > 0) && (origin.x - p_pos.x > p_clip_r)) return;
// 		}
// 	}
// }

bool ShapedString::get_display_controls() const {
	return display_controls;
}

void ShapedString::set_display_controls(bool p_enable) {
	if (display_controls != p_enable) {
		display_controls = p_enable;

		for (int32_t i = 0; i < visual.size(); i++) {
			float run_width = 0.f;
			for (int32_t j = 0; j < visual[i].clusters.size(); j++) {
				if ((visual[i].clusters[j].flags & GLYPH_CONTROL) == GLYPH_CONTROL) {
					visual.write[i].clusters.write[j].repeat = (display_controls) ? 1 : 0;
				}
				if (visual[i].clusters[j].repeat > 0) {
					run_width += visual[i].clusters[j].advance * visual[i].clusters[j].repeat;
				}
			}
			visual.write[i].width = run_width;
		}

		update_metrics(spacing_top, spacing_bottom, base_dir);
	}
}

Vector<Rect2> ShapedString::get_selection_rects(int32_t p_start, int32_t p_end) const {
	Vector<Rect2> ret;
	ERR_FAIL_COND_V(!valid, ret);

	Vector2 origin;
	for (int32_t i = 0; i < visual.size(); i++) {
		for (int32_t j = 0; j < visual[i].clusters.size(); j++) {
			if ((visual[i].clusters[j].flags & GLYPH_CONTROL) == GLYPH_CONTROL && !display_controls) continue;
			if ((visual[i].clusters[j].start >= p_start) && (visual[i].clusters[j].end < p_end)) {
				ret.push_back(Rect2(origin, Size2(visual[i].clusters[j].advance * visual[i].clusters[j].repeat, ascent + descent)));
			} else if ((visual[i].clusters[j].start < p_start) && (visual[i].clusters[j].end >= p_end)) {
				float char_width = (visual[i].clusters[j].advance * visual[i].clusters[j].repeat) / (visual[i].clusters[j].end + 1 - visual[i].clusters[j].start);
				int64_t pos_ofs_s = p_start - visual[i].clusters[j].start;
				int64_t pos_ofs_e = p_end - visual[i].clusters[j].start;
				ret.push_back(Rect2(Point2(origin.x + pos_ofs_s * char_width, origin.y), Size2((pos_ofs_e - pos_ofs_s) * char_width, ascent + descent)));
			} else if ((visual[i].clusters[j].start < p_start) && (visual[i].clusters[j].end >= p_start)) {
				float char_width = (visual[i].clusters[j].advance * visual[i].clusters[j].repeat) / (visual[i].clusters[j].end + 1 - visual[i].clusters[j].start);
				int64_t pos_ofs = p_start - visual[i].clusters[j].start;
				if (visual[i].level % 2 != 0) {
					ret.push_back(Rect2(origin, Size2(pos_ofs * char_width, ascent + descent)));
				} else {
					ret.push_back(Rect2(Point2(origin.x + pos_ofs * char_width, origin.y), Size2((visual[i].clusters[j].advance * visual[i].clusters[j].repeat) - pos_ofs * char_width, ascent + descent)));
				}
			} else if ((visual[i].clusters[j].start < p_end) && (visual[i].clusters[j].end >= p_end)) {
				float char_width = (visual[i].clusters[j].advance * visual[i].clusters[j].repeat) / (visual[i].clusters[j].end + 1 - visual[i].clusters[j].start);
				int64_t pos_ofs = p_end - visual[i].clusters[j].start;
				if (visual[i].level % 2 != 0) {
					ret.push_back(Rect2(Point2(origin.x + pos_ofs * char_width, origin.y), Size2((visual[i].clusters[j].advance * visual[i].clusters[j].repeat) - pos_ofs * char_width, ascent + descent)));
				} else {
					ret.push_back(Rect2(origin, Size2(pos_ofs * char_width, ascent + descent)));
				}
			}
			origin.x += visual[i].clusters[j].advance * visual[i].clusters[j].repeat;
		}
	}
	//merge intersectiong ranges
	int64_t i = 0;
	while (i < ret.size()) {
		int64_t j = i + 1;
		while (j < ret.size()) {
			if (ret[i].position.x + ret[i].size.x == ret[j].position.x) {
				ret.write[i].size.x += ret[j].size.x;
				ret.remove(j);
				continue;
			}
			j++;
		}
		i++;
	}
	return ret;
}

Array ShapedString::_get_selection_rects(int32_t p_start, int32_t p_end) const {
	Array ret;
	ERR_FAIL_COND_V(!valid, ret);
	Vector<Rect2> r = get_selection_rects(p_start, p_end);
	for (int64_t i = 0; i < r.size(); i++) {
		ret.push_back(r[i]);
	}
	return ret;
}

float ShapedString::get_ltr_caret_offset(int32_t p_caret_pos) const {
	ERR_FAIL_COND_V(!valid, -1.f);

	if (visual.size() == 0) return 0.f;
	Vector2 origin;
	for (int32_t i = 0; i < visual.size(); i++) {
		for (int32_t j = 0; j < visual[i].clusters.size(); j++) {
			if ((visual[i].clusters[j].flags & GLYPH_CONTROL) == GLYPH_CONTROL && !display_controls) continue;
			if ((visual[i].clusters[j].flags & GLYPH_VIRTUAL) != GLYPH_VIRTUAL) {
				if ((visual[i].clusters[j].start == p_caret_pos) && (visual[i].level % 2 == 0)) {
					return origin.x;
				} else if ((visual[i].clusters[j].end == p_caret_pos - 1) && (visual[i].level % 2 == 0)) {
					return origin.x + visual[i].clusters[j].advance * visual[i].clusters[j].repeat;
				} else if ((visual[i].clusters[j].start < p_caret_pos) && (visual[i].clusters[j].end >= p_caret_pos) && (visual[i].level % 2 == 0)) {
					float char_width = (visual[i].clusters[j].advance * visual[i].clusters[j].repeat) / (visual[i].clusters[j].end + 1 - visual[i].clusters[j].start);
					int32_t pos_ofs = p_caret_pos - visual[i].clusters[j].start;
					return (visual[i].level % 2 != 0) ? origin.x + visual[i].clusters[j].advance * visual[i].clusters[j].repeat - pos_ofs * char_width : origin.x + pos_ofs * char_width;
				}
			}
			origin.x += visual[i].clusters[j].advance * visual[i].clusters[j].repeat;
		}
	}
	return -1.f;
}

float ShapedString::get_rtl_caret_offset(int32_t p_caret_pos) const {
	ERR_FAIL_COND_V(!valid, -1);

	if (visual.size() == 0) return 0.f;

	Vector2 origin;
	for (int32_t i = 0; i < visual.size(); i++) {
		for (int32_t j = 0; j < visual[i].clusters.size(); j++) {
			if ((visual[i].clusters[j].flags & GLYPH_CONTROL) == GLYPH_CONTROL && !display_controls) continue;
			if ((visual[i].clusters[j].flags & GLYPH_VIRTUAL) != GLYPH_VIRTUAL) {
				if ((visual[i].clusters[j].start == p_caret_pos) && (visual[i].level % 2 != 0)) {
					return origin.x + visual[i].clusters[j].advance * visual[i].clusters[j].repeat;
				} else if ((visual[i].clusters[j].end == p_caret_pos - 1) && (visual[i].level % 2 != 0)) {
					return origin.x;
				} else if ((visual[i].clusters[j].start < p_caret_pos) && (visual[i].clusters[j].end >= p_caret_pos) && (visual[i].level % 2 != 0)) {
					float char_width = (visual[i].clusters[j].advance * visual[i].clusters[j].repeat) / (visual[i].clusters[j].end + 1 - visual[i].clusters[j].start);
					int32_t pos_ofs = p_caret_pos - visual[i].clusters[j].start;
					return (visual[i].level % 2 != 0) ? origin.x + visual[i].clusters[j].advance * visual[i].clusters[j].repeat - pos_ofs * char_width : origin.x + pos_ofs * char_width;
				}
			}
			origin.x += visual[i].clusters[j].advance * visual[i].clusters[j].repeat;
		}
	}
	return -1.f;
}

int32_t ShapedString::get_run_start(int32_t p_pos) const {
	ERR_FAIL_COND_V(!valid, 0);
	for (int32_t i = 0; i < visual.size(); i++) {
		if ((p_pos >= visual[i].start) && (p_pos < visual[i].end)) {
			return visual[i].start;
		}
	}
	return 0;
}

int32_t ShapedString::get_run_end(int32_t p_pos) const {
	ERR_FAIL_COND_V(!valid, 0);
	int32_t max_end = 0;
	for (int32_t i = 0; i < visual.size(); i++) {
		max_end = MAX(max_end, visual[i].end);
		if ((p_pos >= visual[i].start) && (p_pos < visual[i].end)) {
			return visual[i].end;
		}
	}
	return max_end;
}

float ShapedString::get_ascent() const {
	ERR_FAIL_COND_V(!valid, 0.f);
	return ascent;
}

float ShapedString::get_descent() const {
	ERR_FAIL_COND_V(!valid, 0.f);
	return descent;
}

Size2 ShapedString::get_size() const {
	ERR_FAIL_COND_V(!valid, Size2());
	return Size2(width, ascent + descent);
}

Array ShapedString::get_data() const {
	Array ret;
	ERR_FAIL_COND_V(!valid, ret);

	for (int32_t i = 0; i < visual.size(); i++) {
		Dictionary run;
		run["start"] = visual[i].start;
		run["end"] = visual[i].start;
		run["level"] = visual[i].level;
		run["script"] = visual[i].script;
		run["width"] = visual[i].width;
		run["ascent"] = visual[i].ascent;
		run["descent"] = visual[i].descent;
		run["break_type"] = (int)visual[i].break_type;
		Array a_clusters;
		for (int32_t j = 0; j < visual[i].clusters.size(); j++) {
			Dictionary cluster;
			cluster["start"] = visual[i].clusters[j].start;
			cluster["end"] = visual[i].clusters[j].end;
			cluster["advance"] = visual[i].clusters[j].advance;
			cluster["repeat"] = visual[i].clusters[j].repeat;
			cluster["font"] = visual[i].clusters[j].font_impl;
			cluster["font_outline"] = visual[i].clusters[j].font_outline_impl;
			cluster["flags"] = visual[i].clusters[j].flags;
			Array a_glyphs;
			for (int32_t g = 0; g < visual[i].clusters[j].glyphs.size(); g++) {
				Dictionary glyph;
				glyph["index"] = visual[i].clusters[j].glyphs[g].codepoint;
				glyph["offset"] = visual[i].clusters[j].glyphs[g].offset;
				glyph["advacne"] = visual[i].clusters[j].glyphs[g].advance;
				a_glyphs.push_back(glyph);
			}
			cluster["glyphs"] = a_glyphs;
			a_clusters.push_back(cluster);
		}
		run["clusters"] = a_clusters;
		ret.push_back(run);
	}
	return ret;
}

Ref<ShapedString> ShapedString::invalid() {
	Ref<ShapedString> ret;
	ret.instance();
	ret->valid = false;
	return ret;
}

Mutex *ShapedString::shaped_mutex = NULL;
SelfList<ShapedString>::List *ShapedString::shapeds = NULL;

void ShapedString::invalidate_all() {
	if (shaped_mutex) {
		shaped_mutex->lock();
		SelfList<ShapedString> *E = shapeds->first();
		while (E) {
			E->self()->valid = false;
			E = E->next();
		}

		shaped_mutex->unlock();
	}
}

void ShapedString::initialize_self_list() {
	shapeds = memnew(SelfList<ShapedString>::List());
	shaped_mutex = Mutex::create();
}

void ShapedString::finish_self_list() {
	if (shaped_mutex) {
		memdelete(shaped_mutex);
		shaped_mutex = NULL;
	}
	if (shapeds) {
		memdelete(shapeds);
		shapeds = NULL;
	}
}

ShapedString::ShapedString() :
		shaped_list(this) {

	if (shaped_mutex) {
		shaped_mutex->lock();
		shapeds->add(&shaped_list);
		shaped_mutex->unlock();
	}

	valid = false;

	display_controls = false;

	ascent = 0;
	descent = 0;
	width = 0;

	spacing_top = 0;
	spacing_bottom = 0;

	base_dir = TEXT_DIRECTION_INVALID;
}

ShapedString::~ShapedString() {

	if (shaped_mutex) {
		shaped_mutex->lock();
		shapeds->remove(&shaped_list);
		shaped_mutex->unlock();
	}
}
