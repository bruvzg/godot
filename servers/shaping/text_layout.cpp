/*************************************************************************/
/*  text_layout.cpp                                                      */
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

#include "servers/shaping/text_layout.h"
#include "core/translation.h"
#include "scene/resources/font.h"
#include "servers/shaping/shaping_interface.h"
#include "servers/shaping_server.h"

TextLayout::TextLayout() :
		layout_list(this) {

	if (layout_mutex) {
		layout_mutex->lock();
		layouts->add(&layout_list);
		layout_mutex->unlock();
	}

	base_direction = TEXT_DIRECTION_AUTO;
	brk_flags = TEXT_BREAK_ON_HARD_AND_SOFT;
	jst_flags = TEXT_JST_BOTH;
	tab_flags = TEXT_TAB_SPACE;
	halign = HALIGN_LEFT;
	valign = VALIGN_TOP;
	tab_width = 0.f;
	display_control = false;
	display_metrics = true; //TODO: change to false in final version
	trim_edges = true;
	reverse_line_order = false;
	is_vertical = false;
	dispaly_spacing_chars = false;

	line_spacing = 2.f;

	para_direction = TEXT_DIRECTION_AUTO;

	text_shaped = false;
	lines_shaped = false;
	layout_shaped = false;
}

TextLayout::~TextLayout() {
	if (layout_mutex) {
		layout_mutex->lock();
		layouts->remove(&layout_list);
		layout_mutex->unlock();
	}
}

void TextLayout::_bind_methods() {

	ADD_GROUP("Text", "");

	ClassDB::bind_method(D_METHOD("set_text", "text"), &TextLayout::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &TextLayout::get_text);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "set_text", "get_text");

	ClassDB::bind_method(D_METHOD("set_base_direction", "dir"), &TextLayout::set_base_direction);
	ClassDB::bind_method(D_METHOD("get_base_direction"), &TextLayout::get_base_direction);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "base_direction", PROPERTY_HINT_ENUM, "Auto,LTR,RTL"), "set_base_direction", "get_base_direction");

	ADD_GROUP("Debug", "");

	ClassDB::bind_method(D_METHOD("set_display_metrics", "enable"), &TextLayout::set_display_metrics);
	ClassDB::bind_method(D_METHOD("get_display_metrics"), &TextLayout::get_display_metrics);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "display_metrics"), "set_display_metrics", "get_display_metrics");

	ClassDB::bind_method(D_METHOD("set_display_control_chars", "enable"), &TextLayout::set_display_control_chars);
	ClassDB::bind_method(D_METHOD("get_display_control_chars"), &TextLayout::get_display_control_chars);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "display_control_chars"), "set_display_control_chars", "get_display_control_chars");

	ADD_GROUP("Spacing chars", "");

	ClassDB::bind_method(D_METHOD("set_dispaly_spacing_chars", "enable"), &TextLayout::set_dispaly_spacing_chars);
	ClassDB::bind_method(D_METHOD("get_dispaly_spacing_chars"), &TextLayout::get_dispaly_spacing_chars);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "dispaly_spacing_chars"), "set_dispaly_spacing_chars", "get_dispaly_spacing_chars");

	ClassDB::bind_method(D_METHOD("set_tab_icon", "icon"), &TextLayout::set_tab_icon);
	ClassDB::bind_method(D_METHOD("get_tab_icon"), &TextLayout::get_tab_icon);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "tab_icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_tab_icon", "get_tab_icon");

	ClassDB::bind_method(D_METHOD("set_space_icon", "icon"), &TextLayout::set_space_icon);
	ClassDB::bind_method(D_METHOD("get_space_icon"), &TextLayout::get_space_icon);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "space_icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_space_icon", "get_space_icon");

	ClassDB::bind_method(D_METHOD("set_break_icon", "icon"), &TextLayout::set_break_icon);
	ClassDB::bind_method(D_METHOD("get_break_icon"), &TextLayout::get_break_icon);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "break_icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_break_icon", "get_break_icon");

	ADD_GROUP("Layout", "");

	ClassDB::bind_method(D_METHOD("set_target_size", "size"), &TextLayout::set_target_size);
	ClassDB::bind_method(D_METHOD("get_target_size"), &TextLayout::get_target_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "target_size"), "set_target_size", "get_target_size");

	ClassDB::bind_method(D_METHOD("set_h_align", "align"), &TextLayout::set_h_align);
	ClassDB::bind_method(D_METHOD("get_h_align"), &TextLayout::get_h_align);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "h_align", PROPERTY_HINT_ENUM, "Left,Center,Right,Fill (Last line left),Fill (Last line right),Fill (All lines)"), "set_h_align", "get_h_align");

	ClassDB::bind_method(D_METHOD("set_v_align", "align"), &TextLayout::set_v_align);
	ClassDB::bind_method(D_METHOD("get_v_align"), &TextLayout::get_v_align);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "v_align", PROPERTY_HINT_ENUM, "Top,Center,Bottom,Fill"), "set_v_align", "get_v_align");

	ClassDB::bind_method(D_METHOD("set_break_mode", "brk"), &TextLayout::set_break_mode);
	ClassDB::bind_method(D_METHOD("get_break_mode"), &TextLayout::get_break_mode);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "break_mode", PROPERTY_HINT_ENUM, "Do not break,Hard breaks only,Hard and soft breaks,Break anywhere"), "set_break_mode", "get_break_mode");

	ClassDB::bind_method(D_METHOD("set_justification_flags", "flags"), &TextLayout::set_justification_flags);
	ClassDB::bind_method(D_METHOD("get_justification_flags"), &TextLayout::get_justification_flags);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "justification_flags", PROPERTY_HINT_ENUM, "Fit spaces,Fit elongations, Fit both spaces and elongations"), "set_justification_flags", "get_justification_flags");

	ClassDB::bind_method(D_METHOD("set_justification_trim_edges", "enable"), &TextLayout::set_justification_trim_edges);
	ClassDB::bind_method(D_METHOD("get_justification_trim_edges"), &TextLayout::get_justification_trim_edges);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "justification_trim_edges"), "set_justification_trim_edges", "get_justification_trim_edges");

	ClassDB::bind_method(D_METHOD("set_is_vertical", "order"), &TextLayout::set_is_vertical);
	ClassDB::bind_method(D_METHOD("get_is_vertical"), &TextLayout::get_is_vertical);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_vertical"), "set_is_vertical", "get_is_vertical");

	ClassDB::bind_method(D_METHOD("set_reverse_line_order", "order"), &TextLayout::set_reverse_line_order);
	ClassDB::bind_method(D_METHOD("get_reverse_line_order"), &TextLayout::get_reverse_line_order);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "reverse_line_order"), "set_reverse_line_order", "get_reverse_line_order");

	ClassDB::bind_method(D_METHOD("set_tab_width", "width"), &TextLayout::set_tab_width);
	ClassDB::bind_method(D_METHOD("get_tab_width"), &TextLayout::get_tab_width);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "tab_width"), "set_tab_width", "get_tab_width");

	ClassDB::bind_method(D_METHOD("set_tab_flags", "flags"), &TextLayout::set_tab_flags);
	ClassDB::bind_method(D_METHOD("get_tab_flags"), &TextLayout::get_tab_flags);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "tab_flags", PROPERTY_HINT_ENUM, "Constant width,Left tab stops,Right tab stops"), "set_tab_flags", "get_tab_flags");

	ClassDB::bind_method(D_METHOD("set_line_spacing", "line_spacing"), &TextLayout::set_line_spacing);
	ClassDB::bind_method(D_METHOD("get_line_spacing"), &TextLayout::get_line_spacing);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "line_spacing"), "set_line_spacing", "get_line_spacing");

	//ADD_GROUP("Attributes", "");

	ClassDB::bind_method(D_METHOD("add_attribute", "attribute", "value", "start", "end"), &TextLayout::add_attribute);
	ClassDB::bind_method(D_METHOD("remove_attribute", "attribute", "start", "end"), &TextLayout::remove_attribute);
	ClassDB::bind_method(D_METHOD("remove_all_attributes", "start", "end"), &TextLayout::remove_all_attributes);
	ClassDB::bind_method(D_METHOD("clear_attributes"), &TextLayout::clear_attributes);

	ClassDB::bind_method(D_METHOD("hash"), &TextLayout::hash);

	ClassDB::bind_method(D_METHOD("copy"), &TextLayout::copy);

	ClassDB::bind_method(D_METHOD("has_attribute", "pos", "attribute"), &TextLayout::has_attribute);
	ClassDB::bind_method(D_METHOD("get_attribute", "pos", "attribute"), &TextLayout::get_attribute);

	ClassDB::bind_method(D_METHOD("get_paragraph_direction"), &TextLayout::get_paragraph_direction);
	ClassDB::bind_method(D_METHOD("get_dominant_direciton_in_range", "start", "end"), &TextLayout::get_dominant_direciton_in_range);

	ClassDB::bind_method(D_METHOD("get_size"), &TextLayout::get_size);

	ClassDB::bind_method(D_METHOD("get_line_count"), &TextLayout::get_line_count);
	ClassDB::bind_method(D_METHOD("get_line_rect", "index"), &TextLayout::get_line_rect);
	ClassDB::bind_method(D_METHOD("get_line_start", "index"), &TextLayout::get_line_start);
	ClassDB::bind_method(D_METHOD("get_line_end", "index"), &TextLayout::get_line_end);
	ClassDB::bind_method(D_METHOD("get_line_column_count", "index"), &TextLayout::get_line_column_count);

	ClassDB::bind_method(D_METHOD("get_line_ascent", "index"), &TextLayout::get_line_ascent);
	ClassDB::bind_method(D_METHOD("get_line_descent", "index"), &TextLayout::get_line_descent);

	ClassDB::bind_method(D_METHOD("get_replacement_rect", "pos"), &TextLayout::get_replacement_rect);

	ClassDB::bind_method(D_METHOD("hit_test", "pos"), &TextLayout::hit_test);
	ClassDB::bind_method(D_METHOD("get_ltr_caret", "pos"), &TextLayout::get_ltr_caret);
	ClassDB::bind_method(D_METHOD("get_rtl_caret", "p_pos"), &TextLayout::get_rtl_caret);
	ClassDB::bind_method(D_METHOD("get_selection_rects", "start", "end"), &TextLayout::_get_selection_rects);

	ClassDB::bind_method(D_METHOD("get_shaped_data"), &TextLayout::_get_shaped_data);

	ClassDB::bind_method(D_METHOD("draw", "canvas_item", "pos", "clip"), &TextLayout::draw, DEFVAL(false));

	ClassDB::bind_method(D_METHOD("shape_now"), &TextLayout::shape_now);

	ADD_GROUP("", "");
}

