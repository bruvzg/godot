/*************************************************************************/
/*  line_edit.cpp                                                        */
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

#include "line_edit.h"

#include "core/message_queue.h"
#include "core/os/keyboard.h"
#include "core/os/os.h"
#include "core/print_string.h"
#include "core/translation.h"
#include "label.h"
#include "servers/shaping/shaping_interface.h"
#include "servers/shaping_server.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_scale.h"
#include "editor/editor_settings.h"
#endif

void LineEdit::_gui_input(Ref<InputEvent> p_event) {

	Ref<InputEventMouseButton> b = p_event;

	if (b.is_valid()) {

		if (b->is_pressed() && b->get_button_index() == BUTTON_RIGHT && context_menu_enabled) {
			menu->set_position(get_global_transform().xform(get_local_mouse_position()));
			menu->set_size(Vector2(1, 1));
			menu->set_scale(get_global_transform().get_scale());
			menu->popup();
			grab_focus();
			accept_event();
			return;
		}

		if (b->get_button_index() != BUTTON_LEFT)
			return;

		_reset_caret_blink_timer();
		if (b->is_pressed()) {

			accept_event(); //don't pass event further when clicked on text field
			if (!text.empty() && is_editable() && _is_over_clear_button(b->get_position())) {
				clear_button_status.press_attempt = true;
				clear_button_status.pressing_inside = true;
				return;
			}

			shift_selection_check_pre(b->get_shift());

			set_cursor_at_pixel_pos(b->get_position().x);

			if (b->get_shift()) {

				selection_fill_at_cursor();
				selection.creating = true;

			} else {

				if (b->is_doubleclick()) {

					selection.enabled = true;
					selection.begin = 0;
					selection.end = text.length();
					selection.doubleclick = true;
				}

				selection.drag_attempt = false;

				if ((cursor_pos < selection.begin) || (cursor_pos > selection.end) || !selection.enabled) {

					deselect();
					selection.cursor_start = cursor_pos;
					selection.creating = true;
				} else if (selection.enabled) {

					selection.drag_attempt = true;
				}
			}

			update();

		} else {

			if (!text.empty() && is_editable() && clear_button_enabled) {
				bool press_attempt = clear_button_status.press_attempt;
				clear_button_status.press_attempt = false;
				if (press_attempt && clear_button_status.pressing_inside && _is_over_clear_button(b->get_position())) {
					clear();
					return;
				}
			}

			if ((!selection.creating) && (!selection.doubleclick)) {
				deselect();
			}
			selection.creating = false;
			selection.doubleclick = false;

			if (OS::get_singleton()->has_virtual_keyboard())
				OS::get_singleton()->show_virtual_keyboard(text, get_global_rect());
		}

		update();
	}

	Ref<InputEventMouseMotion> m = p_event;

	if (m.is_valid()) {

		if (!text.empty() && is_editable() && clear_button_enabled) {
			bool last_press_inside = clear_button_status.pressing_inside;
			clear_button_status.pressing_inside = clear_button_status.press_attempt && _is_over_clear_button(m->get_position());
			if (last_press_inside != clear_button_status.pressing_inside) {
				update();
			}
		}

		if (m->get_button_mask() & BUTTON_LEFT) {

			if (selection.creating) {
				set_cursor_at_pixel_pos(m->get_position().x);
				selection_fill_at_cursor();
			}
		}
	}

	Ref<InputEventKey> k = p_event;

	if (k.is_valid()) {

		if (!k->is_pressed())
			return;

#ifdef APPLE_STYLE_KEYS
		if (k->get_control() && !k->get_shift() && !k->get_alt() && !k->get_command()) {
			uint32_t remap_key = KEY_UNKNOWN;
			switch (k->get_scancode()) {
				case KEY_F: {
					remap_key = KEY_RIGHT;
				} break;
				case KEY_B: {
					remap_key = KEY_LEFT;
				} break;
				case KEY_P: {
					remap_key = KEY_UP;
				} break;
				case KEY_N: {
					remap_key = KEY_DOWN;
				} break;
				case KEY_D: {
					remap_key = KEY_DELETE;
				} break;
				case KEY_H: {
					remap_key = KEY_BACKSPACE;
				} break;
			}

			if (remap_key != KEY_UNKNOWN) {
				k->set_scancode(remap_key);
				k->set_control(false);
			}
		}
#endif

		unsigned int code = k->get_scancode();

		if (k->get_command()) {

			bool handled = true;

			switch (code) {

				case (KEY_D): { // Swap curent input direction (primary cursor)

					if (input_direction == TEXT_DIRECTION_LTR) {
						input_direction = TEXT_DIRECTION_RTL;
					} else {
						input_direction = TEXT_DIRECTION_LTR;
					}
					set_cursor_position(get_cursor_position());
					update();

				} break;

				case (KEY_X): { // CUT

					if (editable) {
						cut_text();
					}

				} break;

				case (KEY_C): { // COPY

					copy_text();

				} break;

				case (KEY_V): { // PASTE

					if (editable) {

						paste_text();
					}

				} break;

				case (KEY_Z): { // undo / redo
					if (editable) {
						if (k->get_shift()) {
							redo();
						} else {
							undo();
						}
					}
				} break;

				case (KEY_U): { // Delete from start to cursor

					if (editable) {

						deselect();
						text = text.substr(cursor_pos, text.length() - cursor_pos);

						shape_text();

						set_cursor_position(0);
						_text_changed();
					}

				} break;

				case (KEY_Y): { // PASTE (Yank for unix users)

					if (editable) {

						paste_text();
					}

				} break;
				case (KEY_K): { // Delete from cursor_pos to end

					if (editable) {

						deselect();
						text = text.substr(0, cursor_pos);
						_text_changed();
					}

				} break;
				case (KEY_A): { //Select All
					select();
				} break;
#ifdef APPLE_STYLE_KEYS
				case (KEY_LEFT): { // Go to start of text - like HOME key
					set_cursor_position(0);
				} break;
				case (KEY_RIGHT): { // Go to end of text - like END key
					set_cursor_position(text.length());
				} break;
#endif
				default: {
					handled = false;
				}
			}

			if (handled) {
				accept_event();
				return;
			}
		}

		_reset_caret_blink_timer();
		if (!k->get_metakey()) {

			bool handled = true;
			switch (code) {

				case KEY_KP_ENTER:
				case KEY_ENTER: {

					emit_signal("text_entered", text);
					if (OS::get_singleton()->has_virtual_keyboard())
						OS::get_singleton()->hide_virtual_keyboard();

					return;
				} break;

				case KEY_BACKSPACE: {

					if (!editable)
						break;

					if (selection.enabled) {
						selection_delete();
						break;
					}

#ifdef APPLE_STYLE_KEYS
					if (k->get_alt()) {
#else
					if (k->get_alt()) {
						handled = false;
						break;
					} else if (k->get_command()) {
#endif
						int cc = shaped->get_run_start(cursor_pos - 1);
						delete_text(cc, cursor_pos);
						set_cursor_position(cc);
					} else {
						delete_char();
					}

				} break;
				case KEY_KP_4: {
					if (k->get_unicode() != 0) {
						handled = false;
						break;
					}
					FALLTHROUGH;
				}
				case KEY_LEFT: {

#ifndef APPLE_STYLE_KEYS
					if (!k->get_alt())
#endif
						shift_selection_check_pre(k->get_shift());

#ifdef APPLE_STYLE_KEYS
					if (k->get_command()) {
						set_cursor_position(0);
					} else if (k->get_alt()) {

#else
					if (k->get_alt()) {
						handled = false;
						break;
					} else if (k->get_command()) {
#endif
						set_cursor_position(shaped->get_run_start(cursor_pos - 1));
					} else {
						set_cursor_position(get_cursor_position() - 1);
					}

					shift_selection_check_post(k->get_shift());

				} break;
				case KEY_KP_6: {
					if (k->get_unicode() != 0) {
						handled = false;
						break;
					}
					FALLTHROUGH;
				}
				case KEY_RIGHT: {

					shift_selection_check_pre(k->get_shift());

#ifdef APPLE_STYLE_KEYS
					if (k->get_command()) {
						set_cursor_position(text.length());
					} else if (k->get_alt()) {
#else
					if (k->get_alt()) {
						handled = false;
						break;
					} else if (k->get_command()) {
#endif
						set_cursor_position(shaped->get_run_end(cursor_pos));

					} else {
						set_cursor_position(get_cursor_position() + 1);
					}

					shift_selection_check_post(k->get_shift());

				} break;
				case KEY_UP: {

					shift_selection_check_pre(k->get_shift());
					if (get_cursor_position() == 0) handled = false;
					set_cursor_position(0);
					shift_selection_check_post(k->get_shift());
				} break;
				case KEY_DOWN: {

					shift_selection_check_pre(k->get_shift());
					if (get_cursor_position() == text.length()) handled = false;
					set_cursor_position(text.length());
					shift_selection_check_post(k->get_shift());
				} break;
				case KEY_DELETE: {

					if (!editable)
						break;

					if (k->get_shift() && !k->get_command() && !k->get_alt()) {
						cut_text();
						break;
					}

					if (selection.enabled) {
						selection_delete();
						break;
					}

					int text_len = text.length();

					if (cursor_pos == text_len)
						break; // nothing to do

#ifdef APPLE_STYLE_KEYS
					if (k->get_alt()) {
#else
					if (k->get_alt()) {
						handled = false;
						break;
					} else if (k->get_command()) {
#endif
						delete_text(cursor_pos, shaped->get_run_end(cursor_pos));
					} else {
						set_cursor_position(cursor_pos + 1);
						delete_char();
					}

				} break;
				case KEY_KP_7: {
					if (k->get_unicode() != 0) {
						handled = false;
						break;
					}
					FALLTHROUGH;
				}
				case KEY_HOME: {

					shift_selection_check_pre(k->get_shift());
					set_cursor_position(0);
					shift_selection_check_post(k->get_shift());
				} break;
				case KEY_KP_1: {
					if (k->get_unicode() != 0) {
						handled = false;
						break;
					}
					FALLTHROUGH;
				}
				case KEY_END: {

					shift_selection_check_pre(k->get_shift());
					set_cursor_position(text.length());
					shift_selection_check_post(k->get_shift());
				} break;

				default: {

					handled = false;
				} break;
			}

			if (handled) {
				accept_event();
			} else if (!k->get_command()) {
				if (k->get_unicode() >= 32 && k->get_scancode() != KEY_DELETE) {

					if (editable) {
						selection_delete();
						//Handle UTF32 input
						if (k->get_unicode() <= 0xFFFF) {
							CharType ucodestr[2] = {
								static_cast<CharType>(k->get_unicode()),
								0
							};
							append_at_cursor(ucodestr);
						} else {
							CharType ucodestr[3] = {
								static_cast<CharType>((k->get_unicode() - 0x10000) / 0x400 + 0xD800),
								static_cast<CharType>((k->get_unicode() - 0x10000) % 0x400 + 0xDC00),
								0
							};
							append_at_cursor(ucodestr);
						}
						_text_changed();
						accept_event();
					}

				} else {
					return;
				}
			}

			update();
		}

		return;
	}
}

void LineEdit::set_align(Align p_align) {

	ERR_FAIL_INDEX((int)p_align, 4);
	align = p_align;
	update();
}

LineEdit::Align LineEdit::get_align() const {

	return align;
}

void LineEdit::set_text_direction(TextDirection p_text_direction) {

	ERR_FAIL_INDEX((int)p_text_direction, 3);
	if (base_direction != p_text_direction) {
		base_direction = p_text_direction;
		if (base_direction != TEXT_DIRECTION_AUTO) {
			input_direction = base_direction;
		}
		shape_text();
		shape_placeholder();

		menu_dir->set_item_checked(menu_dir->get_item_index(MENU_DIR_AUTO), base_direction == TEXT_DIRECTION_AUTO);
		menu_dir->set_item_checked(menu_dir->get_item_index(MENU_DIR_LTR), base_direction == TEXT_DIRECTION_LTR);
		menu_dir->set_item_checked(menu_dir->get_item_index(MENU_DIR_RTL), base_direction == TEXT_DIRECTION_RTL);
		update();
	}
}

TextDirection LineEdit::get_text_direction() const {

	return base_direction;
}

void LineEdit::set_ot_features(const String &p_features) {

	if (ot_features != p_features) {
		ot_features = p_features;
		shape_text();
		shape_placeholder();
		update();
	}
}

String LineEdit::get_ot_features() const {

	return ot_features;
}

void LineEdit::set_language(const String &p_language) {

	if (language != p_language) {
		language = p_language;
		shape_text();
		shape_placeholder();
		update();
	}
}

String LineEdit::get_language() const {

	return language;
}

Variant LineEdit::get_drag_data(const Point2 &p_point) {

	if (selection.drag_attempt && selection.enabled) {
		String t = text.substr(selection.begin, selection.end - selection.begin);
		Label *l = memnew(Label);
		l->set_text(t);
		set_drag_preview(l);
		return t;
	}

	return Variant();
}

bool LineEdit::can_drop_data(const Point2 &p_point, const Variant &p_data) const {

	return p_data.get_type() == Variant::STRING;
}

void LineEdit::drop_data(const Point2 &p_point, const Variant &p_data) {

	if (p_data.get_type() == Variant::STRING) {
		set_cursor_at_pixel_pos(p_point.x);
		int selected = selection.end - selection.begin;

		text.erase(selection.begin, selected);

		append_at_cursor(p_data);
		shape_text();

		selection.begin = cursor_pos - selected;
		selection.end = cursor_pos;
	}
}

Control::CursorShape LineEdit::get_cursor_shape(const Point2 &p_pos) const {
	if (!text.empty() && is_editable() && _is_over_clear_button(p_pos)) {
		return CURSOR_ARROW;
	}
	return Control::get_cursor_shape(p_pos);
}

bool LineEdit::_is_over_clear_button(const Point2 &p_pos) const {
	if (!clear_button_enabled || !has_point(p_pos)) {
		return false;
	}
	Ref<Texture> icon = Control::get_icon("clear");
	int x_ofs = get_stylebox("normal")->get_offset().x;
	return p_pos.x > get_size().width - icon->get_width() - x_ofs;
}

void LineEdit::_notification(int p_what) {

	switch (p_what) {
#ifdef TOOLS_ENABLED
		case NOTIFICATION_ENTER_TREE: {
			if (Engine::get_singleton()->is_editor_hint() && !get_tree()->is_node_being_edited(this)) {
				cursor_set_blink_enabled(EDITOR_DEF("text_editor/cursor/caret_blink", false));
				cursor_set_blink_speed(EDITOR_DEF("text_editor/cursor/caret_blink_speed", 0.65));

				if (!EditorSettings::get_singleton()->is_connected("settings_changed", this, "_editor_settings_changed")) {
					EditorSettings::get_singleton()->connect("settings_changed", this, "_editor_settings_changed");
				}
			}
		} break;
#endif
		case NOTIFICATION_THEME_CHANGED: {
			shape_text();
			shape_placeholder();
			update();
		} break;
		case NOTIFICATION_RESIZED: {
			window_pos = 0;
			set_cursor_position(get_cursor_position());
		} break;
		case NOTIFICATION_TRANSLATION_CHANGED: {
			placeholder_translated = tr(placeholder);
			shape_placeholder();
			update();
		} break;
		case MainLoop::NOTIFICATION_WM_FOCUS_IN: {
			window_has_focus = true;
			draw_caret = true;
			update();
		} break;
		case MainLoop::NOTIFICATION_WM_FOCUS_OUT: {
			window_has_focus = false;
			draw_caret = false;
			update();
		} break;
		case NOTIFICATION_DRAW: {
			if ((!has_focus() && !menu->has_focus()) || !window_has_focus) {
				draw_caret = false;
			}

			int width, height;

			Size2 size = get_size();
			width = size.width;
			height = size.height;

			RID ci = get_canvas_item();

			Ref<StyleBox> style = get_stylebox("normal");
			if (!is_editable()) {
				style = get_stylebox("read_only");
				draw_caret = false;
			}

			Ref<Font> font = get_font("font");
			if (font.is_null()) return;

			style->draw(ci, Rect2(Point2(), size));

			if (has_focus()) {

				get_stylebox("focus")->draw(ci, Rect2(Point2(), size));
			}

			int x_ofs = 0;
			bool using_placeholder = text.empty() && ime_text.empty();

			if (shaped->require_reload()) const_cast<LineEdit *>(this)->shape_text();
			if (placeholder_shaped->require_reload()) const_cast<LineEdit *>(this)->shape_placeholder();

			Ref<ShapedString> vs = using_placeholder ? placeholder_shaped : shaped;

			switch (align) {

				case ALIGN_FILL:
				case ALIGN_LEFT: {
					x_ofs = style->get_offset().x;
				} break;
				case ALIGN_CENTER: {
					if (window_pos != 0)
						x_ofs = style->get_offset().x;
					else
						x_ofs = MAX(style->get_margin(MARGIN_LEFT), int(width - vs->get_size().width) / 2);
				} break;
				case ALIGN_RIGHT: {
					x_ofs = MAX(style->get_margin(MARGIN_LEFT), int(width - style->get_margin(MARGIN_RIGHT) - vs->get_size().width));
				} break;
			}

			int ofs_max = width - style->get_margin(MARGIN_LEFT) - style->get_margin(MARGIN_RIGHT);

			int y_area = height - style->get_minimum_size().height;
			int caret_h = MIN(y_area, (vs->get_size().height == 0) ? font->get_height() : vs->get_size().height);
#ifdef TOOLS_ENABLED
			int caret_w = Math::round(EDSCALE);
#else
			int caret_w = 1;
#endif

			int y_ofs = style->get_offset().y + (y_area - (caret_h)) / 2;

			Color selection_color = get_color("selection_color");
			Color font_color = is_editable() ? get_color("font_color") : get_color("font_color_uneditable");
			Color font_color_selected = get_color("font_color_selected");
			Color cursor_color = get_color("cursor_color");

			//placeholder color
			if (using_placeholder)
				font_color.a *= placeholder_alpha;

			bool display_clear_icon = !using_placeholder && is_editable() && clear_button_enabled;
			if (right_icon.is_valid() || display_clear_icon) {
				Ref<Texture> r_icon = display_clear_icon ? Control::get_icon("clear") : right_icon;
				Color color_icon(1, 1, 1, !is_editable() ? .5 * .9 : .9);
				if (display_clear_icon) {
					if (clear_button_status.press_attempt && clear_button_status.pressing_inside) {
						color_icon = get_color("clear_button_color_pressed");
					} else {
						color_icon = get_color("clear_button_color");
					}
				}
				r_icon->draw(ci, Point2(width - r_icon->get_width() - style->get_margin(MARGIN_RIGHT), height / 2 - r_icon->get_height() / 2), color_icon);

				if (align == ALIGN_CENTER) {
					if (window_pos == 0) {
						x_ofs = MAX(style->get_margin(MARGIN_LEFT), int(width - vs->get_size().width - r_icon->get_width() - style->get_margin(MARGIN_RIGHT) * 2) / 2);
					}
				} else {
					x_ofs = MAX(style->get_margin(MARGIN_LEFT), x_ofs - r_icon->get_width() - style->get_margin(MARGIN_RIGHT));
				}

				ofs_max -= r_icon->get_width();
			}

			//fill align fit to width
			if (align == ALIGN_FILL) {
				vs = vs->fit_to_width(ofs_max);
			}

			const Vector<Run> &v = vs->get_runs();
			//draw IME box
			if (ime_text.length() > 0) {
				int sel_begin = cursor_pos;
				int sel_end = cursor_pos + ime_text.length();
				Color sel_color = cursor_color;
				sel_color.a *= 0.3;

				Vector<Rect2> sel_rects = vs->get_selection_rects(sel_begin, sel_end);
				for (int64_t i = 0; i < sel_rects.size(); i++) {
					//clip
					sel_rects.write[i].position.y += y_ofs;
					sel_rects.write[i].position.x = MAX(sel_rects[i].position.x + (x_ofs - window_pos), x_ofs);
					if (sel_rects[i].position.x + sel_rects[i].size.x > ofs_max) {
						sel_rects.write[i].size.x = ofs_max - sel_rects[i].position.x;
					}
					VisualServer::get_singleton()->canvas_item_add_rect(ci, Rect2(sel_rects[i].position, Size2(1, sel_rects[i].size.height)), sel_color);
					VisualServer::get_singleton()->canvas_item_add_rect(ci, Rect2(sel_rects[i].position + Point2(sel_rects[i].size.width, 0), Size2(1, sel_rects[i].size.height)), sel_color);
					VisualServer::get_singleton()->canvas_item_add_rect(ci, Rect2(sel_rects[i].position, Size2(sel_rects[i].size.width, 1)), sel_color);
					VisualServer::get_singleton()->canvas_item_add_rect(ci, Rect2(sel_rects[i].position + Point2(0, sel_rects[i].size.height), Size2(sel_rects[i].size.width, 1)), sel_color);
				}
			}
			//draw selection or IME carets
			if (selection.enabled || (ime_text.length() > 0)) {
				int sel_begin = (ime_text.length() == 0) ? selection.begin : cursor_pos + ime_selection.x;
				int sel_end = (ime_text.length() == 0) ? selection.end : cursor_pos + ime_selection.x + ime_selection.y;
				Color sel_color = (ime_text.length() == 0) ? selection_color : cursor_color;

				Vector<Rect2> sel_rects = vs->get_selection_rects(sel_begin, sel_end);
				for (int64_t i = 0; i < sel_rects.size(); i++) {
					//clip
					sel_rects.write[i].position.y += y_ofs;
					sel_rects.write[i].position.x = MAX(sel_rects[i].position.x + (x_ofs - window_pos), x_ofs);
					if (sel_rects[i].position.x + sel_rects[i].size.x > ofs_max) {
						sel_rects.write[i].size.x = ofs_max - sel_rects[i].position.x;
					}
					if (ime_text.length() > 0) {
						sel_rects.write[i].position.y += sel_rects[i].size.y;
						sel_rects.write[i].size.y = caret_w * 2;
					}
					VisualServer::get_singleton()->canvas_item_add_rect(ci, sel_rects[i], sel_color);
				}
			}
			//draw outlines
			int x_glyph_ofs = -window_pos;
			for (int32_t i = 0; i < v.size(); i++) {
				for (int32_t j = 0; j < v[i].clusters.size(); j++) {
					Ref<FontImplementation> fi = v[i].clusters[j].font_outline_impl;
					for (int32_t k = 0; k < v[i].clusters[j].repeat; k++) {
						for (int32_t g = 0; g < v[i].clusters[j].glyphs.size(); g++) {
							if (x_ofs + x_glyph_ofs >= 0) {
								if (((v[i].clusters[j].flags & GLYPH_VALID) == GLYPH_VALID) && fi.is_valid()) {
									fi->draw_glyph(ci, Point2(x_ofs + x_glyph_ofs, y_ofs + vs->get_ascent()) + v[i].clusters[j].glyphs[g].offset, v[i].clusters[j].glyphs[g].codepoint, Color(1, 1, 1));
								}
							}
							x_glyph_ofs += v[i].clusters[j].glyphs[g].advance;
							if ((x_ofs + x_glyph_ofs) > ofs_max) goto end_draw_outline_glyph;
						}
					}
				}
			}
		end_draw_outline_glyph:
			//draw glyphs
			x_glyph_ofs = -window_pos;
			for (int32_t i = 0; i < v.size(); i++) {
				for (int32_t j = 0; j < v[i].clusters.size(); j++) {
					Ref<FontImplementation> fi = v[i].clusters[j].font_impl;
					bool selected = selection.enabled && v[i].clusters[j].start >= selection.begin && v[i].clusters[j].start < selection.end;
					for (int32_t k = 0; k < v[i].clusters[j].repeat; k++) {
						for (int32_t g = 0; g < v[i].clusters[j].glyphs.size(); g++) {
							if (x_ofs + x_glyph_ofs >= 0) {
								if (((v[i].clusters[j].flags & GLYPH_VALID) == GLYPH_VALID) && fi.is_valid()) {
									fi->draw_glyph(ci, Point2(x_ofs + x_glyph_ofs, y_ofs + vs->get_ascent()) + v[i].clusters[j].glyphs[g].offset, v[i].clusters[j].glyphs[g].codepoint, selected ? font_color_selected : font_color);
								} else {
									FontHexBox::draw_glyph(ci, Point2(x_ofs + x_glyph_ofs, y_ofs + vs->get_ascent()) + v[i].clusters[j].glyphs[g].offset, v[i].clusters[j].glyphs[g].codepoint, selected ? font_color_selected : font_color);
								}
							}
							x_glyph_ofs += v[i].clusters[j].glyphs[g].advance;
							if ((x_ofs + x_glyph_ofs) > ofs_max) goto end_draw_glyph;
						}
					}
				}
			}
		end_draw_glyph:
			//draw non IME carets
			if (draw_caret && ime_text.empty()) {
				int ltr = vs->get_ltr_caret_offset(cursor_pos);
				int rtl = vs->get_rtl_caret_offset(cursor_pos);

				if ((rtl == -1) && (ltr == -1)) {
					//No caret -> empty string
					VisualServer::get_singleton()->canvas_item_add_rect(ci, Rect2(Point2(x_ofs - window_pos, y_ofs), Size2(caret_w, caret_h)), cursor_color);
				} else if ((ltr == rtl) || (rtl == -1)) {
					//One caret, match or ltr only
					VisualServer::get_singleton()->canvas_item_add_rect(ci, Rect2(Point2(x_ofs - window_pos + ltr, y_ofs), Size2(caret_w, caret_h)), cursor_color);
				} else if (ltr == -1) {
					//One catet, rtl only
					VisualServer::get_singleton()->canvas_item_add_rect(ci, Rect2(Point2(x_ofs - window_pos + rtl, y_ofs), Size2(caret_w, caret_h)), cursor_color);
				} else {
					//Two carets
					VisualServer::get_singleton()->canvas_item_add_rect(ci, Rect2(Point2(x_ofs - window_pos + ltr, y_ofs + (caret_h) / 2), Size2(caret_w, (caret_h) / 2)), cursor_color);
					VisualServer::get_singleton()->canvas_item_add_rect(ci, Rect2(Point2(x_ofs - window_pos + rtl, y_ofs), Size2(caret_w, (caret_h) / 2)), cursor_color);
				}
			}

			if (has_focus()) {

				OS::get_singleton()->set_ime_active(true);
				OS::get_singleton()->set_ime_position(get_global_position() + Point2(using_placeholder ? 0 : x_ofs, y_ofs + caret_h));
			}
		} break;
		case NOTIFICATION_FOCUS_ENTER: {

			if (!caret_blink_enabled) {
				draw_caret = true;
			}

			OS::get_singleton()->set_ime_active(true);
			Point2 cursor_px_pos = Point2(get_cursor_position(), 1) * get_minimum_size().height;
			OS::get_singleton()->set_ime_position(get_global_position() + cursor_px_pos);

			if (OS::get_singleton()->has_virtual_keyboard())
				OS::get_singleton()->show_virtual_keyboard(text, get_global_rect());

		} break;
		case NOTIFICATION_FOCUS_EXIT: {

			OS::get_singleton()->set_ime_position(Point2());
			OS::get_singleton()->set_ime_active(false);
			ime_text = "";
			ime_selection = Point2();
			shape_text();
			update();

			if (OS::get_singleton()->has_virtual_keyboard())
				OS::get_singleton()->hide_virtual_keyboard();

		} break;
		case MainLoop::NOTIFICATION_OS_IME_UPDATE: {

			if (has_focus()) {
				ime_text = OS::get_singleton()->get_ime_text();
				ime_selection = OS::get_singleton()->get_ime_selection();
				shape_text();
				update();
			}
		} break;
		case MainLoop::NOTIFICATION_SHAPING_INTERFACE_CHANGED: {
			shape_placeholder();
			shape_text();
			update();
		} break;
	}
}

void LineEdit::copy_text() {

	if (selection.enabled && !pass) {
		OS::get_singleton()->set_clipboard(text.substr(selection.begin, selection.end - selection.begin));
	}
}

void LineEdit::cut_text() {

	if (selection.enabled && !pass) {
		OS::get_singleton()->set_clipboard(text.substr(selection.begin, selection.end - selection.begin));
		selection_delete();
	}
}

void LineEdit::paste_text() {

	// Strip escape characters like \n and \t as they can't be displayed on LineEdit.
	String paste_buffer = OS::get_singleton()->get_clipboard().strip_escapes();

	if (paste_buffer != "") {

		if (selection.enabled) selection_delete();
		append_at_cursor(paste_buffer);

		if (!text_changed_dirty) {
			if (is_inside_tree()) {
				MessageQueue::get_singleton()->push_call(this, "_text_changed");
			}
			text_changed_dirty = true;
		}
	}
}

void LineEdit::undo() {
	if (undo_stack_pos == NULL) {
		if (undo_stack.size() <= 1) {
			return;
		}
		undo_stack_pos = undo_stack.back();
	} else if (undo_stack_pos == undo_stack.front()) {
		return;
	}
	undo_stack_pos = undo_stack_pos->prev();
	TextOperation op = undo_stack_pos->get();
	text = op.text;
	set_cursor_position(op.cursor_pos);

	if (expand_to_text_length)
		minimum_size_changed();

	_emit_text_change();
}

void LineEdit::redo() {
	if (undo_stack_pos == NULL) {
		return;
	}
	if (undo_stack_pos == undo_stack.back()) {
		return;
	}
	undo_stack_pos = undo_stack_pos->next();
	TextOperation op = undo_stack_pos->get();
	text = op.text;
	set_cursor_position(op.cursor_pos);

	if (expand_to_text_length)
		minimum_size_changed();

	_emit_text_change();
}

void LineEdit::shift_selection_check_pre(bool p_shift) {

	if (!selection.enabled && p_shift) {
		selection.cursor_start = cursor_pos;
	}
	if (!p_shift)
		deselect();
}

void LineEdit::shift_selection_check_post(bool p_shift) {

	if (p_shift)
		selection_fill_at_cursor();
}

void LineEdit::set_cursor_at_pixel_pos(int p_x) {

	Ref<Font> font = get_font("font");
	if (shaped->require_reload()) const_cast<LineEdit *>(this)->shape_text();

	Ref<StyleBox> style = get_stylebox("normal");
	Size2 size = get_size();
	bool display_clear_icon = !text.empty() && is_editable() && clear_button_enabled;
	int r_icon_width = Control::get_icon("clear")->get_width();

	int pixel_ofs = 0;
	switch (align) {
		case ALIGN_FILL:
		case ALIGN_LEFT: {
			pixel_ofs = int(style->get_offset().x);
		} break;
		case ALIGN_CENTER: {
			if (window_pos != 0)
				pixel_ofs = int(style->get_offset().x);
			else
				pixel_ofs = int(size.width - (shaped->get_size().width)) / 2;
			if (display_clear_icon)
				pixel_ofs -= int(r_icon_width / 2 + style->get_margin(MARGIN_RIGHT));
		} break;
		case ALIGN_RIGHT: {
			pixel_ofs = int(size.width - style->get_margin(MARGIN_RIGHT) - (shaped->get_size().width));
			if (display_clear_icon)
				pixel_ofs -= int(r_icon_width + style->get_margin(MARGIN_RIGHT));
		} break;
	}
	pixel_ofs -= window_pos;
	set_cursor_position(shaped->hit_test(p_x - pixel_ofs));
}

bool LineEdit::cursor_get_blink_enabled() const {
	return caret_blink_enabled;
}

void LineEdit::cursor_set_blink_enabled(const bool p_enabled) {
	caret_blink_enabled = p_enabled;
	if (p_enabled) {
		caret_blink_timer->start();
	} else {
		caret_blink_timer->stop();
	}
	draw_caret = true;
}

float LineEdit::cursor_get_blink_speed() const {
	return caret_blink_timer->get_wait_time();
}

void LineEdit::cursor_set_blink_speed(const float p_speed) {
	ERR_FAIL_COND(p_speed <= 0);
	caret_blink_timer->set_wait_time(p_speed);
}

void LineEdit::_reset_caret_blink_timer() {
	if (caret_blink_enabled) {
		caret_blink_timer->stop();
		caret_blink_timer->start();
		draw_caret = true;
		update();
	}
}

void LineEdit::_toggle_draw_caret() {
	draw_caret = !draw_caret;
	if (is_visible_in_tree() && has_focus() && window_has_focus) {
		update();
	}
}

void LineEdit::delete_char() {

	if ((text.length() <= 0) || (cursor_pos == 0)) return;

	if ((cursor_pos > 1) && ((text[cursor_pos - 1] & 0xFFFFF800) == 0xD800) && ((text[cursor_pos - 1] & 0x400) != 0) && ((text[cursor_pos - 2] & 0xFFFFF800) == 0xD800) && ((text[cursor_pos - 2] & 0x400) == 0)) {
		text.erase(cursor_pos - 2, 2);
		shape_text();
		set_cursor_position(get_cursor_position() - 2);
	} else {
		text.erase(cursor_pos - 1, 1);
		shape_text();
		set_cursor_position(get_cursor_position() - 1);
	}

	_text_changed();
}

void LineEdit::delete_text(int p_from_column, int p_to_column) {

	text.erase(p_from_column, p_to_column - p_from_column);

	shape_text();

	cursor_pos -= CLAMP(cursor_pos - p_from_column, 0, p_to_column - p_from_column);

	if (cursor_pos >= text.length()) {
		cursor_pos = text.length();
	}
	set_cursor_position(cursor_pos);

	if (!text_changed_dirty) {
		if (is_inside_tree()) {
			MessageQueue::get_singleton()->push_call(this, "_text_changed");
		}
		text_changed_dirty = true;
	}
}

void LineEdit::set_text(String p_text) {

	clear_internal();
	append_at_cursor(p_text);
	update();
	cursor_pos = 0;
	window_pos = 0;
}

void LineEdit::clear() {

	clear_internal();
	_text_changed();
}

String LineEdit::get_text() const {

	return text;
}

void LineEdit::set_display_controls(bool p_enable) {
	if (display_control != p_enable) {
		display_control = p_enable;
		menu->set_item_checked(menu->get_item_index(MENU_DISPLAY_UCC), display_control);
		shape_text();
	}
}

bool LineEdit::get_display_controls() const {
	return display_control;
}

void LineEdit::set_placeholder(String p_text) {

	placeholder = p_text;
	placeholder_translated = tr(placeholder);
	shape_placeholder();
	update();
}

String LineEdit::get_placeholder() const {

	return placeholder;
}

void LineEdit::set_placeholder_alpha(float p_alpha) {

	placeholder_alpha = p_alpha;
	update();
}

float LineEdit::get_placeholder_alpha() const {

	return placeholder_alpha;
}

void LineEdit::set_cursor_position(int p_pos) {

	if (p_pos > (int)text.length())
		p_pos = text.length();

	if (p_pos < 0)
		p_pos = 0;

	if (((text[p_pos] & 0xFFFFF800) == 0xD800) && ((text[p_pos] & 0x400) != 0)) {
		if (p_pos >= cursor_pos) {
			p_pos++;
		} else if ((p_pos > 0) && ((text[p_pos - 1] & 0xFFFFF800) == 0xD800) && ((text[p_pos - 1] & 0x400) == 0)) {
			p_pos--;
		}
	}

	Ref<Font> font = get_font("font");
	if (shaped->require_reload()) const_cast<LineEdit *>(this)->shape_text();

	Ref<StyleBox> style = get_stylebox("normal");
	Size2 size = get_size();

	int ofs_max = size.width - style->get_margin(MARGIN_RIGHT) - style->get_margin(MARGIN_LEFT);

	bool display_clear_icon = !text.empty() && is_editable() && clear_button_enabled;
	if (right_icon.is_valid() || display_clear_icon) {
		Ref<Texture> r_icon = display_clear_icon ? Control::get_icon("clear") : right_icon;
		ofs_max -= r_icon->get_width();
	}

	cursor_pos = p_pos;

	int ltr = shaped->get_ltr_caret_offset(cursor_pos);
	int rtl = shaped->get_rtl_caret_offset(cursor_pos);
	int offs = ((input_direction == TEXT_DIRECTION_LTR) && (ltr != -1)) ? ltr : rtl;

	if (offs - window_pos < 0) window_pos += (offs - window_pos);
	if (offs - window_pos >= ofs_max) window_pos += (offs - window_pos - ofs_max);

	update();
}

int LineEdit::get_cursor_position() const {

	return cursor_pos;
}

void LineEdit::append_at_cursor(String p_text) {

	if ((max_length <= 0) || (text.length() + p_text.length() <= max_length)) {

		String pre = text.substr(0, cursor_pos);
		String post = text.substr(cursor_pos, text.length() - cursor_pos);
		text = pre + p_text + post;

		ERR_FAIL_COND(ShapingServer::get_singleton() == NULL);
		ERR_FAIL_COND(ShapingServer::get_singleton()->get_primary_interface().is_null());

		shape_text();
		TextDirection dir = shaped->get_dominant_direciton_in_range(cursor_pos, p_text.length());
		if (dir != TEXT_DIRECTION_AUTO) {
			input_direction = dir;
		}

		set_cursor_position(cursor_pos + p_text.length());
	}
}

void LineEdit::clear_internal() {

	deselect();
	_clear_undo_stack();
	cursor_pos = 0;
	window_pos = 0;
	undo_text = "";
	text = "";
	input_direction = base_direction;
	shaped = ShapedString::invalid();
	update();
}

Size2 LineEdit::get_minimum_size() const {

	Ref<Font> font = get_font("font");
	if (shaped->require_reload()) const_cast<LineEdit *>(this)->shape_text();

	Ref<StyleBox> style = get_stylebox("normal");

	Size2 min = style->get_minimum_size();
	min.height += shaped->get_size().height;

	//minimum size of text
	int space_size = font->get_string_size(" ").x;
	int mstext = get_constant("minimum_spaces") * space_size;

	if (expand_to_text_length) {
		mstext = MAX(mstext, shaped->get_size().width + space_size); //add a spce because some fonts are too exact, and because cursor needs a bit more when at the end
	}

	min.width += mstext;
	return min;
}

/* selection */

void LineEdit::deselect() {

	selection.begin = 0;
	selection.end = 0;
	selection.cursor_start = 0;
	selection.enabled = false;
	selection.creating = false;
	selection.doubleclick = false;
	update();
}

void LineEdit::selection_delete() {

	if (selection.enabled)
		delete_text(selection.begin, selection.end);

	deselect();
}

void LineEdit::set_max_length(int p_max_length) {

	ERR_FAIL_COND(p_max_length < 0);
	max_length = p_max_length;
	set_text(text);
}

int LineEdit::get_max_length() const {

	return max_length;
}

void LineEdit::selection_fill_at_cursor() {

	selection.begin = cursor_pos;
	selection.end = selection.cursor_start;

	if (selection.end < selection.begin) {
		int aux = selection.end;
		selection.end = selection.begin;
		selection.begin = aux;
	}

	selection.enabled = (selection.begin != selection.end);
}

void LineEdit::select_all() {

	if (!text.length())
		return;

	selection.begin = 0;
	selection.end = text.length();
	selection.enabled = true;
	update();
}

void LineEdit::set_editable(bool p_editable) {

	if (editable == p_editable)
		return;

	editable = p_editable;

	// Reorganize context menu.
	menu->clear();

	if (editable) {
		menu->add_item(RTR("Undo"), MENU_UNDO, KEY_MASK_CMD | KEY_Z);
		menu->add_item(RTR("Redo"), MENU_REDO, KEY_MASK_CMD | KEY_MASK_SHIFT | KEY_Z);
	}

	if (editable) {
		menu->add_separator();
		menu->add_item(RTR("Cut"), MENU_CUT, KEY_MASK_CMD | KEY_X);
	}

	menu->add_item(RTR("Copy"), MENU_COPY, KEY_MASK_CMD | KEY_C);

	if (editable) {
		menu->add_item(RTR("Paste"), MENU_PASTE, KEY_MASK_CMD | KEY_V);
	}

	menu->add_separator();
	menu->add_item(RTR("Select All"), MENU_SELECT_ALL, KEY_MASK_CMD | KEY_A);

	if (editable) {
		menu->add_item(RTR("Clear"), MENU_CLEAR);
	}

	menu->add_separator();
	menu->add_submenu_item(RTR("Text writing direction"), "DirMenu");

	menu->add_separator();
	menu->add_check_item(RTR("Display control characters"), MENU_DISPLAY_UCC);
	if (editable) {
		menu->add_submenu_item(RTR("Insert control character"), "CTLMenu");
	}

	update();
}

bool LineEdit::is_editable() const {

	return editable;
}

void LineEdit::set_secret(bool p_secret) {

	pass = p_secret;
	update();
}

bool LineEdit::is_secret() const {

	return pass;
}

void LineEdit::set_secret_character(const String &p_string) {

	// An empty string as the secret character would crash the engine
	// It also wouldn't make sense to use multiple characters as the secret character
	ERR_FAIL_COND_MSG(p_string.length() != 1, "Secret character must be exactly one character long (" + itos(p_string.length()) + " characters given).");

	secret_character = p_string;
	update();
}

String LineEdit::get_secret_character() const {
	return secret_character;
}

void LineEdit::select(int p_from, int p_to) {

	if (p_from == 0 && p_to == 0) {
		deselect();
		return;
	}

	int len = text.length();
	if (p_from < 0)
		p_from = 0;
	if (p_from > len)
		p_from = len;
	if (p_to < 0 || p_to > len)
		p_to = len;

	if (p_from >= p_to)
		return;

	selection.enabled = true;
	selection.begin = p_from;
	selection.end = p_to;
	selection.creating = false;
	selection.doubleclick = false;
	update();
}

bool LineEdit::is_text_field() const {

	return true;
}

void LineEdit::menu_option(int p_option) {

	switch (p_option) {
		case MENU_CUT: {
			if (editable) {
				cut_text();
			}
		} break;
		case MENU_COPY: {

			copy_text();
		} break;
		case MENU_PASTE: {
			if (editable) {
				paste_text();
			}
		} break;
		case MENU_CLEAR: {
			if (editable) {
				clear();
			}
		} break;
		case MENU_SELECT_ALL: {
			select_all();
		} break;
		case MENU_UNDO: {
			if (editable) {
				undo();
			}
		} break;
		case MENU_REDO: {
			if (editable) {
				redo();
			}
		} break;
		case MENU_DIR_AUTO: {
			set_text_direction(TEXT_DIRECTION_AUTO);
		} break;
		case MENU_DIR_LTR: {
			set_text_direction(TEXT_DIRECTION_LTR);
		} break;
		case MENU_DIR_RTL: {
			set_text_direction(TEXT_DIRECTION_RTL);
		} break;
		case MENU_DISPLAY_UCC: {
			set_display_controls(!get_display_controls());
		} break;
		case MENU_INSERT_LRM: {
			CharType ucodestr[2] = { static_cast<CharType>(0x200E), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_RLM: {
			CharType ucodestr[2] = { static_cast<CharType>(0x200F), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_LRE: {
			CharType ucodestr[2] = { static_cast<CharType>(0x202A), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_RLE: {
			CharType ucodestr[2] = { static_cast<CharType>(0x202B), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_LRO: {
			CharType ucodestr[2] = { static_cast<CharType>(0x202D), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_RLO: {
			CharType ucodestr[2] = { static_cast<CharType>(0x202E), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_PDF: {
			CharType ucodestr[2] = { static_cast<CharType>(0x202C), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_ALM: {
			CharType ucodestr[2] = { static_cast<CharType>(0x061C), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_LRI: {
			CharType ucodestr[2] = { static_cast<CharType>(0x2066), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_RLI: {
			CharType ucodestr[2] = { static_cast<CharType>(0x2067), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_FSI: {
			CharType ucodestr[2] = { static_cast<CharType>(0x2068), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_PDI: {
			CharType ucodestr[2] = { static_cast<CharType>(0x2069), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_ZWJ: {
			CharType ucodestr[2] = { static_cast<CharType>(0x200D), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_ZWNJ: {
			CharType ucodestr[2] = { static_cast<CharType>(0x200C), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_WJ: {
			CharType ucodestr[2] = { static_cast<CharType>(0x2060), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_SHY: {
			CharType ucodestr[2] = { static_cast<CharType>(0x00AD), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_RS: {
			CharType ucodestr[2] = { static_cast<CharType>(0x001E), 0 };
			append_at_cursor(ucodestr);
		} break;
		case MENU_INSERT_US: {
			CharType ucodestr[2] = { static_cast<CharType>(0x001F), 0 };
			append_at_cursor(ucodestr);
		}
	}
}

void LineEdit::set_context_menu_enabled(bool p_enable) {
	context_menu_enabled = p_enable;
}

bool LineEdit::is_context_menu_enabled() {
	return context_menu_enabled;
}

PopupMenu *LineEdit::get_menu() const {
	return menu;
}

void LineEdit::_editor_settings_changed() {
#ifdef TOOLS_ENABLED
	cursor_set_blink_enabled(EDITOR_DEF("text_editor/cursor/caret_blink", false));
	cursor_set_blink_speed(EDITOR_DEF("text_editor/cursor/caret_blink_speed", 0.65));
#endif
}

void LineEdit::set_expand_to_text_length(bool p_enabled) {

	expand_to_text_length = p_enabled;
	minimum_size_changed();
	window_pos = 0;
}

bool LineEdit::get_expand_to_text_length() const {
	return expand_to_text_length;
}

void LineEdit::set_clear_button_enabled(bool p_enabled) {
	clear_button_enabled = p_enabled;
	update();
}

bool LineEdit::is_clear_button_enabled() const {
	return clear_button_enabled;
}

void LineEdit::set_right_icon(const Ref<Texture> &p_icon) {
	if (right_icon == p_icon) {
		return;
	}
	right_icon = p_icon;
	update();
}

void LineEdit::shape_text() {

	Ref<Font> font = get_font("font");
	String display_text = (ime_text.length() > 0) ? text.substr(0, cursor_pos) + ime_text + text.substr(cursor_pos, text.length()) : text;
	//shaped = font->shape_string(display_text, 0, display_text.length(), base_direction, language, ot_features);
	shaped = ShapedString::invalid();
	shaped->set_display_controls(display_control);
	minimum_size_changed();
}

void LineEdit::shape_placeholder() {

	Ref<Font> font = get_font("font");
	//placeholder_shaped = font->shape_string(placeholder_translated, 0, placeholder_translated.length(), base_direction, language, ot_features);
	placeholder_shaped = ShapedString::invalid();
	minimum_size_changed();
}

void LineEdit::_text_changed() {

	_emit_text_change();
	_clear_redo();
}

void LineEdit::_emit_text_change() {
	emit_signal("text_changed", text);
	_change_notify("text");
	text_changed_dirty = false;
}

void LineEdit::_clear_redo() {
	_create_undo_state();
	if (undo_stack_pos == NULL) {
		return;
	}

	undo_stack_pos = undo_stack_pos->next();
	while (undo_stack_pos) {
		List<TextOperation>::Element *elem = undo_stack_pos;
		undo_stack_pos = undo_stack_pos->next();
		undo_stack.erase(elem);
	}
	_create_undo_state();
}

void LineEdit::_clear_undo_stack() {
	undo_stack.clear();
	undo_stack_pos = NULL;
	_create_undo_state();
}

void LineEdit::_create_undo_state() {
	TextOperation op;
	op.text = text;
	op.cursor_pos = cursor_pos;
	undo_stack.push_back(op);
}

void LineEdit::_bind_methods() {

	ClassDB::bind_method(D_METHOD("_text_changed"), &LineEdit::_text_changed);
	ClassDB::bind_method(D_METHOD("_toggle_draw_caret"), &LineEdit::_toggle_draw_caret);

	ClassDB::bind_method("_editor_settings_changed", &LineEdit::_editor_settings_changed);

	ClassDB::bind_method(D_METHOD("set_align", "align"), &LineEdit::set_align);
	ClassDB::bind_method(D_METHOD("get_align"), &LineEdit::get_align);

	ClassDB::bind_method(D_METHOD("_gui_input"), &LineEdit::_gui_input);
	ClassDB::bind_method(D_METHOD("clear"), &LineEdit::clear);
	ClassDB::bind_method(D_METHOD("select", "from", "to"), &LineEdit::select, DEFVAL(0), DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("select_all"), &LineEdit::select_all);
	ClassDB::bind_method(D_METHOD("deselect"), &LineEdit::deselect);
	ClassDB::bind_method(D_METHOD("set_text", "text"), &LineEdit::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &LineEdit::get_text);
	ClassDB::bind_method(D_METHOD("get_display_controls"), &LineEdit::get_display_controls);
	ClassDB::bind_method(D_METHOD("set_display_controls", "enable"), &LineEdit::set_display_controls);
	ClassDB::bind_method(D_METHOD("set_text_direction", "direction"), &LineEdit::set_text_direction);
	ClassDB::bind_method(D_METHOD("get_text_direction"), &LineEdit::get_text_direction);
	ClassDB::bind_method(D_METHOD("set_ot_features", "features"), &LineEdit::set_ot_features);
	ClassDB::bind_method(D_METHOD("get_ot_features"), &LineEdit::get_ot_features);
	ClassDB::bind_method(D_METHOD("set_language", "language"), &LineEdit::set_language);
	ClassDB::bind_method(D_METHOD("get_language"), &LineEdit::get_language);
	ClassDB::bind_method(D_METHOD("set_placeholder", "text"), &LineEdit::set_placeholder);
	ClassDB::bind_method(D_METHOD("get_placeholder"), &LineEdit::get_placeholder);
	ClassDB::bind_method(D_METHOD("set_placeholder_alpha", "alpha"), &LineEdit::set_placeholder_alpha);
	ClassDB::bind_method(D_METHOD("get_placeholder_alpha"), &LineEdit::get_placeholder_alpha);
	ClassDB::bind_method(D_METHOD("set_cursor_position", "position"), &LineEdit::set_cursor_position);
	ClassDB::bind_method(D_METHOD("get_cursor_position"), &LineEdit::get_cursor_position);
	ClassDB::bind_method(D_METHOD("set_expand_to_text_length", "enabled"), &LineEdit::set_expand_to_text_length);
	ClassDB::bind_method(D_METHOD("get_expand_to_text_length"), &LineEdit::get_expand_to_text_length);
	ClassDB::bind_method(D_METHOD("cursor_set_blink_enabled", "enabled"), &LineEdit::cursor_set_blink_enabled);
	ClassDB::bind_method(D_METHOD("cursor_get_blink_enabled"), &LineEdit::cursor_get_blink_enabled);
	ClassDB::bind_method(D_METHOD("cursor_set_blink_speed", "blink_speed"), &LineEdit::cursor_set_blink_speed);
	ClassDB::bind_method(D_METHOD("cursor_get_blink_speed"), &LineEdit::cursor_get_blink_speed);
	ClassDB::bind_method(D_METHOD("set_max_length", "chars"), &LineEdit::set_max_length);
	ClassDB::bind_method(D_METHOD("get_max_length"), &LineEdit::get_max_length);
	ClassDB::bind_method(D_METHOD("append_at_cursor", "text"), &LineEdit::append_at_cursor);
	ClassDB::bind_method(D_METHOD("set_editable", "enabled"), &LineEdit::set_editable);
	ClassDB::bind_method(D_METHOD("is_editable"), &LineEdit::is_editable);
	ClassDB::bind_method(D_METHOD("set_secret", "enabled"), &LineEdit::set_secret);
	ClassDB::bind_method(D_METHOD("is_secret"), &LineEdit::is_secret);
	ClassDB::bind_method(D_METHOD("set_secret_character", "character"), &LineEdit::set_secret_character);
	ClassDB::bind_method(D_METHOD("get_secret_character"), &LineEdit::get_secret_character);
	ClassDB::bind_method(D_METHOD("menu_option", "option"), &LineEdit::menu_option);
	ClassDB::bind_method(D_METHOD("get_menu"), &LineEdit::get_menu);
	ClassDB::bind_method(D_METHOD("set_context_menu_enabled", "enable"), &LineEdit::set_context_menu_enabled);
	ClassDB::bind_method(D_METHOD("is_context_menu_enabled"), &LineEdit::is_context_menu_enabled);
	ClassDB::bind_method(D_METHOD("set_clear_button_enabled", "enable"), &LineEdit::set_clear_button_enabled);
	ClassDB::bind_method(D_METHOD("is_clear_button_enabled"), &LineEdit::is_clear_button_enabled);

	ADD_SIGNAL(MethodInfo("text_changed", PropertyInfo(Variant::STRING, "new_text")));
	ADD_SIGNAL(MethodInfo("text_entered", PropertyInfo(Variant::STRING, "new_text")));

	BIND_ENUM_CONSTANT(ALIGN_LEFT);
	BIND_ENUM_CONSTANT(ALIGN_CENTER);
	BIND_ENUM_CONSTANT(ALIGN_RIGHT);
	BIND_ENUM_CONSTANT(ALIGN_FILL);

	BIND_ENUM_CONSTANT(MENU_CUT);
	BIND_ENUM_CONSTANT(MENU_COPY);
	BIND_ENUM_CONSTANT(MENU_PASTE);
	BIND_ENUM_CONSTANT(MENU_CLEAR);
	BIND_ENUM_CONSTANT(MENU_SELECT_ALL);
	BIND_ENUM_CONSTANT(MENU_UNDO);
	BIND_ENUM_CONSTANT(MENU_REDO);
	BIND_ENUM_CONSTANT(MENU_DIR_AUTO);
	BIND_ENUM_CONSTANT(MENU_DIR_LTR);
	BIND_ENUM_CONSTANT(MENU_DIR_RTL);
	BIND_ENUM_CONSTANT(MENU_DISPLAY_UCC);
	BIND_ENUM_CONSTANT(MENU_INSERT_LRM);
	BIND_ENUM_CONSTANT(MENU_INSERT_RLM);
	BIND_ENUM_CONSTANT(MENU_INSERT_LRE);
	BIND_ENUM_CONSTANT(MENU_INSERT_RLE);
	BIND_ENUM_CONSTANT(MENU_INSERT_LRO);
	BIND_ENUM_CONSTANT(MENU_INSERT_RLO);
	BIND_ENUM_CONSTANT(MENU_INSERT_PDF);
	BIND_ENUM_CONSTANT(MENU_INSERT_ALM);
	BIND_ENUM_CONSTANT(MENU_INSERT_LRI);
	BIND_ENUM_CONSTANT(MENU_INSERT_RLI);
	BIND_ENUM_CONSTANT(MENU_INSERT_FSI);
	BIND_ENUM_CONSTANT(MENU_INSERT_PDI);
	BIND_ENUM_CONSTANT(MENU_INSERT_ZWJ);
	BIND_ENUM_CONSTANT(MENU_INSERT_ZWNJ);
	BIND_ENUM_CONSTANT(MENU_INSERT_WJ);
	BIND_ENUM_CONSTANT(MENU_INSERT_SHY);
	BIND_ENUM_CONSTANT(MENU_INSERT_RS);
	BIND_ENUM_CONSTANT(MENU_INSERT_US);
	BIND_ENUM_CONSTANT(MENU_MAX);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "align", PROPERTY_HINT_ENUM, "Left,Center,Right,Fill"), "set_align", "get_align");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_length"), "set_max_length", "get_max_length");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "editable"), "set_editable", "is_editable");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "secret"), "set_secret", "is_secret");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "secret_character"), "set_secret_character", "get_secret_character");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "expand_to_text_length"), "set_expand_to_text_length", "get_expand_to_text_length");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "focus_mode", PROPERTY_HINT_ENUM, "None,Click,All"), "set_focus_mode", "get_focus_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "context_menu_enabled"), "set_context_menu_enabled", "is_context_menu_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clear_button_enabled"), "set_clear_button_enabled", "is_clear_button_enabled");

	ADD_PROPERTY(PropertyInfo(Variant::INT, "text_direction", PROPERTY_HINT_ENUM, "Auto,LTR,RTL"), "set_text_direction", "get_text_direction");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "ot_features"), "set_ot_features", "get_ot_features");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "language"), "set_language", "get_language");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "display_controls"), "set_display_controls", "get_display_controls");
	ADD_GROUP("Placeholder", "placeholder_");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "placeholder_text"), "set_placeholder", "get_placeholder");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "placeholder_alpha", PROPERTY_HINT_RANGE, "0,1,0.001"), "set_placeholder_alpha", "get_placeholder_alpha");
	ADD_GROUP("Caret", "caret_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "caret_blink"), "cursor_set_blink_enabled", "cursor_get_blink_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "caret_blink_speed", PROPERTY_HINT_RANGE, "0.1,10,0.01"), "cursor_set_blink_speed", "cursor_get_blink_speed");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "caret_position"), "set_cursor_position", "get_cursor_position");
}

LineEdit::LineEdit() {

	undo_stack_pos = NULL;
	_create_undo_state();
	align = ALIGN_LEFT;

	shaped = ShapedString::invalid();
	placeholder_shaped = ShapedString::invalid();

	base_direction = TEXT_DIRECTION_AUTO;
	input_direction = TEXT_DIRECTION_LTR;

	cursor_pos = 0;
	window_pos = 0;
	window_has_focus = true;
	max_length = 0;
	pass = false;
	display_control = false;
	secret_character = "*";
	text_changed_dirty = false;
	placeholder_alpha = 0.6;
	clear_button_enabled = false;
	clear_button_status.press_attempt = false;
	clear_button_status.pressing_inside = false;

	deselect();
	set_focus_mode(FOCUS_ALL);
	set_default_cursor_shape(CURSOR_IBEAM);
	set_mouse_filter(MOUSE_FILTER_STOP);

	draw_caret = true;
	caret_blink_enabled = false;
	caret_blink_timer = memnew(Timer);
	add_child(caret_blink_timer);
	caret_blink_timer->set_wait_time(0.65);
	caret_blink_timer->connect("timeout", this, "_toggle_draw_caret");
	cursor_set_blink_enabled(false);

	context_menu_enabled = true;
	menu = memnew(PopupMenu);
	add_child(menu);
	menu->connect("id_pressed", this, "menu_option");

	menu_dir = memnew(PopupMenu);
	menu_dir->set_name("DirMenu");
	menu_dir->add_radio_check_item(RTR("Detect direction automatically"), MENU_DIR_AUTO);
	menu_dir->add_radio_check_item(RTR("Left-to-right"), MENU_DIR_LTR);
	menu_dir->add_radio_check_item(RTR("Right-to-left"), MENU_DIR_RTL);
	menu->add_child(menu_dir);
	menu_dir->connect("id_pressed", this, "menu_option");

	menu_ctl = memnew(PopupMenu);
	menu_ctl->set_name("CTLMenu");
	menu_ctl->add_item(RTR("Left-to-right mark (LRM)"), MENU_INSERT_LRM);
	menu_ctl->add_item(RTR("Right-to-left mark (RLM)"), MENU_INSERT_RLM);
	menu_ctl->add_item(RTR("Start of left-to-right embedding (LRE)"), MENU_INSERT_LRE);
	menu_ctl->add_item(RTR("Start of right-to-left embedding (RLE)"), MENU_INSERT_RLE);
	menu_ctl->add_item(RTR("Start of left-to-right override (LRO)"), MENU_INSERT_LRO);
	menu_ctl->add_item(RTR("Start of right-to-left override (RLO)"), MENU_INSERT_RLO);
	menu_ctl->add_item(RTR("Pop direction formatting (PDF)"), MENU_INSERT_PDF);
	menu_ctl->add_separator();
	menu_ctl->add_item(RTR("Arabic letter mark (ALM)"), MENU_INSERT_ALM);
	menu_ctl->add_item(RTR("Left-to-right isolate (LRI)"), MENU_INSERT_LRI);
	menu_ctl->add_item(RTR("Right-to-left isolate (RLI)"), MENU_INSERT_RLI);
	menu_ctl->add_item(RTR("First strong isolate (FSI)"), MENU_INSERT_FSI);
	menu_ctl->add_item(RTR("Pop direction isolate (PDI)"), MENU_INSERT_PDI);
	menu_ctl->add_separator();
	menu_ctl->add_item(RTR("Zero width joiner (ZWJ)"), MENU_INSERT_ZWJ);
	menu_ctl->add_item(RTR("Zero width non-joiner (ZWNJ)"), MENU_INSERT_ZWNJ);
	menu_ctl->add_item(RTR("Word joiner (WJ)"), MENU_INSERT_WJ);
	menu_ctl->add_item(RTR("Soft hyphen (SHY)"), MENU_INSERT_SHY);
	menu_ctl->add_separator();
	menu_ctl->add_item(RTR("Block separator (RS)"), MENU_INSERT_RS);
	menu_ctl->add_item(RTR("Segment separator (US)"), MENU_INSERT_US);
	menu->add_child(menu_ctl);
	menu_ctl->connect("id_pressed", this, "menu_option");

	menu_dir->set_item_checked(menu_dir->get_item_index(MENU_DIR_AUTO), true);

	editable = false; // initialise to opposite first, so we get past the early-out in set_editable
	set_editable(true);
	expand_to_text_length = false;
}

LineEdit::~LineEdit() {
}
