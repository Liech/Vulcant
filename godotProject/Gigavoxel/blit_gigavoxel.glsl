#[compute]
#version 450

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba16f, binding = 0, set = 0) uniform image2D screen_image;
layout(binding = 0, set = 1) uniform sampler2D voxel_color_tex;
layout(binding = 0, set = 2) uniform sampler2D godot_depth_tex;
layout(binding = 1, set = 2) uniform sampler2D voxel_depth_tex;

void main() {
    ivec2 size = imageSize(screen_image);
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    if (coords.x >= size.x || coords.y >= size.y) return;

    vec2 uv = (vec2(coords) + 0.5) / vec2(size);
    vec2 voxel_uv = vec2(uv.x, 1.0 - uv.y);

    float godot_depth = texture(godot_depth_tex, uv).r;
    float depth = texture(voxel_depth_tex, voxel_uv).r;
    vec4 color = texture(voxel_color_tex, voxel_uv);
    vec4 godot_color = imageLoad(screen_image, coords);

    if (depth >= godot_depth && color.a > 0.0) {
        vec3 final_color = mix(godot_color.rgb, color.rgb, color.a);
        imageStore(screen_image, coords, vec4(final_color, 1.0));
    }
}