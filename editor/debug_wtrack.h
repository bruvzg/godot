/**************************************************************************/
/*  debug_wtrack.h                                                        */
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

class WTrack : public Control {
	GDCLASS(WTrack, Control);

protected:
	void _notification(int p_what) {
		if (p_what == Control::NOTIFICATION_PROCESS) {
			set_position(Vector2());
			set_size(get_window()->get_size());
			queue_redraw();
		} else if (p_what == Control::NOTIFICATION_DRAW) {
			Ref<Font> font = get_theme_font(SceneStringName(font), SNAME("Tree"));
			int font_size = get_theme_font_size(SceneStringName(font_size), SNAME("Tree")) * 0.75;
			Vector2 sz = get_size() / get_window()->get_dpi_scale_factor();

			int c = DisplayServer::get_singleton()->get_screen_count();
			Vector2 max_point;
			for (int i = 0; i < c; i++) {
				max_point = max_point.max(DisplayServer::get_singleton()->screen_get_size(i) + DisplayServer::get_singleton()->screen_get_position(i));
			}
			max_point += Vector2(150, 150);
			double scale = MAX(max_point.x / sz.x, max_point.y / sz.y);
			for (int i = 0; i < c; i++) {
				draw_rect(Rect2((Vector2(75, 75) + Vector2(DisplayServer::get_singleton()->screen_get_position(i))) / scale, Vector2(DisplayServer::get_singleton()->screen_get_size(i)) / scale), Color(1, 0, 0), false, 1, false);
				draw_rect(Rect2((Vector2(75, 75) + Vector2(DisplayServer::get_singleton()->screen_get_usable_rect(i).position)) / scale, Vector2(DisplayServer::get_singleton()->screen_get_usable_rect(i).size) / scale), Color(1, 0, 0), false, 1, false);
				draw_string(font, Vector2(0, font->get_descent(font_size)) + (Vector2(75, 75) + DisplayServer::get_singleton()->screen_get_position(i)) / scale, vformat("scr:%d @%dx%d sz:%dx%d sc:%.2f dpi:%d", i, DisplayServer::get_singleton()->screen_get_position(i).x, DisplayServer::get_singleton()->screen_get_position(i).y, DisplayServer::get_singleton()->screen_get_size(i).x, DisplayServer::get_singleton()->screen_get_size(i).y, DisplayServer::get_singleton()->screen_get_scale(i), DisplayServer::get_singleton()->screen_get_dpi(i)), HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, Color(1, 0.5, 0));
			}
			draw_circle(Vector2(75, 75) / scale, 4, Color(1, 0, 0), true);

			Vector<DisplayServer::WindowID> wl = DisplayServer::get_singleton()->get_window_list();
			for (const DisplayServer::WindowID &w : wl) {
				if (w != get_window()->get_window_id()) {
					Window *ww = Window::get_from_id(w);
					if (DisplayServer::get_singleton()->window_get_popup_safe_rect(w) != Rect2i()) {
						draw_rect(Rect2((Vector2(75, 75) + Vector2(DisplayServer::get_singleton()->window_get_popup_safe_rect(w).position)) / scale, Vector2(DisplayServer::get_singleton()->window_get_popup_safe_rect(w).size) / scale), Color(0, 1, 0, 0.2), true, 1, false);
					}
					draw_rect(Rect2((Vector2(75, 75) + Vector2(DisplayServer::get_singleton()->window_get_position(w))) / scale, Vector2(DisplayServer::get_singleton()->window_get_size(w)) / scale), Color(0, 1, 0), false, 1, false);
					draw_rect(Rect2((Vector2(75, 75) + Vector2(DisplayServer::get_singleton()->window_get_position_with_decorations(w))) / scale, Vector2(DisplayServer::get_singleton()->window_get_size_with_decorations(w)) / scale), Color(0, 1, 0), false, 1, false);
					draw_string(font, Vector2(0, 1 * font->get_height(font_size)) + (Vector2(75, 75) + Vector2(DisplayServer::get_singleton()->window_get_position(w))) / scale, vformat("wid:%d pwid:%d", (int)w, ww->get_parent_visible_window() ? (int)ww->get_parent_visible_window()->get_window_id() : -1), HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, Color(0.5, 1, 0));
					draw_string(font, Vector2(0, 2 * font->get_height(font_size)) + (Vector2(75, 75) + Vector2(DisplayServer::get_singleton()->window_get_position(w))) / scale, vformat("@%dx%d sz:%dx%d scr:%d", DisplayServer::get_singleton()->window_get_position(w).x, DisplayServer::get_singleton()->window_get_position(w).y, DisplayServer::get_singleton()->window_get_size(w).x, DisplayServer::get_singleton()->window_get_size(w).y, DisplayServer::get_singleton()->window_get_current_screen(w)), HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, Color(0.5, 1, 0));
					draw_string(font, Vector2(0, 3 * font->get_height(font_size)) + (Vector2(75, 75) + Vector2(DisplayServer::get_singleton()->window_get_position(w))) / scale, vformat("%s%s%s%s%s%s%s%s%s%s%s%s%s", DisplayServer::get_singleton()->window_get_flag(DisplayServer::WINDOW_FLAG_RESIZE_DISABLED, w) ? "R" : "_", DisplayServer::get_singleton()->window_get_flag(DisplayServer::WINDOW_FLAG_BORDERLESS, w) ? "B" : "_", DisplayServer::get_singleton()->window_get_flag(DisplayServer::WINDOW_FLAG_ALWAYS_ON_TOP, w) ? "T" : "_", DisplayServer::get_singleton()->window_get_flag(DisplayServer::WINDOW_FLAG_TRANSPARENT, w) ? "t" : "_", DisplayServer::get_singleton()->window_get_flag(DisplayServer::WINDOW_FLAG_NO_FOCUS, w) ? "F" : "_", DisplayServer::get_singleton()->window_get_flag(DisplayServer::WINDOW_FLAG_POPUP, w) ? "P" : "_", DisplayServer::get_singleton()->window_get_flag(DisplayServer::WINDOW_FLAG_EXTEND_TO_TITLE, w) ? "E" : "_", DisplayServer::get_singleton()->window_get_flag(DisplayServer::WINDOW_FLAG_MOUSE_PASSTHROUGH, w) ? "S" : "_", DisplayServer::get_singleton()->window_get_flag(DisplayServer::WINDOW_FLAG_SHARP_CORNERS, w) ? "S" : "_", DisplayServer::get_singleton()->window_get_flag(DisplayServer::WINDOW_FLAG_EXCLUDE_FROM_CAPTURE, w) ? "C" : "_", DisplayServer::get_singleton()->window_get_flag(DisplayServer::WINDOW_FLAG_POPUP_WM_HINT, w) ? "p" : "_", DisplayServer::get_singleton()->window_get_flag(DisplayServer::WINDOW_FLAG_MINIMIZE_DISABLED, w) ? "m" : "_", DisplayServer::get_singleton()->window_get_flag(DisplayServer::WINDOW_FLAG_MAXIMIZE_DISABLED, w) ? "M" : "_"), HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, Color(0.5, 1, 0));
					draw_string(font, Vector2(0, 4 * font->get_height(font_size)) + (Vector2(75, 75) + Vector2(DisplayServer::get_singleton()->window_get_position(w))) / scale, vformat("os: %.2f oso: %.2f ds:%.2f cs:%.2f", ww->get_oversampling(), ww->get_oversampling_override(), ww->get_dpi_scale_factor(), ww->get_content_scale_factor()), HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, Color(0.5, 1, 0));
				}
			}
			Vector2 mp = (Vector2(75, 75) + DisplayServer::get_singleton()->mouse_get_position()) / scale;
			draw_circle(mp, 4, Color(0, 0, 1), true);
			draw_string(font, Vector2(0, 1 * font->get_height(font_size)) + mp, vformat("@%dx%d", DisplayServer::get_singleton()->mouse_get_position().x, DisplayServer::get_singleton()->mouse_get_position().y), HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, Color(0, 0, 1));
		}
	}
	static void _bind_methods() {}

public:
	WTrack() {
		set_process(true);
	}
};
