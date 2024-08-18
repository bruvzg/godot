/**************************************************************************/
/*  window_decoration.cpp                                                 */
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

#include "window_decoration.h"

#include "core/math/geometry_2d.h"
#include "scene/main/viewport.h"
#include "scene/main/window.h"

void WindowDecoration::_region_changed() {
	if (id != -1) {
		DisplayServer::WindowID wid = get_viewport()->get_window_id();
		Vector<Point2> polygon_global;
		if (non_rect && polygon.size() >= 3) {
			polygon_global = polygon;
		} else {
			polygon_global.push_back(Vector2());
			polygon_global.push_back(Vector2(get_size().x, 0));
			polygon_global.push_back(get_size());
			polygon_global.push_back(Vector2(0, get_size().y));
		}
		Transform2D t = get_global_transform();
		for (Vector2 &E : polygon_global) {
			E = t.xform(E);
		}
		DisplayServer::get_singleton()->window_change_decoration(id, polygon_global, dec_type, wid);
	}
}

void WindowDecoration::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_non_rectangular_region", "non_rect"), &WindowDecoration::set_non_rectangular_region);
	ClassDB::bind_method(D_METHOD("is_non_rectangular_region"), &WindowDecoration::is_non_rectangular_region);

	ClassDB::bind_method(D_METHOD("set_polygon", "polygon"), &WindowDecoration::set_polygon);
	ClassDB::bind_method(D_METHOD("get_polygon"), &WindowDecoration::get_polygon);

	ClassDB::bind_method(D_METHOD("set_decoration_type", "pressed"), &WindowDecoration::set_decoration_type);
	ClassDB::bind_method(D_METHOD("get_decoration_type"), &WindowDecoration::get_decoration_type);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "non_rectangular_region"), "set_non_rectangular_region", "is_non_rectangular_region");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR2_ARRAY, "polygon"), "set_polygon", "get_polygon");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "decoration_type", PROPERTY_HINT_ENUM, "Resize Top-Left,Resize Top,Resize Top-Right,Resize Left,Resize Right,Resize Bottom-Left,Resize Bottom,Resize Bottom-Right,Move"), "set_decoration_type", "get_decoration_type");
}

void WindowDecoration::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
#ifdef TOOLS_ENABLED
			if (is_part_of_edited_scene()) {
				break;
			}
#endif
			if (DisplayServer::get_singleton()->has_feature(DisplayServer::FEATURE_CLIENT_SIDE_DECORATIONS)) {
				DisplayServer::WindowID wid = get_viewport()->get_window_id();
				if (id != -1) {
					DisplayServer::get_singleton()->window_remove_decoration(id, wid);
				}
				Vector<Point2> polygon_global;
				if (non_rect && polygon.size() >= 3) {
					polygon_global = polygon;
				} else {
					polygon_global.push_back(Vector2());
					polygon_global.push_back(Vector2(get_size().x, 0));
					polygon_global.push_back(get_size());
					polygon_global.push_back(Vector2(0, get_size().y));
				}
				Transform2D t = get_global_transform();
				for (Vector2 &E : polygon_global) {
					E = t.xform(E);
				}
				id = DisplayServer::get_singleton()->window_add_decoration(polygon_global, dec_type, wid);
			}
		} break;

		case NOTIFICATION_EXIT_TREE: {
			if (id != -1) {
				DisplayServer::WindowID wid = get_viewport()->get_window_id();
				DisplayServer::get_singleton()->window_remove_decoration(id, wid);
				id = -1;
			}
		} break;

		default:
			break;
	}
}

void WindowDecoration::set_non_rectangular_region(bool p_non_rect) {
	non_rect = p_non_rect;
	_region_changed();
}

bool WindowDecoration::is_non_rectangular_region() const {
	return non_rect;
}

void WindowDecoration::set_polygon(const Vector<Point2> &p_polygon) {
	polygon = p_polygon;
	_region_changed();
}

Vector<Point2> WindowDecoration::get_polygon() const {
	return polygon;
}

void WindowDecoration::set_decoration_type(DisplayServer::WindowDecorationType p_dec_type) {
	ERR_FAIL_INDEX(p_dec_type, DisplayServer::WINDOW_DECORATION_MAX);

	if (dec_type == p_dec_type) {
		return;
	}

	dec_type = p_dec_type;
	_region_changed();
}

DisplayServer::WindowDecorationType WindowDecoration::get_decoration_type() const {
	return dec_type;
}

WindowDecoration::WindowDecoration() {
	connect("item_rect_changed", callable_mp(this, &WindowDecoration::_region_changed), CONNECT_DEFERRED);
}
