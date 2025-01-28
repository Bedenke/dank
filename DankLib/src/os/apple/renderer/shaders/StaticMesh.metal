#include <metal_stdlib>
using namespace metal;

struct VertexData {
  packed_float3 position;
  packed_float3 normal;
  packed_float2 uv;
};

typedef struct VertexShaderArguments {
    array<const device VertexData *, 16> buffers [[id(0)]];
} VertexShaderArguments;

typedef struct FragmentShaderArguments {
    array<texture2d<float>, 32> textures  [[ id(0)  ]];
} FragmentShaderArguments;


struct CameraUBO {
  float4x4 viewProj;
  float4x4 view;
  float4x4 proj;
  packed_float4 position;
  packed_float4 lightViewPos;
  float nearPlane;
  float farPlane;
  float gamma;
  float time;
};
struct InstanceData {
  float4x4 transform;
  packed_float4 color;
  uint32_t bufferIndex;
  uint32_t textureIndex;
  packed_float2 _pad;
};

struct v2f
{
    float4 position [[position]];
    float4 color;
    float2 uv;
    uint32_t textureIndex;
};

v2f vertex vertexMain(
    device const VertexShaderArguments& vertexArgs [[buffer(0)]],
    device const CameraUBO& camera [[buffer(1)]],
    device const InstanceData* instanceData [[buffer(2)]],
    uint vertexId [[vertex_id]],
    uint instanceId [[instance_id]]) 
{
    const device InstanceData &id = instanceData[ instanceId ];
    const device VertexData &vd = vertexArgs.buffers[id.bufferIndex][vertexId];

    v2f o;
    o.position = camera.viewProj * id.transform * float4( vd.position, 1.0 );
    o.color = id.color;
    o.uv = float2(vd.uv);
    o.textureIndex = id.textureIndex;
    return o;
}

half4 fragment fragmentMain( v2f in [[stage_in]], 
    device FragmentShaderArguments & fragmentShaderArgs [[ buffer(0) ]] )
{
    constexpr sampler s( address::repeat, filter::linear );
    
    float4 color = fragmentShaderArgs.textures[in.textureIndex].sample(s, in.uv) * in.color;

    return half4(color);
}