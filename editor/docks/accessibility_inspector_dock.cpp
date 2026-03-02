/**************************************************************************/
/*  accessibility_inspector_dock.cpp                                      */
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

#include "accessibility_inspector_dock.h"

#include "core/object/callable_mp.h"
#include "editor/debugger/editor_debugger_node.h"
#include "editor/debugger/script_editor_debugger.h"
#include "editor/inspector/editor_inspector.h"
#include "editor/run/editor_run_bar.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/box_container.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/option_button.h"
#include "scene/gui/split_container.h"
#include "scene/gui/tree.h"
#include "servers/display/accessibility_server.h"

class ItemInfo : public RefCounted {
	GDCLASS(ItemInfo, RefCounted);

public:
	RID rid;
	String name;
	String extra_info;
	AccessibilityServerEnums::AccessibilityRole role = AccessibilityServerEnums::ROLE_UNKNOWN;
	Variant value;
	String meta;
	uint64_t flags = 0;
	String debug_info;

protected:
	bool _set(const StringName &p_name, const Variant &p_value) {
		return false;
	}
	bool _get(const StringName &p_name, Variant &r_ret) const {
		if (p_name == "name") {
			r_ret = name;
			return true;
		} else if (p_name == "extra_info") {
			r_ret = extra_info;
			return true;
		} else if (p_name == "role") {
			r_ret = role;
			return true;
		} else if (p_name == "value") {
			r_ret = value;
			return true;
		} else if (p_name == "meta") {
			r_ret = meta;
			return true;
		} else if (p_name == "flags") {
			r_ret = flags;
			return true;
		} else if (p_name == "driver_debug_info") {
			r_ret = debug_info;
			return true;
		}
		return false;
	}
	void _get_property_list(List<PropertyInfo> *p_list) const {
		p_list->push_back(PropertyInfo(Variant::STRING, "name", PROPERTY_HINT_NONE, ""));
		p_list->push_back(PropertyInfo(Variant::STRING, "extra_info", PROPERTY_HINT_NONE, ""));
		p_list->push_back(PropertyInfo(Variant::INT, "role", PROPERTY_HINT_ENUM, "Unknown,Default Button,Audio,Video,Static Text,Container,Panel,Button,Link,Check Box,Radio Button,Check Button,Scroll Bar,Scroll View,Splitter,Slider,Spin Button,Progress Indicator,Text Field,Multiline Text Field,Color Picker,Table,Cell,Row,Row Group,Row Header,Column Header,Tree,Tree Item,List,List Item,List Box,List Box Option,Tab Bar,Tab,Tab Panel,Menu Bar,Menu,Menu Item,Menu Item Check Box,Menu Item Radio,Image,Window,Title Bar,Dialog,Tooltip,Region,Text Run"));
		p_list->push_back(PropertyInfo(Variant::NIL, "value", PROPERTY_HINT_NONE, ""));
		p_list->push_back(PropertyInfo(Variant::STRING, "meta", PROPERTY_HINT_NONE, ""));
		p_list->push_back(PropertyInfo(Variant::INT, "flags", PROPERTY_HINT_FLAGS, "Hidden,Multiselectable,Required,Visited,Busy,Modal,Touch Passthrough,Readonly,Disabled,Clips Children"));
		p_list->push_back(PropertyInfo(Variant::STRING, "driver_debug_info", PROPERTY_HINT_MULTILINE_TEXT, ""));
	}
};

