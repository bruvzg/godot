/**************************************************************************/
/*  accessibility_server_mock.cpp                                         */
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

#include "accessibility_server_mock.h"

#ifdef TOOLS_ENABLED

#include "core/debugger/engine_debugger.h"
#include "core/variant/typed_array.h"
#include "servers/display/display_server.h"
#include "servers/text/text_server.h"

#ifdef TOOLS_ENABLED

Array AccessibilityServerMock::debug_get_window_list() {
	Array arr;
	for (const KeyValue<DisplayServerEnums::WindowID, WindowData> &wd : windows) {
		arr.push_back(wd.key);
		arr.push_back(wd.value.root_id);
	}
	return arr;
}

Array AccessibilityServerMock::debug_get_node_info(const RID &p_rid) {
	Array arr;
	arr.push_back(p_rid);

	AccessibilityElement *ae = rid_owner.get_or_null(p_rid);
	if (ae) {
		arr.push_back(ae->role);
		TypedArray<RID> c;
		for (const RID &r : ae->children) {
			c.push_back(r);
		}
		for (const RID &r : ae->indirect_children) {
			c.push_back(r);
		}
		arr.push_back(ae->parent);
		arr.push_back(c);
		arr.push_back(ae->meta);
		arr.push_back(ae->name);
		arr.push_back(ae->name_extra_info);
		TypedArray<AccessibilityServerEnums::AccessibilityAction> actions;
		for (const KeyValue<AccessibilityServerEnums::AccessibilityAction, Callable> &act : ae->actions) {
			actions.push_back(act.key);
		}
		arr.push_back(actions);
		arr.push_back(ae->run);
		arr.push_back(ae->value);
		arr.push_back(ae->flags);
		arr.push_back(String());
		arr.push_back(ae->window_id);
	}
	return arr;
}

void AccessibilityServerMock::debug_trigger_action(const RID &p_rid, AccessibilityServerEnums::AccessibilityAction p_action, const Variant &p_data) {
	AccessibilityElement *ae = rid_owner.get_or_null(p_rid);
	ERR_FAIL_NULL(ae);

	Variant rq_data = p_data;
	if (!ae->actions.has(p_action) && ae->role == AccessibilityServerEnums::ROLE_TEXT_RUN && p_action == AccessibilityServerEnums::ACTION_SCROLL_INTO_VIEW) {
		AccessibilityElement *root_ae = static_cast<AccessibilityServerMock *>(singleton)->rid_owner.get_or_null(ae->parent);
		ERR_FAIL_NULL(root_ae);
		ae = root_ae;
		rq_data = ae->run;
	}

	if (ae->actions.has(p_action)) {
		Callable &cb = ae->actions[p_action];
		cb.call_deferred(rq_data);
	}
}

bool AccessibilityServerMock::is_ancestor_of(const RID &p_rid, const RID &p_parent) const {
	AccessibilityElement *ae = rid_owner.get_or_null(p_rid);
	while (ae) {
		if (ae->parent.is_null()) {
			return false;
		}
		if (ae->parent == p_parent) {
			return true;
		}
		ae = rid_owner.get_or_null(ae->parent);
	}
	return false;
}

Error AccessibilityServerMock::parse_message(void *p_user, const String &p_msg, const Array &p_args, bool &r_captured) {
	ERR_FAIL_NULL_V(singleton, ERR_UNCONFIGURED);

	if (p_msg == "rq_window_list") {
		EngineDebugger::get_singleton()->send_message("accessibility:window_list", singleton->debug_get_window_list());
	} else if (p_msg == "rq_node_info") {
		ERR_FAIL_COND_V(p_args.size() < 1, ERR_INVALID_DATA);
		RID rid = p_args[0];
		EngineDebugger::get_singleton()->send_message("accessibility:ae_info", singleton->debug_get_node_info(rid));
	} else if (p_msg == "rq_action") {
		ERR_FAIL_COND_V(p_args.size() < 3, ERR_INVALID_DATA);
		RID rid = p_args[0];
		AccessibilityServerEnums::AccessibilityAction action = p_args[1];
		Variant data = p_args[2];
		singleton->debug_trigger_action(rid, action, data);
	} else if (p_msg == "rq_state") {
		Array arr;
		arr.push_back(get_singleton()->get_mode());
		arr.push_back(DisplayServer::get_singleton()->accessibility_should_increase_contrast());
		arr.push_back(DisplayServer::get_singleton()->accessibility_should_reduce_animation());
		arr.push_back(DisplayServer::get_singleton()->accessibility_should_reduce_transparency());
		arr.push_back(DisplayServer::get_singleton()->accessibility_screen_reader_active());
		EngineDebugger::get_singleton()->send_message("accessibility:state", arr);
	}

	return OK;
}

