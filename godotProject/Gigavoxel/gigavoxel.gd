extends Node

@export var cam : Camera3D
@export var light : OmniLight3D
@export var world_environment : WorldEnvironment

var renderer : Gigavoxel
var effect : GigavoxelCompositor

func _ready() -> void:
	#renderer = Gigavoxel.new()
	#renderer.initialize()
	
	# Create and inject the effect
	#effect = GigavoxelCompositor.new()
	#effect.renderer = renderer
	
	# Add it to the active compositor
	#var compositor = world_environment.compositor
	#var effects_list = compositor.compositor_effects
	#effects_list.append(effect)
	#compositor.compositor_effects = effects_list
	pass

func _process(_delta: float) -> void:
	# Feed the latest data to the effect so the Render Thread can use it
	#effect.cam_transform = cam.global_transform
	#effect.light_pos = light.position
	pass
