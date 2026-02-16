/**************************************************************************/
/*  winrt_displayinfo.cpp                                                 */
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

#include "winrt_dispalyinfo.h"

#ifdef WINRT_ENABLED

#include "core/os/memory.h"
#include "core/variant/variant.h"

#ifndef _MSC_VER
#define ____FIReference_1_boolean_INTERFACE_DEFINED__ // Bug in MinGW headers.
#endif

GODOT_GCC_WARNING_PUSH
GODOT_GCC_WARNING_IGNORE("-Wnon-virtual-dtor")
GODOT_GCC_WARNING_IGNORE("-Wctor-dtor-privacy")
GODOT_GCC_WARNING_IGNORE("-Wshadow")
GODOT_GCC_WARNING_IGNORE("-Wstrict-aliasing")
GODOT_CLANG_WARNING_PUSH
GODOT_CLANG_WARNING_IGNORE("-Wnon-virtual-dtor")

#include <dispatcherqueue.h>
#include <windows.graphics.display.interop.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.System.h>

#include <wil/cppwinrt.h>

GODOT_GCC_WARNING_POP
GODOT_CLANG_WARNING_POP

using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::System;

struct WinRTDispalyInfo::WindowData {
	int64_t id = 0;
	DispatcherQueueController controller{ nullptr };
	DisplayInformation info{ nullptr };
	Callable cb;
};

WinRTDispalyInfo::WindowData *WinRTDispalyInfo::create_wd(uint64_t p_hwnd, const Callable &p_cb, int64_t p_window_id) {
	WindowData *wd = memnew(WindowData);
	DispatcherQueueOptions options{ sizeof(options), DQTYPE_THREAD_CURRENT, DQTAT_COM_NONE };
	HRESULT res = CreateDispatcherQueueController(options, reinterpret_cast<ABI::Windows::System::IDispatcherQueueController **>(winrt::put_abi(wd->controller)));
	if (SUCCEEDED(res)) {
		wd->id = p_window_id;
		wd->cb = p_cb;
		try {
			wd->info = wil::capture_interop<DisplayInformation>(&IDisplayInformationStaticsInterop::GetForWindow, (HWND)p_hwnd);
			wd->info.OrientationChanged([wd](auto &&, auto &&) {
				wd->cb.call_deferred(wd->id, (int64_t)wd->info.CurrentOrientation());
			});
		} catch (...) {
			// ERR
		}
	}
	return wd;
}

void WinRTDispalyInfo::destroy_wd(WinRTDispalyInfo::WindowData *p_wd) {
	if (p_wd) {
		p_wd->info = nullptr;
		p_wd->controller.ShutdownQueueAsync().get();
		memdelete(p_wd);
	}
}

#else

struct WinRTDispalyInfo::WindowData {};

WinRTDispalyInfo::WindowData *WinRTDispalyInfo::create_wd(uint64_t p_hwnd, const Callable &p_cb, int64_t p_window_id) {
	return nullptr;
}

void WinRTDispalyInfo::destroy_wd(WinRTDispalyInfo::WindowData *p_wd) {
	//NOP
}

#endif