#endif

bool AccessibilityServerMock::window_create(DisplayServerEnums::WindowID p_window_id, void *p_handle) {
	ERR_FAIL_COND_V(windows.has(p_window_id), false);

	WindowData &wd = windows[p_window_id];

	AccessibilityElement *ae = memnew(AccessibilityElement);
	ae->role = AccessibilityServerEnums::ROLE_WINDOW;
	ae->window_id = p_window_id;
	wd.root_id = rid_owner.make_rid(ae);

#ifdef TOOLS_ENABLED
	if (EngineDebugger::get_singleton() || debug_cb.is_valid()) {
		Array arr;
		arr.push_back(p_window_id);
		arr.push_back(wd.root_id);
		if (EngineDebugger::get_singleton()) {
			EngineDebugger::get_singleton()->send_message("accessibility:activate_window", arr);
		}
		if (debug_cb.is_valid()) {
			debug_cb.call(DebugCallbackEvent::DEBUG_CB_WINDOW_ACTIVATE, arr);
		}
	}
#endif

	return true;
}

void AccessibilityServerMock::window_destroy(DisplayServerEnums::WindowID p_window_id) {
	WindowData *wd = windows.getptr(p_window_id);
	ERR_FAIL_NULL(wd);

#ifdef TOOLS_ENABLED
	if (EngineDebugger::get_singleton() || debug_cb.is_valid()) {
		Array arr;
		arr.push_back(p_window_id);
		arr.push_back(wd->root_id);
		if (EngineDebugger::get_singleton()) {
			EngineDebugger::get_singleton()->send_message("accessibility:deactivate_window", arr);
		}
		if (debug_cb.is_valid()) {
			debug_cb.call(DebugCallbackEvent::DEBUG_CB_WINDOW_DEACTIVATE, arr);
		}
	}
#endif

	free_element(wd->root_id);

	windows.erase(p_window_id);
}

RID AccessibilityServerMock::create_element(DisplayServerEnums::WindowID p_window_id, AccessibilityServerEnums::AccessibilityRole p_role) {
	AccessibilityElement *ae = memnew(AccessibilityElement);
	ae->role = p_role;
	ae->window_id = p_window_id;
	RID rid = rid_owner.make_rid(ae);

	return rid;
}

RID AccessibilityServerMock::create_sub_element(const RID &p_parent_rid, AccessibilityServerEnums::AccessibilityRole p_role, int p_insert_pos) {
	AccessibilityElement *parent_ae = rid_owner.get_or_null(p_parent_rid);
	ERR_FAIL_NULL_V(parent_ae, RID());

	WindowData *wd = windows.getptr(parent_ae->window_id);
	ERR_FAIL_NULL_V(wd, RID());

	AccessibilityElement *ae = memnew(AccessibilityElement);
	ae->role = p_role;
	ae->window_id = parent_ae->window_id;
	ae->parent = p_parent_rid;
	ae->owned_by_parent = true;
	RID rid = rid_owner.make_rid(ae);
	if (p_insert_pos == -1) {
		parent_ae->children.push_back(rid);
	} else {
		parent_ae->children.insert(p_insert_pos, rid);
	}
	wd->update.insert(rid);

	return rid;
}