Ref<TextLayout> TextLayout::copy() const {
	Ref<TextLayout> ret;
	ret.instance();

	ret->text = text;
	ret->base_direction = base_direction;
	ret->brk_flags = brk_flags;
	ret->jst_flags = jst_flags;
	ret->tab_flags = tab_flags;
	ret->halign = halign;
	ret->valign = valign;
	ret->target_size = target_size;
	ret->tab_width = tab_width;
	ret->display_metrics = display_metrics;
	ret->display_control = display_control;
	ret->reverse_line_order = reverse_line_order;
	ret->is_vertical = is_vertical;
	ret->trim_edges = trim_edges;
	ret->dispaly_spacing_chars = dispaly_spacing_chars;
	ret->tab_icon = tab_icon;
	ret->space_icon = space_icon;
	ret->break_icon = break_icon;
	ret->text_attribs = text_attribs;
	ret->style_attribs = style_attribs;
	ret->line_spacing = line_spacing;
	ret->text_shaped = text_shaped;
	ret->para_direction = para_direction;
	ret->visual = visual;
	ret->lines_shaped = lines_shaped;
	ret->lines = lines;
	ret->layout_shaped = layout_shaped;
	ret->real_size = real_size;

	return ret;
}

uint32_t TextLayout::hash() const {
	uint32_t hash = text.hash();

	uint32_t flags = 0;
	if (display_metrics) flags |= 1;
	if (display_control) flags |= 2;
	if (reverse_line_order) flags |= 4;
	if (is_vertical) flags |= 8;
	if (trim_edges) flags |= 16;
	if (dispaly_spacing_chars) flags |= 32;
	flags |= base_direction << 6;
	flags |= brk_flags << 9;
	flags |= jst_flags << 12;
	flags |= tab_flags << 15;
	flags |= halign << 18;
	flags |= valign << 21;

	hash = hash_djb2_one_32(flags, hash);
	hash = hash_djb2_one_float(target_size.x, hash);
	hash = hash_djb2_one_float(target_size.y, hash);
	hash = hash_djb2_one_float(tab_width, hash);
	hash = hash_djb2_one_float(line_spacing, hash);

	hash = hash_djb2_one_32(hash_one_uint64((size_t)tab_icon.ptr()), hash);
	hash = hash_djb2_one_32(hash_one_uint64((size_t)space_icon.ptr()), hash);
	hash = hash_djb2_one_32(hash_one_uint64((size_t)break_icon.ptr()), hash);

	for (const AMap::Element *E = text_attribs.front(); E; E = E->next()) {
		hash = hash_djb2_one_32(E->key(), hash);
		const TextAttributes *K = NULL;
		while ((K = E->get().next(K))) {
			hash = hash_djb2_one_32(*K, hash);
			hash = hash_djb2_one_32(E->get()[*K], hash);
		}
	}
	for (const AMap::Element *E = style_attribs.front(); E; E = E->next()) {
		hash = hash_djb2_one_32(E->key(), hash);
		const TextAttributes *K = NULL;
		while ((K = E->get().next(K))) {
			hash = hash_djb2_one_32(*K, hash);
			hash = hash_djb2_one_32(E->get()[*K], hash);
		}
	}
	return hash;
}

void TextLayout::set_text(const String &p_text) {
	if (text != p_text) {
		text = p_text;
		text_shaped = false; //invalidate shaped text
	}
}

String TextLayout::get_text() const {
	return text;
}

void TextLayout::set_display_metrics(bool p_enable) {
	display_metrics = p_enable;
}

bool TextLayout::get_display_metrics() const {
	return display_metrics;
}

void TextLayout::set_display_control_chars(bool p_enable) {
	if (display_control != p_enable) {
		display_control = p_enable;
		text_shaped = false; //invalidate shaped text
	}
}

bool TextLayout::get_display_control_chars() const {
	return display_control;
}

void TextLayout::set_dispaly_spacing_chars(bool p_enable) {
	dispaly_spacing_chars = p_enable;
}

bool TextLayout::get_dispaly_spacing_chars() const {
	return dispaly_spacing_chars;
}

void TextLayout::set_tab_icon(const Ref<Texture> &p_icon) {
	tab_icon = p_icon;
}

Ref<Texture> TextLayout::get_tab_icon() const {
	return tab_icon;
}

void TextLayout::set_space_icon(const Ref<Texture> &p_icon) {
	space_icon = p_icon;
}

Ref<Texture> TextLayout::get_space_icon() const {
	return space_icon;
}

void TextLayout::set_break_icon(const Ref<Texture> &p_icon) {
	break_icon = p_icon;
}

Ref<Texture> TextLayout::get_break_icon() const {
	return break_icon;
}

void TextLayout::set_base_direction(TextDirection p_dir) {
	if (base_direction != p_dir) {
		base_direction = p_dir;
		text_shaped = false; //invalidate shaped text
	}
}

TextDirection TextLayout::get_base_direction() const {
	return base_direction;
}

void TextLayout::set_break_mode(TextBreakFlags p_brk) {
	if (brk_flags != p_brk) {
		brk_flags = p_brk;
		lines_shaped = false; //invalidate lines
	}
}

TextBreakFlags TextLayout::get_break_mode() const {
	return brk_flags;
}

void TextLayout::set_h_align(HAlign p_align) {
	if (halign != p_align) {
		halign = p_align;
		layout_shaped = false; //invalidate layout
	}
}

HAlign TextLayout::get_h_align() const {
	return halign;
}

void TextLayout::set_v_align(VAlign p_align) {
	if (valign != p_align) {
		valign = p_align;
		layout_shaped = false; //invalidate layout
	}
}

VAlign TextLayout::get_v_align() const {
	return valign;
}

void TextLayout::set_justification_flags(TextJustificationFlags p_jst) {
	if (jst_flags != p_jst) {
		jst_flags = p_jst;
		layout_shaped = false; //invalidate layout
	}
}

TextJustificationFlags TextLayout::get_justification_flags() const {
	return jst_flags;
}

void TextLayout::set_justification_trim_edges(bool p_enable) {
	if (trim_edges != p_enable) {
		trim_edges = p_enable;
		layout_shaped = false; //invalidate layout
	}
}

bool TextLayout::get_justification_trim_edges() const {
	return trim_edges;
}

void TextLayout::set_target_size(const Size2 &p_size) {
	if (target_size != p_size) {
		target_size = p_size;
		if ((brk_flags != TEXT_BREAK_ON_HARD) && (brk_flags != TEXT_BREAK_ON_NONE)) lines_shaped = false; //if breaks depends on width, invalidate lines
		layout_shaped = false; //invalidate layout
	}
}

Size2 TextLayout::get_target_size() const {
	return target_size;
}

void TextLayout::set_is_vertical(bool p_is_vertical) {
	if (is_vertical != p_is_vertical) {
		is_vertical = p_is_vertical;
		text_shaped = false; //text layout
	}
}

bool TextLayout::get_is_vertical() const {
	return is_vertical;
}

void TextLayout::set_reverse_line_order(bool p_order) {
	if (reverse_line_order != p_order) {
		reverse_line_order = p_order;
		layout_shaped = false; //invalidate layout
	}
}

bool TextLayout::get_reverse_line_order() const {
	return reverse_line_order;
}

void TextLayout::set_tab_width(float p_width) {
	if (tab_width != p_width) {
		tab_width = p_width;
		lines_shaped = false; //invalidate lines
	}
}

float TextLayout::get_tab_width() const {
	return tab_width;
}

void TextLayout::set_tab_flags(TextTabluationFlags p_flags) {
	if (tab_flags != p_flags) {
		tab_flags = p_flags;
		lines_shaped = false; //invalidate lines
	}
}

TextTabluationFlags TextLayout::get_tab_flags() const {
	return tab_flags;
}

void TextLayout::set_line_spacing(float p_line_spacing) {
	if (line_spacing != p_line_spacing) {
		line_spacing = p_line_spacing;
		layout_shaped = false; //invalidate layout
	}
}

float TextLayout::get_line_spacing() const {
	return line_spacing;
}

bool TextLayout::_compare_attributes(const HashMap<TextAttributes, Variant> &p_first, const HashMap<TextAttributes, Variant> &p_second) const {
	if (p_first.size() != p_second.size()) return false;
	const TextAttributes *K = NULL;
	while ((K = p_first.next(K))) {
		const Variant *E = p_first.getptr(*K);
		const Variant *F = p_second.getptr(*K);
		if ((!F) || (!E) || (*E != *F)) return false;
	}
	return true;
}

void TextLayout::_ensure_break(AMap &p_map, int64_t p_key) {
	const AMap::Element *E = p_map.find_closest(p_key);
	p_map[p_key] = (E) ? E->get() : HashMap<TextAttributes, Variant>();
}

void TextLayout::_optimize_attributes(AMap &p_map) {
	Vector<int> erase_list;
	for (const AMap::Element *E = p_map.front(); E; E = E->next()) {
		if (E->prev() && (_compare_attributes(E->get(), E->prev()->get()))) {
			erase_list.push_back(E->key());
		}
	}
	for (int64_t i = 0; i < erase_list.size(); i++) {
		p_map.erase(erase_list[i]);
	}
}

void TextLayout::add_attribute(TextAttributes p_attribute, const Variant &p_value, int p_start, int p_end) {

	if (p_start > p_end) {
		SWAP(p_start, p_end);
	}
	if (p_attribute < TEXT_ATTRIB_MAX_TEXT) {
		text_shaped = false; //invalidate shaped text

		_ensure_break(text_attribs, p_start);
		_ensure_break(text_attribs, p_end);

		AMap::Element *E = text_attribs.find(p_start);
		while (E && ((E->key() < p_end))) {
			E->get()[p_attribute] = p_value;
			E = E->next();
		}
		_optimize_attributes(text_attribs);
	} else {
		_ensure_break(style_attribs, p_start);
		_ensure_break(style_attribs, p_end);

		AMap::Element *E = style_attribs.find(p_start);
		while (E && ((E->key() < p_end))) {
			E->get()[p_attribute] = p_value;
			E = E->next();
		}
		_optimize_attributes(style_attribs);
	}
}

