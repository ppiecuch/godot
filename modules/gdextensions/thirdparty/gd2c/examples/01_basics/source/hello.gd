extends Node

# Example: basic variable declarations, arithmetic, function calls, print

var health: int = 100
var speed: float = 3.5
var name: String = "Player"

func _ready():
	print("Hello from ", name)
	print("Health: ", health)

func add(a: int, b: int) -> int:
	return a + b

func take_damage(amount: int):
	health -= amount
	if health <= 0:
		health = 0
		print(name, " has died!")

func heal(amount: int):
	health += amount
	if health > 100:
		health = 100

func get_info() -> String:
	return name + " HP:" + str(health)
