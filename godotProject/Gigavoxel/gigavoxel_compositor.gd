@tool
class_name GigavoxelCompositor extends CompositorEffect

var renderer : Gigavoxel
var rd : RenderingDevice
var current_res : Vector2i = Vector2i(0, 0)

func _init() -> void:
	effect_callback_type = CompositorEffect.EFFECT_CALLBACK_TYPE_POST_TRANSPARENT
	enabled = true
	
func _setup_compute():
	rd = RenderingServer.get_rendering_device()
	if not rd: 
		return false
	
	renderer = Gigavoxel.new()
	renderer.initialize()
	var filename = "C:/Users/nicol/Downloads/sphere.vdb";
	renderer.load_svo(filename)
	return true

func _render_callback(callback_type: int, render_data: RenderData) -> void:
	if rd == null:
		if not _setup_compute(): return
		
	if not renderer or callback_type != EFFECT_CALLBACK_TYPE_POST_TRANSPARENT: 
		return

	rd.capture_timestamp("Compositor_Start")

	var scene_buffers = render_data.get_render_scene_buffers()
	var screen_tex = scene_buffers.get_color_layer(0)
	var format = rd.texture_get_format(screen_tex);
	var size = Vector2i(format.get_width(),format.get_height());
	var scene_data : RenderSceneDataRD = render_data.get_render_scene_data()
	
	if not scene_data: 
		return
	
	renderer.set_scene(scene_data)
	var lights := Lightkeeper.get_all();
	if (lights.size() > 0):
		renderer.set_light(lights)
	renderer.set_resolution(size)	
	#renderer.render()	
	
	rd.capture_timestamp("After_CPP_Renderer")

	#renderer.blit_depth(render_data)
	renderer.render_svo(render_data)

func _notification(_what):
	pass
