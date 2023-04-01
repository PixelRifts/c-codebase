
struct VS_Out {
    float1 tex_idx   : TexIndex;
    float2 tex_coord : TexCoord;
    float4 color     : Color;
    float4 pos       : SV_Position;
};

cbuffer ActualConstants {
    matrix u_projection;
};

VS_Out main(
    float2 pos : Position,
    float2 tex_coord : TexCoord,
    float1 tex_idx : TexIndex,
    float4 color : Color
) {
    VS_Out ret;
    ret.pos       = mul(u_projection, float4(pos, 0.0, 1.0));
    ret.tex_idx   = tex_idx;
    ret.tex_coord = tex_coord;
    ret.color     = color;
    return ret;
}
