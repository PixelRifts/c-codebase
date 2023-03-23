struct VS_Out {
    float4 pos : SV_Position;
    float4 color : TEXCOORD0;
};


VS_Out main(float2 pos : Position, float4 color : Color) {
    VS_Out ret;
    ret.pos = float4(pos.x, pos.y, 0.0, 1.0);
    ret.color = color;
    return ret;
}
