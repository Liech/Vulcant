extends Node

var  gen : SphereExample;
@export var cam : Camera3D;
@export var light : OmniLight3D;
var tex : Texture2DRD;

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	gen = SphereExample.new()
	gen.initialize(Vector2i(400,400))
	tex = gen.get_result_texture()
	$TextureRect.texture = tex
	pass
	
# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta: float) -> void:
	gen.update_view(cam.global_transform,light.position)
	gen.render()
	$TextureRect.texture = tex
	pass