RID AccessibilityServerMock::create_sub_text_edit_elements(const RID &p_parent_rid, const RID &p_shaped_text, float p_min_height, int p_insert_pos, bool p_is_last_line) {
	AccessibilityElement *parent_ae = rid_owner.get_or_null(p_parent_rid);
	ERR_FAIL_NULL_V(parent_ae, RID());

	WindowData *wd = windows.getptr(parent_ae->window_id);
	ERR_FAIL_NULL_V(wd, RID());

	AccessibilityElement *root_ae = memnew(AccessibilityElement);
	root_ae->role = AccessibilityServerEnums::ROLE_CONTAINER;
	root_ae->window_id = parent_ae->window_id;
	root_ae->parent = p_parent_rid;
	root_ae->owned_by_parent = true;
	RID root_rid = rid_owner.make_rid(root_ae);
	if (p_insert_pos == -1) {
		parent_ae->children.push_back(root_rid);
	} else {
		parent_ae->children.insert(p_insert_pos, root_rid);
	}
	wd->update.insert(root_rid);

	int64_t run_count = 0; // Note: runs in visual order.
	Vector2i full_range;

	if (p_shaped_text.is_valid()) {
		run_count = TS->shaped_get_run_count(p_shaped_text);
		full_range = TS->shaped_text_get_range(p_shaped_text);
	}

	// Create text element for each run.
	Vector<AccessibilityElement *> text_elements;
	for (int64_t i = 0; i < run_count; i++) {
		const Vector2i range = TS->shaped_get_run_range(p_shaped_text, i);
		String t = TS->shaped_get_run_text(p_shaped_text, i);

		if (t.is_empty()) {
			continue;
		}

		AccessibilityElement *ae = memnew(AccessibilityElement);
		ae->role = AccessibilityServerEnums::ROLE_TEXT_RUN;
		ae->window_id = parent_ae->window_id;
		ae->parent = root_rid;
		ae->owned_by_parent = true;
		ae->run = Vector3i(range.x, range.y, i);

		text_elements.push_back(ae);
	}
	if (!p_is_last_line || text_elements.is_empty()) {
		// Add "\n" at the end.
		AccessibilityElement *ae = memnew(AccessibilityElement);
		ae->role = AccessibilityServerEnums::ROLE_TEXT_RUN;
		ae->window_id = parent_ae->window_id;
		ae->parent = root_rid;
		ae->owned_by_parent = true;
		ae->run = Vector3i(full_range.y, full_range.y, run_count);

		text_elements.push_back(ae);
	}

	// Sort runs in logical order.
	struct RunCompare {
		_FORCE_INLINE_ bool operator()(const AccessibilityElement *l, const AccessibilityElement *r) const {
			return l->run.x < r->run.x;
		}
	};
	text_elements.sort_custom<RunCompare>();
	for (int i = 0; i < text_elements.size(); i++) {
		RID rid = rid_owner.make_rid(text_elements[i]);
		root_ae->children.push_back(rid);
		wd->update.insert(rid);
	}

	return root_rid;
}

bool AccessibilityServerMock::has_element(const RID &p_id) const {
	return rid_owner.owns(p_id);
}

void AccessibilityServerMock::_free_recursive(WindowData *p_wd, const RID &p_id) {
	if (p_wd && p_wd->update.has(p_id)) {
		p_wd->update.erase(p_id);
	}
	AccessibilityElement *ae = rid_owner.get_or_null(p_id);
	for (const RID &rid : ae->children) {
		_free_recursive(p_wd, rid);
	}
	memdelete(ae);
	rid_owner.free(p_id);
}

void AccessibilityServerMock::free_element(const RID &p_id) {
	ERR_FAIL_COND_MSG(in_accessibility_update, "Element can't be removed inside NOTIFICATION_ACCESSIBILITY_UPDATE notification.");

	AccessibilityElement *ae = rid_owner.get_or_null(p_id);
	if (ae) {
		WindowData *wd = windows.getptr(ae->window_id);
		AccessibilityElement *parent_ae = rid_owner.get_or_null(ae->parent);
		if (ae->owned_by_parent && parent_ae) {
			parent_ae->children.erase(p_id);
		}
		_free_recursive(wd, p_id);
	}
}

void AccessibilityServerMock::element_set_meta(const RID &p_id, const Variant &p_meta) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");

	AccessibilityElement *ae = rid_owner.get_or_null(p_id);
	ERR_FAIL_NULL(ae);
	ae->meta = p_meta;
}

Variant AccessibilityServerMock::element_get_meta(const RID &p_id) const {
	const AccessibilityElement *ae = rid_owner.get_or_null(p_id);
	ERR_FAIL_NULL_V(ae, Variant());
	return ae->meta;
}

void AccessibilityServerMock::update_set_focus(const RID &p_id) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");

	if (p_id.is_valid() && rid_owner.owns(p_id)) {
		focus = p_id;
	} else {
		focus = RID();
	}
}

RID AccessibilityServerMock::get_window_root(DisplayServerEnums::WindowID p_window_id) const {
	const WindowData *wd = windows.getptr(p_window_id);
	ERR_FAIL_NULL_V(wd, RID());

	return wd->root_id;
}

