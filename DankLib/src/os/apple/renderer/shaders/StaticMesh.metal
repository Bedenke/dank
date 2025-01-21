#include <metal_stdlib>
using namespace metal;

struct VertexData {
  packed_float3 position;
  packed_float3 normal;
  packed_float2 uv;
};

struct v2f
{
    float4 position [[position]];
    half3 color;
    float2 uv;
};

v2f vertex vertexMain( device const VertexData* vertexData [[buffer(0)]],
                               uint vertexId [[vertex_id]])
{
    const device VertexData& vd = vertexData[ vertexId ];

    v2f o;
    o.position = float4( vd.position, 1.0 );
    o.color = half3 ( vd.normal );
    o.uv = float2(vd.uv);
    return o;
}

half4 fragment fragmentMain( v2f in [[stage_in]], 
texture2d< half, access::sample > tex [[texture(0)]] )
{
    constexpr sampler s( address::repeat, filter::linear );
    half3 texel = tex.sample( s, in.uv ).rgb;

    // return half4( in.color, 1 );
    return half4( texel, 1 );
}