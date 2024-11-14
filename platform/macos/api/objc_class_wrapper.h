/**************************************************************************/
/*  objc_class_wrapper.h                                                  */
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

#ifndef OBJC_CLASS_WRAPPER_H
#define OBJC_CLASS_WRAPPER_H

#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

class ObjCClassWrapper;
class ObjCObject;
class ObjCClass : public RefCounted {
	GDCLASS(ObjCClass, RefCounted);
	friend class ObjCClassWrapper;
	friend class ObjCObject;

#ifdef MACOS_ENABLED
	struct ObjCClassInternal;
	ObjCClassInternal *internal = nullptr;
	Ref<ObjCClass> parent_class;
	bool parent_class_set = false;

	static void _convert_type(const String &p_type_string, uint32_t &r_type, uint32_t &r_size);
	bool _method_call(const Ref<ObjCObject> &p_instance, const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error, Variant &r_ret);
	void _update_methods_list();
#endif

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_objc_class_name"), &ObjCClass::get_objc_class_name);
		ClassDB::bind_method(D_METHOD("get_objc_method_list"), &ObjCClass::get_objc_method_list);
		ClassDB::bind_method(D_METHOD("get_objc_parent_class"), &ObjCClass::get_objc_parent_class);

		ClassDB::bind_method(D_METHOD("create_instance"), &ObjCClass::create_instance);
	}

public:
#ifdef MACOS_ENABLED
	virtual Variant callp(const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error) override;
#endif
	String get_objc_class_name() const;
	TypedArray<Dictionary> get_objc_method_list() const;
	Ref<ObjCClass> get_objc_parent_class() const;

	Ref<ObjCObject> create_instance() const;

#ifdef MACOS_ENABLED
	virtual String to_string() override;
#endif

	ObjCClass();
	~ObjCClass();
};

class ObjCObject : public RefCounted {
	GDCLASS(ObjCObject, RefCounted);
	friend class ObjCClassWrapper;
	friend class ObjCClass;

#ifdef MACOS_ENABLED
	struct ObjCObjectInternal;
	ObjCObjectInternal *internal = nullptr;

	Ref<ObjCClass> base_class;
#endif

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_objc_class"), &ObjCObject::get_objc_class);
	}

public:
#ifdef MACOS_ENABLED
	virtual Variant callp(const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error) override;
#endif

	Ref<ObjCClass> get_objc_class() const;

#ifdef MACOS_ENABLED
	virtual String to_string() override;
#endif

	ObjCObject();
	~ObjCObject();
};

class ObjCClassWrapper : public Object {
	GDCLASS(ObjCClassWrapper, Object);

	static ObjCClassWrapper *singleton;
#ifdef MACOS_ENABLED
	HashMap<StringName, Ref<ObjCClass>> class_info;
#endif

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("wrap", "name"), &ObjCClassWrapper::wrap);
	}

public:
	static ObjCClassWrapper *get_singleton() { return singleton; }

	Ref<ObjCClass> wrap(const String &p_class);

	ObjCClassWrapper() { singleton = this; }
	~ObjCClassWrapper() { singleton = nullptr; }
};

#endif // OBJC_CLASS_WRAPPER_H
