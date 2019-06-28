/*************************************************************************/
/*  shaping_server.cpp                                                   */
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

#include "shaping_server.h"
#include "core/project_settings.h"

#include "shaping/shaping_interface.h"

ShapingServer *ShapingServer::singleton = NULL;

ShapingServer *ShapingServer::get_singleton() {

	return singleton;
}

ShapingServer::ShapingServer() {

	singleton = this;
}

ShapingServer::~ShapingServer() {

	primary_interface.unref();

	while (interfaces.size() > 0) {
		interfaces.remove(0);
	}

	singleton = NULL;
}

void ShapingServer::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_interface_count"), &ShapingServer::get_interface_count);
	ClassDB::bind_method(D_METHOD("get_interface", "idx"), &ShapingServer::get_interface);
	ClassDB::bind_method(D_METHOD("get_interfaces"), &ShapingServer::get_interfaces);
	ClassDB::bind_method(D_METHOD("find_interface", "name"), &ShapingServer::find_interface);

	ClassDB::bind_method(D_METHOD("get_primary_interface"), &ShapingServer::get_primary_interface);
	ClassDB::bind_method(D_METHOD("set_primary_interface", "interface"), &ShapingServer::set_primary_interface);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "primary_interface"), "set_primary_interface", "get_primary_interface");

	ADD_SIGNAL(MethodInfo("interface_added", PropertyInfo(Variant::STRING, "interface_name")));
	ADD_SIGNAL(MethodInfo("interface_removed", PropertyInfo(Variant::STRING, "interface_name")));
}

void ShapingServer::add_interface(const Ref<ShapingInterface> &p_interface) {

	ERR_FAIL_COND(p_interface.is_null());

	for (int i = 0; i < interfaces.size(); i++) {

		if (interfaces[i] == p_interface) {
			ERR_PRINT("Interface was already added");
			return;
		};
	};

	interfaces.push_back(p_interface);
	print_verbose("ShapingServer: interface added: " + p_interface->get_name() + (p_interface->get_info().empty() ? "" : (" (" + p_interface->get_info() + ")")));

	if (primary_interface.is_null() || (primary_interface == fallback_interface)) {
		primary_interface = p_interface;
		print_verbose("ShapingServer: primary interface set to: " + primary_interface->get_name());
		if (OS::get_singleton()->get_main_loop()) {
			OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_SHAPING_INTERFACE_CHANGED);
		}

		ShapedString::invalidate_all();
		TextLayout::invalidate_all();
		FontImplementation::invalidate_all();
	}

	emit_signal("interface_added", p_interface->get_name());
}

void ShapingServer::remove_interface(const Ref<ShapingInterface> &p_interface) {

	ERR_FAIL_COND(p_interface.is_null());

	if (primary_interface == p_interface) {
		primary_interface = fallback_interface;
		print_verbose("ShapingServer: primary interface set to reset to fallback interface: " + primary_interface->get_name());
		if (OS::get_singleton()->get_main_loop()) {
			OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_SHAPING_INTERFACE_CHANGED);
		}

		ShapedString::invalidate_all();
		TextLayout::invalidate_all();
		FontImplementation::invalidate_all();
	}

	int idx = -1;
	for (int i = 0; i < interfaces.size(); i++) {

		if (interfaces[i] == p_interface) {

			idx = i;
			break;
		};
	};

	ERR_FAIL_COND(idx == -1);

	print_verbose("ShapingServer: Removed interface" + p_interface->get_name());

	emit_signal("interface_removed", p_interface->get_name());
	interfaces.remove(idx);
}

int ShapingServer::get_interface_count() const {

	return interfaces.size();
}

Ref<ShapingInterface> ShapingServer::get_interface(int p_index) const {

	ERR_FAIL_INDEX_V(p_index, interfaces.size(), NULL);

	return interfaces[p_index];
}

Ref<ShapingInterface> ShapingServer::find_interface(const String &p_name) const {

	int idx = -1;
	for (int i = 0; i < interfaces.size(); i++) {

		if (interfaces[i]->get_name() == p_name) {

			idx = i;
			break;
		};
	};

	ERR_FAIL_COND_V(idx == -1, NULL);

	return interfaces[idx];
}

Array ShapingServer::get_interfaces() const {

	Array ret;

	for (int i = 0; i < interfaces.size(); i++) {
		Dictionary iface_info;

		iface_info["id"] = i;
		iface_info["name"] = interfaces[i]->get_name();
		iface_info["is_primary"] = (interfaces[i] == primary_interface);
		iface_info["is_fallback"] = (interfaces[i] == fallback_interface);

		ret.push_back(iface_info);
	};

	return ret;
}

Ref<ShapingInterface> ShapingServer::get_primary_interface() const {

	return primary_interface;
}

void ShapingServer::set_primary_interface(const Ref<ShapingInterface> &p_primary_interface) {

	primary_interface = p_primary_interface;

	bool found = false;
	for (int i = 0; i < interfaces.size(); i++) {

		if (interfaces[i] == p_primary_interface) {
			found = true;
			break;
		};
	};
	if (!found) {
		interfaces.push_back(p_primary_interface);
	}

	print_verbose("ShapingServer: Primary interface set to: " + primary_interface->get_name());
	if (OS::get_singleton()->get_main_loop()) {
		OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_SHAPING_INTERFACE_CHANGED);
	}

	ShapedString::invalidate_all();
	TextLayout::invalidate_all();
	FontImplementation::invalidate_all();
}

void ShapingServer::clear_primary_interface_if(const Ref<ShapingInterface> &p_primary_interface) {

	if ((primary_interface == p_primary_interface) && (primary_interface != fallback_interface)) {
		print_verbose("ShapingServer: Clearing primary interface");
		primary_interface.unref();
	};
}

void ShapingServer::set_fallback_interface(const Ref<ShapingInterface> &p_fallback_interface) {

	fallback_interface = p_fallback_interface;
}
