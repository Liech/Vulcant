@tool
class_name RaycastCompositor extends CompositorEffect

var raycaster : SphereExample
var cam_transform : Transform3D
var light_pos : Vector3

var rd : RenderingDevice
var shader : RID
var pipeline : RID
var linear_sampler : RID

var current_res : Vector2i = Vector2i(0, 0)

func _init() -> void:
	effect_callback_type = CompositorEffect.EFFECT_CALLBACK_TYPE_POST_TRANSPARENT
	enabled = true
	# We don't call _setup_compute here because RD might be null in _init
	
func _setup_compute():
	# Try to get the device
	rd = RenderingServer.get_rendering_device()
	if not rd: return false
	
	var shader_file = load("res://blit.glsl")
	var spirv = shader_file.get_spirv()
	shader = rd.shader_create_from_spirv(spirv)
	pipeline = rd.compute_pipeline_create(shader)
	
	var samp_state = RDSamplerState.new()
	samp_state.mag_filter = RenderingDevice.SAMPLER_FILTER_LINEAR
	samp_state.min_filter = RenderingDevice.SAMPLER_FILTER_LINEAR
	linear_sampler = rd.sampler_create(samp_state)
	return true

func _render_callback(callback_type: int, render_data: RenderData) -> void:
	# 1. Safety Check: Ensure RD and Shader are ready
	if rd == null:
		if not _setup_compute(): return # Exit if we still can't get RD
		
	if not raycaster or callback_type != EFFECT_CALLBACK_TYPE_POST_TRANSPARENT: 
		return
	
	var scene_buffers = render_data.get_render_scene_buffers()
	var screen_tex = scene_buffers.get_color_layer(0)
	var size = scene_buffers.get_internal_size()
	var scene_data : RenderSceneDataRD = render_data.get_render_scene_data()
	if not scene_data: return
	# 2. Sync Resolution
	if size != current_res:
		current_res = size
		raycaster.initialize(size)
	
	# 3. Update and Render C++
	var actual_cam_transform : Transform3D = scene_data.get_cam_transform()
	raycaster.update_view(actual_cam_transform, light_pos)
	raycaster.render()
	var ray_rid = raycaster.get_rid()
	if not ray_rid.is_valid(): return

	# 4. Compute Dispatch
	var unif_screen = RDUniform.new()
	unif_screen.uniform_type = RenderingDevice.UNIFORM_TYPE_IMAGE
	unif_screen.binding = 0
	unif_screen.add_id(screen_tex)
	var set_screen = UniformSetCacheRD.get_cache(shader, 0, [unif_screen])

	var unif_ray = RDUniform.new()
	unif_ray.uniform_type = RenderingDevice.UNIFORM_TYPE_SAMPLER_WITH_TEXTURE
	unif_ray.binding = 0
	unif_ray.add_id(linear_sampler)
	unif_ray.add_id(ray_rid)
	var set_ray = UniformSetCacheRD.get_cache(shader, 1, [unif_ray])

	var x_groups = (size.x + 7) / 8
	var y_groups = (size.y + 7) / 8
	
	var compute_list = rd.compute_list_begin() # Should no longer be null
	rd.compute_list_bind_compute_pipeline(compute_list, pipeline)
	rd.compute_list_bind_uniform_set(compute_list, set_screen, 0)
	rd.compute_list_bind_uniform_set(compute_list, set_ray, 1)
	rd.compute_list_dispatch(compute_list, x_groups, y_groups, 1)
	rd.compute_list_end()

func _notification(what):
	if what == NOTIFICATION_PREDELETE and rd:
		if shader.is_valid(): rd.free_rid(shader)
		if linear_sampler.is_valid(): rd.free_rid(linear_sampler)
