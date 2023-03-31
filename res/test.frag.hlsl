Texture2D t : register(t0);
SamplerState t_sampler : register(s0);

cbuffer TESTBUFFER {
	float1 val;
	float4 color;
};

float4 main(float2 tex_coord : TexCoord) : SV_Target {
    return color * t.Sample(t_sampler, tex_coord);
}