class ActionInfo : public RefCounted {
	GDCLASS(ActionInfo, RefCounted);

public:
	Variant action_data;
	mutable String data_name = "nop";
	AccessibilityServerEnums::AccessibilityAction action = AccessibilityServerEnums::ACTION_CLICK;

protected:
	bool _set(const StringName &p_name, const Variant &p_value) {
		if (p_name == data_name) {
			action_data = p_value;
			return true;
		}
		return false;
	}
	bool _get(const StringName &p_name, Variant &r_ret) const {
		if (p_name == data_name) {
			r_ret = action_data;
			return true;
		}
		return false;
	}
	void _get_property_list(List<PropertyInfo> *p_list) const {
		switch (action) {
			case AccessibilityServerEnums::AccessibilityAction::ACTION_CLICK:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_FOCUS:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_BLUR:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_COLLAPSE:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_EXPAND:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_DECREMENT:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_INCREMENT:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_HIDE_TOOLTIP:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_SHOW_TOOLTIP:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_SHOW_CONTEXT_MENU:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_INTO_VIEW: {
				// No data used by action.
				data_name = "nop";
			} break;
			case AccessibilityServerEnums::AccessibilityAction::ACTION_SET_TEXT_SELECTION:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_REPLACE_SELECTED_TEXT: {
				// Not supported.
				data_name = "nop";
			} break;
			case AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_BACKWARD:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_DOWN:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_FORWARD:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_LEFT:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_RIGHT:
			case AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_UP: {
				data_name = "scroll_unit";
				p_list->push_back(PropertyInfo(Variant::INT, "scroll_unit", PROPERTY_HINT_ENUM, "Item,Page"));
			} break;
			case AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_TO_POINT: {
				data_name = "point";
				p_list->push_back(PropertyInfo(Variant::VECTOR2, "point", PROPERTY_HINT_NONE, ""));
			} break;
			case AccessibilityServerEnums::AccessibilityAction::ACTION_SET_SCROLL_OFFSET: {
				data_name = "offset";
				p_list->push_back(PropertyInfo(Variant::VECTOR2, "offset", PROPERTY_HINT_NONE, ""));
			} break;
			case AccessibilityServerEnums::AccessibilityAction::ACTION_SET_VALUE: {
				data_name = "value";
				p_list->push_back(PropertyInfo(Variant::NIL, "value", PROPERTY_HINT_NONE, ""));
			} break;
			case AccessibilityServerEnums::AccessibilityAction::ACTION_CUSTOM: {
				data_name = "custom_action_id";
				p_list->push_back(PropertyInfo(Variant::INT, "custom_action_id", PROPERTY_HINT_NONE, ""));
			} break;
		}
	}
};

bool AccessibilityDebuggerPlugin::has_capture(const String &p_capture) const {
	return p_capture == "accessibility";
}

bool AccessibilityDebuggerPlugin::capture(const String &p_message, const Array &p_data, int p_index) {
	if (p_message == "accessibility:window_list") {
		AccessibilityInspectorDock::get_singleton()->_initial_update(p_data);
		return true;
	} else if (p_message == "accessibility:ae_info") {
		ERR_FAIL_COND_V(p_data.size() != 13, false);
		if (!AccessibilityInspectorDock::get_singleton()->_update_or_add_node(p_data[0], p_data)) {
			AccessibilityInspectorDock::get_singleton()->pending_update.push_back(p_data[0]);
		}
		return true;
	} else if (p_message == "accessibility:state") {
		ERR_FAIL_COND_V(p_data.size() != 5, false);
		AccessibilityInspectorDock::get_singleton()->st_contrast->set_disabled(!p_data[1]);
		AccessibilityInspectorDock::get_singleton()->st_anim->set_disabled(!p_data[2]);
		AccessibilityInspectorDock::get_singleton()->st_trans->set_disabled(!p_data[3]);
		AccessibilityInspectorDock::get_singleton()->st_active->set_disabled(!p_data[4]);
		return true;
	} else if (p_message == "accessibility:activate_window") {
		ERR_FAIL_COND_V(p_data.size() != 2, false);
		AccessibilityInspectorDock::get_singleton()->_debug_callback(AccessibilityServer::DEBUG_CB_WINDOW_ACTIVATE, p_data);
		return true;
	} else if (p_message == "accessibility:deactivate_window") {
		ERR_FAIL_COND_V(p_data.size() != 2, false);
		AccessibilityInspectorDock::get_singleton()->_debug_callback(AccessibilityServer::DEBUG_CB_WINDOW_DEACTIVATE, p_data);
		return true;
	} else if (p_message == "accessibility:update_window") {
		ERR_FAIL_COND_V(p_data.size() < 2, false);
		AccessibilityInspectorDock::get_singleton()->_debug_callback(AccessibilityServer::DEBUG_CB_WINDOW_TREE_UPDATE, p_data);
		return true;
	}
	return false;
}

