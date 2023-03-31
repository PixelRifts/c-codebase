struct VS_Out {
    float2 tex_coord : TexCoord;
    float4 pos : SV_Position;
};

VS_Out main(float2 pos : Position, float2 tex_coord : TexCoord) {
    VS_Out ret;
    ret.pos = float4(pos.x, pos.y, 0.0, 1.0);
    ret.tex_coord = tex_coord;
    return ret;
}
