/**************************************************************************/
/*  dock_icon_proxy.mm                                                    */
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

#include "dock_icon_proxy.h"
#include "native_menu_macos.h"

#include "core/object/callable_mp.h"
#include "core/os/os.h"
#include "core/version.h"
#include "servers/display/display_server.h"

#import <AppKit/AppKit.h>

#ifdef TOOLS_ENABLED

String _get_socket_path() {
	return OS::get_singleton()->get_temp_path().path_join(vformat("godot_icon_proxy_%s.sock", GODOT_VERSION_FULL_BUILD));
}

bool DockIconProxyServer::start_server() {
	native_menu = memnew(NativeMenuMacOS);

	NSMenu *dock_menu = [[NSMenu alloc] initWithTitle:@"_dock"];
	[dock_menu setAutoenablesItems:NO];

	native_menu->_register_system_menus(nullptr, nullptr, nullptr, nullptr, dock_menu);

	server.instantiate();
	Error err = server->listen(_get_socket_path());
	if (err != OK) {
		print_verbose("Dock Proxy: Server listen error");
		return false;
	}

	thread_running = true;
	thread.start(DockIconProxyServer::thread_main, this);

	return true;
}

void DockIconProxyServer::thread_main(void *p_userdata) {
	DockIconProxyServer *self = static_cast<DockIconProxyServer *>(p_userdata);
	while (self->thread_running) {
		Ref<StreamPeerUDS> peer = self->server->take_connection();
		{
			MutexLock lock(self->client_mutex);
			if (peer.is_valid()) {
				self->clients.push_back(ClientInfo(peer));
				self->first_connected = true;
			}
			bool forward_list = false;
			for (List<ClientInfo>::Element *E = self->clients.front(); E;) {
				ClientInfo &client = E->get();
				client.peer->poll();
				if (client.peer->get_status() != StreamPeerSocket::STATUS_CONNECTED && client.peer->get_status() != StreamPeerSocket::STATUS_CONNECTING) {
					List<ClientInfo>::Element *F = E;
					E = E->next();
					self->clients.erase(F);
					forward_list = true;
					continue;
				}
				if (client.peer->get_available_bytes() > 1) {
					DockIconProxyCommand cmd = (DockIconProxyCommand)client.peer->get_8();
					switch (cmd) {
						case DOCK_ICON_PROXY_CMD_UPDATE_WINDOW_LIST: {
							client.pid = client.peer->get_64();
							for (KeyValue<ForeignWindowsID, String> &w : self->windows) {
								if (w.key.pid == client.pid) {
									self->windows.erase(w.key);
								}
							}
							uint32_t count = client.peer->get_32();
							for (uint32_t i = 0; i < count; i++) {
								DisplayServerEnums::WindowID wid = client.peer->get_64();
								String title = client.peer->get_string();
								self->windows[ForeignWindowsID{ client.pid, wid }] = title;
							}
							forward_list = true;
						} break;
						case DOCK_ICON_PROXY_CMD_ACTIVATE: {
							ProcessID pid = client.peer->get_64();
							DisplayServerEnums::WindowID wid = client.peer->get_64();
							self->send_activate(pid, wid);
						} break;
						case DOCK_ICON_PROXY_CMD_REQUSET_ATTENTION: {
							[NSApp requestUserAttention:NSCriticalRequest];
						} break;
						case DOCK_ICON_PROXY_CMD_KEY_CHANGED: {
							self->last_pid = client.pid;
						} break;
						case DOCK_ICON_PROXY_CMD_QUIT: {
							for (KeyValue<ForeignWindowsID, String> &w : self->windows) {
								if (w.key.pid == client.pid) {
									self->windows.erase(w.key);
								}
							}
							forward_list = true;
							List<ClientInfo>::Element *F = E;
							E = E->next();
							self->clients.erase(F);
							forward_list = true;
							continue;
						} break;
					}
				}
				E = E->next();
			}
			if (forward_list) {
				RID dock = self->native_menu->get_system_menu(NativeMenu::DOCK_MENU_ID);
				self->native_menu->clear(dock);
				for (ClientInfo &client : self->clients) {
					client.peer->put_8(DOCK_ICON_PROXY_CMD_UPDATE_WINDOW_LIST);
					client.peer->put_32(self->windows.size());
					for (const KeyValue<ForeignWindowsID, String> &w : self->windows) {
						client.peer->put_64(w.key.pid);
						client.peer->put_64(w.key.wid);
						client.peer->put_string(w.value);

						self->native_menu->add_item(dock, w.value, callable_mp(self, &DockIconProxyServer::_global_menu_select), Callable(), Array{ w.key.pid, w.key.wid });
					}
				}
			}
			if (self->clients.is_empty() && self->first_connected) {
				if (self->quit_timeout == 0) {
					self->quit_timeout = OS::get_singleton()->get_ticks_msec();
				} else {
					if (OS::get_singleton()->get_ticks_msec() - self->quit_timeout) {
						[NSApp terminate:nil];
						return;
					}
				}
			}
		}
		OS::get_singleton()->delay_usec(50000);
	}
}

void DockIconProxyServer::_global_menu_select(const Variant &p_tag) {
	Array tag = p_tag;
	if (tag.size() != 2) {
		return;
	}
	send_activate(tag[0], tag[1]);
}

void DockIconProxyServer::send_activate(ProcessID p_pid, DisplayServerEnums::WindowID p_wid) {
	// TODO call from dock menu
	MutexLock lock(client_mutex);
	if (p_pid == 0) {
		p_pid = last_pid;
	}
	for (ClientInfo &client : clients) {
		if (client.pid == p_pid) {
			client.peer->put_8(DOCK_ICON_PROXY_CMD_ACTIVATE);
			client.peer->put_64(p_wid);
		}
	}
}

