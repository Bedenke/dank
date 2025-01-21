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

struct v2f
{
    float4 position [[position]];
    half3 color;
    float2 uv;
};

v2f vertex vertexMain(
    device const VertexShaderArguments& vertexArgs [[buffer(0)]],
    uint vertexId [[vertex_id]]) 
{
    const device VertexData &vd = vertexArgs.buffers[0][vertexId];

    v2f o;
    o.position = float4( vd.position, 1.0 );
    o.color = half3 ( vd.normal );
    o.uv = float2(vd.uv);
    return o;
}

half4 fragment fragmentMain( v2f in [[stage_in]], 
    device FragmentShaderArguments & fragmentShaderArgs [[ buffer(0) ]] )
{
    constexpr sampler s( address::repeat, filter::linear );
    
    float4 color = fragmentShaderArgs.textures[0].sample(s, in.uv);

    return half4(color);
}