Some improvements of _gdscript_ taken from https://github.com/dbsGen/my_godot_modules:

```
	func _ready():

		# Easier connection
		# -----------------
		self.connect("renamed", on_renamed)

		# Lambda expressions
		# ------------------
		var hello = "hello"
		var lambda_func = func():
			print(hello)

		# or
		# --
		func lambda_func2():
			print(hello)

		# Lambda_func is a function object
		# --------------------------------
		lambda_func.apply()
		lambda_func2.apply()

		# A native function object
		# ------------------------
		var native_func = self.set_name
		native_func.apply("new_name")


	func on_renamed():
		print(get_name())
```
