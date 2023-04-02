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

float1 RoundedRectSDF(float2 sample_pos, float2 rect_center, float2 rect_half_size, float1 r) {
	float2 d2 = (abs(rect_center - sample_pos) - rect_half_size + float2(r, r));
	return min(max(d2.x, d2.y), 0.0) + length(max(d2, 0.0)) - r;
}

bool rect_contains_point(float4 a, float2 p) {
    return a.x <= p.x && a.y <= p.y && a.x + a.z >= p.x && a.y + a.w >= p.y;
}

float4 main(
    float2 size      : BoxSize,
    float2 center    : BoxCenter,
    float1 tex_idx   : TexIndex,
    float2 tex_coord : TexCoord,
    float4 color     : Color,
    float4 clip_quad : ClipQuad,
    float3 rounding_softness_and_edgesize : RoundingSoftnessAndEdgeSize,
    float2 sampling_loc : SamplingLoc
) : SV_Target {
    float2 half_boxsize = size / 2.0;
    if (!rect_contains_point(clip_quad, sampling_loc)) {
        discard;
        return float4(1, 1, 1, 1);
    }
    
    float4 final_color = float4(0, 0, 0, 0);
    [branch] switch (int(tex_idx)) {
        case 0: final_color = color * te0.Sample(se0, tex_coord); break;
        case 1: final_color = color * te1.Sample(se1, tex_coord); break;
        case 2: final_color = color * te2.Sample(se2, tex_coord); break;
        case 3: final_color = color * te3.Sample(se3, tex_coord); break;
        case 4: final_color = color * te4.Sample(se4, tex_coord); break;
        case 5: final_color = color * te5.Sample(se5, tex_coord); break;
        case 6: final_color = color * te6.Sample(se6, tex_coord); break;
        case 7: final_color = color * te7.Sample(se7, tex_coord); break;
        default: discard; return float4(1, 1, 1, 1);
    }
    
    // Rounding
    float1 softness = rounding_softness_and_edgesize.y;
    float2 softness_padding = float2(max(0, softness*2-1), max(0, softness*2-1));
    float1 dist = RoundedRectSDF(sampling_loc, center, half_boxsize-softness_padding,
                                 rounding_softness_and_edgesize.x);
    float1 sdf_factor = 1.0 - smoothstep(0, 2*softness, dist);
    final_color *= sdf_factor;


    // Edges
    float1 edge_size = rounding_softness_and_edgesize.z;
    if (edge_size != 0) {
        float2 interior_half_size = half_boxsize - float2(edge_size, edge_size); 
        
		float1 interior_radius_reduce_f = min(interior_half_size.x/half_boxsize.x,
											 interior_half_size.y/half_boxsize.y);
		
		float1 interior_corner_radius = (rounding_softness_and_edgesize.x
										* interior_radius_reduce_f * interior_radius_reduce_f);
		
		float1 inside_d = RoundedRectSDF(sampling_loc, center, interior_half_size-softness_padding,
										 interior_corner_radius);
		final_color *= smoothstep(0, 2*softness, inside_d);
    }
    return final_color;
}
