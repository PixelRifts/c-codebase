struct VS_Out {
    float4 pos : SV_Position;
    float4 color : TEXCOORD0;
};

float4 main(VS_Out input) : SV_Target {
    return input.color;
}
