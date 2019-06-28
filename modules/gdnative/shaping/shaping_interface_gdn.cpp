/*************************************************************************/
/*  shaping_interface_gdn.cpp                                            */
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

#include "shaping_interface_gdn.h"
#include "core/translation.h"
#include "font_implementation_gdn.h"
#include "scene/resources/font.h"

ShapingInterfaceGDNative::ShapingInterfaceGDNative() {
	print_verbose("Construct shaping gdnative interface\n");
	data = NULL;
	interface = NULL;
}

ShapingInterfaceGDNative::~ShapingInterfaceGDNative() {
	print_verbose("Destruct shaping gdnative interface\n");
	if (interface != NULL) {
		interface->destructor(data);
		data = NULL;
		interface = NULL;
	};
}

void ShapingInterfaceGDNative::_bind_methods() {
	//NOP
}

void ShapingInterfaceGDNative::set_interface(const godot_shaping_interface_gdnative *p_interface) {
	if (interface != NULL) {
		interface->destructor(data);
		data = NULL;
		interface = NULL;
	};
	interface = p_interface;
	data = interface->constructor((godot_object *)this);
}

StringName ShapingInterfaceGDNative::get_name() const {
	ERR_FAIL_COND_V(interface == NULL, StringName("Invalid"));
	godot_string result = interface->get_name(data);
	StringName name = *(String *)&result;
	godot_string_destroy(&result);
	return name;
}

int32_t ShapingInterfaceGDNative::get_capabilities() const {
	ERR_FAIL_COND_V(interface == NULL, 0);
	return interface->get_capabilities(data);
}

String ShapingInterfaceGDNative::get_info() const {
	ERR_FAIL_COND_V(interface == NULL, StringName(""));
	godot_string result = interface->get_info(data);
	String info = *(String *)&result;
	godot_string_destroy(&result);
	return info;
}

Ref<FontImplementation> ShapingInterfaceGDNative::create_font_implementation(const FontData *p_data, FontData::CacheID p_id) const {
	Ref<FontImplementationGDN> ret;
	ret.instance();
	ret->interface = interface;
	ret->interface_data = data;
	ret->font_data = interface->create_font_impl(data, (const godot_object *)p_data);
	ret->valid = (ret->font_data != NULL);
	return ret;
}

TextDirection ShapingInterfaceGDNative::get_paragraph_direction(TextDirection p_base_direction, const String &p_text, const String &p_language) const {
	ERR_FAIL_COND_V(interface == NULL, TEXT_DIRECTION_INVALID);
	//TODO ADD VERSION 2
	return TEXT_DIRECTION_INVALID;
}

Vector<ShapingInterface::SourceRun> ShapingInterfaceGDNative::get_bidi_runs(const String &p_text, int p_start, int p_end, TextDirection p_para_direction) const {
	Vector<SourceRun> runs;
	//TODO ADD VERSION 2
	return runs;
}

Vector<ShapingInterface::SourceRun> ShapingInterfaceGDNative::get_script_runs(const String &p_text, int p_start, int p_end, TextDirection p_para_direction) const {
	Vector<SourceRun> runs;
	//TODO ADD VERSION 2
	return runs;
}

Vector<ShapingInterface::SourceRun> ShapingInterfaceGDNative::get_break_runs(const String &p_text, int p_start, int p_end, const String &p_language) const {
	Vector<SourceRun> runs;
	//TODO ADD VERSION 2
	return runs;
}

Run ShapingInterfaceGDNative::shape_run2(const String &p_text, int p_start, int p_end, const Font *p_font, const String &p_features, const String &p_language, uint32_t p_level, uint32_t p_script, uint32_t p_break, bool p_prefer_vertical) const {
	Run r;
	//TODO ADD VERSION 2
	return r;
}

Vector<float> ShapingInterfaceGDNative::get_ligature_caret_offsets(const Ref<FontImplementation> &p_font_imp, TextDirection p_direction, uint32_t p_glyph) const {
	Vector<float> ret;
	//TODO ADD VERSION 2
	return ret;
}

//S VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE
TextDirection ShapingInterfaceGDNative::analyse_text(const String &p_text, int32_t p_start, int32_t p_end, TextDirection p_base_direction, const String &p_language, Vector<Run> &p_runs) const {
	ERR_FAIL_COND_V(interface == NULL, TEXT_DIRECTION_INVALID);
	const godot_string *text = (const godot_string *)&p_text;
	const godot_string *language = (const godot_string *)&p_language;
	godot_object *runs = (godot_object *)&p_runs;
	return (TextDirection)interface->analyse_text(data, text, p_start, p_end, p_base_direction, language, runs);
}