void AccessibilityServerMock::update_if_active(const Callable &p_callable) {
	ERR_FAIL_COND(!p_callable.is_valid());
	update_cb = p_callable;
	for (KeyValue<DisplayServerEnums::WindowID, WindowData> &window : windows) {
		WindowData &wd = window.value;
		in_accessibility_update = true;
		if (update_cb.is_valid()) {
			update_cb.call(window.key);
		}
		in_accessibility_update = false;

#ifdef TOOLS_ENABLED
		bool dbg_en = EngineDebugger::get_singleton() || debug_cb.is_valid();
		Array arr;
		if (dbg_en) {
			arr.push_back(window.key);
			arr.push_back(focus);
		}
#endif
		for (const RID &rid : wd.update) {
			AccessibilityElement *ae = rid_owner.get_or_null(rid);
			if (ae) {
#ifdef TOOLS_ENABLED
				if (dbg_en) {
					arr.push_back(rid);
				}
#endif
			}
		}
		wd.update.clear();
#ifdef TOOLS_ENABLED
		if (dbg_en) {
			if (EngineDebugger::get_singleton()) {
				EngineDebugger::get_singleton()->send_message("accessibility:update_window", arr);
			}
			if (debug_cb.is_valid()) {
				debug_cb.call(DebugCallbackEvent::DEBUG_CB_WINDOW_TREE_UPDATE, arr);
			}
		}
#endif
	}
	update_cb = Callable();
}

void AccessibilityServerMock::set_window_rect(DisplayServerEnums::WindowID p_window_id, const Rect2 &p_rect_out, const Rect2 &p_rect_in) {}

void AccessibilityServerMock::set_window_focused(DisplayServerEnums::WindowID p_window_id, bool p_focused) {}

void AccessibilityServerMock::update_set_role(const RID &p_id, AccessibilityServerEnums::AccessibilityRole p_role) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");

	AccessibilityElement *ae = rid_owner.get_or_null(p_id);
	ERR_FAIL_NULL(ae);
	if (ae->role == p_role) {
		return;
	}
	ae->role = p_role;
}

void AccessibilityServerMock::update_set_name(const RID &p_id, const String &p_name) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");

	AccessibilityElement *ae = rid_owner.get_or_null(p_id);
	ERR_FAIL_NULL(ae);

	ae->name = p_name;
}

void AccessibilityServerMock::update_set_extra_info(const RID &p_id, const String &p_name_extra_info) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");

	AccessibilityElement *ae = rid_owner.get_or_null(p_id);
	ERR_FAIL_NULL(ae);

	ae->name_extra_info = p_name_extra_info;
}

void AccessibilityServerMock::update_set_description(const RID &p_id, const String &p_description) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_value(const RID &p_id, const String &p_value) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");

	AccessibilityElement *ae = rid_owner.get_or_null(p_id);
	ERR_FAIL_NULL(ae);

	ae->value = p_value;
}

void AccessibilityServerMock::update_set_tooltip(const RID &p_id, const String &p_tooltip) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_bounds(const RID &p_id, const Rect2 &p_rect) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_transform(const RID &p_id, const Transform2D &p_transform) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_clear_children(const RID &p_id) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");

	AccessibilityElement *ae = rid_owner.get_or_null(p_id);
	ERR_FAIL_NULL(ae);
	ae->indirect_children.clear();
}

void AccessibilityServerMock::update_add_child(const RID &p_id, const RID &p_child_id) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");

	AccessibilityElement *ae = rid_owner.get_or_null(p_id);
	ERR_FAIL_NULL(ae);
	AccessibilityElement *other_ae = rid_owner.get_or_null(p_child_id);
	ERR_FAIL_NULL(other_ae);
	ERR_FAIL_COND(other_ae->window_id != ae->window_id);
	other_ae->parent = p_id;
	ae->indirect_children.push_back(p_child_id);
}