void AccessibilityInspectorDock::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_TRANSLATION_CHANGED:
		case NOTIFICATION_LAYOUT_DIRECTION_CHANGED:
		case NOTIFICATION_THEME_CHANGED: {
			capture_button->set_button_icon(get_editor_theme_icon(SNAME("Debug")));
			st_contrast->set_button_icon(get_editor_theme_icon(SNAME("HighContrast")));
			st_anim->set_button_icon(get_editor_theme_icon(SNAME("NoAnimation")));
			st_trans->set_button_icon(get_editor_theme_icon(SNAME("NoTransparency")));
			st_active->set_button_icon(get_editor_theme_icon(SNAME("Accessibility")));
			run_action->set_button_icon(get_editor_theme_icon(SNAME("Play")));
			next_search->set_button_icon(get_editor_theme_icon(SNAME("MoveDown")));
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			if (!running) {
				break;
			}

			// Trigger update.
			List<RID> skipped;
			for (const RID &E : pending_update) {
				if (E == get_accessibility_element() || AccessibilityServer::get_singleton()->is_ancestor_of(E, get_accessibility_element())) {
					continue;
				}
				if (local) {
					Array node_info = AccessibilityServer::get_singleton()->debug_get_node_info(E);
					if (node_info.size() != 1 && !_update_or_add_node(E, node_info)) {
						skipped.push_back(E);
					}
				} else {
					_request_node_info(E);
				}
			}
			pending_update.clear();
			for (const RID &E : skipped) {
				if (nodes.has(E)) {
					pending_update.push_back(E);
				}
			}
			if (local) {
				_local_update_pending();
			} else {
				EditorDebuggerNode::get_singleton()->get_current_debugger()->send_message("accessibility:rq_state", Array());
			}

			// Fade timers.
			if (!is_visible_in_tree()) {
				break;
			}
			for (List<ItemFadeTimer>::Element *E = timers.front(); E;) {
				double delta = get_process_delta_time();
				List<ItemFadeTimer>::Element *N = E->next();
				ItemFadeTimer &item = E->get();
				TreeItem *tree_item = ObjectDB::get_instance<TreeItem>(item.item);
				if (!tree_item) {
					timers.erase(E);
				} else {
					item.remaining -= delta;
					if (item.remaining <= 0.0) {
						tree_item->clear_custom_bg_color(0);
						tree_item->clear_custom_bg_color(1);
						if (tree_item->get_metadata(0) == Variant()) {
							memdelete(tree_item);
						}
						timers.erase(E);
					} else {
						tree_item->set_custom_bg_color(0, Color(item.color.r, item.color.g, item.color.b, item.remaining / item.max));
						tree_item->set_custom_bg_color(1, Color(item.color.r, item.color.g, item.color.b, item.remaining / item.max));
					}
				}
				E = N;
			}
		} break;
	}
}

String AccessibilityInspectorDock::_get_action_name(AccessibilityServerEnums::AccessibilityAction p_action) const {
	if (action_names.has(p_action)) {
		return action_names[p_action];
	}
	return vformat(TTR("Invalid Action %d"), p_action);
}

String AccessibilityInspectorDock::_get_role_name(AccessibilityServerEnums::AccessibilityRole p_role) const {
	if (role_names.has(p_role)) {
		return role_names[p_role];
	}
	return vformat(TTR("Invalid Role %d"), p_role);
}

String AccessibilityInspectorDock::_get_role_icon(AccessibilityServerEnums::AccessibilityRole p_role) const {
	if (role_icons.has(p_role)) {
		return role_icons[p_role];
	}
	return String();
}

void AccessibilityInspectorDock::_search_next() {
	_search_text_changed(search->get_text(), true);
}

void AccessibilityInspectorDock::_search_text_changed(const String &p_text, bool p_next) {
	if (!p_next) {
		current_search_result = RID();
	}
	if (p_text.is_empty()) {
		return;
	}
	bool skip = p_next;
	for (const KeyValue<RID, TreeItem *> &E : nodes) {
		if (E.key == current_search_result) {
			skip = false;
			continue;
		}
		if (skip) {
			continue;
		}
		if (Variant(E.key).stringify().begins_with("RID(" + p_text)) {
			E.value->select(0);
			tree->scroll_to_item(E.value, true);
			current_search_result = E.key;
			return;
		}
		Array meta = E.value->get_metadata(0);
		if (meta.size() == 13) {
			if (meta[5].operator String().contains(p_text)) {
				E.value->select(1);
				tree->scroll_to_item(E.value, true);
				current_search_result = E.key;
				return;
			}
		}
	}
	if (p_next) {
		_search_text_changed(p_text, false);
	}
}

void AccessibilityInspectorDock::_remove_item(TreeItem *p_item) {
	if (p_item == nullptr || p_item->get_metadata(0) == Variant()) {
		return;
	}
	for (List<ItemFadeTimer>::Element *E = timers.front(); E;) {
		List<ItemFadeTimer>::Element *N = E->next();
		if (E->get().item == p_item->get_instance_id()) {
			timers.erase(E);
		}
		E = N;
	}
	Array old_meta = p_item->get_metadata(0);
	ERR_FAIL_COND(old_meta.size() != 13);
	if (current_search_result == old_meta[0]) {
		_search_text_changed(search->get_text(), false);
	}
	TypedArray<RID> old_children = old_meta[3];
	for (int i = 0; i < old_children.size(); i++) {
		const RID &c = old_children[i];
		ERR_FAIL_COND(!nodes.has(c) && !pending_update.find(c));
		if (nodes.has(c)) {
			_remove_item(nodes[c]);
			nodes.erase(c);
		}
		pending_update.erase(c);
	}

	p_item->set_metadata(0, Variant());
	ItemFadeTimer ft = { p_item->get_instance_id(), 1, 1, Color(1, 0, 0) };
	timers.push_back(ft);
}

