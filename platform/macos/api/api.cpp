/**************************************************************************/
/*  api.cpp                                                               */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "api.h"

#include "objc_class_wrapper.h"

#include "core/config/engine.h"

static ObjCClassWrapper *objc_class_wrapper = nullptr;

void register_macos_api() {
	objc_class_wrapper = memnew(ObjCClassWrapper);

	GDREGISTER_CLASS(ObjCClass);
	GDREGISTER_CLASS(ObjCObject);
	GDREGISTER_CLASS(ObjCClassWrapper);
	Engine::get_singleton()->add_singleton(Engine::Singleton("ObjCClassWrapper", ObjCClassWrapper::get_singleton()));
}

void unregister_macos_api() {
	memdelete(objc_class_wrapper);
}

#ifndef MACOS_ENABLED
ObjCClassWrapper *ObjCClassWrapper::singleton = nullptr;

String ObjCClass::get_objc_class_name() const { return String(); }
TypedArray<Dictionary> ObjCClass::get_objc_method_list() const { return TypedArray<Dictionary>(); }
Ref<ObjCClass> ObjCClass::get_objc_parent_class() const { return Ref<ObjCClass>(); }
Ref<ObjCObject> ObjCClass::create_instance() const { return Ref<ObjCObject>(); }
ObjCClass::ObjCClass() {}
ObjCClass::~ObjCClass() {}

Ref<ObjCClass> ObjCObject::get_objc_class() const { return Ref<ObjCClass>(); }
ObjCObject::ObjCObject() {}
ObjCObject::~ObjCObject() {}

Ref<ObjCClass> ObjCClassWrapper::wrap(const String &p_class) { return Ref<ObjCClass>(); }
#endif