void AccessibilityServerMock::update_add_related_controls(const RID &p_id, const RID &p_related_id) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_add_related_details(const RID &p_id, const RID &p_related_id) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_add_related_described_by(const RID &p_id, const RID &p_related_id) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_add_related_flow_to(const RID &p_id, const RID &p_related_id) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_add_related_labeled_by(const RID &p_id, const RID &p_related_id) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_add_related_radio_group(const RID &p_id, const RID &p_related_id) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_active_descendant(const RID &p_id, const RID &p_other_id) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_next_on_line(const RID &p_id, const RID &p_other_id) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_previous_on_line(const RID &p_id, const RID &p_other_id) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_member_of(const RID &p_id, const RID &p_group_id) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_in_page_link_target(const RID &p_id, const RID &p_other_id) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_error_message(const RID &p_id, const RID &p_other_id) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_live(const RID &p_id, AccessibilityServerEnums::AccessibilityLiveMode p_live) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_add_action(const RID &p_id, AccessibilityServerEnums::AccessibilityAction p_action, const Callable &p_callable) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");

	AccessibilityElement *ae = rid_owner.get_or_null(p_id);
	ERR_FAIL_NULL(ae);

	ae->actions[p_action] = p_callable;
}

void AccessibilityServerMock::update_add_custom_action(const RID &p_id, int p_action_id, const String &p_action_description) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_table_row_count(const RID &p_id, int p_count) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_table_column_count(const RID &p_id, int p_count) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_table_row_index(const RID &p_id, int p_index) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_table_column_index(const RID &p_id, int p_index) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_table_cell_position(const RID &p_id, int p_row_index, int p_column_index) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_table_cell_span(const RID &p_id, int p_row_span, int p_column_span) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_list_item_count(const RID &p_id, int p_size) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_list_item_index(const RID &p_id, int p_index) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_list_item_level(const RID &p_id, int p_level) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_list_item_selected(const RID &p_id, bool p_selected) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_list_item_expanded(const RID &p_id, bool p_expanded) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_popup_type(const RID &p_id, AccessibilityServerEnums::AccessibilityPopupType p_popup) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_checked(const RID &p_id, bool p_checekd) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_num_value(const RID &p_id, double p_position) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");

	AccessibilityElement *ae = rid_owner.get_or_null(p_id);
	ERR_FAIL_NULL(ae);

	ae->value = p_position;
}

void AccessibilityServerMock::update_set_num_range(const RID &p_id, double p_min, double p_max) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_num_step(const RID &p_id, double p_step) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_num_jump(const RID &p_id, double p_jump) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_scroll_x(const RID &p_id, double p_position) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_scroll_x_range(const RID &p_id, double p_min, double p_max) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_scroll_y(const RID &p_id, double p_position) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_scroll_y_range(const RID &p_id, double p_min, double p_max) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_text_decorations(const RID &p_id, bool p_underline, bool p_strikethrough, bool p_overline, const Color &p_color) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_text_align(const RID &p_id, HorizontalAlignment p_align) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_text_selection(const RID &p_id, const RID &p_text_start_id, int p_start_char, const RID &p_text_end_id, int p_end_char) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_flag(const RID &p_id, AccessibilityServerEnums::AccessibilityFlags p_flag, bool p_value) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");

	AccessibilityElement *ae = rid_owner.get_or_null(p_id);
	ERR_FAIL_NULL(ae);

	if (p_value) {
		ae->flags |= (1 << p_flag);
	} else {
		ae->flags &= ~(1 << p_flag);
	}
}

void AccessibilityServerMock::update_set_classname(const RID &p_id, const String &p_classname) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_placeholder(const RID &p_id, const String &p_placeholder) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_language(const RID &p_id, const String &p_language) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_text_orientation(const RID &p_id, bool p_vertical) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_list_orientation(const RID &p_id, bool p_vertical) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_shortcut(const RID &p_id, const String &p_shortcut) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_url(const RID &p_id, const String &p_url) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_role_description(const RID &p_id, const String &p_description) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_state_description(const RID &p_id, const String &p_description) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_color_value(const RID &p_id, const Color &p_color) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");

	AccessibilityElement *ae = rid_owner.get_or_null(p_id);
	ERR_FAIL_NULL(ae);

	ae->value = p_color;
}

void AccessibilityServerMock::update_set_background_color(const RID &p_id, const Color &p_color) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

void AccessibilityServerMock::update_set_foreground_color(const RID &p_id, const Color &p_color) {
	ERR_FAIL_COND_MSG(!in_accessibility_update, "Accessibility updates are only allowed inside the NOTIFICATION_ACCESSIBILITY_UPDATE notification.");
}

AccessibilityServerMock::AccessibilityServerMock() {
#ifdef TOOLS_ENABLED
	EngineDebugger::register_message_capture("accessibility", EngineDebugger::Capture(nullptr, AccessibilityServerMock::parse_message));
#endif // DEBUG_ENABLED
}

AccessibilityServerMock::~AccessibilityServerMock() {}

#endif