void AccessibilityInspectorDock::_request_node_info(const RID &p_rid) {
	if (local) {
		pending_update.push_back(p_rid);
	} else {
		Array rq_data = { p_rid };
		EditorDebuggerNode::get_singleton()->get_current_debugger()->send_message("accessibility:rq_node_info", rq_data);
	}
}

void AccessibilityInspectorDock::_initial_update(const Array &p_windows) {
	for (int i = 0; i < p_windows.size() / 2; i++) {
		_initial_window_update(p_windows[i * 2], p_windows[i * 2 + 1]);
	}
}

void AccessibilityInspectorDock::_initial_window_update(DisplayServerEnums::WindowID p_id, const RID &p_root_node) {
	TreeItem *t = tree->create_item(root);

	t->set_text(0, Variant(p_root_node).stringify());
	t->set_text(1, vformat(TTR("Window %d"), p_id));
	t->set_text(2, _get_role_name(AccessibilityServerEnums::ROLE_WINDOW));
	t->set_icon(0, get_editor_theme_icon(_get_role_icon(AccessibilityServerEnums::ROLE_WINDOW)));
	Array node_info = { p_root_node, AccessibilityServerEnums::ROLE_WINDOW, RID(), TypedArray<RID>(), Variant(), String(), String(), TypedArray<AccessibilityServerEnums::AccessibilityAction>(), Vector3i(), Variant(), 0, String(), p_id };
	t->set_metadata(0, node_info);
	nodes[p_root_node] = t;
	_request_node_info(p_root_node);
}

bool AccessibilityInspectorDock::_update_or_add_node(const RID &p_rid, const Array &p_node_info) {
	ERR_FAIL_COND_V(p_rid.is_null(), false);
	ERR_FAIL_COND_V(p_node_info.size() != 13, false);
	ERR_FAIL_COND_V(p_node_info[0] != p_rid, false);

	if (nodes.has(p_rid)) {
		TreeItem *t = nodes[p_rid];

		Array old_meta = t->get_metadata(0);
		ERR_FAIL_COND_V(old_meta.size() != 13, false);

		TypedArray<RID> old_children = old_meta[3];
		TypedArray<RID> new_children = p_node_info[3];
		for (int i = 0; i < old_children.size(); i++) {
			const RID &c = old_children[i];
			if (!new_children.has(c)) {
				ERR_FAIL_COND_V(!nodes.has(c) && !pending_update.find(c), false);
				if (nodes.has(c)) {
					_remove_item(nodes[c]);
					nodes.erase(c);
				}
				pending_update.erase(c);
			}
		}
		for (int i = 0; i < new_children.size(); i++) {
			_request_node_info(new_children[i]);
		}

		t->set_text(0, Variant(p_rid).stringify());
		t->set_text(1, p_node_info[5]);
		t->set_text(2, _get_role_name(p_node_info[1]));
		t->set_icon(0, get_editor_theme_icon(_get_role_icon(p_node_info[1])));
		for (const KeyValue<DisplayServerEnums::WindowID, RID> &sel : focused_node) {
			if (sel.value == p_rid) {
				t->set_icon(3, get_editor_theme_icon("ListSelect"));
				break;
			}
		}
		t->set_metadata(0, p_node_info);
	} else {
		RID parent_rid = p_node_info[2];
		if (!nodes.has(parent_rid)) {
			return false;
		}
		TreeItem *p = nodes[parent_rid];
		TreeItem *t = tree->create_item(p);

		TypedArray<RID> new_children = p_node_info[3];
		for (int i = 0; i < new_children.size(); i++) {
			_request_node_info(new_children[i]);
		}

		t->set_text(0, Variant(p_rid).stringify());
		t->set_text(1, p_node_info[5]);
		t->set_text(2, _get_role_name(p_node_info[1]));
		t->set_icon(0, get_editor_theme_icon(_get_role_icon(p_node_info[1])));
		for (const KeyValue<DisplayServerEnums::WindowID, RID> &sel : focused_node) {
			if (sel.value == p_rid) {
				t->set_icon(3, get_editor_theme_icon("ListSelect"));
				break;
			}
		}
		t->set_metadata(0, p_node_info);

		nodes[p_rid] = t;

		ItemFadeTimer ft = { t->get_instance_id(), 1, 1, Color(0, 1, 0) };
		timers.push_back(ft);
	}
	return true;
}