bool ShapingInterfaceGDNative::shape_run(const String &p_text, const Font *p_font, Run &p_run, const String &p_language, const String &p_features) const {
	ERR_FAIL_COND_V(interface == NULL, false);
	Vector<Ref<FontImplementation>> fonts;
	Vector<Ref<FontImplementation>> fonts_outline;
	p_font->get_font_implementations(fonts, fonts_outline, p_run.script);
	const godot_string *text = (const godot_string *)&p_text;
	godot_object *run = (godot_object *)&p_run;
	const godot_string *language = (const godot_string *)&p_language;
	const godot_string *features = (const godot_string *)&p_features;
	return interface->shape_run(data, text, (godot_object *)&fonts, (godot_object *)&fonts_outline, run, language, features);
}

TextDirection ShapingInterfaceGDNative::get_string_direction(const String &p_text) const {
	ERR_FAIL_COND_V(interface == NULL, TEXT_DIRECTION_AUTO);
	const godot_string *text = (const godot_string *)&p_text;
	return (TextDirection)interface->get_string_direction(data, text);
}
//E VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE //VERSION 1 REMOVE

TextDirection ShapingInterfaceGDNative::get_locale_direction(const String &p_text) const {
	ERR_FAIL_COND_V(interface == NULL, TEXT_DIRECTION_AUTO);
	const godot_string *text = (const godot_string *)&p_text;
	return (TextDirection)interface->get_locale_direction(data, text);
}

/*************************************************************************/
/* GDNative helper callbacks                                             */
/*************************************************************************/

