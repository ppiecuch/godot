extends Node

# Example: Array, Dictionary, for-in, indexing

func array_basics() -> int:
	var arr = [1, 2, 3, 4, 5]
	arr.append(6)
	var sum = 0
	for item in arr:
		sum += item
	return sum

func array_construct():
	var empty = []
	var typed = [1, "two", 3.0, true]
	var nested = [[1, 2], [3, 4]]
	print(empty.size(), typed.size(), nested.size())

func dict_basics() -> String:
	var data = {
		"name": "Goblin",
		"health": 50,
		"damage": 10
	}
	data["armor"] = 5
	var name = data["name"]
	return name

func dict_iteration():
	var scores = {"alice": 100, "bob": 85, "charlie": 92}
	for key in scores:
		print(key, ": ", scores[key])

func array_slice():
	var arr = [0, 1, 2, 3, 4, 5]
	var first_three = arr.slice(0, 2)
	var last_two = arr.slice(4, 5)
	print(first_three, last_two)

func array_of_vectors():
	var points = [
		Vector2(0, 0),
		Vector2(10, 0),
		Vector2(10, 10),
		Vector2(0, 10),
	]
	var center = Vector2.ZERO
	for p in points:
		center += p
	center /= points.size()
	print("Center: ", center)