void AccessibilityInspectorDock::_local_update_pending() {
	st_contrast->set_disabled(!DisplayServer::get_singleton()->accessibility_should_increase_contrast());
	st_anim->set_disabled(!DisplayServer::get_singleton()->accessibility_should_reduce_animation());
	st_trans->set_disabled(!DisplayServer::get_singleton()->accessibility_should_reduce_transparency());
	st_active->set_disabled(!DisplayServer::get_singleton()->accessibility_screen_reader_active());
}

void AccessibilityInspectorDock::_debug_callback(int p_event, const Array &p_data) {
	switch ((AccessibilityServer::DebugCallbackEvent)p_event) {
		case AccessibilityServer::DEBUG_CB_WINDOW_ACTIVATE: {
			ERR_FAIL_COND(p_data.size() != 2);
			DisplayServerEnums::WindowID wid = p_data[0];
			RID rid = p_data[1];
			focused_node[wid] = rid;
			_initial_window_update(wid, rid);
		} break;
		case AccessibilityServer::DEBUG_CB_WINDOW_DEACTIVATE: {
			ERR_FAIL_COND(p_data.size() != 2);
			DisplayServerEnums::WindowID wid = p_data[0];
			if (nodes.has(focused_node[wid])) {
				nodes[focused_node[wid]]->set_icon(2, Ref<Texture2D>());
			}
			focused_node.erase(wid);

			RID c = p_data[1];
			ERR_FAIL_COND(!nodes.has(c) && !pending_update.find(c));
			if (nodes.has(c)) {
				_remove_item(nodes[c]);
				nodes.erase(c);
			}
			pending_update.erase(c);
		} break;
		case AccessibilityServer::DEBUG_CB_WINDOW_TREE_UPDATE: {
			ERR_FAIL_COND(p_data.size() < 2);
			DisplayServerEnums::WindowID wid = p_data[0];
			RID focus = p_data[1];
			if (nodes.has(focused_node[wid])) {
				nodes[focused_node[wid]]->set_icon(2, Ref<Texture2D>());
			}
			focused_node[wid] = focus;
			if (nodes.has(focused_node[wid])) {
				nodes[focused_node[wid]]->set_icon(2, get_editor_theme_icon("ListSelect"));
			}
			for (int i = 2; i < p_data.size(); i++) {
				_request_node_info(p_data[i]);
			}
		} break;
	}
}

void AccessibilityInspectorDock::_toggle_capture() {
	if (running) {
		running = false;
		debugger = Ref<AccessibilityDebuggerPlugin>();
		AccessibilityServer::get_singleton()->debug_set_update_callback(Callable());

		timers.clear();
		nodes.clear();
		pending_update.clear();
		focused_node.clear();
		tree->clear();
		capture_button->set_text(TTRC("Start Monitoring"));
		target->set_disabled(false);
	} else {
		running = true;
		capture_button->set_text(TTRC("Stop Monitoring"));
		target->set_disabled(true);
		root = tree->create_item(); // Add root node.

		if (target->get_selected() == 0) {
			// Project capture.
			local = false;
			debugger.instantiate();
			EditorDebuggerNode::get_singleton()->get_current_debugger()->send_message("accessibility:rq_window_list", Array());
		} else {
			// Editor capture.
			local = true;
			AccessibilityServer::get_singleton()->debug_set_update_callback(callable_mp(this, &AccessibilityInspectorDock::_debug_callback));
			_initial_update(AccessibilityServer::get_singleton()->debug_get_window_list());
		}
	}
}

void AccessibilityInspectorDock::_action_selected(int p_action) {
	AccessibilityServerEnums::AccessibilityAction act = actions->get_item_metadata(p_action);
	action_info->edit(nullptr);
	action->action = act;
	action->action_data = Variant();
	action_info->edit(action.ptr());
	action->notify_property_list_changed();

	if (act != AccessibilityServerEnums::AccessibilityAction::ACTION_SET_TEXT_SELECTION && act != AccessibilityServerEnums::AccessibilityAction::ACTION_REPLACE_SELECTED_TEXT) {
		run_action->set_disabled(false);
	}
}

void AccessibilityInspectorDock::_run_selected_action() {
	TreeItem *item = tree->get_selected();
	if (!item) {
		return;
	}
	const Array &meta = item->get_metadata(0);
	ERR_FAIL_COND(meta.size() != 13);
	_run_action(meta[0], action->action, action->action_data);
}

