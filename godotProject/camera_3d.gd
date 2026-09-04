extends Camera3D

@export_group("Movement")
@export var movement_speed: float = 10.0
@export var acceleration: float = 8.0
@export var deceleration: float = 10.0

@export_group("Mouse Look")
@export var mouse_sensitivity: float = 0.15
@export var tilt_upper_limit: float = 90.0
@export var tilt_lower_limit: float = -90.0

var _velocity: Vector3 = Vector3.ZERO
var _rotation_input: float
var _tilt_input: float
var _mouse_rotation: Vector3

func _ready():
	# Automatically captures mouse when the game starts
	Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
	_mouse_rotation = rotation_degrees

func _input(event):
	# Handle mouse movement
	if event is InputEventMouseMotion and Input.get_mouse_mode() == Input.MOUSE_MODE_CAPTURED:
		_rotation_input = -event.relative.x * mouse_sensitivity
		_tilt_input = -event.relative.y * mouse_sensitivity

	# Toggle mouse capture with ESCAPE (No Input Map needed)
	if event is InputEventKey and event.pressed and event.keycode == KEY_ESCAPE:
		if Input.get_mouse_mode() == Input.MOUSE_MODE_CAPTURED:
			Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
		else:
			Input.mouse_mode = Input.MOUSE_MODE_CAPTURED

func _process(delta):
	if Input.get_mouse_mode() == Input.MOUSE_MODE_CAPTURED:
		_update_rotation(delta)
		_update_movement(delta)

func _update_rotation(_delta):
	_mouse_rotation.x += _tilt_input
	_mouse_rotation.x = clamp(_mouse_rotation.x, tilt_lower_limit, tilt_upper_limit)
	_mouse_rotation.y += _rotation_input
	
	rotation_degrees = _mouse_rotation
	
	_rotation_input = 0.0
	_tilt_input = 0.0

func _update_movement(delta):
	var input_dir = Vector3.ZERO
	
	# Using specific KEY constants so no Project Settings are required
	if Input.is_key_pressed(KEY_W): input_dir.z -= 1
	if Input.is_key_pressed(KEY_S): input_dir.z += 1
	if Input.is_key_pressed(KEY_A): input_dir.x -= 1
	if Input.is_key_pressed(KEY_D): input_dir.x += 1
	if Input.is_key_pressed(KEY_E): input_dir.y += 1
	if Input.is_key_pressed(KEY_Q): input_dir.y -= 1
	
	# Normalizing prevents diagonal movement from being faster
	var direction = (transform.basis * input_dir.normalized())
	
	if input_dir.length() > 0:
		_velocity = _velocity.lerp(direction * movement_speed, acceleration * delta)
	else:
		_velocity = _velocity.lerp(Vector3.ZERO, deceleration * delta)
	
	global_translate(_velocity * delta)
