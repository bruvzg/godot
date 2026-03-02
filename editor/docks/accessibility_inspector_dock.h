/**************************************************************************/
/*  accessibility_inspector_dock.h                                        */
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

#pragma once

#include "editor/debugger/editor_debugger_plugin.h"
#include "editor/docks/editor_dock.h"
#include "servers/display/display_server.h"

class Button;
class EditorInspector;
class MenuButton;
class OptionButton;
class Tree;
class TreeItem;
class ItemInfo;
class ActionInfo;
class VBoxContainer;
class LineEdit;

class AccessibilityDebuggerPlugin : public EditorDebuggerPlugin {
	GDCLASS(AccessibilityDebuggerPlugin, EditorDebuggerPlugin);

public:
	AccessibilityDebuggerPlugin() {}

	virtual bool has_capture(const String &p_capture) const override;
	virtual bool capture(const String &p_message, const Array &p_data, int p_index) override;
};

class AccessibilityInspectorDock : public EditorDock {
	GDCLASS(AccessibilityInspectorDock, EditorDock);
	friend AccessibilityDebuggerPlugin;

	static inline AccessibilityInspectorDock *singleton = nullptr;

	static inline HashMap<AccessibilityServerEnums::AccessibilityRole, String> role_names;
	static inline HashMap<AccessibilityServerEnums::AccessibilityRole, String> role_icons;
	static inline HashMap<AccessibilityServerEnums::AccessibilityAction, String> action_names;

	struct ItemFadeTimer {
		ObjectID item;
		double max = 60.0;
		double remaining = 0.0;
		Color color;
	};
	List<ItemFadeTimer> timers;
	Ref<ItemInfo> info;
	Ref<ActionInfo> action;

	Button *capture_button = nullptr;
	OptionButton *target = nullptr;
	Tree *tree = nullptr;
	Button *st_contrast = nullptr;
	Button *st_anim = nullptr;
	Button *st_trans = nullptr;
	Button *st_active = nullptr;
	EditorInspector *node_info = nullptr;
	OptionButton *actions = nullptr;
	EditorInspector *action_info = nullptr;
	Button *run_action = nullptr;
	VBoxContainer *action_container = nullptr;
	LineEdit *search = nullptr;
	Button *next_search = nullptr;

	TreeItem *root = nullptr;

	bool running = false;
	bool local = false;
	List<RID> pending_update;
	HashMap<RID, TreeItem *> nodes;
	HashMap<DisplayServerEnums::WindowID, RID> focused_node;
	Ref<AccessibilityDebuggerPlugin> debugger;

	String _get_role_name(AccessibilityServerEnums::AccessibilityRole p_role) const;
	String _get_role_icon(AccessibilityServerEnums::AccessibilityRole p_role) const;
	String _get_action_name(AccessibilityServerEnums::AccessibilityAction p_action) const;

	RID current_search_result;
	void _search_text_changed(const String &p_text, bool p_next);
	void _search_next();

	void _toggle_capture();
	void _action_selected(int p_action);
	void _run_selected_action();
	void _run_action(const RID &p_rid, AccessibilityServerEnums::AccessibilityAction p_action, const Variant &p_data);
	void _node_selected();

	void _local_update_pending();
	void _request_node_info(const RID &p_rid);
	void _initial_update(const Array &p_windows);
	void _initial_window_update(DisplayServerEnums::WindowID p_id, const RID &p_root_node);
	bool _update_or_add_node(const RID &p_rid, const Array &p_node_info);

	void _debug_callback(int p_event, const Array &p_data);

	void _remove_item(TreeItem *p_item);

protected:
	void _notification(int p_what);

public:
	static AccessibilityInspectorDock *get_singleton() { return singleton; }

	AccessibilityInspectorDock();
	~AccessibilityInspectorDock();
};