void AccessibilityInspectorDock::_run_action(const RID &p_rid, AccessibilityServerEnums::AccessibilityAction p_action, const Variant &p_data) {
	if (!running) {
		return;
	}
	if (local) {
		AccessibilityServer::get_singleton()->debug_trigger_action(p_rid, p_action, p_data);
	} else {
		Array act_data = { p_rid, p_action, p_data };
		EditorDebuggerNode::get_singleton()->get_current_debugger()->send_message("accessibility:rq_action", act_data);
	}
}

void AccessibilityInspectorDock::_node_selected() {
	TreeItem *item = tree->get_selected();
	node_info->edit(nullptr);
	action_info->edit(nullptr);
	run_action->set_disabled(true);
	action_container->set_visible(false);
	if (item && item->get_metadata(0) != Variant()) {
		const Array &meta = item->get_metadata(0);
		ERR_FAIL_COND(meta.size() != 13);
		info->name = meta[5];
		info->extra_info = meta[6];
		info->role = meta[1];
		info->value = meta[9];
		info->meta = meta[4];
		info->flags = meta[10];
		info->debug_info = meta[11];
		info->rid = item->get_metadata(0);
		node_info->edit(info.ptr());
		info->notify_property_list_changed();
		actions->clear();
		TypedArray<AccessibilityServerEnums::AccessibilityAction> action_list = meta[7];
		for (int i = 0; i < action_list.size(); i++) {
			actions->add_item(_get_action_name(action_list[i]));
			actions->set_item_metadata(i, action_list[i]);
		}
		if (!action_list.is_empty()) {
			actions->select(0);
			_action_selected(0);
			action_container->set_visible(true);
		}
	} else {
		info->rid = RID();
		actions->clear();
	}
}

