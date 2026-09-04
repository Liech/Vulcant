R"glsl(
#[compute]
#version 450

layout(local_size_x = 64) in;

layout(set = 0, binding = 0, std430) readonly buffer Source { uint data[]; } src;
layout(set = 0, binding = 1, std430) writeonly buffer Dest { uint data[]; } dst;

layout(push_constant) uniform Params {
    uint src_offset;
    uint dst_offset;
    uint count;
} p;

void main() {
    uint idx = gl_GlobalInvocationID.x;
    if (idx < p.count) {
        dst.data[idx + p.dst_offset] = src.data[idx + p.src_offset];
    }
}
)glsl"