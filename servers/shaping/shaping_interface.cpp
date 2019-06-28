/*************************************************************************/
/*  shaping_interface.cpp                                                */
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

#include "servers/shaping/shaping_interface.h"
#include "servers/shaping_server.h"

ShapingInterface::SourceRun::SourceRun() {
	start = -1;
	end = -1;
	value = 0;
}

ShapingInterface::SourceRun::SourceRun(int p_start, int p_end, uint32_t p_value) {
	start = p_start;
	end = p_end;
	value = p_value;
}

void ShapingInterface::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_name"), &ShapingInterface::get_name);
	ClassDB::bind_method(D_METHOD("get_capabilities"), &ShapingInterface::get_capabilities);

	ClassDB::bind_method(D_METHOD("get_info"), &ShapingInterface::get_info);

	ClassDB::bind_method(D_METHOD("get_string_direction", "text"), &ShapingInterface::get_string_direction);
	ClassDB::bind_method(D_METHOD("get_locale_direction", "lang"), &ShapingInterface::get_locale_direction);

	ClassDB::bind_method(D_METHOD("set_is_primary", "enable"), &ShapingInterface::set_is_primary);
	ClassDB::bind_method(D_METHOD("get_is_primary"), &ShapingInterface::get_is_primary);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_primary"), "set_is_primary", "get_is_primary");
}

bool ShapingInterface::get_is_primary() const {

	ShapingServer *text_server = ShapingServer::get_singleton();
	ERR_FAIL_NULL_V(text_server, false);

	return text_server->get_primary_interface() == this;
}

void ShapingInterface::set_is_primary(bool p_is_primary) {

	ShapingServer *text_server = ShapingServer::get_singleton();
	ERR_FAIL_NULL(text_server);

	if (p_is_primary) {
		text_server->set_primary_interface(this);
	} else {
		text_server->clear_primary_interface_if(this);
	};
}
