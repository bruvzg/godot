/**************************************************************************/
/*  dock_icon_proxy.h                                                     */
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

#ifdef TOOLS_ENABLED

#include "core/io/uds_server.h"
#include "core/os/process_id.h"
#include "servers/display/display_server_enums.h"

class NativeMenuMacOS;

struct ForeignWindowsID {
	ProcessID pid;
	DisplayServerEnums::WindowID wid;

	bool operator==(const ForeignWindowsID &p_val) const {
		return (pid == p_val.pid) && (wid == p_val.wid);
	}

	static uint32_t hash(const ForeignWindowsID &p_val) {
		uint32_t h = hash_murmur3_one_32(p_val.pid);
		return hash_fmix32(hash_murmur3_one_32(p_val.wid, h));
	}
};

enum DockIconProxyCommand {
	DOCK_ICON_PROXY_CMD_UPDATE_WINDOW_LIST,
	DOCK_ICON_PROXY_CMD_ACTIVATE,
	DOCK_ICON_PROXY_CMD_REQUSET_ATTENTION,
	DOCK_ICON_PROXY_CMD_KEY_CHANGED,
	DOCK_ICON_PROXY_CMD_QUIT,
};

class DockIconProxyServer : public Object {
	GDSOFTCLASS(DockIconProxyServer, Object);

	struct ClientInfo {
		Ref<StreamPeerUDS> peer;
		ProcessID pid = 0;

		ClientInfo() {}
		ClientInfo(const Ref<StreamPeerUDS> &p_peer) { peer = p_peer; }
	};

	ProcessID last_pid = 0;
	Ref<UDSServer> server;
	List<ClientInfo> clients;
	HashMap<ForeignWindowsID, String, ForeignWindowsID> windows;
	Mutex client_mutex;

	NativeMenuMacOS *native_menu = nullptr;

	Thread thread;
	bool thread_running = false;
	bool first_connected = false;
	int64_t quit_timeout = 0;

	static void thread_main(void *p_userdata);
	void _global_menu_select(const Variant &p_tag);

public:
	bool start_server();

	void send_activate(ProcessID p_pid, DisplayServerEnums::WindowID p_wid);
	void send_quit();

	~DockIconProxyServer();
};

class DockIconProxyClient : public Object {
	GDSOFTCLASS(DockIconProxyClient, Object);

	Ref<StreamPeerUDS> peer;
	HashMap<DisplayServerEnums::WindowID, String> windows;
	HashMap<ForeignWindowsID, String, ForeignWindowsID> foreign_windows;

	void _post_window_list();
	void _global_menu_select(const Variant &p_tag);

public:
	bool connect_to_proxy();
	void poll();

	void add_window(DisplayServerEnums::WindowID p_id);
	void remove_window(DisplayServerEnums::WindowID p_id);
	void set_window_title(DisplayServerEnums::WindowID p_id, const String &p_title);

	void send_activate(ProcessID p_pid, DisplayServerEnums::WindowID p_wid);
	void send_key_changed();
	void send_request_attention();

	~DockIconProxyClient();
};

#endif
