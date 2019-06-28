/*************************************************************************/
/*  test_shaping_layout.cpp                                              */
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

#include "test_shaping_layout.h"

#include "core/io/image_loader.h"
#include "core/os/os.h"
#include "core/print_string.h"

#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/color_rect.h"
#include "scene/gui/file_dialog.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/panel.h"
#include "scene/gui/slider.h"
#include "scene/gui/spin_box.h"

#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"

#include "servers/shaping/shaping_interface.h"
#include "servers/shaping/text_layout.h"
#include "servers/shaping_server.h"

namespace TestShapingLayout {

class TestNode : public ColorRect {
	GDCLASS(TestNode, ColorRect);

	OptionButton *font_select;
	OptionButton *sample_select;
	HSlider *wslider;

	Ref<TextLayout> layout;

	Vector<String> samples;
	Vector<Ref<Font>> fonts;

protected:
	void _font_change(int p_item_idx) {
		ERR_FAIL_COND(p_item_idx < 0 || p_item_idx >= fonts.size());
		layout->add_attribute(TEXT_ATTRIB_FONT, fonts[p_item_idx], 0, layout->get_text().length());

		update();
	}

	void _sample_change(int p_item_idx) {
		ERR_FAIL_COND(p_item_idx < 0 || p_item_idx >= samples.size());
		layout->set_text(samples[p_item_idx]);
		layout->add_attribute(TEXT_ATTRIB_FONT, fonts[font_select->get_selected()], 0, samples[p_item_idx].length());

		update();
	}

	void _width_change(float p_value) {

		layout->set_target_size(Size2(p_value, p_value));

		update();
	}

	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("_font_change", "item_idx"), &TestNode::_font_change);
		ClassDB::bind_method(D_METHOD("_sample_change", "item_idx"), &TestNode::_sample_change);

		ClassDB::bind_method(D_METHOD("_width_change", "value"), &TestNode::_width_change);
	}

	virtual void _gui_input(const Ref<InputEvent> &p_gui_input) {
		//TODO
	}

	virtual void _notification(int p_notification) {
		if (p_notification == NOTIFICATION_ENTER_TREE) {
			//get_tree()->get_root()->connect("size_changed", this, "_on_resize");
		}
		if (p_notification == NOTIFICATION_DRAW) {
			RID ci = get_canvas_item();
			VisualServer::get_singleton()->canvas_item_add_rect(ci, Rect2(Point2(), OS::get_singleton()->get_window_size()), Color(0.3, 0.3, 0.3, 1));
			layout->draw(ci, Point2(50, 250), false);
		}
	}

public:
	TestNode() {
		set_frame_color(Color(0.3, 0.3, 0.3, 1));

		String test_data;
		List<String> args = OS::get_singleton()->get_cmdline_args();
		for (int i = 0; i < args.size(); i++) {
			if (args[i].begins_with("--test_data")) {
				Vector<String> arg_tokens = args[i].split("=");
				if (arg_tokens.size() == 2) {
					test_data = arg_tokens[1];
				}
			}
		}

		Panel *top_menu = memnew(Panel);
		top_menu->set_anchor(MARGIN_RIGHT, Control::ANCHOR_END);
		top_menu->set_margin(MARGIN_BOTTOM, 55.0);
		add_child(top_menu);

		VBoxContainer *main = memnew(VBoxContainer);
		top_menu->add_child(main);

		HBoxContainer *font_options = memnew(HBoxContainer);
		font_options->set_h_size_flags(SIZE_EXPAND_FILL);
		main->add_child(font_options);

		//Label *font_select_lbl = memnew(Label);
		//font_select_lbl->set_text("Font:");
		//font_select_lbl->set_valign(Label::VALIGN_CENTER);
		//font_options->add_child(font_select_lbl);

		font_select = memnew(OptionButton);
		DirAccess *da = DirAccess::open(test_data.plus_file("fonts"));
		fonts.push_back(get_font("font"));
		font_select->add_item("[B] default font");
		if (da) {
			da->list_dir_begin();
			String file_name = da->get_next();
			while (file_name != String()) {
				if (!file_name.begins_with(".") && da->current_is_dir()) {
					String dp = test_data.plus_file("fonts").plus_file(file_name);
					DirAccess *daf = DirAccess::open(dp);
					Ref<Font> font;
					font.instance();
					if (daf) {
						daf->list_dir_begin();
						String data_name = daf->get_next();
						while (data_name != String()) {
							if (!data_name.begins_with(".") && !daf->current_is_dir() && (data_name.get_extension() == "ttf" || data_name.get_extension() == "otf" || data_name.get_extension() == "fon")) {
								Ref<FontData> data;
								data.instance();
								data->set_font_data_path(dp.plus_file(data_name));
								font->add_font_data(data);
							}
							data_name = daf->get_next();
						}
					}
					if (font->get_font_data_count() > 0) {
						fonts.push_back(font);
						font_select->add_item("[E] " + file_name);
					}
				}
				file_name = da->get_next();
			}
			memdelete(da);
		}
		font_select->connect("item_selected", this, "_font_change");
		font_select->set_tooltip("Font");
		font_options->add_child(font_select);

		sample_select = memnew(OptionButton);
		//add builtin samples
		samples.push_back("Test test test");
		sample_select->add_item("[B] Test");

		//add external samples
		da = DirAccess::open(test_data.plus_file("samples"));
		if (da) {
			da->list_dir_begin();
			String file_name = da->get_next();
			while (file_name != String()) {
				if (!file_name.begins_with(".") && !da->current_is_dir() && file_name.get_extension() == "txt") {
					String data = FileAccess::get_file_as_string(test_data.plus_file("samples").plus_file(file_name));
					if (!data.empty()) {
						samples.push_back(data);
						sample_select->add_item("[E] " + file_name.get_basename());
					}
				}
				file_name = da->get_next();
			}
			memdelete(da);
		}
		sample_select->connect("item_selected", this, "_sample_change");
		font_options->add_child(sample_select);

		wslider = memnew(HSlider);
		wslider->set_custom_minimum_size(Size2(200, 20));
		wslider->set_max(1000);
		wslider->set_min(50);
		wslider->set_step(5);
		wslider->set_value(300);
		wslider->connect("value_changed", this, "_width_change");
		font_options->add_child(wslider);

		layout.instance();
		layout->set_text(samples[0]);
		layout->add_attribute(TEXT_ATTRIB_FONT, get_font("font"), 0, samples[0].length());
	}

	~TestNode() {
		//meh
	}
};

class TestMainLoop : public SceneTree {

	virtual void request_quit() {

		quit();
	}

	virtual void init() {

		SceneTree::init();

		TestNode *frame = memnew(TestNode);
		frame->set_anchor(MARGIN_RIGHT, Control::ANCHOR_END);
		frame->set_anchor(MARGIN_BOTTOM, Control::ANCHOR_END);
		frame->set_end(Point2(0, 0));

		Ref<Theme> t = memnew(Theme);
		frame->set_theme(t);

		get_root()->add_child(frame);
	}
};

MainLoop *test() {

	return memnew(TestMainLoop);
}

} // namespace TestShapingLayout