void TextLayout::remove_attribute(TextAttributes p_attribute, int p_start, int p_end) {

	if (p_start > p_end) {
		SWAP(p_start, p_end);
	}
	if (p_attribute < TEXT_ATTRIB_MAX_TEXT) {
		text_shaped = false; //invalidate shaped text

		_ensure_break(text_attribs, p_start);
		_ensure_break(text_attribs, p_end);

		AMap::Element *E = text_attribs.find(p_start);
		while (E && ((E->key() < p_end))) {
			E->get().erase(p_attribute);
			E = E->next();
		}
		_optimize_attributes(text_attribs);
	} else {
		_ensure_break(style_attribs, p_start);
		_ensure_break(style_attribs, p_end);

		AMap::Element *E = style_attribs.find(p_start);
		while (E && ((E->key() < p_end))) {
			E->get().erase(p_attribute);
			E = E->next();
		}
		_optimize_attributes(style_attribs);
	}
}

void TextLayout::remove_all_attributes(int p_start, int p_end) {
	text_shaped = false; //invalidate shaped text

	if (p_start > p_end) {
		SWAP(p_start, p_end);
	}
	_ensure_break(text_attribs, p_start);
	_ensure_break(text_attribs, p_end);

	AMap::Element *E = text_attribs.find(p_start);
	while (E && ((E->key() < p_end))) {
		E->get().clear();
		E = E->next();
	}
	_optimize_attributes(text_attribs);

	_ensure_break(style_attribs, p_start);
	_ensure_break(style_attribs, p_end);

	E = style_attribs.find(p_start);
	while (E && ((E->key() < p_end))) {
		E->get().clear();
		E = E->next();
	}
	_optimize_attributes(style_attribs);
}

void TextLayout::clear_attributes() {
	text_shaped = false; //invalidate shaped text

	text_attribs.clear();
	style_attribs.clear();

	_ensure_break(text_attribs, 0);
	_ensure_break(style_attribs, 0);
}

bool TextLayout::has_attribute(int p_pos, TextAttributes p_attribute) const {
	if (p_attribute < TEXT_ATTRIB_MAX_TEXT) {
		const AMap::Element *E = text_attribs.find_closest(p_pos);
		if (!E) return false;
		return E->get().has(p_attribute);
	} else {
		const AMap::Element *E = style_attribs.find_closest(p_pos);
		if (!E) return false;
		return E->get().has(p_attribute);
	}
}

Variant TextLayout::get_attribute(int p_pos, TextAttributes p_attribute) const {
	if (p_attribute < TEXT_ATTRIB_MAX_TEXT) {
		const AMap::Element *E = text_attribs.find_closest(p_pos);
		if (!E) return false;
		return E->get()[p_attribute];
	} else {
		const AMap::Element *E = style_attribs.find_closest(p_pos);
		if (!E) return false;
		return E->get()[p_attribute];
	}
}

TextDirection TextLayout::get_paragraph_direction() const {
	_reshape();
	return para_direction;
}

TextDirection TextLayout::get_dominant_direciton_in_range(int p_start, int p_end) const {
	_reshape();
	int rtl = 0;
	int ltr = 0;

	if (p_start > p_end) {
		SWAP(p_start, p_end);
	}

	for (int32_t i = 0; i < lines.size(); i++) {
		if ((lines[i].start > p_end) || (lines[i].end < p_start)) continue;
		for (int32_t j = 0; j < lines[i].columns.size(); j++) {
			for (int32_t k = 0; k < lines[i].columns[j].visual.size(); k++) {
				if ((lines[i].columns[j].visual[k].end > p_start) && (lines[i].columns[j].visual[k].start < p_end)) {
					int mag = MIN(p_end, lines[i].columns[j].visual[i].end) - MAX(p_start, lines[i].columns[j].visual[k].start);
					if (lines[i].columns[j].visual[k].level % 2 == 0)
						ltr += mag;
					else
						rtl += mag;
				}
			}
		}
	}
	if (ltr == rtl) return TEXT_DIRECTION_AUTO; //neutral
	if (ltr > rtl)
		return TEXT_DIRECTION_LTR;
	else
		return TEXT_DIRECTION_RTL;
	return TEXT_DIRECTION_INVALID;
}

Size2 TextLayout::get_size() const {
	_reshape();
	return real_size;
}

const Vector<Run> &TextLayout::get_runs() const {
	return visual;
}

int TextLayout::get_line_count() const {
	_reshape();
	return lines.size();
}

Rect2 TextLayout::get_line_rect(int p_index) const {
	_reshape();
	ERR_FAIL_COND_V(p_index < 0 || p_index >= lines.size(), Rect2());
	return lines[p_index].rect;
}

int TextLayout::get_line_start(int p_index) const {
	_reshape();
	ERR_FAIL_COND_V(p_index < 0 || p_index >= lines.size(), -1);
	return lines[p_index].start;
}

int TextLayout::get_line_end(int p_index) const {
	_reshape();
	ERR_FAIL_COND_V(p_index < 0 || p_index >= lines.size(), -1);
	return lines[p_index].end;
}

int TextLayout::get_line_column_count(int p_index) const {
	_reshape();
	ERR_FAIL_COND_V(p_index < 0 || p_index >= lines.size(), -1);
	return lines[p_index].columns.size();
}

float TextLayout::get_line_ascent(int p_index) const {
	_reshape();
	ERR_FAIL_COND_V(p_index < 0 || p_index >= lines.size(), 0.f);
	return lines[p_index].ascent;
}

float TextLayout::get_line_descent(int p_index) const {
	_reshape();
	ERR_FAIL_COND_V(p_index < 0 || p_index >= lines.size(), 0.f);
	return lines[p_index].descent;
}

const Vector<Run> &TextLayout::get_line_runs(int p_index, int p_col) const { //UNSAFE
	_reshape();
	return lines[p_index].columns[p_col].visual;
}

Rect2 TextLayout::get_replacement_rect(int p_pos) const {
	_reshape();

	const AMap::Element *E = text_attribs.find(p_pos);
	if (!E || !E->get().has(TEXT_ATTRIB_REPLACEMENT)) {
		return Rect2(); //no replacement rect at position
	}

	for (int32_t i = 0; i < lines.size(); i++) {
		if ((lines[i].start > p_pos) || (lines[i].end < p_pos)) continue;
		Point2 offset = lines[i].rect.position;
		for (int32_t j = 0; j < lines[i].columns.size(); j++) {
			if (tab_flags != TEXT_TAB_RIGHT) offset += Point2(lines[i].columns[j].offset, 0.f);
			for (int32_t k = 0; k < lines[i].columns[j].visual.size(); k++) {
				if ((lines[i].start <= p_pos) && (lines[i].end >= p_pos)) {
					for (int32_t l = 0; l < lines[i].columns[j].visual[k].clusters.size(); l++) {
						if ((lines[i].columns[j].visual[k].clusters[l].end == p_pos) && (lines[i].columns[j].visual[k].clusters[l].start == p_pos)) {
							return Rect2(_translate_coordinates(offset), _translate_coordinates(E->get()[TEXT_ATTRIB_REPLACEMENT]));
						}
						offset += Point2(lines[i].columns[j].visual[k].width, 0.f);
					}
				} else {
					offset += Point2(lines[i].columns[j].visual[k].width, 0.f);
				}
			}
			if (tab_flags == TEXT_TAB_RIGHT) offset += Point2(lines[i].columns[j].offset, 0.f);
		}
	}
	return Rect2(); //nothing found
}

int TextLayout::hit_test(const Point2 &p_pos) const {
	_reshape();

	Point2 pos = _translate_coordinates(p_pos);

	for (int32_t i = 0; i < lines.size(); i++) {
		if ((pos.y < lines[i].rect.position.y) || (pos.y > lines[i].rect.position.y + lines[i].rect.size.y)) continue;
		Point2 offset = pos - lines[i].rect.position;
		for (int32_t j = 0; j < lines[i].columns.size(); j++) {
			if (tab_flags != TEXT_TAB_RIGHT) offset += Point2(lines[i].columns[j].offset, 0.f);

			if (offset.x < 0) {
				//Hit before first cluster
				if (lines[i].columns[j].visual[0].level % 2 != 0) {
					return lines[i].columns[j].visual[0].end + 1;
				} else {
					return lines[i].columns[j].visual[0].start;
				}
			}

			float pixel_ofs = 0.f;
			for (int32_t k = 0; k < lines[i].columns[j].visual.size(); k++) {
				for (int32_t l = 0; l < lines[i].columns[j].visual[k].clusters.size(); l++) {
					const Cluster &cl = lines[i].columns[j].visual[k].clusters[l];
					if ((cl.flags & GLYPH_CONTROL) == GLYPH_CONTROL && !display_control) continue;
					if ((cl.flags & GLYPH_VALID) == GLYPH_VALID) {
						if ((cl.flags & GLYPH_VIRTUAL) != GLYPH_VIRTUAL) {
							if ((offset.x >= pixel_ofs) && (offset.x <= pixel_ofs + cl.advance * cl.repeat)) {
								int count = (cl.end + 1 - cl.start);
								float char_width = float(cl.advance * cl.repeat) / float(count);
								int32_t pos_ofs = round(float(offset.x - pixel_ofs) / char_width);
								if ((cl.flags & GLYPH_SURROGATE) == GLYPH_SURROGATE) {
									//do not allow mid cluster hit for surrogates
									pos_ofs = (pos_ofs < count / 2) ? 0 : count;
								}
								if (lines[i].columns[j].visual[k].level % 2 != 0) {
									return cl.end + 1 - pos_ofs;
								} else {
									return cl.start + pos_ofs;
								}
							} else {
							}
						}
					}
					pixel_ofs += cl.advance * cl.repeat;
				}
			}

			//Hit after last cluster
			if ((lines[i].columns[j].visual.size() > 0) && (offset.x > pixel_ofs)) {
				if (lines[i].columns[j].visual[lines[i].columns[j].visual.size() - 1].level % 2 != 0) {
					return lines[i].columns[j].visual[lines[i].columns[j].visual.size() - 1].start;
				} else {
					return lines[i].columns[j].visual[lines[i].columns[j].visual.size() - 1].end + 1;
				}
			}
			if (tab_flags != TEXT_TAB_RIGHT) offset += Point2(lines[i].columns[j].offset, 0.f);
		}
	}

	return -1;
}

