/**************************************************************************/
/*  objc_class_wrapper.mm                                                 */
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

#include "api/objc_class_wrapper.h"

#import <objc/runtime.h>

#import <Foundation/Foundation.h>

ObjCClassWrapper *ObjCClassWrapper::singleton = nullptr;

struct ObjCMethodInfo {
	bool static_method = false;
	Vector<uint32_t> param_types;
	Vector<uint32_t> param_sizes;
	uint32_t return_type = 0;
	uint32_t return_size = 0;
	SEL selector = nil;
};

struct ObjCClass::ObjCClassInternal {
	Class objc_class = nil;

	bool methods_loadead = false;
	HashMap<StringName, ObjCMethodInfo> methods;
};

struct ObjCObject::ObjCObjectInternal {
	id objc_instance = nil;
};

/**************************************************************************/
/* Class                                                                  */
/**************************************************************************/

bool ObjCClass::_method_call(const Ref<ObjCObject> &p_instance, const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error, Variant &r_ret) {
	if (internal->objc_class == nil) {
		r_error.error = Callable::CallError::CALL_ERROR_INSTANCE_IS_NULL;
		return false;
	}

	if (!internal->methods_loadead) {
		_update_methods_list();
	}

	HashMap<StringName, ObjCMethodInfo>::Iterator M = internal->methods.find(p_method);
	if (!M) {
		Ref<ObjCClass> parent = get_objc_parent_class();
		if (parent.is_null()) {
			return false;
		}
		return parent->_method_call(p_instance, p_method, p_args, p_argcount, r_error, r_ret);
	}

	int pc = M->value.param_types.size();
	if (p_argcount < pc) {
		r_error.error = Callable::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS;
		r_error.expected = pc;
		print_line("_c", p_argcount, pc);
		return false;
	}
	if (p_argcount > pc) {
		r_error.error = Callable::CallError::CALL_ERROR_TOO_MANY_ARGUMENTS;
		r_error.expected = pc;
		print_line("_d", p_argcount, pc);
		return false;
	}

	NSMethodSignature *method_signature;
	if (p_instance.is_valid()) {
		method_signature = [internal->objc_class instanceMethodSignatureForSelector:M->value.selector];
	} else {
		method_signature = [internal->objc_class methodSignatureForSelector:M->value.selector];
	}
	NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:method_signature];
	[invocation setTarget:(p_instance.is_valid()) ? p_instance->internal->objc_instance : internal->objc_class];
	[invocation setSelector:M->value.selector];

	for (int i = 0; i < pc; i++) { // 0 - self (of type id), 1 - _cmd (of type SEL).
		switch (M->value.param_types[i]) {
			case Variant::BOOL: {
				bool b_val = (bool)*p_args[i];
				[invocation setArgument:&b_val atIndex:i + 2];
			} break;
			case Variant::INT: {
				if (M->value.param_sizes[i] == 1) {
					uint8_t i_val = (uint8_t)*p_args[i];
					[invocation setArgument:&i_val atIndex:i + 2];
				} else if (M->value.param_sizes[i] == 2) {
					uint16_t i_val = (uint16_t)*p_args[i];
					[invocation setArgument:&i_val atIndex:i + 2];
				} else if (M->value.param_sizes[i] == 4) {
					uint32_t i_val = (uint32_t)*p_args[i];
					[invocation setArgument:&i_val atIndex:i + 2];
				} else if (M->value.param_sizes[i] == 8) {
					uint64_t i_val = (uint64_t)*p_args[i];
					[invocation setArgument:&i_val atIndex:i + 2];
				}
			} break;
			case Variant::FLOAT: {
				if (M->value.param_sizes[i] == 4) {
					float f_val = (float)*p_args[i];
					[invocation setArgument:&f_val atIndex:i + 2];
				} else if (M->value.param_sizes[i] == 8) {
					double f_val = (double)*p_args[i];
					[invocation setArgument:&f_val atIndex:i + 2];
				}
			} break;
			case Variant::STRING: {
				//TODO
			} break;
			case Variant::OBJECT: {
				Ref<ObjCObject> obj = *p_args[i];
				if (obj.is_null()) {
					Ref<ObjCClass> cls = *p_args[i];
					if (cls.is_null()) {
						r_error.error = Callable::CallError::CALL_ERROR_INVALID_ARGUMENT;
						r_error.argument = i;
						r_error.expected = M->value.param_types[i];
						return false;
					} else {
						[invocation setArgument:&cls->internal->objc_class atIndex:i + 2];
					}
				} else {
					[invocation setArgument:&obj->internal->objc_instance atIndex:i + 2];
				}
			} break;
			default: {
				r_error.error = Callable::CallError::CALL_ERROR_INVALID_ARGUMENT;
				r_error.argument = i;
				return false;
			}
		}
	}
	@try {
		[invocation invoke];
	} @catch (NSException *exception) {
		ERR_PRINT("NSException: " + String::utf8([exception reason].UTF8String));
		r_error.error = Callable::CallError::CALL_ERROR_INVALID_METHOD;
		return false;
	}

	switch (M->value.return_type) {
		case Variant::BOOL: {
			bool b_val;
			[invocation getReturnValue:&b_val];
			r_ret = b_val;
		} break;
		case Variant::INT: {
			if (M->value.return_size == 1) {
				uint8_t i_val;
				[invocation getReturnValue:&i_val];
				r_ret = i_val;
			} else if (M->value.return_size == 2) {
				uint16_t i_val;
				[invocation getReturnValue:&i_val];
				r_ret = i_val;
			} else if (M->value.return_size == 4) {
				uint32_t i_val;
				[invocation getReturnValue:&i_val];
				r_ret = i_val;
			} else if (M->value.return_size == 8) {
				uint64_t i_val;
				[invocation getReturnValue:&i_val];
				r_ret = i_val;
			}
		} break;
		case Variant::FLOAT: {
			if (M->value.return_size == 4) {
				float f_val;
				[invocation getReturnValue:&f_val];
				r_ret = f_val;
			} else if (M->value.return_size == 8) {
				double f_val;
				[invocation getReturnValue:&f_val];
				r_ret = f_val;
			}
		} break;
		case Variant::STRING: {
			//TODO
		} break;
		case Variant::OBJECT: {
			__unsafe_unretained id obj = nil;
			[invocation getReturnValue:&obj];
			Ref<ObjCObject> inst;
			inst.instantiate();
			inst->internal->objc_instance = obj;
			inst->base_class = ObjCClassWrapper::get_singleton()->wrap(String::utf8([NSStringFromClass([obj class]) UTF8String]));
			r_ret = inst;
		} break;
		default: {
			r_error.error = Callable::CallError::CALL_ERROR_INVALID_METHOD;
			return false;
		}
	}
	return true;
}

