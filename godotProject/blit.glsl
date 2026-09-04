#[compute]
#version 450

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba16f, binding = 0, set = 0) uniform image2D screen_image;
layout(binding = 0, set = 1) uniform sampler2D raycast_texture;

void main() {
	ivec2 screen_size = imageSize(screen_image);
	ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

	if (coords.x >= screen_size.x || coords.y >= screen_size.y) return;

	// Calculate UVs
	vec2 uv = vec2(coords) / vec2(screen_size);
	
	// INVERT Y AXIS: Flip the V component
	uv.y = 1.0 - uv.y; 
	
	vec4 background = imageLoad(screen_image, coords);
	vec4 foreground = texture(raycast_texture, uv);

	vec3 final_color = mix(background.rgb, foreground.rgb, foreground.a);
	
	imageStore(screen_image, coords, vec4(final_color, 1.0));
}