Rect2 TextLayout::get_ltr_caret(int p_pos) const {
	ERR_FAIL_COND_V(ShapingServer::get_singleton() == NULL, Rect2());
	ERR_FAIL_COND_V(ShapingServer::get_singleton()->get_primary_interface().is_null(), Rect2());
	const Ref<ShapingInterface> &si = ShapingServer::get_singleton()->get_primary_interface();

	_reshape();

	for (int32_t i = 0; i < lines.size(); i++) {
		if ((lines[i].start > p_pos) || (lines[i].end < p_pos)) continue;
		Point2 offset = lines[i].rect.position;
		for (int32_t j = 0; j < lines[i].columns.size(); j++) {
			if (tab_flags != TEXT_TAB_RIGHT) offset += Point2(lines[i].columns[j].offset, 0.f);
			Vector2 origin = offset;
			for (int32_t k = 0; k < lines[i].columns[j].visual.size(); k++) {
				const Run &r = lines[i].columns[j].visual[k];
				for (int32_t l = 0; l < r.clusters.size(); l++) {
					const Cluster &cl = r.clusters[l];
					if ((cl.flags & GLYPH_CONTROL) == GLYPH_CONTROL && !display_control) continue;
					if ((cl.flags & GLYPH_VIRTUAL) != GLYPH_VIRTUAL) {
						if ((cl.start == p_pos) && (r.level % 2 == 0)) {
							return Rect2(_translate_coordinates(origin), _translate_coordinates(Point2(1, lines[i].ascent + lines[i].descent)));
						} else if ((cl.end == p_pos - 1) && (r.level % 2 == 0)) {
							origin = origin + Point2(cl.advance * cl.repeat, 0);
							return Rect2(_translate_coordinates(origin), _translate_coordinates(Point2(1, lines[i].ascent + lines[i].descent)));
						} else if ((cl.start < p_pos) && (cl.end >= p_pos) && (r.level % 2 == 0)) {
							int32_t pos_ofs = p_pos - cl.start;
							if (cl.glyphs.size() == 1) {
								Vector<float> offs = si->get_ligature_caret_offsets(cl.font_impl, TEXT_DIRECTION_LTR, cl.glyphs[0].codepoint);
								if (offs.size() > pos_ofs) {
									return Rect2(_translate_coordinates(origin + Point2(offs[pos_ofs], 0)), _translate_coordinates(Point2(1, lines[i].ascent + lines[i].descent)));
								}
							}
							float char_width = (cl.advance * cl.repeat) / (cl.end + 1 - cl.start);
							origin = (r.level % 2 != 0) ? origin + Point2(cl.advance * cl.repeat - pos_ofs * char_width, 0) : origin + Point2(pos_ofs * char_width, 0);
							return Rect2(_translate_coordinates(origin), _translate_coordinates(Point2(1, lines[i].ascent + lines[i].descent)));
						}
					}
					origin.x += cl.advance * cl.repeat;
				}
			}
			if (tab_flags == TEXT_TAB_RIGHT) offset += Point2(lines[i].columns[j].offset, 0.f);
		}
	}

	return Rect2();
}

Rect2 TextLayout::get_rtl_caret(int p_pos) const {
	ERR_FAIL_COND_V(ShapingServer::get_singleton() == NULL, Rect2());
	ERR_FAIL_COND_V(ShapingServer::get_singleton()->get_primary_interface().is_null(), Rect2());
	const Ref<ShapingInterface> &si = ShapingServer::get_singleton()->get_primary_interface();

	_reshape();

	for (int32_t i = 0; i < lines.size(); i++) {
		if ((lines[i].start > p_pos) || (lines[i].end < p_pos)) continue;
		Point2 offset = lines[i].rect.position;
		for (int32_t j = 0; j < lines[i].columns.size(); j++) {
			if (tab_flags != TEXT_TAB_RIGHT) offset += Point2(lines[i].columns[j].offset, 0.f);
			Vector2 origin = offset;
			for (int32_t k = 0; k < lines[i].columns[j].visual.size(); k++) {
				const Run &r = lines[i].columns[j].visual[k];
				for (int32_t l = 0; l < r.clusters.size(); l++) {
					const Cluster &cl = r.clusters[l];
					if ((cl.flags & GLYPH_CONTROL) == GLYPH_CONTROL && !display_control) continue;
					if ((cl.flags & GLYPH_VIRTUAL) != GLYPH_VIRTUAL) {
						if ((cl.start == p_pos) && (r.level % 2 != 0)) {
							return Rect2(_translate_coordinates(origin), _translate_coordinates(Point2(1, lines[i].ascent + lines[i].descent)));
						} else if ((cl.end == p_pos - 1) && (r.level % 2 != 0)) {
							origin = origin + Point2(cl.advance * cl.repeat, 0);
							return Rect2(_translate_coordinates(origin), _translate_coordinates(Point2(1, lines[i].ascent + lines[i].descent)));
						} else if ((cl.start < p_pos) && (cl.end >= p_pos) && (r.level % 2 != 0)) {
							int32_t pos_ofs = p_pos - cl.start;
							if (cl.glyphs.size() == 1) {
								Vector<float> offs = si->get_ligature_caret_offsets(cl.font_impl, TEXT_DIRECTION_RTL, cl.glyphs[0].codepoint);
								if (offs.size() > pos_ofs) {
									return Rect2(_translate_coordinates(origin + Point2(offs[pos_ofs], 0)), _translate_coordinates(Point2(1, lines[i].ascent + lines[i].descent)));
								}
							}
							float char_width = (cl.advance * cl.repeat) / (cl.end + 1 - cl.start);
							origin = (r.level % 2 != 0) ? origin + Point2(cl.advance * cl.repeat - pos_ofs * char_width, 0) : origin + Point2(pos_ofs * char_width, 0);
							return Rect2(_translate_coordinates(origin), _translate_coordinates(Point2(1, lines[i].ascent + lines[i].descent)));
						}
					}
					origin.x += cl.advance * cl.repeat;
				}
			}
			if (tab_flags == TEXT_TAB_RIGHT) offset += Point2(lines[i].columns[j].offset, 0.f);
		}
	}

	return Rect2();
}