Variant ObjCClass::callp(const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error) {
	Variant ret;
	if (_method_call(Ref<ObjCObject>(), p_method, p_args, p_argcount, r_error, ret)) {
		return ret;
	}

	return RefCounted::callp(p_method, p_args, p_argcount, r_error);
}

String ObjCClass::get_objc_class_name() const {
	ERR_FAIL_COND_V(internal->objc_class == nil, String());
	NSString *name = NSStringFromClass(internal->objc_class);
	return String::utf8(name.UTF8String);
}

void ObjCClass::_convert_type(const String &p_type_string, uint32_t &r_type, uint32_t &r_size) {
	r_type = Variant::NIL;
	r_size = 0;

	if (p_type_string == "v") {
		r_type = Variant::NIL;
	} else if (p_type_string == "c") {
		r_type = Variant::INT; // Signed.
		r_size = 1;
	} else if (p_type_string == "i" || p_type_string == "s") {
		r_type = Variant::INT; // Signed.
		r_size = 2;
	} else if (p_type_string == "l") {
		r_type = Variant::INT; // Signed.
		r_size = 4;
	} else if (p_type_string == "q") {
		r_type = Variant::INT; // Signed.
		r_size = 8;
	} else if (p_type_string == "C") {
		r_type = Variant::INT; // Unsigned.
		r_size = 1;
	} else if (p_type_string == "I" || p_type_string == "S") {
		r_type = Variant::INT; // Unsigned.
		r_size = 2;
	} else if (p_type_string == "L") {
		r_type = Variant::INT; // Unsigned.
		r_size = 4;
	} else if (p_type_string == "Q") {
		r_type = Variant::INT; // Unsigned.
		r_size = 8;
	} else if (p_type_string == "f") {
		r_type = Variant::FLOAT;
		r_size = 4;
	} else if (p_type_string == "d") {
		r_type = Variant::FLOAT;
		r_size = 8;
	} else if (p_type_string == "B") {
		r_type = Variant::BOOL;
	} else if (p_type_string == "*") {
		r_type = Variant::STRING;
	} else if (p_type_string == "@") {
		r_type = Variant::OBJECT;
	} else if (p_type_string == "#") {
		r_type = Variant::OBJECT; // Class.
	} else if (p_type_string == ":") {
		//TODO selector
	} else if (p_type_string.begins_with("[")) {
		//TODO array
	} else if (p_type_string.begins_with("{")) {
		//TODO struct
	} else if (p_type_string.begins_with("(")) {
		//TODO union
	} else if (p_type_string.begins_with("^")) {
		//TODO raw pointer
	}
}

