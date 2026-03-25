extends Node

# Example: common game pattern — state machine
# Shows enums, match, member variables, method dispatch

enum State { IDLE, RUNNING, JUMPING, FALLING, DEAD }

var current_state: int = State.IDLE
var velocity: Vector2 = Vector2.ZERO
var is_grounded: bool = true

const SPEED = 200.0
const JUMP_FORCE = -400.0
const GRAVITY = 980.0

func _physics_process(delta: float):
	match current_state:
		State.IDLE:
			_state_idle(delta)
		State.RUNNING:
			_state_running(delta)
		State.JUMPING:
			_state_jumping(delta)
		State.FALLING:
			_state_falling(delta)
		State.DEAD:
			pass

func _state_idle(_delta: float):
	velocity.x = 0
	if Input.is_action_pressed("move_right") or Input.is_action_pressed("move_left"):
		current_state = State.RUNNING
	if Input.is_action_just_pressed("jump") and is_grounded:
		current_state = State.JUMPING
		velocity.y = JUMP_FORCE

func _state_running(delta: float):
	if Input.is_action_pressed("move_right"):
		velocity.x = SPEED
	elif Input.is_action_pressed("move_left"):
		velocity.x = -SPEED
	else:
		current_state = State.IDLE
	if Input.is_action_just_pressed("jump") and is_grounded:
		current_state = State.JUMPING
		velocity.y = JUMP_FORCE
	if not is_grounded:
		current_state = State.FALLING

func _state_jumping(delta: float):
	velocity.y += GRAVITY * delta
	if velocity.y > 0:
		current_state = State.FALLING

func _state_falling(delta: float):
	velocity.y += GRAVITY * delta
	if is_grounded:
		if abs(velocity.x) > 0.1:
			current_state = State.RUNNING
		else:
			current_state = State.IDLE
