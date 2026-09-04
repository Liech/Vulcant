@tool
class_name RedTintEffect extends CompositorEffect

var rd: RenderingDevice
var shader: RID
var pipeline: RID

func _init() -> void:
	rd = RenderingServer.get_rendering_device()
	if !rd: return
	var shader_file = load("res://red_tint/red_tint.glsl")
	var spirv = shader_file.get_spirv()
	shader = rd.shader_create_from_spirv(spirv)
	pipeline = rd.compute_pipeline_create(shader)

func _notification(what):
	if what == NOTIFICATION_PREDELETE and shader.is_valid():
		rd.free_rid(shader) # Pipeline is freed automatically with shader

func _render_callback(_type: int, render_data: RenderData) -> void:
	var scene_buffers: RenderSceneBuffersRD = render_data.get_render_scene_buffers()
	if !scene_buffers: return
	
	var size = scene_buffers.get_internal_size()
	var x_groups = (size.x - 1) / 8 + 1
	var y_groups = (size.y - 1) / 8 + 1

	for view in scene_buffers.get_view_count():
		var screen_tex = scene_buffers.get_color_layer(view)
		
		# Set 0: The texture as an Image (Write)
		var uniform_write = RDUniform.new()
		uniform_write.uniform_type = RenderingDevice.UNIFORM_TYPE_IMAGE
		uniform_write.binding = 0
		uniform_write.add_id(screen_tex)
		var set_write = UniformSetCacheRD.get_cache(shader, 0, [uniform_write])

		var compute_list = rd.compute_list_begin()
		rd.compute_list_bind_compute_pipeline(compute_list, pipeline)
		rd.compute_list_bind_uniform_set(compute_list, set_write, 0)
		rd.compute_list_dispatch(compute_list, x_groups, y_groups, 1)
		rd.compute_list_end()