void ObjCClass::_update_methods_list() {
	char buf[1024];

	unsigned int m_count = 0;
	Method *m_list = class_copyMethodList(internal->objc_class, &m_count);
	for (unsigned int i = 0; i < m_count; i++) {
		ObjCMethodInfo method;
		method.selector = method_getName(m_list[i]);
		method.static_method = false;

		unsigned int arg_count = method_getNumberOfArguments(m_list[i]);
		for (unsigned int j = 2; j < arg_count; j++) {
			method_getArgumentType(m_list[i], j, &buf[0], 1024);
			uint32_t t = 0;
			uint32_t s = 0;
			_convert_type(String::utf8(buf, 1024), t, s);
			method.param_types.push_back(t);
			method.param_sizes.push_back(s);
		}

		method_getReturnType(m_list[i], &buf[0], 1024);
		_convert_type(String::utf8(buf, 1024), method.return_type, method.return_size);

		internal->methods[String::utf8(sel_getName(method.selector)).replace(":", "_")] = method;
	}
	free(m_list);

	m_list = class_copyMethodList(object_getClass(internal->objc_class), &m_count);
	for (unsigned int i = 0; i < m_count; i++) {
		ObjCMethodInfo method;
		method.selector = method_getName(m_list[i]);
		method.static_method = true;

		unsigned int arg_count = method_getNumberOfArguments(m_list[i]);
		for (unsigned int j = 2; j < arg_count; j++) {
			method_getArgumentType(m_list[i], j, &buf[0], 1024);
			uint32_t t = 0;
			uint32_t s = 0;
			_convert_type(String::utf8(buf, 1024), t, s);
			method.param_types.push_back(t);
			method.param_sizes.push_back(s);
		}

		method_getReturnType(m_list[i], &buf[0], 1024);
		_convert_type(String::utf8(buf, 1024), method.return_type, method.return_size);

		internal->methods[String::utf8(sel_getName(method.selector)).replace(":", "_")] = method;
	}
	free(m_list);

	internal->methods_loadead = true;
}

