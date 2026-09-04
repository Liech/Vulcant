#[compute]
#version 450

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

// Use rgba16f to match Godot's internal HDR scene buffer format
layout(rgba16f, binding = 0, set = 0) uniform image2D screen_image;

void main() {
	ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
	ivec2 size = imageSize(screen_image);

	if (uv.x >= size.x || uv.y >= size.y) return;

	vec4 color = imageLoad(screen_image, uv);
	
	// Apply a simple red tint (Multiply green and blue by 0.5)
	color.gb *= 0.5; 
	
	imageStore(screen_image, uv, color);
}