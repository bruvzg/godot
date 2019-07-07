/*************************************************************************/
/*  spannable.cpp                                                        */
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

#include "spannable.h"
#include "core/hashfuncs.h"

void Spannable::_bind_methods() {

	ClassDB::bind_method(D_METHOD("hash"), &Spannable::hash);

	ClassDB::bind_method(D_METHOD("add_attribute", "attribute", "value", "start", "end"), &Spannable::add_attribute);
	ClassDB::bind_method(D_METHOD("remove_attribute", "attribute", "start", "end"), &Spannable::remove_attribute);
	ClassDB::bind_method(D_METHOD("has_attribute", "attribute", "index"), &Spannable::has_attribute);
	ClassDB::bind_method(D_METHOD("get_attribute", "attribute", "index"), &Spannable::get_attribute);
	ClassDB::bind_method(D_METHOD("get_run_start", "attribute", "index"), &Spannable::get_run_start);
	ClassDB::bind_method(D_METHOD("get_run_end", "attribute", "index"), &Spannable::get_run_end);
	ClassDB::bind_method(D_METHOD("remove_attributes", "start", "end"), &Spannable::remove_attributes);
	ClassDB::bind_method(D_METHOD("clear_attributes"), &Spannable::clear_attributes);

	BIND_ENUM_CONSTANT(TEXT_ATTRIBUTE_FONT);
	BIND_ENUM_CONSTANT(TEXT_ATTRIBUTE_OT_FEATURES);
	BIND_ENUM_CONSTANT(TEXT_ATTRIBUTE_LANGUAGE);
	BIND_ENUM_CONSTANT(TEXT_ATTRIBUTE_COLOR);
	BIND_ENUM_CONSTANT(TEXT_ATTRIBUTE_OUTLINE_COLOR);

	BIND_ENUM_CONSTANT(TEXT_ATTRIBUTE_USER);
}

void Spannable::ensure_break(int64_t p_index) {

	Map<int64_t, Map<TextAttribute, Variant> >::Element *E = attributes.find_closest(p_index);
	attributes[p_index] = (E) ? E->get() : Map<TextAttribute, Variant>();
}

bool Spannable::compare_attributes(const Map<TextAttribute, Variant> &p_first, const Map<TextAttribute, Variant> &p_second) const {

	if (p_first.size() != p_second.size())
		return false;

	for (const Map<TextAttribute, Variant>::Element *E = p_first.front(); E; E = E->next()) {
		const Map<TextAttribute, Variant>::Element *F = p_second.find(E->key());
		if ((!F) || (E->get() != F->get())) return false;
	}
	return true;
}

void Spannable::optimize_attributes() {

	Vector<int64_t> erase_list;
	for (const Map<int64_t, Map<TextAttribute, Variant> >::Element *E = attributes.front(); E; E = E->next()) {
		if (E->prev() && (compare_attributes(E->get(), E->prev()->get()))) {
			erase_list.push_back(E->key());
		}
	}

	for (int64_t i = 0; i < erase_list.size(); i++) {
		attributes.erase(erase_list[i]);
	}
}

uint32_t Spannable::hash() const {

	uint32_t hashv = string.hash();

	for (const Map<int64_t, Map<TextAttribute, Variant> >::Element *E = attributes.front(); E; E = E->next()) {
		hashv = ((hashv << 5) + hashv) + hash_one_uint64(E->key());
		for (const Map<TextAttribute, Variant>::Element *F = E->get().front(); F; F = F->next()) {
			hashv = ((hashv << 5) + hashv) + F->get().hash();
			hashv = ((hashv << 5) + hashv) + hash_one_uint64(F->key());
		}
	}

	return hashv;
}

void Spannable::set_string(const String &p_string) {

	string = p_string;
	attributes.clear();
	emit_signal("attributes_changed");
}

String Spannable::get_string() const {

	return string;
}

void Spannable::add_attribute(TextAttribute p_attribute, Variant p_value, int64_t p_start, int64_t p_end) {

	if (p_end == -1)
		p_end = string.length();

	if (p_start < 0 || p_end > string.length() || p_start > p_end) {
		ERR_PRINTS("Invalid substring range [" + String::num_int64(p_start) + " ..." + String::num_int64(p_end) + "] / " + String::num_int64(string.length()));
		ERR_FAIL_COND(true);
	}

	ensure_break(0);
	ensure_break(p_start);

	if (p_end < string.length())
		ensure_break(p_end);

	Map<int64_t, Map<TextAttribute, Variant> >::Element *E = attributes.find(p_start);
	while (E && ((E->key() < p_end) || (p_end == string.length()))) {
		E->get()[p_attribute] = p_value;
		E = E->next();
	}
	optimize_attributes();

	emit_signal("attributes_changed");
}