Vector<Rect2> TextLayout::get_selection_rects(int p_start, int p_end) const {
	_reshape();
	Vector<Rect2> ret;
	for (int32_t i = 0; i < lines.size(); i++) {
		if ((lines[i].start > p_end) || (lines[i].end < p_start)) continue;
		Point2 offset = lines[i].rect.position;
		for (int32_t j = 0; j < lines[i].columns.size(); j++) {
			if (tab_flags != TEXT_TAB_RIGHT) offset += Point2(lines[i].columns[j].offset, 0.f);
			Vector2 origin = offset;
			for (int32_t k = 0; k < lines[i].columns[j].visual.size(); k++) {
				const Run &r = lines[i].columns[j].visual[k];
				for (int32_t l = 0; l < r.clusters.size(); l++) {
					const Cluster &cl = r.clusters[l];
					if ((cl.flags & GLYPH_CONTROL) == GLYPH_CONTROL && !display_control) continue;
					if ((cl.start >= p_start) && (cl.end < p_end)) {
						ret.push_back(Rect2(origin, Size2(cl.advance * cl.repeat, lines[i].ascent + lines[i].descent)));
					} else if ((cl.start < p_start) && (cl.end >= p_end)) {
						float char_width = (cl.advance * cl.repeat) / (cl.end + 1 - cl.start);
						int64_t pos_ofs_s = p_start - cl.start;
						int64_t pos_ofs_e = p_end - cl.start;
						ret.push_back(Rect2(Point2(origin.x + pos_ofs_s * char_width, origin.y), Size2((pos_ofs_e - pos_ofs_s) * char_width, lines[i].ascent + lines[i].descent)));
					} else if ((cl.start < p_start) && (cl.end >= p_start)) {
						float char_width = (cl.advance * cl.repeat) / (cl.end + 1 - cl.start);
						int64_t pos_ofs = p_start - cl.start;
						if (r.level % 2 != 0) {
							ret.push_back(Rect2(origin, Size2(pos_ofs * char_width, lines[i].ascent + lines[i].descent)));
						} else {
							ret.push_back(Rect2(Point2(origin.x + pos_ofs * char_width, origin.y), Size2((cl.advance * cl.repeat) - pos_ofs * char_width, lines[i].ascent + lines[i].descent)));
						}
					} else if ((cl.start < p_end) && (cl.end >= p_end)) {
						float char_width = (cl.advance * cl.repeat) / (cl.end + 1 - cl.start);
						int64_t pos_ofs = p_end - cl.start;
						if (r.level % 2 != 0) {
							ret.push_back(Rect2(Point2(origin.x + pos_ofs * char_width, origin.y), Size2((cl.advance * cl.repeat) - pos_ofs * char_width, lines[i].ascent + lines[i].descent)));
						} else {
							ret.push_back(Rect2(origin, Size2(pos_ofs * char_width, lines[i].ascent + lines[i].descent)));
						}
					}
					origin.x += cl.advance * cl.repeat;
				}
			}
			if (tab_flags == TEXT_TAB_RIGHT) offset += Point2(lines[i].columns[j].offset, 0.f);
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
	//apply v layout
	for (int i = 0; i < ret.size(); i++) {
		ret.write[i] = Rect2(_translate_coordinates(ret[i].position), _translate_coordinates(ret[i].size));
	}
	return ret;
}

Array TextLayout::_get_selection_rects(int p_start, int p_end) const {
	Array ret;
	Vector<Rect2> r = get_selection_rects(p_start, p_end);
	for (int64_t i = 0; i < r.size(); i++) {
		ret.push_back(r[i]);
	}
	return ret;
}

Dictionary TextLayout::_get_shaped_data() const {
	_reshape();

	Dictionary ret;

	ret["size"] = real_size;
	ret["paragraph_dir"] = para_direction;

	Array a_lines;
	for (int32_t i = 0; i < lines.size(); i++) {
		Dictionary line;
		line["start"] = lines[i].start;
		line["end"] = lines[i].end;
		line["ascent"] = lines[i].ascent;
		line["descent"] = lines[i].descent;
		line["rect"] = Rect2(_translate_coordinates(lines[i].rect.position), _translate_coordinates(lines[i].rect.size));
		Array a_cols;
		for (int32_t j = 0; j < lines[i].columns.size(); j++) {
			Dictionary col;
			col["offset"] = lines[i].columns[j].offset;
			Array a_runs;
			for (int32_t k = 0; k < lines[i].columns[j].visual.size(); k++) {
				Dictionary run;
				run["start"] = lines[i].columns[j].visual[k].start;
				run["end"] = lines[i].columns[j].visual[k].start;
				run["level"] = lines[i].columns[j].visual[k].level;
				run["script"] = lines[i].columns[j].visual[k].script;
				run["advance"] = lines[i].columns[j].visual[k].width;
				run["ascent"] = lines[i].columns[j].visual[k].ascent;
				run["descent"] = lines[i].columns[j].visual[k].descent;
				run["break_type"] = (int)lines[i].columns[j].visual[k].break_type;
				Array a_clusters;
				for (int32_t l = 0; l < lines[i].columns[j].visual[k].clusters.size(); l++) {
					Dictionary cluster;
					cluster["start"] = lines[i].columns[j].visual[k].clusters[l].start;
					cluster["end"] = lines[i].columns[j].visual[k].clusters[l].end;
					cluster["advance"] = lines[i].columns[j].visual[k].clusters[l].advance;
					cluster["repeat"] = lines[i].columns[j].visual[k].clusters[l].repeat;
					cluster["font"] = lines[i].columns[j].visual[k].clusters[l].font_impl;
					cluster["font_outline"] = lines[i].columns[j].visual[k].clusters[l].font_outline_impl;
					cluster["flags"] = lines[i].columns[j].visual[k].clusters[l].flags;
					Array a_glyphs;
					for (int32_t m = 0; m < lines[i].columns[j].visual[k].clusters[l].glyphs.size(); m++) {
						Dictionary glyph;
						glyph["index"] = lines[i].columns[j].visual[k].clusters[l].glyphs[m].codepoint;
						glyph["offset"] = lines[i].columns[j].visual[k].clusters[l].glyphs[m].offset;
						glyph["advacne"] = lines[i].columns[j].visual[k].clusters[l].glyphs[m].advance;
						a_glyphs.push_back(glyph);
					}
					cluster["glyphs"] = a_glyphs;
					a_clusters.push_back(cluster);
				}
				run["clusters"] = a_clusters;
				a_runs.push_back(run);
			}
			col["runs"] = a_runs;
			a_cols.push_back(col);
		}
		line["columns"] = a_cols;
		a_lines.push_back(line);
	}
	ret["lines"] = a_lines;
	return ret;
}

void TextLayout::draw(RID p_canvas_item, const Point2 &p_pos, bool p_clip) const {
	_reshape();

	/*DEBUG*/ if (display_metrics)
		printf("\n_____draw_____\n");
	if (display_metrics) {
		//draw background rect (R)
		_draw_rect_border(p_canvas_item, Rect2(p_pos - Point2(5, 5), _translate_coordinates(target_size) + Point2(10, 10)), Color(1, 0, 0, 0.3), 2); //target rect
		_draw_rect_border(p_canvas_item, Rect2(p_pos - Point2(5, 5), _translate_coordinates(real_size) + Point2(10, 10)), Color(1, 0, 0, 1), 2); //real rect
	}

	//TODO clip

	for (int32_t i = 0; i < lines.size(); i++) {
		/*DEBUG*/ if (display_metrics)
			printf(" line: %d  ", i);
		Point2 col_offset = p_pos + _translate_coordinates(lines[i].rect.position);
		if (display_metrics) {
			//draw line rect (G)
			_draw_rect_border(p_canvas_item, Rect2(col_offset + _translate_coordinates(Point2(0, -lines[i].ascent)), _translate_coordinates(lines[i].rect.size)), Color(0, 1, 0, 1), 1);

			_draw_rect_border(p_canvas_item, Rect2(col_offset + _translate_coordinates(Point2(0, -lines[i].ascent)), _translate_coordinates(Point2(lines[i].rect.size.x, 2))), Color(0, 1, 1, 1), 1, true);
			_draw_rect_border(p_canvas_item, Rect2(col_offset + _translate_coordinates(Point2(0, lines[i].descent)), _translate_coordinates(Point2(lines[i].rect.size.x, 2))), Color(1, 1, 0, 1), 1, true);
		}
		for (int32_t j = 0; j < lines[i].columns.size(); j++) {
			/*DEBUG*/ if (display_metrics)
				printf("C:%d(", j);
			if (tab_flags != TEXT_TAB_RIGHT) col_offset += _translate_coordinates(Point2(lines[i].columns[j].offset, 0.f));

			if (display_metrics) {
				//draw col marker (G)
				_draw_rect_border(p_canvas_item, Rect2(col_offset + _translate_coordinates(Point2(0, -lines[i].ascent)), _translate_coordinates(Point2(3, lines[i].rect.size.y))), Color(0, 0.5, 0, 0.5), 1, true);
			}

			Point2 gl_offset = col_offset;
			for (int32_t k = 0; k < lines[i].columns[j].visual.size(); k++) {
				const Run &r = lines[i].columns[j].visual[k];
				for (int32_t l = 0; l < r.clusters.size(); l++) {
					const Cluster &cl = r.clusters[l];
					const AMap::Element *E = style_attribs.find_closest(cl.start);
					if ((cl.flags & GLYPH_CONTROL) && !display_control) continue;
					float rise = (E && E->get().has(TEXT_ATTRIB_RISE)) ? (float)E->get()[TEXT_ATTRIB_RISE] : 0.f;
					Color color = (E && E->get().has(TEXT_ATTRIB_OUTLINE_COLOR)) ? (Color)E->get()[TEXT_ATTRIB_OUTLINE_COLOR] : Color(1, 1, 1);
					if ((cl.flags & GLYPH_VALID) && cl.font_outline_impl.is_valid()) {
						//draw glyphs outline
						for (int32_t m = 0; m < cl.glyphs.size(); m++) {
							const Glyph &gl = cl.glyphs[m];
							cl.font_outline_impl->draw_glyph(p_canvas_item, gl_offset + _translate_coordinates(gl.offset + Point2(0, rise)), gl.codepoint, color, (cl.flags & GLYPH_ROTATE_CW));
							gl_offset += _translate_coordinates(Point2(gl.advance, 0.f));
						}
					} else {
						for (int32_t m = 0; m < cl.glyphs.size(); m++) {
							const Glyph &gl = cl.glyphs[m];
							gl_offset += _translate_coordinates(Point2(gl.advance, 0.f));
						}
					}
				}
			}
			gl_offset = col_offset;
			for (int32_t k = 0; k < lines[i].columns[j].visual.size(); k++) {
				const Run &r = lines[i].columns[j].visual[k];
				/*DEBUG*/ if (display_metrics)
					printf("R:%d..%d{  ", r.start, r.end);
				for (int32_t l = 0; l < r.clusters.size(); l++) {
					const Cluster &cl = r.clusters[l];
					const AMap::Element *E = style_attribs.find_closest(cl.start);
					if ((cl.flags & GLYPH_CONTROL) && !display_control) continue;
					float rise = (E && E->get().has(TEXT_ATTRIB_RISE)) ? (float)E->get()[TEXT_ATTRIB_RISE] : 0.f;
					Color color = (E && E->get().has(TEXT_ATTRIB_COLOR)) ? (Color)E->get()[TEXT_ATTRIB_COLOR] : Color(1, 1, 1);
					Color ucolor = (E && E->get().has(TEXT_ATTRIB_UNDERLINE_COLOR)) ? (Color)E->get()[TEXT_ATTRIB_UNDERLINE_COLOR] : Color(1, 1, 1, 0);
					if ((cl.flags & GLYPH_VALID) && cl.font_impl.is_valid()) {
						if (dispaly_spacing_chars) {
							//draw spacing icons
							if ((cl.flags & GLYPH_TAB) && tab_icon.is_valid()) {
								tab_icon->draw(p_canvas_item, gl_offset + _translate_coordinates(Point2(0, -r.ascent)), color);
							} else if ((cl.flags & GLYPH_SPACE) && space_icon.is_valid()) {
								space_icon->draw(p_canvas_item, gl_offset + _translate_coordinates(Point2(0, -r.ascent)), color);
							} else if (l == r.clusters.size() && k == lines[i].columns[j].visual.size() && j == lines[i].columns.size() && break_icon.is_valid()) {
								break_icon->draw(p_canvas_item, gl_offset + _translate_coordinates(Point2(cl.advance, -r.ascent)), color);
							}
						}
						//draw glyphs
						/*DEBUG*/ if (display_metrics)
							printf("C:%d..%d[", cl.start, cl.end);
						for (int32_t m = 0; m < cl.glyphs.size(); m++) {
							const Glyph &gl = cl.glyphs[m];
							/*DEBUG*/ if (display_metrics)
								printf("%d ", gl.codepoint);
							cl.font_impl->draw_glyph(p_canvas_item, gl_offset + _translate_coordinates(gl.offset + Point2(0, rise)), gl.codepoint, color, (cl.flags & GLYPH_ROTATE_CW));
							//draw decarations
							if (ucolor.a != 0) {
								_draw_rect_border(p_canvas_item, Rect2(gl_offset + _translate_coordinates(Point2(0, cl.font_impl->get_underline_position())), _translate_coordinates(Point2(gl.advance, cl.font_impl->get_underline_thickness()))), ucolor, 1, true);
							}
							gl_offset += _translate_coordinates(Point2(gl.advance, 0.f));
						}
						/*DEBUG*/ if (display_metrics)
							printf("]");
					} else {
						//draw hex box - TODO
						/*DEBUG*/ if (display_metrics)
							printf("H:%d..%d[", cl.start, cl.end);
						for (int32_t m = 0; m < cl.glyphs.size(); m++) {
							const Glyph &gl = cl.glyphs[m];
							/*DEBUG*/ if (display_metrics)
								printf("%d ", gl.codepoint);
							FontHexBox::draw_glyph(p_canvas_item, gl_offset + _translate_coordinates(gl.offset + Point2(0, rise)), gl.codepoint, color);
							gl_offset += _translate_coordinates(Point2(gl.advance, 0.f));
						}
						/*DEBUG*/ if (display_metrics)
							printf("]");
					}
				}
				/*DEBUG*/ if (display_metrics)
					printf("} ");
			}
			if (tab_flags == TEXT_TAB_RIGHT) col_offset += _translate_coordinates(Point2(lines[i].columns[j].offset, 0.f));
			/*DEBUG*/ if (display_metrics)
				printf(")");
		}
		/*DEBUG*/ if (display_metrics)
			printf("\n");
	}
}

void TextLayout::shape_now() {
	_reshape();
}

_ALWAYS_INLINE_ void TextLayout::_draw_rect_border(RID p_canvas_item, const Rect2 &p_rect, const Color &p_color, float p_width, bool p_filled, bool p_antialiased) const {
	if (p_filled) {
		VisualServer::get_singleton()->canvas_item_add_rect(p_canvas_item, p_rect, p_color);
	} else {
		float offset;
		if (p_width >= 2) {
			offset = p_width / 2.0;
		} else {
			offset = 0.0;
		}

		VisualServer::get_singleton()->canvas_item_add_line(
				p_canvas_item,
				p_rect.position + Point2(-offset, 0),
				p_rect.position + Size2(p_rect.size.width + offset, 0),
				p_color,
				p_width,
				p_antialiased);
		VisualServer::get_singleton()->canvas_item_add_line(
				p_canvas_item,
				p_rect.position + Point2(0, offset),
				p_rect.position + Size2(0, p_rect.size.height - offset),
				p_color,
				p_width,
				p_antialiased);
		VisualServer::get_singleton()->canvas_item_add_line(
				p_canvas_item,
				p_rect.position + Point2(-offset, p_rect.size.height),
				p_rect.position + Size2(p_rect.size.width + offset, p_rect.size.height),
				p_color,
				p_width,
				p_antialiased);
		VisualServer::get_singleton()->canvas_item_add_line(
				p_canvas_item,
				p_rect.position + Point2(p_rect.size.width, offset),
				p_rect.position + Size2(p_rect.size.width, p_rect.size.height - offset),
				p_color,
				p_width,
				p_antialiased);
	}
}

_ALWAYS_INLINE_ Point2 TextLayout::_translate_coordinates(const Point2 &p_point) const {
	if (is_vertical) {
		return Point2(p_point.y, p_point.x);
	} else {
		return p_point;
	}
}

void TextLayout::_reshape() const {
	ERR_FAIL_COND(ShapingServer::get_singleton() == NULL);
	ERR_FAIL_COND(ShapingServer::get_singleton()->get_primary_interface().is_null());

	const Ref<ShapingInterface> &si = ShapingServer::get_singleton()->get_primary_interface();
	const String &locale = TranslationServer::get_singleton()->get_locale();

	if (!text_shaped) {
		/*DEBUG*/ if (display_metrics)
			printf("\n_____reshape:text_____\n");
		para_direction = si->get_paragraph_direction(base_direction, text, locale);

		Vector<ShapingInterface::SourceRun> bidi_runs = si->get_bidi_runs(text, 0, text.length(), para_direction);
		Vector<ShapingInterface::SourceRun> script_runs = si->get_script_runs(text, 0, text.length(), para_direction);

		visual.clear();
		lines.clear();

		for (int i = 0; i < bidi_runs.size(); i++) {
			/*DEBUG*/ if (display_metrics)
				printf(" bidi -> %d %d D:%s\n", bidi_runs[i].start, bidi_runs[i].end, (bidi_runs[i].value % 2 == 0) ? "L" : "R");
			//shape bidi runs
			if (bidi_runs[i].value % 2 == 0) {
				//LTR
				int j = 0;
				while (j < script_runs.size() && script_runs[j].end < bidi_runs[i].start)
					j++;
				int bs_start = bidi_runs[i].start;
				int bs_end = MIN(bidi_runs[i].end, script_runs[j].end);
				while (bs_start < bidi_runs[i].end) {
					/*DEBUG*/ if (display_metrics)
						printf("  bd+src l -> %d %d D:%s S:%x\n", bs_start, bs_end, (bidi_runs[i].value % 2 == 0) ? "L" : "R", script_runs[j].value);
					//shape bidi + script subruns
					const AMap::Element *E = text_attribs.find_closest(bs_start);
					int bsa_start = (E) ? MAX(bs_start, E->key()) : bs_start;
					int bsa_end = (E->next()) ? MIN(bs_end, E->next()->key()) : bs_end;
					while (true) {
						/*DEBUG*/ if (display_metrics)
							printf("   bd+src+a l -> %d %d D:%s S:%x\n", bsa_start, bsa_end, (bidi_runs[i].value % 2 == 0) ? "L" : "R", script_runs[j].value);
						//shape bidi + script + attributte(font/language) subruns
						if (E->get().has(TEXT_ATTRIB_REPLACEMENT)) {
							/*DEBUG*/ if (display_metrics)
								printf("    repl\n");
							//add replacement object run
							Size2 bl_sz = (Size2)E->get()[TEXT_ATTRIB_REPLACEMENT];
							float bl_px = CLAMP(0.f, 1.f, (E->get().has(TEXT_ATTRIB_REPLACEMENT_BL_OFFSET)) ? (float)E->get()[TEXT_ATTRIB_REPLACEMENT_BL_OFFSET] : 0.5f);
							if (bsa_start != bsa_end) WARN_PRINTS("Invalid replacemnet object bounds");
							Run r;
							r.start = bsa_start;
							r.end = bsa_end;
							r.ascent = bl_sz.height * bl_px;
							r.descent = bl_sz.height - (bl_sz.height * bl_px);
							r.width = bl_sz.width;
							r.level = bidi_runs[i].value;
							r.script = TAG('Q', 'a', 'b', 'j'); /*private use script code*/
							r.break_type = RUN_BREAK_SOFT;
							visual.push_back(r);
						} else {
							String lang = (E->get().has(TEXT_ATTRIB_LANGUAGE)) ? (String)E->get()[TEXT_ATTRIB_LANGUAGE] : locale;
							String ftr = (E->get().has(TEXT_ATTRIB_OT_FEATURES)) ? (String)E->get()[TEXT_ATTRIB_OT_FEATURES] : "";
							const Ref<Font> font = (E->get().has(TEXT_ATTRIB_FONT)) ? (Ref<Font>)E->get()[TEXT_ATTRIB_FONT] : NULL;
							Vector<ShapingInterface::SourceRun> brk_runs = si->get_break_runs(text, bsa_start, bsa_end, lang);
							int k = 0;
							while (k < brk_runs.size() && brk_runs[k].end < bsa_start)
								k++;
							int bsab_start = bsa_start;
							int bsab_end = MIN(bsa_end, brk_runs[k].end);
							while (bsab_start < bsa_end) {
								/*DEBUG*/ if (display_metrics)
									printf("    bd+src+a+brk l -> %d %d D:%s S:%x B:%d\n", bsab_start, bsab_end, (bidi_runs[i].value % 2 == 0) ? "L" : "R", script_runs[j].value, brk_runs[k].value);
								//shape bidi + script + attribute + wordbreak subruns
								visual.push_back(si->shape_run2(text, bsab_start, bsab_end, font.ptr(), ftr, lang, bidi_runs[i].value, script_runs[j].value, brk_runs[k].value, is_vertical));
								//advance
								if ((brk_runs[k].end <= bsab_end) && (k < brk_runs.size() - 1)) k++;
								bsab_start = bsab_end;
								bsab_end = MIN(bsa_end, brk_runs[k].end);
							}
						}
						if (E->next() && (E->next()->key() <= bsa_end)) E = E->next();
						if (bsa_end == bs_end) break;
						bsa_start = bsa_end;
						bsa_end = (E->next()) ? MIN(E->next()->key(), bs_end) : bs_end;
					}
					//advance
					if ((script_runs[j].end <= bs_end) && (j < script_runs.size() - 1)) j++;
					bs_start = bs_end;
					bs_end = MIN(bidi_runs[i].end, script_runs[j].end);
				}
			} else {
				//RTL
				int j = script_runs.size() - 1;
				while (j >= 0 && script_runs[j].start > bidi_runs[i].end)
					j--;
				int bs_end = bidi_runs[i].end;
				int bs_start = MAX(bidi_runs[i].start, script_runs[j].start);
				while (bs_end > bidi_runs[i].start) {
					/*DEBUG*/ if (display_metrics)
						printf("  bd+src r -> %d %d D:%s S:%x\n", bs_start, bs_end, (bidi_runs[i].value % 2 == 0) ? "L" : "R", script_runs[j].value);
					//shape bidi + script subruns
					const AMap::Element *E = text_attribs.find_closest(bs_end - 1);
					int bsa_start = (E) ? MAX(bs_start, E->key()) : bs_start;
					int bsa_end = (E->next()) ? MIN(bs_end, E->next()->key()) : bs_end;
					while (true) {
						/*DEBUG*/ if (display_metrics)
							printf("   bd+src+a r -> %d %d D:%s S:%x\n", bsa_start, bsa_end, (bidi_runs[i].value % 2 == 0) ? "L" : "R", script_runs[j].value);
						//shape bidi + script + attributte(font/language) subruns
						if (E->get().has(TEXT_ATTRIB_REPLACEMENT)) {
							/*DEBUG*/ if (display_metrics)
								printf("    repl\n");
							//add replacement object run
							Size2 bl_sz = (Size2)E->get()[TEXT_ATTRIB_REPLACEMENT];
							float bl_px = CLAMP(0.f, 1.f, (E->get().has(TEXT_ATTRIB_REPLACEMENT_BL_OFFSET)) ? (float)E->get()[TEXT_ATTRIB_REPLACEMENT_BL_OFFSET] : 0.5f);
							if (bsa_start != bsa_end) WARN_PRINTS("Invalid replacemnet object bounds");
							Run r;
							r.start = bsa_start;
							r.end = bsa_end;
							r.ascent = bl_sz.height * bl_px;
							r.descent = bl_sz.height - (bl_sz.height * bl_px);
							r.width = bl_sz.width;
							r.level = bidi_runs[i].value;
							r.script = TAG('Q', 'a', 'b', 'j'); /*private use script code*/
							r.break_type = RUN_BREAK_SOFT;
							visual.push_back(r);
						} else {
							String lang = (E->get().has(TEXT_ATTRIB_LANGUAGE)) ? (String)E->get()[TEXT_ATTRIB_LANGUAGE] : locale;
							String ftr = (E->get().has(TEXT_ATTRIB_OT_FEATURES)) ? (String)E->get()[TEXT_ATTRIB_OT_FEATURES] : "";
							const Ref<Font> font = (E->get().has(TEXT_ATTRIB_FONT)) ? (Ref<Font>)E->get()[TEXT_ATTRIB_FONT] : NULL;
							Vector<ShapingInterface::SourceRun> brk_runs = si->get_break_runs(text, bsa_start, bsa_end, lang);
							int k = brk_runs.size() - 1;
							while (k >= 0 && brk_runs[k].start > bsa_end)
								k--;
							int bsab_end = bsa_end;
							int bsab_start = MAX(bsa_start, brk_runs[k].start);
							while (bsab_end > bsa_start) {
								/*DEBUG*/ if (display_metrics)
									printf("    bd+src+a+brk r -> %d %d D:%s S:%x B:%d\n", bsab_start, bsab_end, (bidi_runs[i].value % 2 == 0) ? "L" : "R", script_runs[j].value, brk_runs[k].value);
								//shape bidi + script + attribute + wordbreak subruns
								visual.push_back(si->shape_run2(text, bsab_start, bsab_end, font.ptr(), ftr, lang, bidi_runs[i].value, script_runs[j].value, brk_runs[k].value, is_vertical));
								//advance
								if ((brk_runs[k].start >= bsab_start) && (k > 0)) k--;
								bsab_end = bsab_start;
								bsab_start = MAX(bsa_start, brk_runs[k].start);
							}
						}
						if (E->prev() && (E->key() >= bsa_start)) E = E->prev();
						if (bsa_start == bs_start) break;
						bsa_end = bsa_start;
						bsa_start = (E->prev()) ? MAX(E->key(), bs_start) : bs_start;
					}
					//advance
					if ((script_runs[j].start >= bs_start) && (j > 0)) j--;
					bs_end = bs_start;
					bs_start = MAX(bidi_runs[i].start, script_runs[j].start);
				}
			}
		}
		lines_shaped = false;
		text_shaped = true;
	}

	if (!lines_shaped) {
		/*DEBUG*/ if (display_metrics)
			printf("\n_____reshape:lines_____\n");
		lines.clear();

		//break lines
		Vector<Run> logical = visual;
		logical.sort_custom<RunCompare>();

		float line_width = 0.f;
		float line_width_at_i = 0.f;
		int line_start_run = 0;
		int line_start_pos = 0;
		int line_end_run = -1;
		int i = 0;
		while (i < logical.size()) {
			bool break_line = false;

			//set line_width to next tab stop or extend it to tab size
			if (tab_flags == TEXT_TAB_SPACE) {
				line_width += tab_width * logical[i].indent;
			} else {
				if (tab_width > 0) {
					line_width = (Math::floor(line_width / tab_width) + logical[i].indent) * tab_width;
				}
			}
			line_width += logical[i].width;
			/*DEBUG*/ if (display_metrics)
				printf(" r: %d(%d...%d) W:%f (+%f I:%d)", i, logical[i].start, logical[i].end, line_width, logical[i].width, logical[i].indent);
			if (brk_flags == TEXT_BREAK_ON_NONE) {
				line_end_run = logical.size() - 1;
				break_line = true;
				/*DEBUG*/ if (display_metrics)
					printf("  NB");
			}
			if ((target_size.width > 0) && (line_width > target_size.width) && (line_end_run != -1)) {
				break_line = true;
				/*DEBUG*/ if (display_metrics)
					printf("  OVERFLOW");
			}
			if ((brk_flags != TEXT_BREAK_ON_HARD) && !break_line && ((logical[i].break_type == RUN_BREAK_SOFT))) {
				line_end_run = i;
				line_width_at_i = line_width;
				/*DEBUG*/ if (display_metrics)
					printf("  soft set");
			}
			if (!break_line && ((logical[i].break_type == RUN_BREAK_HARD) || (i == logical.size() - 1))) {
				line_end_run = i;
				line_width_at_i = line_width;
				break_line = true;
				/*DEBUG*/ if (display_metrics)
					printf("  HARD");
			}
			/*DEBUG*/ if (display_metrics)
				printf("\n");
			if (break_line) {
				if ((brk_flags == TEXT_BREAK_ON_ANY) && (line_width_at_i > target_size.width)) {
					/*DEBUG*/ if (display_metrics)
						printf("   TODO: subrun\n");
					for (int j = logical[line_end_run].clusters.size() - 1; j >= 0; j--) {
						//TODO
					}
					//TODO - roll back on a cluster basis (do not reshape -> cut existing runs in two)
					//set line_start_pos and set line_width to (neg used part of run) instead of 0.f
				} else {
					Line l;
					l.start = line_start_pos;
					l.end = logical[line_end_run].end;
					l.ascent = 0.f;
					l.descent = 0.f;
					/*DEBUG*/ if (display_metrics)
						printf("  line %d %d\n", l.start, l.end);

					int col_start = line_start_pos;
					int col_next = -1;

					for (int c = line_start_run; c <= line_end_run + 1; c++) {
						//TODO skip indent on partial run
						if (c > line_end_run) { //flush to the end of run, TODO partial flush
							col_next = logical[line_end_run].end;
						} else if ((logical[c].indent == 0) || (col_next == -1)) {
							col_next = logical[c].end;
							continue;
						}
						/*DEBUG*/ if (display_metrics)
							printf("   col %d %d\n", col_start, col_next);

						Column col;
						Vector<ShapingInterface::SourceRun> line_bidi = si->get_bidi_runs(text, col_start, col_next, para_direction);
						for (int j = 0; j < line_bidi.size(); j++) {
							if (line_bidi[j].value % 2 == 0) {
								//LTR
								for (int k = line_start_run; k <= line_end_run; k++) {
									if ((logical[k].start >= line_bidi[j].start) && (logical[k].end <= line_bidi[j].end)) {
										//copy full
										col.visual.push_back(logical[k]);
										l.ascent = MAX(l.ascent, logical[k].ascent);
										l.descent = MAX(l.descent, logical[k].descent);
									} else if (unlikely((logical[k].end > line_bidi[j].start) && (logical[k].end <= line_bidi[j].end))) {
										//copy line_bidi[j].start to logical[k].end
										//TODO
									} else if (unlikely((logical[k].start < line_bidi[j].end) && (logical[k].start >= line_bidi[j].start))) {
										//copy logical[k].start to line_bidi[j].end
										//TODO
									}
								}
							} else {
								//RTL
								for (int k = line_end_run; k >= line_start_run; k--) {
									if ((logical[k].start >= line_bidi[j].start) && (logical[k].end <= line_bidi[j].end)) {
										//copy full
										col.visual.push_back(logical[k]);
										l.ascent = MAX(l.ascent, logical[k].ascent);
										l.descent = MAX(l.descent, logical[k].descent);
									} else if (unlikely((logical[k].end > line_bidi[j].start) && (logical[k].end <= line_bidi[j].end))) {
										//copy line_bidi[j].start to logical[k].end
										//TODO
									} else if (unlikely((logical[k].start < line_bidi[j].end) && (logical[k].start >= line_bidi[j].start))) {
										//copy logical[k].start to line_bidi[j].end
										//TODO
									}
								}
							}
						}
						col_start = col_next;
						col_next = -1;

						if (tab_flags == TEXT_TAB_RIGHT) {
							l.columns.insert(0, col);
						} else {
							l.columns.push_back(col); //LEFT and SPACE
						}
					}

					lines.push_back(l);
					line_width = 0.f;
					i = line_end_run;

					line_start_pos = (line_end_run + 1 < logical.size()) ? logical[line_end_run + 1].start : -1;
					line_start_run = line_end_run + 1;
					line_end_run = -1;
				}
			}
			i++;
		}

		layout_shaped = false;
		lines_shaped = true;
	}

	if (!layout_shaped) {
		/*DEBUG*/ if (display_metrics)
			printf("\n_____reshape:layout_____\n");
		//set column offset and calc line width (rect.size.width)
		for (int i = 0; i < lines.size(); i++) {
			lines.write[i].rect.size.x = 0;
			for (int j = 0; j < lines[i].columns.size(); j++) {
				/*DEBUG*/ if (display_metrics)
					printf("  l:%d/%d c:%d/%d (R:%d)", i, lines.size(), j, lines[i].columns.size(), lines[i].columns[j].visual.size());
				lines.write[i].columns.write[j].offset = 0;
				if (lines[i].columns[j].visual.size() > 0) {
					float col_w = 0.f;
					for (int k = 0; k < lines[i].columns[j].visual.size(); k++) {
						col_w += lines[i].columns[j].visual[k].width;
					}
					/*DEBUG*/ if (display_metrics)
						printf("  CC:%f", col_w);
					if (tab_flags == TEXT_TAB_SPACE) {
						/*DEBUG*/ if (display_metrics)
							printf("  WT:%f I:%d", tab_width, lines[i].columns[j].visual[0].indent);
						lines.write[i].columns.write[j].offset = tab_width * lines[i].columns[j].visual[0].indent;
						lines.write[i].rect.size.x += col_w + tab_width * lines[i].columns[j].visual[0].indent;
					} else {
						/*DEBUG*/ if (display_metrics)
							printf("  WX:%f I:%d <=%f ", tab_width, lines[i].columns[j].visual[0].indent, lines[i].rect.size.x);
						float off = (tab_width > 0) ? ((Math::floor(lines[i].rect.size.x / tab_width) + lines[i].columns[j].visual[0].indent) * tab_width) : 0.f;
						/*DEBUG*/ if (display_metrics)
							printf("  WT:%f", off);
						lines.write[i].columns.write[j].offset = off - lines[i].rect.size.x;
						lines.write[i].rect.size.x = off + col_w;
					}
				}
				/*DEBUG*/ if (display_metrics)
					printf(" off: %f\n", lines.write[i].columns.write[j].offset);
			}
			lines.write[i].rect.size.y = lines[i].ascent + lines[i].descent;
			/*DEBUG*/ if (display_metrics)
				printf("  line h: %f w: %f\n", lines.write[i].rect.size.y, lines.write[i].rect.size.x);
		}
		//calc and apply x_offset (set rect.pos.x) and calc total height (real_size.height)
		/*DEBUG*/ if (display_metrics)
			printf("halign\n");
		real_size.width = 0.f;
		real_size.height = 0.f;
		for (int i = 0; i < lines.size(); i++) {
			real_size.height += lines[i].rect.size.height + line_spacing;
			/*DEBUG*/ if (display_metrics)
				printf("   l:%d h: %f w: %f\n", i, lines[i].rect.size.height, lines.write[i].rect.size.width);
			switch (halign) {
				case HALIGN_LEFT: {
					real_size.width = MAX(real_size.width, lines.write[i].rect.size.width);
				} break;
				case HALIGN_CENTER: {
					lines.write[i].rect.position.x = (target_size.width - lines.write[i].rect.size.width) / 2.f;
					real_size.width = MAX(real_size.width, lines.write[i].rect.size.width);
				} break;
				case HALIGN_RIGHT: {
					lines.write[i].rect.position.x = (target_size.width - lines.write[i].rect.size.width);
					real_size.width = MAX(real_size.width, lines.write[i].rect.size.width);
				} break;
				case HALIGN_FILL_LEFT:
				case HALIGN_FILL_RIGHT:
				case HALIGN_FILL_ALL: {
					if (i == lines.size() - 1) {
						if (halign == HALIGN_FILL_LEFT) {
							real_size.width = MAX(real_size.width, lines.write[i].rect.size.width);
							break;
						} else if (halign == HALIGN_FILL_RIGHT) {
							lines.write[i].rect.position.x = (target_size.width - lines.write[i].rect.size.width);
							real_size.width = MAX(real_size.width, lines.write[i].rect.size.width);
							break;
						}
					}
					if (lines[i].columns.size() > 0) {
						Vector<Run> &v = lines.write[i].columns.write[lines[i].columns.size() - 1].visual;
						int32_t elongations = 0;
						int32_t spaces = 0;

						int32_t skip_left_run = 0;
						int32_t skip_left_gl = 0;
						for (int32_t j = 0; j < v.size(); j++) {
							for (int32_t k = 0; k < v[j].clusters.size(); k++) {
								if ((v[j].clusters[k].flags & GLYPH_SPACE) != GLYPH_SPACE) {
									skip_left_run = i;
									skip_left_gl = j;
									goto end_lskip;
								} else if (trim_edges) {
									for (int32_t g = 0; g < v[j].clusters[k].glyphs.size(); g++) {
										v.write[j].clusters.write[k].glyphs.write[g].advance = 0;
									}
									v.write[j].width -= v.write[j].clusters.write[k].advance * v[j].clusters[k].repeat;
									v.write[j].clusters.write[k].advance = 0;
								}
							}
						}
					end_lskip:

						int32_t skip_right_run = v.size() - 1;
						int32_t skip_right_gl = 0;
						for (int32_t j = v.size() - 1; j >= 0; j--) {
							skip_right_gl = v[j].clusters.size() - 1;
							for (int32_t k = v[j].clusters.size() - 1; k >= 0; k--) {
								if ((v[j].clusters[k].flags & GLYPH_SPACE) != GLYPH_SPACE) {
									skip_right_run = i;
									skip_right_gl = j;
									goto end_rskip;
								} else if (trim_edges) {
									for (int32_t g = 0; g < v[j].clusters[k].glyphs.size(); g++) {
										v.write[j].clusters.write[k].glyphs.write[g].advance = 0;
									}
									v.write[j].width -= v.write[j].clusters.write[k].advance * v[j].clusters[k].repeat;
									v.write[j].clusters.write[k].advance = 0;
								}
							}
						}
					end_rskip:

						float old_width = 0;
						for (int32_t j = 0; j < v.size(); j++) {
							old_width += v[j].width;
						}

						for (int32_t j = skip_left_run; j <= skip_right_run; j++) {
							int32_t gl_s = (j == skip_left_run) ? skip_left_gl : 0;
							int32_t gl_e = (j == skip_right_run) ? skip_right_gl : v[j].clusters.size() - 1;
							for (int32_t k = gl_s; k <= gl_e; k++) {
								if ((v[j].clusters[k].flags & GLYPH_VALID) == GLYPH_VALID) {
									if (v.write[j].clusters.write[k].glyphs.size() > 0) {
										if ((v[j].clusters[k].flags & GLYPH_ELONGATION) == GLYPH_ELONGATION) elongations++;
										if ((v[j].clusters[k].flags & GLYPH_SPACE) == GLYPH_SPACE) spaces++;
									}
								}
							}
						}

						float d_width = (target_size.width - old_width);

						if (elongations > 0) {
							float d_elong = d_width / float(elongations);
							for (int32_t j = skip_left_run; j <= skip_right_run; j++) {
								int32_t gl_s = (j == skip_left_run) ? skip_left_gl : 0;
								int32_t gl_e = (j == skip_right_run) ? skip_right_gl : v[j].clusters.size() - 1;
								for (int32_t k = gl_s; k <= gl_e; k++) {
									if ((v[j].clusters[k].flags & GLYPH_VALID) == GLYPH_VALID) {
										if ((v[j].clusters[k].flags & GLYPH_ELONGATION) == GLYPH_ELONGATION) {
											int count = MAX(0, v[j].clusters[k].repeat + Math::floor(d_elong / v[j].clusters[k].advance));
											float delta = (count - v[j].clusters[k].repeat) * v[j].clusters[k].advance;
											if (delta < d_width) {
												v.write[j].clusters.write[k].repeat = count;
												v.write[j].width += delta;
												d_width -= delta;
											}
										}
									}
								}
							}
						}
						if (spaces > 0) {
							float d_space = d_width / float(spaces);
							for (int32_t j = skip_left_run; j <= skip_right_run; j++) {
								int32_t gl_s = (j == skip_left_run) ? skip_left_gl : 0;
								int32_t gl_e = (j == skip_right_run) ? skip_right_gl : v[j].clusters.size() - 1;
								for (int32_t k = gl_s; k <= gl_e; k++) {
									float min_space = 2.f;
									if (v[j].clusters[k].font_impl.is_valid()) {
										min_space = v[j].clusters[k].font_impl->get_advance(v[j].clusters[k].font_impl->get_glyph(' ')) / 5.f; //20% of a space
									}
									if ((v[j].clusters[k].flags & GLYPH_VALID) == GLYPH_VALID) {
										if ((v[j].clusters[k].flags & GLYPH_SPACE) == GLYPH_SPACE) {
											if (v[j].clusters[k].glyphs.size() == 1) {
												bool virt = ((v[j].clusters[k].flags & GLYPH_VIRTUAL) == GLYPH_VIRTUAL);
												float delta = Math::ceil((d_space / float(v[j].clusters[k].repeat)));
												if ((v.write[j].clusters.write[k].glyphs[0].advance + delta) <= ((virt) ? 0 : min_space)) {
													delta = ((virt) ? 0 : min_space) - v.write[j].clusters.write[k].glyphs[0].advance;
												}
												if (delta > d_width) {
													if (d_width > 0)
														delta = d_width;
													else
														delta = 0;
												}
												v.write[j].clusters.write[k].glyphs.write[0].advance += delta;
												v.write[j].clusters.write[k].advance += delta;
												v.write[j].width += delta * v[j].clusters[k].repeat;
												d_width -= delta * v[j].clusters[k].repeat;
											}
										}
									}
								}
							}
						}
						lines.write[i].rect.size.width = target_size.width;
						real_size.width = target_size.width;
					}
				} break;
			}
		}
		/*DEBUG*/ if (display_metrics)
			printf("rs -> h: %f w: %f\n", real_size.height, real_size.width);

		//calc y_offset and spacing
		float y_offset = 0.f;
		float y_spacing = line_spacing;
		switch (valign) {
			case VALIGN_TOP: {
				//NOP
			} break;
			case VALIGN_CENTER: {
				y_offset = (target_size.height - real_size.height) / 2.f;
			} break;
			case VALIGN_BOTTOM: {
				y_offset = (target_size.height - real_size.height);
			} break;
			case VALIGN_FILL: {
				y_spacing = MAX(0.f, (target_size.height - real_size.height) / (lines.size() - 1));
			} break;
		}
		/*DEBUG*/ if (display_metrics)
			printf("yoff -> %f    sp -> %f\n", y_offset, y_spacing);

		//apply y_offset (rect.pos.y)
		if (reverse_line_order) {
			for (int i = lines.size() - 1; i >= 0; i--) {
				y_offset += lines[i].ascent;
				lines.write[i].rect.position.y = y_offset;
				y_offset += lines[i].descent + y_spacing;
				/*DEBUG*/ if (display_metrics)
					printf("  l(rev): %d pos: %f %f\n", i, lines.write[i].rect.position.x, lines.write[i].rect.position.y);
			}
		} else {
			for (int i = 0; i < lines.size(); i++) {
				y_offset += lines[i].ascent;
				lines.write[i].rect.position.y = y_offset;
				y_offset += lines[i].descent + y_spacing;
				/*DEBUG*/ if (display_metrics)
					printf("  l(fwd): %d pos: %f %f\n", i, lines.write[i].rect.position.x, lines.write[i].rect.position.y);
			}
		}

		layout_shaped = true;
	}
}

Mutex *TextLayout::layout_mutex = NULL;
SelfList<TextLayout>::List *TextLayout::layouts = NULL;

void TextLayout::invalidate_all() {
	if (layout_mutex) {
		layout_mutex->lock();
		SelfList<TextLayout> *E = layouts->first();
		while (E) {
			E->self()->text_shaped = false;
			E->self()->lines.clear();
			E->self()->visual.clear();
			E = E->next();
		}
		layout_mutex->unlock();
	}
}

void TextLayout::initialize_self_list() {
	if (!layout_mutex) {
		layout_mutex = Mutex::create();
	}
	if (!layouts) {
		layouts = memnew(SelfList<TextLayout>::List());
	}
}

void TextLayout::finish_self_list() {
	if (layout_mutex) {
		memdelete(layout_mutex);
		layout_mutex = NULL;
	}
	if (layouts) {
		memdelete(layouts);
		layouts = NULL;
	}
}
