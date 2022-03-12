## Collection of simple geometric/vector fonts

_Godot_ interface to collection of simple/basic geometric/vector fonts (like _stb_easy_font_).
_Note_, that some variants of display might require my fork of _Godot_ since I have extended canvas item coordinates from ```Vector2``` to ```Vector3``` (for better mesh integration).

Main class responsible for mesh generatio is ```GdGeomFonts```. As this class is not a ```Node```-based it needs to be properly integrated with eg. ```_draw``` method. Simple example of usage _GdGeomFonts_ class:

```
extends Node2D

var font:GdGeomFonts = GdGeomFonts.new()
var rot:float = 0
var animate:bool = false

func _ready():
	animate = $toggle_animate.pressed

func _draw():
	font.clear()
	font.set_transform(Transform2D().rotated(rot))
	font.easy_font_add_text("0123456789", Vector2(5,5))
	font.easy_font_add_text("ABC DEF GHI JKL", Vector2(5,14))
	font.easy_font_add_text_xform("SCALED STB EASY FONT", Transform().scaled(Vector3(2,4,1)), Vector2(5,23))
	font.bob_font_add_text("GALACTIC FORTRESS", Vector3(50,50,0), 5)
	font.bob_font_add_text_xform("FONT 3D", Transform().rotated(Vector3(1,1,0).normalized(), PI/5), Vector3(75,125,0), 10, false)
	font.bob_font_add_text_xform("FONT 3D", Transform().rotated(Vector3(1,1,0).normalized(), PI/5), Vector3(75,125,0), 10, false)
	font.easy_font_add_text_xform("SCALED STB EASY FONT", Transform().scaled(Vector3(2,3,1)).rotated(Vector3(1,1,0).normalized(), -PI/4), Vector2(5,100))
	font.easy_font_add_text_xform("SCALED STB EASY FONT", Transform().scaled(Vector3(4,4,1)).rotated(Vector3(1,1,0).normalized(), -PI/4), Vector2(5,60))
	font.easy_font_add_text_xform("SCALED STB EASY FONT", Transform().scaled(Vector3(4,6,1)).rotated(Vector3(1,1,0).normalized(), -PI/4), Vector2(5,60))
	font.finish()
	pass

func _timer():
	if animate:
		rot += deg2rad(0.5)
	update()

func _toggle_anim(state):
	animate = state
```