void Spannable::remove_attribute(TextAttribute p_attribute, int64_t p_start, int64_t p_end) {

	if (p_end == -1)
		p_end = string.length();

	if (p_start < 0 || p_end > string.length() || p_start > p_end) {
		ERR_PRINTS("Invalid substring range [" + String::num_int64(p_start) + " ..." + String::num_int64(p_end) + "] / " + String::num_int64(string.length()));
		ERR_FAIL_COND(true);
	}

	ensure_break(p_start);

	if (p_end < string.length())
		ensure_break(p_end);

	Map<int64_t, Map<TextAttribute, Variant> >::Element *E = attributes.find(p_start);
	while (E && (E->key() < p_end)) {
		E->get().erase(p_attribute);
		E = E->next();
	}
	optimize_attributes();

	emit_signal("attributes_changed");
}

void Spannable::remove_attributes(int64_t p_start, int64_t p_end) {

	if (p_end == -1)
		p_end = string.length();

	if (p_start < 0 || p_end > string.length() || p_start > p_end) {
		ERR_PRINTS("Invalid substring range [" + String::num_int64(p_start) + " ..." + String::num_int64(p_end) + "] / " + String::num_int64(string.length()));
		ERR_FAIL_COND(true);
	}

	ensure_break(p_start);

	if (p_end < string.length())
		ensure_break(p_end);

	Map<int64_t, Map<TextAttribute, Variant> >::Element *E = attributes.find(p_start);
	while (E && (E->key() < p_end)) {
		E->get().clear();
		E = E->next();
	}
	optimize_attributes();

	emit_signal("attributes_changed");
}

void Spannable::clear_attributes() {

	attributes.clear();
	emit_signal("attributes_changed");
}

bool Spannable::has_attribute(TextAttribute p_attribute, int64_t p_index) const {

	if (p_index < 0 || p_index > string.length()) {
		ERR_PRINTS("Invalid substring range [" + String::num_int64(p_index) + "] / " + String::num_int64(string.length()));
		ERR_FAIL_COND_V(true, false);
	}

	const Map<int64_t, Map<TextAttribute, Variant> >::Element *E = attributes.find_closest(p_index);
	if (!E) {
		return false;
	}
	return E->get().has(p_attribute);
}

Variant Spannable::get_attribute(TextAttribute p_attribute, int64_t p_index) const {

	if (p_index < 0 || p_index > string.length()) {
		ERR_PRINTS("Invalid substring range [" + String::num_int64(p_index) + "] / " + String::num_int64(string.length()));
		ERR_FAIL_COND_V(true, Variant());
	}

	const Map<int64_t, Map<TextAttribute, Variant> >::Element *E = attributes.find_closest(p_index);
	if (!E) {
		return Variant();
	}
	if (E->get().has(p_attribute)) {
		return E->get()[p_attribute];
	} else {
		return Variant();
	}
}

int64_t Spannable::get_run_start(TextAttribute p_attribute, int64_t p_index) const {

	if (p_index < 0 || p_index > string.length()) {
		ERR_PRINTS("Invalid substring range [" + String::num_int64(p_index) + "] / " + String::num_int64(string.length()));
		ERR_FAIL_COND_V(true, -1);
	}

	const Map<int64_t, Map<TextAttribute, Variant> >::Element *E = attributes.find_closest(p_index);
	if (E) {
		ERR_PRINTS("Attribute not set");
		ERR_FAIL_COND_V(true, -1);
	}
	return E->key();
}

int64_t Spannable::get_run_end(TextAttribute p_attribute, int64_t p_index) const {

	if (p_index < 0 || p_index > string.length()) {
		ERR_PRINTS("Invalid substring range [" + String::num_int64(p_index) + "] / " + String::num_int64(string.length()));
		ERR_FAIL_COND_V(true, -1);
	}

	const Map<int64_t, Map<TextAttribute, Variant> >::Element *E = attributes.find_closest(p_index);
	E = E->next();
	if (E) {
		ERR_PRINTS("Attribute not set");
		ERR_FAIL_COND_V(true, -1);
	}
	return E->key();
}

Ref<Spannable> Spannable::substr(int64_t p_start, int64_t p_end) const {

	Ref<Spannable> ret;
	ret.instance();

	//Copy string
	ret->string = string.substr(p_start, p_end);

	//Copy attributes
	const Map<int64_t, Map<TextAttribute, Variant> >::Element *E = attributes.find_closest(p_start);
	while (E && (E->key() < p_end)) {
		ret->attributes[MAX(0, E->key() - p_start)] = E->get();
		E = E->next();
	}

	return ret;
}