void DockIconProxyServer::send_quit() {
	// TODO call from dock menu
	MutexLock lock(client_mutex);
	for (ClientInfo &client : clients) {
		client.peer->put_8(DOCK_ICON_PROXY_CMD_QUIT);
	}
}

DockIconProxyServer::~DockIconProxyServer() {
	if (thread_running) {
		thread_running = false;
		thread.wait_to_finish();
	}
	if (native_menu) {
		memdelete(native_menu);
	}
}

/**************************************************************************/

bool DockIconProxyClient::connect_to_proxy() {
	peer.instantiate();
	Error err = peer->connect_to_host(_get_socket_path());
	if (err != OK) {
		return false;
	}
	while (peer->get_status() == StreamPeerSocket::STATUS_CONNECTING) {
		peer->poll();
	}
	if (peer->get_status() == StreamPeerSocket::STATUS_ERROR) {
		return false;
	}
	return true;
}

void DockIconProxyClient::poll() {
	if (peer.is_null() || peer->get_status() == StreamPeerSocket::STATUS_ERROR) {
		return;
	}
	peer->poll();
	if (peer->get_status() == StreamPeerSocket::STATUS_ERROR) {
		peer = Ref<StreamPeerUDS>();
	}
	if (peer->get_available_bytes() > 1) {
		DockIconProxyCommand cmd = (DockIconProxyCommand)peer->get_8();
		switch (cmd) {
			case DOCK_ICON_PROXY_CMD_UPDATE_WINDOW_LIST: {
				NativeMenu *nm = NativeMenu::get_singleton();
				RID window = nm->get_system_menu(NativeMenu::WINDOW_MENU_ID);
				for (int i = nm->get_item_count(window) - 1; i >= 0; i--) {
					Array tag = nm->get_item_tag(window, i);
					if (tag.size() == 3 && tag[0] == "@DockProxy") {
						nm->remove_item(window, i);
					}
				}
				ProcessID own_pid = OS::get_singleton()->get_process_id();

				foreign_windows.clear();
				uint32_t count = peer->get_32();
				for (uint32_t i = 0; i < count; i++) {
					ProcessID pid = peer->get_64();
					DisplayServerEnums::WindowID wid = peer->get_64();
					String title = peer->get_string();
					foreign_windows[ForeignWindowsID{ pid, wid }] = title;
					if (pid != own_pid) {
						nm->add_item(window, title, callable_mp(this, &DockIconProxyClient::_global_menu_select), Callable(), Array{ "@DockProxy", pid, wid });
					}
				}
			} break;
			case DOCK_ICON_PROXY_CMD_ACTIVATE: {
				DisplayServerEnums::WindowID wid = peer->get_64();
				DisplayServer::get_singleton()->window_move_to_foreground(wid);
			} break;
			case DOCK_ICON_PROXY_CMD_QUIT: {
				//TODO request close
			} break;
			case DOCK_ICON_PROXY_CMD_KEY_CHANGED:
			case DOCK_ICON_PROXY_CMD_REQUSET_ATTENTION: {
				// Never used.
			} break;
		}
	}
}

void DockIconProxyClient::_global_menu_select(const Variant &p_tag) {
	Array tag = p_tag;
	if (tag.size() != 3 || tag[0] != "@DockProxy") {
		return;
	}
	send_activate(tag[1], tag[2]);
}

void DockIconProxyClient::_post_window_list() {
	if (peer.is_null() || peer->get_status() == StreamPeerSocket::STATUS_ERROR) {
		return;
	}
	peer->put_8(DOCK_ICON_PROXY_CMD_UPDATE_WINDOW_LIST);
	peer->put_64(OS::get_singleton()->get_process_id());
	peer->put_32(windows.size());
	for (const KeyValue<DisplayServerEnums::WindowID, String> &w : windows) {
		peer->put_64(w.key);
		peer->put_string(w.value);
	}
}

void DockIconProxyClient::add_window(DisplayServerEnums::WindowID p_id) {
	windows[p_id] = vformat("Window %d", p_id);
	_post_window_list();
}

void DockIconProxyClient::remove_window(DisplayServerEnums::WindowID p_id) {
	windows.erase(p_id);
	_post_window_list();
}

void DockIconProxyClient::set_window_title(DisplayServerEnums::WindowID p_id, const String &p_title) {
	if (!windows.has(p_id)) {
		return;
	}
	windows[p_id] = p_title;
	_post_window_list();
}

void DockIconProxyClient::send_activate(ProcessID p_pid, DisplayServerEnums::WindowID p_wid) {
	if (peer.is_null() || peer->get_status() == StreamPeerSocket::STATUS_ERROR) {
		return;
	}
	peer->put_8(DOCK_ICON_PROXY_CMD_ACTIVATE);
	peer->put_64(p_pid);
	peer->put_64(p_wid);
}

void DockIconProxyClient::send_request_attention() {
	if (peer.is_null() || peer->get_status() == StreamPeerSocket::STATUS_ERROR) {
		return;
	}
	peer->put_8(DOCK_ICON_PROXY_CMD_REQUSET_ATTENTION);
}

void DockIconProxyClient::send_key_changed() {
	if (peer.is_null() || peer->get_status() == StreamPeerSocket::STATUS_ERROR) {
		return;
	}
	peer->put_8(DOCK_ICON_PROXY_CMD_KEY_CHANGED);
}

DockIconProxyClient::~DockIconProxyClient() {
	if (peer.is_null() || peer->get_status() == StreamPeerSocket::STATUS_ERROR) {
		return;
	}
	peer->put_8(DOCK_ICON_PROXY_CMD_QUIT);
	peer->disconnect_from_host();
	peer->poll();
}

#endif
