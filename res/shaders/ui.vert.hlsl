struct VS_Out {
    float2 size      : BoxSize;
    float2 center    : BoxCenter;
    float1 tex_idx   : TexIndex;
    float2 tex_coord : TexCoord;
    float4 color     : Color;
    float4 clip_quad : ClipQuad;
    float3 rounding_softness_and_edgesize : RoundingSoftnessAndEdgeSize;
    float2 sampling_loc : SamplingLoc;

    float4 pos       : SV_Position;
};

cbuffer ActualConstants {
    matrix u_projection;
};

static const float2 vertex_multiplers[] = {
    float2(-.5, -.5),
    float2(+.5, -.5),
    float2(+.5, +.5),
    float2(-.5, -.5),
    float2(+.5, +.5),
    float2(-.5, +.5),
};

VS_Out main(
    float2 size : BoxSize,
    float2 center : BoxCenter,
    float2 tex_coord : TexCoord,
    float1 tex_idx : TexIndex,
    float4 color : Color,
    float4 clip_quad : ClipQuad,
    float3 rounding_softness_and_edgesize : RoundingSoftnessAndEdgeSize,
    uint   vertex_id : SV_VertexID
) {
    VS_Out ret;
    float2 pos = center + (size * vertex_multiplers[vertex_id % 6]);

    ret.pos       = mul(u_projection, float4(pos, 0.0, 1.0));
    ret.size      = size;
    ret.center    = center;
    ret.tex_idx   = tex_idx;
    ret.tex_coord = tex_coord;
    ret.color     = color;
    ret.clip_quad = clip_quad;
    ret.rounding_softness_and_edgesize = rounding_softness_and_edgesize;
    ret.sampling_loc = pos;
    return ret;
}
