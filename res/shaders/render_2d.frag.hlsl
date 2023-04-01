Texture2D te0 : register(t0);
Texture2D te1 : register(t1);
Texture2D te2 : register(t2);
Texture2D te3 : register(t3);
Texture2D te4 : register(t4);
Texture2D te5 : register(t5);
Texture2D te6 : register(t6);
Texture2D te7 : register(t7);

SamplerState se0 : register(s0);
SamplerState se1 : register(s1);
SamplerState se2 : register(s2);
SamplerState se3 : register(s3);
SamplerState se4 : register(s4);
SamplerState se5 : register(s5);
SamplerState se6 : register(s6);
SamplerState se7 : register(s7);

float4 main(
    float1 tex_idx   : TexIndex,
    float2 tex_coord : TexCoord,
    float4 color     : Color
) : SV_Target {

    [branch] switch (int(tex_idx)) {
        case 0: return color * te0.Sample(se0, tex_coord);
        case 1: return color * te1.Sample(se1, tex_coord);
        case 2: return color * te2.Sample(se2, tex_coord);
        case 3: return color * te3.Sample(se3, tex_coord);
        case 4: return color * te4.Sample(se4, tex_coord);
        case 5: return color * te5.Sample(se5, tex_coord);
        case 6: return color * te6.Sample(se6, tex_coord);
        case 7: return color * te7.Sample(se7, tex_coord);
        default: discard; break;
    }
    return float4(1, 1, 1, 1);
}
