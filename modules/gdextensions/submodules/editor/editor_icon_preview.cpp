/**************************************************************************/
/*  editor_icon_preview.cpp                                               */
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

#include "scene/resources/resource_format_text.h"

const String SELECT_ICON_MSG = "Select any icon.";
const String ICON_SIZE_MSG = "Icon size: ";
const String NUMBER_ICONS_MSG = "Found: ";
const String SNIPPET_TEMPLATE = "get_icon(\"%s\", \"EditorIcons\")";

const int MIN_ICON_SIZE = 16;
const int MAX_ICON_SIZE = 128;

// UI description
static const char *_ui = R"(
[gd_scene load_steps=2 format=2]

[node name="window" type="AcceptDialog"]
margin_right = 300.0
margin_bottom = 240.0
rect_min_size = Vector2( 600, 480 )
focus_mode = 2
window_title = "Editor Icons"
resizable = true

[node name="body" type="VBoxContainer" parent="."]
margin_left = 8.0
margin_top = 8.0
margin_right = 592.0
margin_bottom = 444.0
size_flags_horizontal = 3
size_flags_vertical = 3
custom_constants/separation = 16
__meta__ = {
"_edit_use_anchors_": false
}

[node name="search" type="HBoxContainer" parent="body"]
margin_right = 584.0
margin_bottom = 24.0

[node name="box" type="LineEdit" parent="body/search"]
margin_right = 484.0
margin_bottom = 24.0
size_flags_horizontal = 3
placeholder_text = "Search icons..."
caret_blink = true
caret_blink_speed = 0.5

[node name="found" type="LineEdit" parent="body/search"]
margin_left = 488.0
margin_right = 584.0
margin_bottom = 24.0
rect_min_size = Vector2( 96, 0 )
focus_mode = 0
text = "Found: 0"
editable = false

[node name="icons" type="HSplitContainer" parent="body"]
margin_top = 40.0
margin_right = 584.0
margin_bottom = 456.0
size_flags_horizontal = 3
size_flags_vertical = 3
dragger_visibility = 1

[node name="previews" type="ScrollContainer" parent="body/icons"]
margin_right = 396.0
margin_bottom = 416.0
size_flags_horizontal = 3
size_flags_vertical = 3
__meta__ = {
"_edit_use_anchors_": false
}

[node name="container" type="GridContainer" parent="body/icons/previews"]
margin_right = 396.0
margin_bottom = 416.0
size_flags_horizontal = 3
size_flags_vertical = 3
columns = 18

[node name="info" type="PanelContainer" parent="body/icons"]
self_modulate = Color( 0.615686, 0.615686, 0.615686, 1 )
margin_left = 408.0
margin_right = 584.0
margin_bottom = 416.0
rect_min_size = Vector2( 176, 0 )
size_flags_vertical = 3

[node name="icon" type="VBoxContainer" parent="body/icons/info"]
margin_left = 7.0
margin_top = 7.0
margin_right = 169.0
margin_bottom = 409.0
size_flags_horizontal = 3
size_flags_vertical = 3
size_flags_stretch_ratio = 0.25

[node name="label" type="Label" parent="body/icons/info/icon"]
margin_right = 162.0
margin_bottom = 14.0
text = "Select any icon."
autowrap = true
__meta__ = {
"_edit_use_anchors_": false
}

[node name="preview" type="TextureRect" parent="body/icons/info/icon"]
margin_top = 18.0
margin_right = 128.0
margin_bottom = 146.0
rect_min_size = Vector2( 128, 128 )
size_flags_horizontal = 0
size_flags_vertical = 0
size_flags_stretch_ratio = 0.25
expand = true
stretch_mode = 5

[node name="size" type="Label" parent="body/icons/info/icon"]
margin_top = 150.0
margin_right = 162.0
margin_bottom = 164.0
autowrap = true
__meta__ = {
"_edit_use_anchors_": false
}

[node name="copied" type="Label" parent="body/icons/info/icon"]
visible = false
margin_top = 3.0
margin_right = 223.0
margin_bottom = 28.0
text = "Copied to clipboard!"
autowrap = true

[node name="space" type="Control" parent="body/icons/info/icon"]
margin_top = 168.0
margin_right = 162.0
margin_bottom = 340.0
size_flags_horizontal = 3
size_flags_vertical = 3

[node name="params" type="VBoxContainer" parent="body/icons/info/icon"]
margin_top = 344.0
margin_right = 162.0
margin_bottom = 378.0

[node name="icon" type="Label" parent="body/icons/info/icon/params"]
margin_right = 162.0
margin_bottom = 14.0
text = "Icons preview size"
__meta__ = {
"_edit_use_anchors_": false
}

[node name="size" type="HBoxContainer" parent="body/icons/info/icon/params"]
margin_top = 18.0
margin_right = 162.0
margin_bottom = 34.0

[node name="range" type="HSlider" parent="body/icons/info/icon/params/size"]
margin_right = 138.0
margin_bottom = 16.0
focus_mode = 0
size_flags_horizontal = 3
min_value = 16.0
max_value = 128.0
value = 16.0
__meta__ = {
"_edit_use_anchors_": false
}

[node name="pixels" type="Label" parent="body/icons/info/icon/params/size"]
margin_left = 142.0
margin_top = 1.0
margin_right = 162.0
margin_bottom = 15.0
text = "16 px"
__meta__ = {
"_edit_use_anchors_": false
}

[node name="save" type="Button" parent="body/icons/info/icon"]
margin_top = 382.0
margin_right = 162.0
margin_bottom = 402.0
text = "Save"

[connection signal="about_to_show" from="." to="." method="_on_window_about_to_show"]
[connection signal="popup_hide" from="." to="." method="_on_window_popup_hide"]
[connection signal="resized" from="." to="." method="_on_window_resized"]
[connection signal="visibility_changed" from="." to="." method="_on_window_visibility_changed"]
[connection signal="text_changed" from="body/search/box" to="." method="_on_search_text_changed"]
[connection signal="mouse_exited" from="body/icons/previews/container" to="." method="_on_container_mouse_exited"]
[connection signal="value_changed" from="body/icons/info/icon/params/size/range" to="." method="_on_size_changed"]
[connection signal="pressed" from="body/icons/info/icon/save" to="." method="_on_save_pressed"]
)";

static RES _load_from_data(const String &p_data, const String &p_path) {
	ResourceFormatLoaderText rl;
	return rl.load_from_data(p_data, p_path);
}