AccessibilityInspectorDock::AccessibilityInspectorDock() {
	singleton = this;
	set_name(TTRC("Accessibility"));
	set_icon_name("AccessibilityDock");
	set_default_slot(EditorDock::DOCK_SLOT_BOTTOM);
	set_available_layouts(EditorDock::DOCK_LAYOUT_HORIZONTAL | EditorDock::DOCK_LAYOUT_FLOATING);
	set_focus_mode(FOCUS_ALL);
	set_process_internal(true);

	info.instantiate();
	action.instantiate();

	HSplitContainer *base_container = memnew(HSplitContainer);
	base_container->set_h_size_flags(SIZE_EXPAND_FILL);
	base_container->set_v_size_flags(SIZE_EXPAND_FILL);
	add_child(base_container);

	VBoxContainer *tree_container = memnew(VBoxContainer);
	tree_container->set_h_size_flags(SIZE_EXPAND_FILL);
	tree_container->set_stretch_ratio(0.6);
	base_container->add_child(tree_container);

	HBoxContainer *tree_toolbar = memnew(HBoxContainer);
	tree_container->add_child(tree_toolbar);

	Label *target_lbl = memnew(Label);
	target_lbl->set_text(TTRC("Target tree:"));
	tree_toolbar->add_child(target_lbl);

	target = memnew(OptionButton);
	target->set_accessibility_name(TTRC("Source"));
	target->add_item(TTRC("Running Project"));
	target->add_item(TTRC("Editor"));
	target->set_theme_type_variation(SceneStringName(FlatButton));
	tree_toolbar->add_child(target);

	capture_button = memnew(Button);
	capture_button->set_theme_type_variation(SceneStringName(FlatButton));
	capture_button->set_text(TTRC("Start Monitoring"));
	capture_button->connect(SceneStringName(pressed), callable_mp(this, &AccessibilityInspectorDock::_toggle_capture));
	tree_toolbar->add_child(capture_button);

	tree_toolbar->add_spacer();

	search = memnew(LineEdit);
	search->set_placeholder(TTRC("Search (RID or Name)"));
	search->set_h_size_flags(SIZE_EXPAND_FILL);
	search->connect("text_changed", callable_mp(this, &AccessibilityInspectorDock::_search_text_changed).bind(false));
	tree_toolbar->add_child(search);

	next_search = memnew(Button);
	next_search->set_theme_type_variation(SceneStringName(FlatButton));
	next_search->set_tooltip_text(TTR("Next Match"));
	next_search->connect(SceneStringName(pressed), callable_mp(this, &AccessibilityInspectorDock::_search_next));
	tree_toolbar->add_child(next_search);

	tree = memnew(Tree);
	tree->set_custom_minimum_size(Size2(350, 0) * EDSCALE);
	tree->set_v_size_flags(SIZE_EXPAND_FILL);
	tree->set_h_size_flags(SIZE_EXPAND_FILL);
	tree->set_hide_root(true);
	tree->set_columns(4);
	tree->set_column_titles_visible(true);

	tree->set_column_title(0, TTRC("RID"));
	tree->set_column_expand(0, true);
	tree->set_column_clip_content(0, true);
	tree->set_column_custom_minimum_width(0, 50 * EDSCALE);

	tree->set_column_title(1, TTRC("Name"));
	tree->set_column_expand(1, true);
	tree->set_column_clip_content(1, true);
	tree->set_column_custom_minimum_width(1, 175 * EDSCALE);

	tree->set_column_title(2, TTRC("Role"));
	tree->set_column_expand(2, false);
	tree->set_column_clip_content(2, true);
	tree->set_column_custom_minimum_width(2, 125 * EDSCALE);

	tree->set_column_expand(3, false);
	tree->set_column_custom_minimum_width(3, 32 * EDSCALE);

	tree->set_theme_type_variation("TreeSecondary");
	tree->connect(SceneStringName(item_selected), callable_mp(this, &AccessibilityInspectorDock::_node_selected));
	tree_container->add_child(tree);

	HBoxContainer *status_bar = memnew(HBoxContainer);
	tree_container->add_child(status_bar);

	st_contrast = memnew(Button);
	st_contrast->set_tooltip_text(TTRC("High Contrast"));
	status_bar->add_child(st_contrast);

	st_anim = memnew(Button);
	st_anim->set_tooltip_text(TTRC("Reduce Animation"));
	status_bar->add_child(st_anim);

	st_trans = memnew(Button);
	st_trans->set_tooltip_text(TTRC("Reduce Transparency"));
	status_bar->add_child(st_trans);

	st_active = memnew(Button);
	st_active->set_tooltip_text(TTRC("Screen Reader Active"));
	status_bar->add_child(st_active);

	VBoxContainer *info_container = memnew(VBoxContainer);
	info_container->set_h_size_flags(SIZE_EXPAND_FILL);
	info_container->set_stretch_ratio(0.3);
	base_container->add_child(info_container);

	node_info = memnew(EditorInspector);
	node_info->set_v_size_flags(SIZE_EXPAND_FILL);
	node_info->set_autoclear(true);
	node_info->set_custom_minimum_size(Size2(100, 100));
	node_info->set_hide_script(true);
	node_info->set_hide_metadata(true);
	node_info->set_read_only(true);
	info_container->add_margin_child(TTRC("Node"), node_info, true);

	action_container = memnew(VBoxContainer);
	info_container->add_margin_child(TTRC("Action"), action_container, false);
	action_container->set_visible(false);

	actions = memnew(OptionButton);
	actions->set_accessibility_name(TTRC("Action"));
	actions->set_theme_type_variation(SceneStringName(FlatButton));
	actions->connect(SceneStringName(item_selected), callable_mp(this, &AccessibilityInspectorDock::_action_selected));
	action_container->add_child(actions);

	action_info = memnew(EditorInspector);
	action_info->set_autoclear(true);
	action_info->set_custom_minimum_size(Size2(100, 100));
	action_info->set_hide_script(true);
	action_info->set_hide_metadata(true);
	action_info->set_scroll_hint_mode(ScrollContainer::SCROLL_HINT_MODE_TOP_AND_LEFT);
	action_container->add_child(action_info);

	run_action = memnew(Button);
	run_action->set_h_size_flags(SIZE_SHRINK_BEGIN);
	run_action->set_text("Run Action");
	run_action->set_theme_type_variation(SceneStringName(FlatButton));
	run_action->connect(SceneStringName(pressed), callable_mp(this, &AccessibilityInspectorDock::_run_selected_action));
	run_action->set_disabled(true);
	action_container->add_child(run_action);

	if (unlikely(role_icons.is_empty())) {
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_UNKNOWN] = "Control";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_DEFAULT_BUTTON] = "BaseButton";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_AUDIO] = "AudioStreamPlayer";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_VIDEO] = "VideoStreamPlayer";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_STATIC_TEXT] = "Label";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_CONTAINER] = "Container";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_PANEL] = "Panel";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_BUTTON] = "Button";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_LINK] = "LinkButton";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_CHECK_BOX] = "CheckBox";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_RADIO_BUTTON] = "CheckBox";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_CHECK_BUTTON] = "CheckButton";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_SCROLL_BAR] = "VScrollBar";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_SCROLL_VIEW] = "ScrollContainer";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_SPLITTER] = "VSeparator";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_SLIDER] = "VSlider";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_SPIN_BUTTON] = "SpinBox";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_PROGRESS_INDICATOR] = "ProgressBar";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_TEXT_FIELD] = "LineEdit";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_MULTILINE_TEXT_FIELD] = "TextEdit";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_COLOR_PICKER] = "ColorPicker";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_TABLE] = "Node";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_CELL] = "Node";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_ROW] = "Node";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_ROW_GROUP] = "Node";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_ROW_HEADER] = "Node";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_COLUMN_HEADER] = "Node";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_TREE] = "Tree";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_TREE_ITEM] = "Item";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_LIST] = "GraphNode";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_LIST_ITEM] = "Item";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_LIST_BOX] = "ItemList";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_LIST_BOX_OPTION] = "Item";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_TAB_BAR] = "TabBar";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_TAB] = "Item";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_TAB_PANEL] = "TabContainer";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_MENU_BAR] = "MenuBar";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_MENU] = "PopupMenu";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_MENU_ITEM] = "Item";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_MENU_ITEM_CHECK_BOX] = "Item";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_MENU_ITEM_RADIO] = "Item";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_IMAGE] = "TextureRect";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_WINDOW] = "Window";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_TITLE_BAR] = "Node";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_DIALOG] = "AcceptDialog";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_TOOLTIP] = "Node";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_REGION] = "Container";
		role_icons[AccessibilityServerEnums::AccessibilityRole::ROLE_TEXT_RUN] = "Font";
	}
	if (unlikely(role_names.is_empty())) {
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_UNKNOWN] = "Unknown";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_DEFAULT_BUTTON] = "Default Button";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_AUDIO] = "Audio";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_VIDEO] = "Video";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_STATIC_TEXT] = "Static Text";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_CONTAINER] = "Container";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_PANEL] = "Panel";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_BUTTON] = "Button";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_LINK] = "Link";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_CHECK_BOX] = "Check Box";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_RADIO_BUTTON] = "Radio Button";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_CHECK_BUTTON] = "Check Button";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_SCROLL_BAR] = "Scroll Bar";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_SCROLL_VIEW] = "Scroll View";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_SPLITTER] = "Splitter";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_SLIDER] = "Slider";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_SPIN_BUTTON] = "Spin Button";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_PROGRESS_INDICATOR] = "Progress Indicator";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_TEXT_FIELD] = "Text Field";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_MULTILINE_TEXT_FIELD] = "Multiline Text Field";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_COLOR_PICKER] = "Color Picker";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_TABLE] = "Table";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_CELL] = "Cell";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_ROW] = "Row";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_ROW_GROUP] = "Row Group";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_ROW_HEADER] = "Row Header";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_COLUMN_HEADER] = "Column Header";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_TREE] = "Tree";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_TREE_ITEM] = "Tree Item";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_LIST] = "List";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_LIST_ITEM] = "List Item";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_LIST_BOX] = "List Box";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_LIST_BOX_OPTION] = "List Box Option";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_TAB_BAR] = "Tab Bar";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_TAB] = "Tab";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_TAB_PANEL] = "Tab Panel";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_MENU_BAR] = "Menu Bar";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_MENU] = "Menu";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_MENU_ITEM] = "Menu Item";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_MENU_ITEM_CHECK_BOX] = "Menu Item Check Box";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_MENU_ITEM_RADIO] = "Menu Item Radio";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_IMAGE] = "Image";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_WINDOW] = "Window";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_TITLE_BAR] = "Title Bar";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_DIALOG] = "Dialog";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_TOOLTIP] = "Tooltip";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_REGION] = "Region";
		role_names[AccessibilityServerEnums::AccessibilityRole::ROLE_TEXT_RUN] = "Text Run";
	}
	if (unlikely(action_names.is_empty())) {
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_CLICK] = "Click";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_FOCUS] = "Focus";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_BLUR] = "Blue";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_COLLAPSE] = "Collapse";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_EXPAND] = "Expand";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_DECREMENT] = "Dectement";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_INCREMENT] = "Increment";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_HIDE_TOOLTIP] = "Hide Tooltip";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_SHOW_TOOLTIP] = "Show Tooltip";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_SET_TEXT_SELECTION] = "Set Text Selection";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_REPLACE_SELECTED_TEXT] = "Replace Selected Text";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_BACKWARD] = "Scroll Backward";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_DOWN] = "Scroll Down";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_FORWARD] = "Scroll Forward";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_LEFT] = "Scroll Left";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_RIGHT] = "Scroll Right";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_UP] = "Scroll Up";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_INTO_VIEW] = "Scroll into View";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_TO_POINT] = "Scroll to Point";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_SET_SCROLL_OFFSET] = "Set Scroll Offset";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_SET_VALUE] = "Set Value";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_SHOW_CONTEXT_MENU] = "Show Context Menu";
		action_names[AccessibilityServerEnums::AccessibilityAction::ACTION_CUSTOM] = "Custom";
	}
}

AccessibilityInspectorDock::~AccessibilityInspectorDock() {
	singleton = nullptr;
}
