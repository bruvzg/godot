/*************************************************************************/
/*  shaping_server.h                                                     */
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

#ifndef SHAPING_SERVER_H
#define SHAPING_SERVER_H

#include "core/os/os.h"
#include "core/os/thread_safe.h"
#include "core/reference.h"
#include "core/variant.h"

class ShapingInterface;

class ShapingServer : public Object {
	GDCLASS(ShapingServer, Object);
	_THREAD_SAFE_CLASS_

	Vector<Ref<ShapingInterface>> interfaces;
	Ref<ShapingInterface> primary_interface;
	Ref<ShapingInterface> fallback_interface;

protected:
	static ShapingServer *singleton;

	static void _bind_methods();

public:
	static ShapingServer *get_singleton();

	void add_interface(const Ref<ShapingInterface> &p_interface);
	void remove_interface(const Ref<ShapingInterface> &p_interface);
	int get_interface_count() const;
	Ref<ShapingInterface> get_interface(int p_index) const;
	Ref<ShapingInterface> find_interface(const String &p_name) const;
	Array get_interfaces() const;

	Ref<ShapingInterface> get_primary_interface() const;
	void set_primary_interface(const Ref<ShapingInterface> &p_primary_interface);
	void clear_primary_interface_if(const Ref<ShapingInterface> &p_primary_interface);

	void set_fallback_interface(const Ref<ShapingInterface> &p_fallback_interface);

	ShapingServer();
	~ShapingServer();
};

#endif