TypedArray<Dictionary> ObjCClass::get_objc_method_list() const {
	ERR_FAIL_COND_V(internal->objc_class == nil, TypedArray<Dictionary>());
	if (!internal->methods_loadead) {
		const_cast<ObjCClass *>(this)->_update_methods_list();
	}

	TypedArray<Dictionary> method_list;

	for (const KeyValue<StringName, ObjCMethodInfo> &mi : internal->methods) {
		Dictionary method;
		method["name"] = mi.key;
		method["default_args"] = Array();
		method["flags"] = (mi.value.static_method) ? (METHOD_FLAGS_DEFAULT | METHOD_FLAG_STATIC) : (METHOD_FLAGS_DEFAULT);

		{
			Array a;
			for (const uint32_t &t : mi.value.param_types) {
				Dictionary d;
				d["type"] = t;
				if (t == Variant::OBJECT) {
					d["hint"] = PROPERTY_HINT_RESOURCE_TYPE;
					d["hint_string"] = "ObjCObject";
				} else {
					d["hint"] = 0;
					d["hint_string"] = "";
				}
				a.push_back(d);
			}
			method["args"] = a;
		}
		{
			Dictionary d;
			d["type"] = mi.value.return_type;
			if (mi.value.return_type == Variant::OBJECT) {
				d["hint"] = PROPERTY_HINT_RESOURCE_TYPE;
				d["hint_string"] = "ObjCObject";
			} else {
				d["hint"] = 0;
				d["hint_string"] = "";
			}
			method["return_type"] = d;
		}

		method_list.push_back(method);
	}

	return method_list;
}

Ref<ObjCClass> ObjCClass::get_objc_parent_class() const {
	ERR_FAIL_COND_V(internal->objc_class == nil, Ref<ObjCClass>());

	if (parent_class_set) {
		return parent_class;
	}

	Class parent_objc_class = class_getSuperclass(internal->objc_class);
	if (parent_objc_class != nil) {
		const_cast<ObjCClass *>(this)->parent_class = ObjCClassWrapper::get_singleton()->wrap(String::utf8([NSStringFromClass(parent_objc_class) UTF8String]));
	}
	const_cast<ObjCClass *>(this)->parent_class_set = true;

	return parent_class;
}

Ref<ObjCObject> ObjCClass::create_instance() const {
	ERR_FAIL_COND_V(internal->objc_class == nil, Ref<ObjCObject>());

	Ref<ObjCObject> inst;
	inst.instantiate();
	inst->internal->objc_instance = class_createInstance(internal->objc_class, 0);
	inst->base_class = this;
	return inst;
}

String ObjCClass::to_string() {
	if (internal->objc_class == nil) {
		return "<ObjCClass:nil>";
	}
	return "<ObjCClass:" + get_objc_class_name() + ">";
}

ObjCClass::ObjCClass() {
	internal = memnew(ObjCClassInternal);
}

ObjCClass::~ObjCClass() {
	memdelete(internal);
}

/**************************************************************************/
/* Object                                                                 */
/**************************************************************************/

Variant ObjCObject::callp(const StringName &p_method, const Variant **p_args, int p_argcount, Callable::CallError &r_error) {
	if (base_class.is_null()) {
		return Variant();
	}

	Variant ret;
	if (base_class->_method_call(this, p_method, p_args, p_argcount, r_error, ret)) {
		return ret;
	}

	return RefCounted::callp(p_method, p_args, p_argcount, r_error);
}

Ref<ObjCClass> ObjCObject::get_objc_class() const {
	return base_class;
}

String ObjCObject::to_string() {
	if (base_class.is_null() || internal->objc_instance == nil) {
		return "<ObjCObject:nil>";
	}
	NSString *dsc = [internal->objc_instance description];
	return "<ObjCObject:" + base_class->get_objc_class_name() + " \"" + String::utf8([dsc UTF8String]) + "\">";
}

ObjCObject::ObjCObject() {
	internal = memnew(ObjCObjectInternal);
}

ObjCObject::~ObjCObject() {
	memdelete(internal);
}

/**************************************************************************/
/* Wrapper                                                                */
/**************************************************************************/

Ref<ObjCClass> ObjCClassWrapper::wrap(const String &p_class) {
	Class objc_class = NSClassFromString([NSString stringWithUTF8String:p_class.utf8().get_data()]);
	ERR_FAIL_COND_V(objc_class == nil, Ref<ObjCClass>());

	HashMap<StringName, Ref<ObjCClass>>::Iterator M = class_info.find(p_class);
	if (M) {
		return M->value;
	}

	Ref<ObjCClass> new_class;
	new_class.instantiate();
	new_class->internal->objc_class = objc_class;
	class_info[p_class] = new_class;

	return new_class;
}
