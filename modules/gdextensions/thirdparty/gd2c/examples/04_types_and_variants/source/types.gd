extends Node

# Example: typed variables, casts, type checks, variant operations

var pos: Vector2 = Vector2(10, 20)
var label: String = "entity"
var active: bool = true
var data: Dictionary = {}

func type_checks(obj) -> String:
	if obj is Node:
		return "it's a Node"
	if obj is Vector2:
		return "it's a Vector2"
	return "unknown type"

func construct_types():
	var v2 = Vector2(1.0, 2.0)
	var v3 = Vector3(1.0, 2.0, 3.0)
	var col = Color(1, 0, 0, 1)
	var rect = Rect2(0, 0, 100, 100)
	var t2d = Transform2D()
	print(v2, v3, col, rect, t2d)

func cast_example(value) -> int:
	var result: int = int(value)
	return result

func string_operations() -> String:
	var greeting = "Hello"
	var target = "World"
	var msg = greeting + " " + target + "!"
	var upper = msg.to_upper()
	var length = msg.length()
	print("Length: ", length)
	return upper

func vector_math() -> Vector2:
	var a = Vector2(3, 4)
	var b = Vector2(1, 0)
	var result = a + b * 2.0
	var normalized = result.normalized()
	var dot = a.dot(b)
	print("Dot product: ", dot)
	return normalized
