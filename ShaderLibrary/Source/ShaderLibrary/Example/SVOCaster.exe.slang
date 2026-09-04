import Raycast.Ray;
import Raycast.Intersect;
import Render.Scene;
import SVO.SVO;
import SVO.SVONode;
import SVO.Raycast;
import Raycast.Accumulator.AccumulateIntensity;
import Raycast.VolumeAccumulator;

[format("rgba32f")]
[[vk::binding(0, 0)]] 
RWTexture2D<float4> resultImage;

[[vk::binding(0, 1)]] 
ConstantBuffer<Render.Scene> scene;

[[vk::binding(0, 2)]] ConstantBuffer<uint> SvoDimension;
[[vk::binding(1, 2)]] StructuredBuffer<SVO.SVONode<float>> SvoNodes;

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    SVO.SVO<float> tree = SVO.SVO<float>::create(SvoDimension, SvoNodes);

    uint2 pixel_coords = dispatchThreadID.xy;
    uint2 dims;
    resultImage.GetDimensions(dims.x, dims.y);
    if (pixel_coords.x >= dims.x || pixel_coords.y >= dims.y) 
        return;

    float2 uv = (float2(pixel_coords) + 0.5) / float2(dims);
    Raycast.Ray r = scene.get_ray(uv);
    Raycast.AccumulateIntensity acc;
    SVO.raycast<float, Raycast.AccumulateIntensity>(r, tree, acc);
    float val = acc.currentValue;

    if (acc.errorCode == Raycast.AccumulatorError.StackOverflow) { 
        resultImage[pixel_coords] = float4(1.0f, 0, 0, 1.0f);
    }
    else if (acc.errorCode == Raycast.AccumulatorError.MaxDepthReached) {
        resultImage[pixel_coords] = float4(val, val, 1.0f, 1.0f);
    }
    else {
        resultImage[pixel_coords] = float4(val, val, val, val);
    }
}