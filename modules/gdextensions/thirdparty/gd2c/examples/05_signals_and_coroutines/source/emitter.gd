extends Node

# Example: signals, emit, connect

signal health_changed(new_health)
signal died()

var health: int = 100

func take_damage(amount: int):
	health -= amount
	emit_signal("health_changed", health)
	if health <= 0:
		health = 0
		emit_signal("died")

# NOTE: yield/coroutines are detected by gd2c but the
# make_coroutine transform is NOT YET IMPLEMENTED.
# The following would fail at compile time:
#
# func async_example():
#     yield(get_tree().create_timer(1.0), "timeout")
#     print("1 second later")
