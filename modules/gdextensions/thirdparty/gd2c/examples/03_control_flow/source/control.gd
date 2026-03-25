extends Node

# Example: all control flow constructs

func if_elif_else(x: int) -> String:
	if x > 0:
		return "positive"
	elif x < 0:
		return "negative"
	else:
		return "zero"

func while_loop(n: int) -> int:
	var sum = 0
	var i = 0
	while i < n:
		sum += i
		i += 1
	return sum

func for_range(n: int) -> int:
	var sum = 0
	for i in range(n):
		sum += i
	return sum

func for_array() -> int:
	var items = [10, 20, 30]
	var total = 0
	for item in items:
		total += item
	return total

func match_example(cmd: String) -> int:
	match cmd:
		"attack":
			return 1
		"defend":
			return 2
		"heal":
			return 3
		_:
			return 0

func early_return(x: int) -> int:
	if x < 0:
		return -1
	var result = x * 2
	return result

func nested_loops() -> int:
	var count = 0
	for i in range(3):
		for j in range(3):
			if i == j:
				continue
			count += 1
	return count  # should be 6

func loop_with_break() -> int:
	var result = 0
	for i in range(100):
		if i >= 5:
			break
		result += i
	return result  # should be 10 (0+1+2+3+4)
