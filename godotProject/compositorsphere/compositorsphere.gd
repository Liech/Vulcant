extends Node

@export var cam : Camera3D
@export var light : OmniLight3D
@export var world_environment : WorldEnvironment

var gen : SphereExample
var effect : RaycastCompositor

func _ready() -> void:
	gen = SphereExample.new()
	gen.initialize(Vector2i(400, 400))
	
	# Create and inject the effect
	effect = RaycastCompositor.new()
	effect.raycaster = gen
	
	# Add it to the active compositor
	var compositor = world_environment.compositor
	var effects_list = compositor.compositor_effects
	effects_list.append(effect)
	compositor.compositor_effects = effects_list
	print("Effect added. Array size: ", world_environment.compositor.compositor_effects.size())

func _process(_delta: float) -> void:
	# Feed the latest data to the effect so the Render Thread can use it
	effect.cam_transform = cam.global_transform
	effect.light_pos = light.position