extern "C" {

//ShapingInterface registration
void GDAPI godot_shaping_register_interface(const godot_shaping_interface_gdnative *p_interface) {
	Ref<ShapingInterfaceGDNative> new_interface;
	new_interface.instance();
	new_interface->set_interface(p_interface);
	ShapingServer::get_singleton()->add_interface(new_interface);
}

//Font direct access helper functions
float GDAPI godot_shaping_font_oversampling() {
	return Font::get_oversampling();
}

int GDAPI godot_shaping_hexbox_advance(uint32_t p_unicode) {
	return FontHexBox::get_advance(p_unicode);
}

int GDAPI godot_shaping_hexbox_ascent() {
	return FontHexBox::get_ascent();
}

int GDAPI godot_shaping_hexbox_descent() {
	return FontHexBox::get_descent();
}

//FontData direct access helper functions
godot_string GDAPI godot_shaping_font_data_get_type(godot_object *p_fontdata) {
	FontData *fontdata = (FontData *)p_fontdata;
	godot_string ret;
	ERR_FAIL_COND_V(fontdata == NULL, ret);
	memnew_placement(&ret, String(fontdata->_get_font_id()));
	return ret;
}

godot_string GDAPI godot_shaping_font_data_get_filename(godot_object *p_fontdata) {
	FontData *fontdata = (FontData *)p_fontdata;
	godot_string ret;
	ERR_FAIL_COND_V(fontdata == NULL, ret);
	memnew_placement(&ret, String(fontdata->get_font_data_path()));
	return ret;
}

void godot_shaping_font_data_get_static_font_data(godot_object *p_fontdata, const uint8_t **o_font_mem, size_t *o_font_mem_size) {
	FontData *fontdata = (FontData *)p_fontdata;
	ERR_FAIL_COND(fontdata == NULL);
	fontdata->_get_static_font_data(o_font_mem, o_font_mem_size);
}

//Vector<Runs> direct access helper functions
int32_t GDAPI godot_shaping_runs_size(const godot_object *p_vector) {
	Vector<Run> *vector = (Vector<Run> *)p_vector;
	ERR_FAIL_COND_V(vector == NULL, 0);
	return vector->size();
}

void GDAPI godot_shaping_runs_clear(godot_object *p_vector) {
	Vector<Run> *vector = (Vector<Run> *)p_vector;
	ERR_FAIL_COND(vector == NULL);
	vector->clear();
}

void GDAPI godot_shaping_runs_push_back(godot_object *p_vector, int32_t p_start, int32_t p_end, int32_t p_level, uint32_t p_script, uint8_t p_break_type) {
	Vector<Run> *vector = (Vector<Run> *)p_vector;
	ERR_FAIL_COND(vector == NULL);

	Run run;
	run.start = p_start;
	run.end = p_end;
	run.level = p_level;
	run.script = p_script;
	run.break_type = (RunBreakType)p_break_type;

	vector->push_back(run);
}

godot_object *GDAPI godot_shaping_runs_get(const godot_object *p_vector, int32_t p_index) {
	Vector<Run> *vector = (Vector<Run> *)p_vector;
	ERR_FAIL_COND_V(vector == NULL, NULL);
	ERR_FAIL_COND_V(p_index < 0 || p_index > vector->size(), NULL);
	return (godot_object *)((*vector).ptrw() + p_index);
}

//Run direct access helper functions
void GDAPI godot_shaping_run_set_bounds(godot_object *p_run, int32_t p_start, int32_t p_end, int32_t p_level, uint32_t p_script) {
	Run *run = (Run *)p_run;
	ERR_FAIL_COND(run == NULL);
	run->start = p_start;
	run->end = p_end;
	run->level = p_level;
	run->script = p_script;
}

void GDAPI godot_shaping_run_get_bounds(const godot_object *p_run, int32_t *o_start, int32_t *o_end, int32_t *o_level, uint32_t *o_script) {
	Run *run = (Run *)p_run;
	ERR_FAIL_COND(run == NULL);
	if (o_start) *o_start = run->start;
	if (o_end) *o_end = run->end;
	if (o_level) *o_level = run->level;
	if (o_script) *o_script = run->script;
}

void GDAPI godot_shaping_run_set_metrics(godot_object *p_run, int p_ascent, int p_descent, int p_width) {
	Run *run = (Run *)p_run;
	ERR_FAIL_COND(run == NULL);
	run->width = p_width;
	run->ascent = p_ascent;
	run->descent = p_descent;
}

void GDAPI godot_shaping_run_get_metrics(const godot_object *p_run, int *o_ascent, int *o_descent, int *o_width) {
	Run *run = (Run *)p_run;
	ERR_FAIL_COND(run == NULL);
	if (o_width) *o_width = run->width;
	if (o_ascent) *o_ascent = run->ascent;
	if (o_descent) *o_descent = run->descent;
}

//Cluster/Glyph direct access helper functions (write only)
void GDAPI godot_shaping_run_cluster_clear(godot_object *p_run) {
	Run *run = (Run *)p_run;
	ERR_FAIL_COND(run == NULL);

	run->clusters.clear();
}

void GDAPI godot_shaping_run_cluster_push_back(godot_object *p_run, int32_t p_start, int32_t p_end, int p_advance, int32_t p_repeat, void *p_font, void *p_outl_font, uint8_t p_flags) {
	Run *run = (Run *)p_run;
	ERR_FAIL_COND(run == NULL);

	Cluster cl;
	cl.start = p_start;
	cl.end = p_end;
	cl.advance = p_advance;
	cl.repeat = p_repeat;
	cl.font_impl = Ref<FontImplementation>((FontImplementationGDN *)p_font);
	cl.font_outline_impl = Ref<FontImplementation>((FontImplementationGDN *)p_outl_font);
	cl.flags = p_flags;

	run->clusters.push_back(cl);
}

void GDAPI godot_shaping_run_glyph_push_back(godot_object *p_run, uint32_t p_codepoint, float p_offset_x, float p_offset_y, int p_advance) {
	Run *run = (Run *)p_run;
	ERR_FAIL_COND(run == NULL);
	ERR_FAIL_COND(run->clusters.size() == 0);

	Glyph gl;
	gl.codepoint = p_codepoint;
	gl.offset = Vector2(p_offset_x, p_offset_y);
	gl.advance = p_advance;

	run->clusters.write[run->clusters.size() - 1].glyphs.push_back(gl);
}

//Vector<FontImplementation> direct access helper functions
int32_t GDAPI godot_shaping_font_imp_vector_size(godot_object *p_vector) {
	Vector<Ref<FontImplementation>> *vector = (Vector<Ref<FontImplementation>> *)p_vector;
	ERR_FAIL_COND_V(vector == NULL, 0);
	return vector->size();
}

godot_object *GDAPI godot_shaping_font_imp_vector_item(godot_object *p_vector, int32_t p_index) {
	Vector<Ref<FontImplementation>> *vector = (Vector<Ref<FontImplementation>> *)p_vector;
	ERR_FAIL_COND_V(vector == NULL, NULL);
	ERR_FAIL_COND_V(p_index < 0 || p_index >= vector->size(), NULL);
	return (godot_object *)(*vector)[p_index].ptr();
}

//FontImplementation direct access helper functions
void *GDAPI godot_shaping_font_imp_native_data(godot_object *p_font) {
	FontImplementation *font_impl = (FontImplementation *)p_font;
	ERR_FAIL_COND_V(font_impl == NULL, NULL);

	return font_impl->get_native_handle();
}

//TranslationServer direct access helper functions
godot_string GDAPI godot_shaping_get_locale() {
	godot_string ret;
	memnew_placement(&ret, String(TranslationServer::get_singleton()->get_locale()));
	return ret;
}

} //extern "C"
