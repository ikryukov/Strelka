#include "mdl_types.hlsl"
#include "mdl_runtime.hlsl"

void mdl_bsdf_scattering_0_init(inout Shading_state_material state)
{
    return;
}

void mdl_bsdf_scattering_0_sample(inout Bsdf_sample_data sret_ptr, in Shading_state_material state)
{
    int phi_in;
    int phi_out;
    float2 phi_in4;
    float2 phi_out5;
    float3 phi_in6;
    float3 phi_out7;
    float2 phi_in13;
    float2 phi_out14;
    float3 phi_in15;
    float3 phi_out16;
    float phi_in77;
    float phi_out78;
    float phi_in79;
    float phi_out80;
    float tmp0 = sret_ptr.xi.z;
    if (tmp0 < 1.0) {
        float3 tmp1 = state.text_coords[0];
        float3 tmp2 = state.tangent_u[0];
        float3 tmp3 = state.tangent_v[0];
        phi_in = 1;
        phi_in4 = float2(0.0, 0.0);
        phi_in6 = float3(0.0, 0.0, 0.0);
        if (tex_width_2d(2, int2(0, 0), 0.0) != 0) {
            phi_in = 1;
            phi_in4 = float2(0.0, 0.0);
            phi_in6 = float3(0.0, 0.0, 0.0);
            if (tex_height_2d(2, int2(0, 0), 0.0) != 0) {
                float2 tmp8 = tmp1.xy;
                float3 tmp9 = tex_lookup_float3_2d(2, tmp8, 1, 1, float2(0.0, 1.0), float2(0.0, 1.0), 0.0) * float3(2.0, 2.0, 2.0) + float3(-1.0, -1.0, -1.0);
                float3 tmp10 = tmp9.xxx * tmp2 + tmp9.yyy * tmp3 + tmp9.zzz * state.normal;
                float3 tmp11 = float3(sqrt(tmp10.x * tmp10.x + tmp10.y * tmp10.y + tmp10.z * tmp10.z), 0.0, 0.0);
                float3 tmp12 = tmp10 / tmp11.xxx;
                phi_in = 0;
                phi_in4 = tmp8;
                phi_in6 = tmp12;
            }
        }
        phi_out = phi_in;
        phi_out5 = phi_in4;
        phi_out7 = phi_in6;
        phi_in13 = phi_out5;
        phi_in15 = phi_out7;
        if (phi_out != 0) {
            float3 tmp17 = state.normal;
            float2 tmp18 = tmp1.xy;
            phi_in13 = tmp18;
            phi_in15 = tmp17;
        }
        phi_out14 = phi_in13;
        phi_out16 = phi_in15;
        float4 tmp19 = tex_lookup_float4_2d(1, phi_out14, 1, 1, float2(0.0, 1.0), float2(0.0, 1.0), 0.0);
        float tmp20 = tmp19.x > 0.0 ? tmp19.x : 0.0;
        float tmp21 = tmp20 < 1.0 ? tmp20 : 1.0;
        float tmp22 = tmp19.y > 0.0 ? tmp19.y : 0.0;
        float tmp23 = tmp22 < 1.0 ? tmp22 : 1.0;
        float tmp24 = tmp19.z > 0.0 ? tmp19.z : 0.0;
        float tmp25 = tmp24 < 1.0 ? tmp24 : 1.0;
        float3 tmp26 = state.geom_normal;
        float tmp27 = sret_ptr.k1.x;
        float tmp28 = sret_ptr.k1.y;
        float tmp29 = sret_ptr.k1.z;
        float3 tmp30 = float3(tmp27, tmp28, tmp29);
        float3 tmp31 = tmp30 * tmp26;
        float tmp32 = asfloat((asint(tmp31.x + tmp31.y + tmp31.z) & -2147483648) | 1065353216);
        float3 tmp33 = float3(tmp26.x * tmp32, tmp26.y * tmp32, tmp26.z * tmp32);
        float3 tmp34 = tmp33 * phi_out16;
        float tmp35 = asfloat((asint(tmp34.x + tmp34.y + tmp34.z) & -2147483648) | 1065353216);
        float tmp36 = phi_out16.x * tmp35;
        float tmp37 = phi_out16.y * tmp35;
        float tmp38 = phi_out16.z * tmp35;
        float3 tmp39 = float3(tmp36, tmp37, tmp38);
        float3 tmp40 = tmp39.zxy;
        float3 tmp41 = tmp39.yzx;
        float3 tmp42 = tmp40 * tmp2.yzx - tmp41 * tmp2.zxy;
        float3 tmp43 = tmp42 * tmp42;
        float tmp44 = tmp43.x + tmp43.y + tmp43.z;
        if (tmp44 < 1e-08) {
            sret_ptr.pdf = 0.0;
            sret_ptr.event_type = 0;
        } else {
            float tmp45 = 1.0 / sqrt(tmp44);
            float tmp46 = tmp45 * tmp42.x;
            float tmp47 = tmp45 * tmp42.y;
            float tmp48 = tmp45 * tmp42.z;
            float3 tmp49 = float3(tmp46, tmp47, tmp48);
            float3 tmp50 = tmp49.zxy * tmp41 - tmp49.yzx * tmp40;
            if (sret_ptr.ior1.x == -1.0) {
                sret_ptr.ior1.x = 1.0;
                sret_ptr.ior1.y = 1.0;
                sret_ptr.ior1.z = 1.0;
            }
            if (sret_ptr.ior2.x == -1.0) {
                sret_ptr.ior2.x = 1.0;
                sret_ptr.ior2.y = 1.0;
                sret_ptr.ior2.z = 1.0;
            }
            float3 tmp51 = tmp39 * tmp30;
            float tmp52 = abs(tmp51.x + tmp51.y + tmp51.z);
            float3 tmp53 = tmp50 * tmp30;
            float3 tmp54 = tmp49 * tmp30;
            float tmp55 = sret_ptr.xi.y;
            float tmp56 = sret_ptr.xi.x * 6.283185;
            float tmp57 = sin(tmp56);
            float tmp58 = cos(tmp56);
            float tmp59 = sqrt(0.09337474 / (tmp57 * tmp57 + tmp58 * tmp58)) * 3.272542;
            float tmp60 = tmp59 * tmp58;
            float tmp61 = tmp59 * tmp57;
            float tmp62 = tmp55 / ((tmp61 * tmp61 + tmp60 * tmp60) * (10.70953 - tmp55 * 10.70953));
            float tmp63 = tmp62 + 1.0;
            float tmp64 = sqrt(tmp62 / tmp63);
            float tmp65 = tmp64 * tmp61;
            float tmp66 = 1.0 / tmp63;
            float tmp67 = sqrt(tmp66);
            float tmp68 = tmp64 * tmp60;
            float tmp69 = tmp67 * tmp52;
            float tmp70 = tmp68 * (tmp54.x + tmp54.y + tmp54.z) + tmp65 * (tmp53.x + tmp53.y + tmp53.z);
            float tmp71 = tmp70 + tmp69;
            float tmp72 = tmp69 - tmp70;
            float tmp73 = tmp72 > 0.0 ? tmp72 : 0.0;
            float tmp74 = tmp73 / (tmp73 + (tmp71 > 0.0 ? tmp71 : 0.0));
            if (tmp0 < tmp74) {
                sret_ptr.xi.z = tmp0 / tmp74;
                float tmp75 = -tmp65;
                float tmp76 = -tmp68;
                phi_in77 = tmp75;
                phi_in79 = tmp76;
            } else {
                sret_ptr.xi.z = (tmp0 - tmp74) / (1.0 - tmp74);
                phi_in77 = tmp65;
                phi_in79 = tmp68;
            }
            phi_out78 = phi_in77;
            phi_out80 = phi_in79;
            if (tmp67 == 0.0) {
                sret_ptr.pdf = 0.0;
                sret_ptr.event_type = 0;
            } else {
                float tmp81 = phi_out78 * tmp50.x + tmp67 * tmp36 + phi_out80 * tmp46;
                float tmp82 = phi_out78 * tmp50.y + tmp67 * tmp37 + phi_out80 * tmp47;
                float tmp83 = phi_out78 * tmp50.z + tmp67 * tmp38 + phi_out80 * tmp48;
                float3 tmp84 = float3(tmp81, tmp82, tmp83);
                float3 tmp85 = tmp84 * tmp30;
                float tmp86 = tmp85.x + tmp85.y + tmp85.z;
                if (tmp86 > 0.0) {
                    float tmp87 = tmp86 * 2.0;
                    float tmp88 = tmp87 * tmp81 - tmp27;
                    float tmp89 = tmp87 * tmp82 - tmp28;
                    float tmp90 = tmp87 * tmp83 - tmp29;
                    sret_ptr.k2.x = tmp88;
                    sret_ptr.k2.y = tmp89;
                    sret_ptr.k2.z = tmp90;
                    sret_ptr.event_type = 10;
                    float3 tmp91 = float3(tmp88, tmp89, tmp90);
                    float3 tmp92 = tmp91 * tmp33;
                    if (tmp92.x + tmp92.y + tmp92.z > 0.0) {
                        sret_ptr.bsdf_over_pdf.x = 1.0;
                        sret_ptr.bsdf_over_pdf.y = 1.0;
                        sret_ptr.bsdf_over_pdf.z = 1.0;
                        float3 tmp93 = tmp91 * tmp39;
                        float3 tmp94 = tmp91 * tmp84;
                        float tmp95 = tmp67 * 2.0;
                        float tmp96 = tmp95 * tmp52 / tmp86;
                        float tmp97 = tmp96 > 1.0 ? 1.0 : tmp96;
                        float tmp98 = abs(tmp93.x + tmp93.y + tmp93.z) * tmp95 / abs(tmp94.x + tmp94.y + tmp94.z);
                        float tmp99 = tmp98 > 1.0 ? 1.0 : tmp98;
                        float tmp100 = tmp97 < tmp99 ? tmp97 : tmp99;
                        if (tmp100 > 0.0) {
                            float tmp101 = tmp100 / tmp97;
                            sret_ptr.bsdf_over_pdf.x = tmp101;
                            sret_ptr.bsdf_over_pdf.y = tmp101;
                            sret_ptr.bsdf_over_pdf.z = tmp101;
                            float tmp102 = phi_out78 * 3.272542;
                            float tmp103 = phi_out80 * 3.272542;
                            float tmp104 = tmp102 * tmp102 + tmp66 + tmp103 * tmp103;
                            sret_ptr.pdf = tmp67 * 3.408951 / (tmp104 * tmp104) * (0.25 / tmp69) * tmp97;
                            sret_ptr.handle = 0;
                        } else {
                            sret_ptr.pdf = 0.0;
                            sret_ptr.event_type = 0;
                        }
                    } else {
                        sret_ptr.pdf = 0.0;
                        sret_ptr.event_type = 0;
                    }
                } else {
                    sret_ptr.pdf = 0.0;
                    sret_ptr.event_type = 0;
                }
            }
            sret_ptr.bsdf_over_pdf.x = sret_ptr.bsdf_over_pdf.x * tmp21;
            sret_ptr.bsdf_over_pdf.y = sret_ptr.bsdf_over_pdf.y * tmp23;
            sret_ptr.bsdf_over_pdf.z = sret_ptr.bsdf_over_pdf.z * tmp25;
        }
    } else {
        sret_ptr.xi.z = (tmp0 + -1.0) / 0.0;
        sret_ptr.pdf = 0.0;
        sret_ptr.event_type = 0;
    }
    return;
}

void mdl_bsdf_scattering_0_evaluate(inout Bsdf_evaluate_data sret_ptr, in Shading_state_material state)
{
    int phi_in;
    int phi_out;
    float2 phi_in3;
    float2 phi_out4;
    float3 phi_in5;
    float3 phi_out6;
    float2 phi_in12;
    float2 phi_out13;
    float3 phi_in14;
    float3 phi_out15;
    int phi_in35;
    int phi_out36;
    float phi_in37;
    float phi_out38;
    sret_ptr.bsdf_diffuse = (float3)0;
    sret_ptr.bsdf_glossy = (float3)0;
    float3 tmp0 = state.text_coords[0];
    float3 tmp1 = state.tangent_u[0];
    float3 tmp2 = state.tangent_v[0];
    phi_in = 1;
    phi_in3 = float2(0.0, 0.0);
    phi_in5 = float3(0.0, 0.0, 0.0);
    if (tex_width_2d(2, int2(0, 0), 0.0) != 0) {
        phi_in = 1;
        phi_in3 = float2(0.0, 0.0);
        phi_in5 = float3(0.0, 0.0, 0.0);
        if (tex_height_2d(2, int2(0, 0), 0.0) != 0) {
            float2 tmp7 = tmp0.xy;
            float3 tmp8 = tex_lookup_float3_2d(2, tmp7, 1, 1, float2(0.0, 1.0), float2(0.0, 1.0), 0.0) * float3(2.0, 2.0, 2.0) + float3(-1.0, -1.0, -1.0);
            float3 tmp9 = tmp8.xxx * tmp1 + tmp8.yyy * tmp2 + tmp8.zzz * state.normal;
            float3 tmp10 = float3(sqrt(tmp9.x * tmp9.x + tmp9.y * tmp9.y + tmp9.z * tmp9.z), 0.0, 0.0);
            float3 tmp11 = tmp9 / tmp10.xxx;
            phi_in = 0;
            phi_in3 = tmp7;
            phi_in5 = tmp11;
        }
    }
    phi_out = phi_in;
    phi_out4 = phi_in3;
    phi_out6 = phi_in5;
    phi_in12 = phi_out4;
    phi_in14 = phi_out6;
    if (phi_out != 0) {
        float3 tmp16 = state.normal;
        float2 tmp17 = tmp0.xy;
        phi_in12 = tmp17;
        phi_in14 = tmp16;
    }
    phi_out13 = phi_in12;
    phi_out15 = phi_in14;
    float4 tmp18 = tex_lookup_float4_2d(1, phi_out13, 1, 1, float2(0.0, 1.0), float2(0.0, 1.0), 0.0);
    float3 tmp19 = state.geom_normal;
    float tmp20 = sret_ptr.k1.x;
    float tmp21 = sret_ptr.k1.y;
    float tmp22 = sret_ptr.k1.z;
    float3 tmp23 = float3(tmp20, tmp21, tmp22);
    float3 tmp24 = tmp23 * tmp19;
    float tmp25 = asfloat((asint(tmp24.x + tmp24.y + tmp24.z) & -2147483648) | 1065353216);
    float3 tmp26 = float3(tmp19.x * tmp25, tmp19.y * tmp25, tmp19.z * tmp25);
    float3 tmp27 = tmp26 * phi_out15;
    float tmp28 = asfloat((asint(tmp27.x + tmp27.y + tmp27.z) & -2147483648) | 1065353216);
    float3 tmp29 = float3(phi_out15.x * tmp28, phi_out15.y * tmp28, phi_out15.z * tmp28);
    float3 tmp30 = tmp29.zxy;
    float3 tmp31 = tmp29.yzx;
    float3 tmp32 = tmp30 * tmp1.yzx - tmp31 * tmp1.zxy;
    float3 tmp33 = tmp32 * tmp32;
    float tmp34 = tmp33.x + tmp33.y + tmp33.z;
    if (tmp34 < 1e-08) {
        sret_ptr.pdf = 0.0;
        phi_in35 = 0;
        phi_in37 = 0.0;
    } else {
        float tmp39 = 1.0 / sqrt(tmp34);
        float3 tmp40 = float3(tmp39 * tmp32.x, tmp39 * tmp32.y, tmp39 * tmp32.z);
        float3 tmp41 = tmp40.zxy * tmp31 - tmp40.yzx * tmp30;
        if (sret_ptr.ior1.x == -1.0) {
            sret_ptr.ior1.x = 1.0;
            sret_ptr.ior1.y = 1.0;
            sret_ptr.ior1.z = 1.0;
        }
        if (sret_ptr.ior2.x == -1.0) {
            sret_ptr.ior2.x = 1.0;
            sret_ptr.ior2.y = 1.0;
            sret_ptr.ior2.z = 1.0;
        }
        float tmp42 = sret_ptr.k2.x;
        float tmp43 = sret_ptr.k2.y;
        float tmp44 = sret_ptr.k2.z;
        float3 tmp45 = float3(tmp42, tmp43, tmp44);
        float3 tmp46 = tmp45 * tmp26;
        if (tmp46.x + tmp46.y + tmp46.z < 0.0) {
            sret_ptr.pdf = 0.0;
            phi_in35 = 0;
            phi_in37 = 0.0;
        } else {
            float3 tmp47 = tmp29 * tmp23;
            float tmp48 = abs(tmp47.x + tmp47.y + tmp47.z);
            float3 tmp49 = tmp45 * tmp29;
            float tmp50 = tmp42 + tmp20;
            float tmp51 = tmp43 + tmp21;
            float tmp52 = tmp44 + tmp22;
            float3 tmp53 = float3(sqrt(tmp51 * tmp51 + tmp50 * tmp50 + tmp52 * tmp52), 0.0, 0.0);
            float3 tmp54 = float3(tmp50, tmp51, tmp52) / tmp53.xxx;
            float3 tmp55 = tmp54 * tmp29;
            float tmp56 = tmp55.x + tmp55.y + tmp55.z;
            float3 tmp57 = tmp54 * tmp23;
            float tmp58 = tmp57.x + tmp57.y + tmp57.z;
            float3 tmp59 = tmp54 * tmp45;
            float tmp60 = tmp59.x + tmp59.y + tmp59.z;
            if (tmp60 < 0.0 || (tmp56 < 0.0 || tmp58 < 0.0)) {
                sret_ptr.pdf = 0.0;
                phi_in35 = 0;
                phi_in37 = 0.0;
            } else {
                float3 tmp61 = tmp54 * tmp41;
                float3 tmp62 = tmp54 * tmp40;
                float tmp63 = (tmp61.x + tmp61.y + tmp61.z) * 3.272542;
                float tmp64 = (tmp62.x + tmp62.y + tmp62.z) * 3.272542;
                float tmp65 = tmp63 * tmp63 + tmp56 * tmp56 + tmp64 * tmp64;
                float tmp66 = tmp56 * 2.0;
                float tmp67 = tmp66 * tmp48 / tmp58;
                float tmp68 = tmp67 > 1.0 ? 1.0 : tmp67;
                float tmp69 = tmp66 * abs(tmp49.x + tmp49.y + tmp49.z) / tmp60;
                float tmp70 = tmp69 > 1.0 ? 1.0 : tmp69;
                float tmp71 = tmp56 * 3.408951 / (tmp65 * tmp65) * (0.25 / (tmp56 * tmp48));
                float tmp72 = tmp71 * (tmp68 < tmp70 ? tmp68 : tmp70);
                float tmp73 = tmp71 * tmp68;
                sret_ptr.pdf = tmp73;
                int tmp74 = asint(tmp73);
                phi_in35 = tmp74;
                phi_in37 = tmp72;
            }
        }
    }
    phi_out36 = phi_in35;
    phi_out38 = phi_in37;
    float tmp75 = tmp18.x > 0.0 ? tmp18.x : 0.0;
    float tmp76 = tmp18.y > 0.0 ? tmp18.y : 0.0;
    float tmp77 = tmp18.z > 0.0 ? tmp18.z : 0.0;
    sret_ptr.bsdf_glossy.x = phi_out38 * (tmp75 < 1.0 ? tmp75 : 1.0);
    sret_ptr.bsdf_glossy.y = phi_out38 * (tmp76 < 1.0 ? tmp76 : 1.0);
    sret_ptr.bsdf_glossy.z = phi_out38 * (tmp77 < 1.0 ? tmp77 : 1.0);
    sret_ptr.pdf = asfloat(phi_out36);
    return;
}

void mdl_bsdf_scattering_0_pdf(inout Bsdf_pdf_data sret_ptr, in Shading_state_material state)
{
    int phi_in;
    int phi_out;
    float3 phi_in3;
    float3 phi_out4;
    float3 phi_in9;
    float3 phi_out10;
    int phi_in28;
    int phi_out29;
    float3 tmp0 = state.text_coords[0];
    float3 tmp1 = state.tangent_u[0];
    float3 tmp2 = state.tangent_v[0];
    phi_in = 1;
    phi_in3 = float3(0.0, 0.0, 0.0);
    if (tex_width_2d(2, int2(0, 0), 0.0) != 0) {
        phi_in = 1;
        phi_in3 = float3(0.0, 0.0, 0.0);
        if (tex_height_2d(2, int2(0, 0), 0.0) != 0) {
            float3 tmp5 = tex_lookup_float3_2d(2, tmp0.xy, 1, 1, float2(0.0, 1.0), float2(0.0, 1.0), 0.0) * float3(2.0, 2.0, 2.0) + float3(-1.0, -1.0, -1.0);
            float3 tmp6 = tmp5.xxx * tmp1 + tmp5.yyy * tmp2 + tmp5.zzz * state.normal;
            float3 tmp7 = float3(sqrt(tmp6.x * tmp6.x + tmp6.y * tmp6.y + tmp6.z * tmp6.z), 0.0, 0.0);
            float3 tmp8 = tmp6 / tmp7.xxx;
            phi_in = 0;
            phi_in3 = tmp8;
        }
    }
    phi_out = phi_in;
    phi_out4 = phi_in3;
    phi_in9 = phi_out4;
    if (phi_out != 0) {
        float3 tmp11 = state.normal;
        phi_in9 = tmp11;
    }
    phi_out10 = phi_in9;
    float3 tmp12 = state.geom_normal;
    float tmp13 = sret_ptr.k1.x;
    float tmp14 = sret_ptr.k1.y;
    float tmp15 = sret_ptr.k1.z;
    float3 tmp16 = float3(tmp13, tmp14, tmp15);
    float3 tmp17 = tmp16 * tmp12;
    float tmp18 = asfloat((asint(tmp17.x + tmp17.y + tmp17.z) & -2147483648) | 1065353216);
    float3 tmp19 = float3(tmp12.x * tmp18, tmp12.y * tmp18, tmp12.z * tmp18);
    float3 tmp20 = tmp19 * phi_out10;
    float tmp21 = asfloat((asint(tmp20.x + tmp20.y + tmp20.z) & -2147483648) | 1065353216);
    float3 tmp22 = float3(phi_out10.x * tmp21, phi_out10.y * tmp21, phi_out10.z * tmp21);
    float3 tmp23 = tmp22.zxy;
    float3 tmp24 = tmp22.yzx;
    float3 tmp25 = tmp23 * tmp1.yzx - tmp24 * tmp1.zxy;
    float3 tmp26 = tmp25 * tmp25;
    float tmp27 = tmp26.x + tmp26.y + tmp26.z;
    if (tmp27 < 1e-08) {
        sret_ptr.pdf = 0.0;
        phi_in28 = 0;
    } else {
        float tmp30 = 1.0 / sqrt(tmp27);
        float3 tmp31 = float3(tmp30 * tmp25.x, tmp30 * tmp25.y, tmp30 * tmp25.z);
        float3 tmp32 = tmp31.zxy * tmp24 - tmp31.yzx * tmp23;
        if (sret_ptr.ior1.x == -1.0) {
            sret_ptr.ior1.x = 1.0;
            sret_ptr.ior1.y = 1.0;
            sret_ptr.ior1.z = 1.0;
        }
        if (sret_ptr.ior2.x == -1.0) {
            sret_ptr.ior2.x = 1.0;
            sret_ptr.ior2.y = 1.0;
            sret_ptr.ior2.z = 1.0;
        }
        float tmp33 = sret_ptr.k2.x;
        float tmp34 = sret_ptr.k2.y;
        float tmp35 = sret_ptr.k2.z;
        float3 tmp36 = float3(tmp33, tmp34, tmp35);
        float3 tmp37 = tmp36 * tmp19;
        if (tmp37.x + tmp37.y + tmp37.z < 0.0) {
            sret_ptr.pdf = 0.0;
            phi_in28 = 0;
        } else {
            float3 tmp38 = tmp22 * tmp16;
            float tmp39 = tmp33 + tmp13;
            float tmp40 = tmp34 + tmp14;
            float tmp41 = tmp35 + tmp15;
            float3 tmp42 = float3(sqrt(tmp40 * tmp40 + tmp39 * tmp39 + tmp41 * tmp41), 0.0, 0.0);
            float3 tmp43 = float3(tmp39, tmp40, tmp41) / tmp42.xxx;
            float3 tmp44 = tmp43 * tmp22;
            float tmp45 = tmp44.x + tmp44.y + tmp44.z;
            float3 tmp46 = tmp43 * tmp16;
            float tmp47 = tmp46.x + tmp46.y + tmp46.z;
            float3 tmp48 = tmp43 * tmp36;
            if (tmp48.x + tmp48.y + tmp48.z < 0.0 || (tmp45 < 0.0 || tmp47 < 0.0)) {
                sret_ptr.pdf = 0.0;
                phi_in28 = 0;
            } else {
                float3 tmp49 = tmp43 * tmp32;
                float3 tmp50 = tmp43 * tmp31;
                float tmp51 = (tmp49.x + tmp49.y + tmp49.z) * 3.272542;
                float tmp52 = (tmp50.x + tmp50.y + tmp50.z) * 3.272542;
                float tmp53 = tmp51 * tmp51 + tmp45 * tmp45 + tmp52 * tmp52;
                float tmp54 = tmp45 * abs(tmp38.x + tmp38.y + tmp38.z);
                float tmp55 = tmp54 * 2.0 / tmp47;
                float tmp56 = (tmp55 > 1.0 ? 1.0 : tmp55) * (0.25 / tmp54) * (tmp45 * 3.408951 / (tmp53 * tmp53));
                sret_ptr.pdf = tmp56;
                int tmp57 = asint(tmp56);
                phi_in28 = tmp57;
            }
        }
    }
    phi_out29 = phi_in28;
    sret_ptr.pdf = asfloat(phi_out29);
    return;
}

void mdl_edf_emission_0_init(inout Shading_state_material state)
{
    return;
}

void mdl_edf_emission_0_sample(inout Edf_sample_data sret_ptr, in Shading_state_material state)
{
    float phi_in;
    float phi_out;
    float phi_in4;
    float phi_out5;
    float phi_in6;
    float phi_out7;
    float phi_in15;
    float phi_out16;
    float phi_in17;
    float phi_out18;
    float phi_in19;
    float phi_out20;
    int phi_in43;
    int phi_out44;
    float3 tmp0 = state.normal;
    float3 tmp1 = state.geom_normal;
    float tmp2 = sret_ptr.xi.x;
    float tmp3 = sret_ptr.xi.y;
    phi_in = 1.0;
    phi_in4 = 0.0;
    phi_in6 = 0.0;
    if (!(tmp2 == 0.0 && tmp3 == 0.0)) {
        float tmp8 = tmp2 * 2.0;
        float tmp9 = tmp3 * 2.0;
        float tmp10 = tmp8 < 1.0 ? tmp8 : tmp8 + -2.0;
        float tmp11 = tmp9 < 1.0 ? tmp9 : tmp9 + -2.0;
        float tmp12 = tmp10 * tmp10;
        float tmp13 = tmp11 * tmp11;
        if (tmp12 > tmp13) {
            float tmp14 = tmp11 / tmp10 * -0.7853982;
            phi_in15 = tmp10;
            phi_in17 = tmp14;
            phi_in19 = tmp12;
        } else {
            float tmp21 = tmp10 / tmp11 * 0.7853982 + -1.570796;
            phi_in15 = tmp11;
            phi_in17 = tmp21;
            phi_in19 = tmp13;
        }
        phi_out16 = phi_in15;
        phi_out18 = phi_in17;
        phi_out20 = phi_in19;
        float tmp22 = 1.0 - phi_out20;
        phi_in = 1.0;
        phi_in4 = 0.0;
        phi_in6 = 0.0;
        if (tmp22 > 0.0) {
            float tmp23 = sin(phi_out18) * phi_out16;
            float tmp24 = sqrt(tmp22);
            float tmp25 = cos(phi_out18) * phi_out16;
            phi_in = tmp24;
            phi_in4 = tmp23;
            phi_in6 = tmp25;
        }
    }
    phi_out = phi_in;
    phi_out5 = phi_in4;
    phi_out7 = phi_in6;
    float3 tmp26 = state.tangent_u[0];
    float3 tmp27 = tmp0.zxy;
    float3 tmp28 = tmp0.yzx;
    float3 tmp29 = tmp26.yzx * tmp27 - tmp26.zxy * tmp28;
    float3 tmp30 = tmp29 * tmp29;
    float tmp31 = tmp30.x + tmp30.y + tmp30.z;
    if (tmp31 < 1e-08) {
        sret_ptr.k1 = (float3)0;
        sret_ptr.pdf = float(0);
        sret_ptr.edf_over_pdf = (float3)0;
        sret_ptr.event_type = int(0);
    } else {
        float tmp32 = 1.0 / sqrt(tmp31);
        float tmp33 = tmp32 * tmp29.x;
        float tmp34 = tmp32 * tmp29.y;
        float tmp35 = tmp32 * tmp29.z;
        float3 tmp36 = float3(tmp33, tmp34, tmp35);
        float3 tmp37 = tmp36.zxy * tmp28 - tmp36.yzx * tmp27;
        float tmp38 = tmp33 * phi_out7 + phi_out * tmp0.x + tmp37.x * phi_out5;
        float tmp39 = tmp34 * phi_out7 + phi_out * tmp0.y + tmp37.y * phi_out5;
        float tmp40 = tmp35 * phi_out7 + phi_out * tmp0.z + tmp37.z * phi_out5;
        float3 tmp41 = float3(sqrt(tmp38 * tmp38 + tmp39 * tmp39 + tmp40 * tmp40), 0.0, 0.0);
        float3 tmp42 = float3(tmp38, tmp39, tmp40) / tmp41.xxx;
        sret_ptr.k1.x = tmp42.x;
        sret_ptr.k1.y = tmp42.y;
        sret_ptr.k1.z = tmp42.z;
        phi_in43 = 0;
        if (phi_out > 0.0) {
            float3 tmp45 = tmp42 * tmp1;
            phi_in43 = 0;
            if (tmp45.x + tmp45.y + tmp45.z > 0.0) {
                sret_ptr.pdf = phi_out * 0.3183099;
                sret_ptr.edf_over_pdf.x = 1.0;
                sret_ptr.edf_over_pdf.y = 1.0;
                sret_ptr.edf_over_pdf.z = 1.0;
                sret_ptr.event_type = 1;
                sret_ptr.handle = 0;
                phi_in43 = 1;
            }
        }
        phi_out44 = phi_in43;
        if (phi_out44 == 0) {
            sret_ptr.k1 = (float3)0;
            sret_ptr.pdf = float(0);
            sret_ptr.edf_over_pdf = (float3)0;
            sret_ptr.event_type = int(0);
        }
    }
    return;
}

void mdl_edf_emission_0_evaluate(inout Edf_evaluate_data sret_ptr, in Shading_state_material state)
{
    float3 tmp0 = state.normal;
    float3 tmp1 = state.geom_normal;
    float3 tmp2 = float3(sret_ptr.k1.x, sret_ptr.k1.y, sret_ptr.k1.z);
    float3 tmp3 = tmp2 * tmp1;
    float tmp4 = asfloat((asint(tmp3.x + tmp3.y + tmp3.z) & -2147483648) | 1065353216);
    float3 tmp5 = float3(tmp1.x * tmp4, tmp1.y * tmp4, tmp1.z * tmp4) * tmp0;
    float tmp6 = asfloat((asint(tmp5.x + tmp5.y + tmp5.z) & -2147483648) | 1065353216);
    float3 tmp7 = float3(tmp0.x * tmp6, tmp0.y * tmp6, tmp0.z * tmp6) * tmp2;
    float tmp8 = tmp7.x + tmp7.y + tmp7.z;
    float tmp9 = tmp8 > 0.0 ? tmp8 : 0.0;
    sret_ptr.cos = tmp9;
    sret_ptr.pdf = tmp9 * 0.3183099;
    sret_ptr.edf.x = 0.3183099;
    sret_ptr.edf.y = 0.3183099;
    sret_ptr.edf.z = 0.3183099;
    return;
}

void mdl_edf_emission_0_pdf(inout Edf_pdf_data sret_ptr, in Shading_state_material state)
{
    float3 tmp0 = state.normal;
    float3 tmp1 = state.geom_normal;
    float3 tmp2 = float3(sret_ptr.k1.x, sret_ptr.k1.y, sret_ptr.k1.z);
    float3 tmp3 = tmp2 * tmp1;
    float tmp4 = asfloat((asint(tmp3.x + tmp3.y + tmp3.z) & -2147483648) | 1065353216);
    float3 tmp5 = float3(tmp1.x * tmp4, tmp1.y * tmp4, tmp1.z * tmp4) * tmp0;
    float tmp6 = asfloat((asint(tmp5.x + tmp5.y + tmp5.z) & -2147483648) | 1065353216);
    float3 tmp7 = float3(tmp0.x * tmp6, tmp0.y * tmp6, tmp0.z * tmp6) * tmp2;
    float tmp8 = tmp7.x + tmp7.y + tmp7.z;
    sret_ptr.pdf = (tmp8 > 0.0 ? tmp8 : 0.0) * 0.3183099;
    return;
}

float3 mdl_edf_emission_intensity_0(in Shading_state_material state)
{
    return float3(0.0, 0.0, 0.0);
}

void mdl_bsdf_scattering_1_init(inout Shading_state_material state)
{
    return;
}

void mdl_bsdf_scattering_1_sample(inout Bsdf_sample_data sret_ptr, in Shading_state_material state)
{
    int phi_in;
    int phi_out;
    float2 phi_in4;
    float2 phi_out5;
    float3 phi_in6;
    float3 phi_out7;
    float2 phi_in13;
    float2 phi_out14;
    float3 phi_in15;
    float3 phi_out16;
    float phi_in77;
    float phi_out78;
    float phi_in79;
    float phi_out80;
    float tmp0 = sret_ptr.xi.z;
    if (tmp0 < 1.0) {
        float3 tmp1 = state.text_coords[0];
        float3 tmp2 = state.tangent_u[0];
        float3 tmp3 = state.tangent_v[0];
        phi_in = 1;
        phi_in4 = float2(0.0, 0.0);
        phi_in6 = float3(0.0, 0.0, 0.0);
        if (tex_width_2d(4, int2(0, 0), 0.0) != 0) {
            phi_in = 1;
            phi_in4 = float2(0.0, 0.0);
            phi_in6 = float3(0.0, 0.0, 0.0);
            if (tex_height_2d(4, int2(0, 0), 0.0) != 0) {
                float2 tmp8 = tmp1.xy;
                float3 tmp9 = tex_lookup_float3_2d(4, tmp8, 1, 1, float2(0.0, 1.0), float2(0.0, 1.0), 0.0) * float3(2.0, 2.0, 2.0) + float3(-1.0, -1.0, -1.0);
                float3 tmp10 = tmp9.xxx * tmp2 + tmp9.yyy * tmp3 + tmp9.zzz * state.normal;
                float3 tmp11 = float3(sqrt(tmp10.x * tmp10.x + tmp10.y * tmp10.y + tmp10.z * tmp10.z), 0.0, 0.0);
                float3 tmp12 = tmp10 / tmp11.xxx;
                phi_in = 0;
                phi_in4 = tmp8;
                phi_in6 = tmp12;
            }
        }
        phi_out = phi_in;
        phi_out5 = phi_in4;
        phi_out7 = phi_in6;
        phi_in13 = phi_out5;
        phi_in15 = phi_out7;
        if (phi_out != 0) {
            float3 tmp17 = state.normal;
            float2 tmp18 = tmp1.xy;
            phi_in13 = tmp18;
            phi_in15 = tmp17;
        }
        phi_out14 = phi_in13;
        phi_out16 = phi_in15;
        float4 tmp19 = tex_lookup_float4_2d(3, phi_out14, 1, 1, float2(0.0, 1.0), float2(0.0, 1.0), 0.0);
        float tmp20 = tmp19.x > 0.0 ? tmp19.x : 0.0;
        float tmp21 = tmp20 < 1.0 ? tmp20 : 1.0;
        float tmp22 = tmp19.y > 0.0 ? tmp19.y : 0.0;
        float tmp23 = tmp22 < 1.0 ? tmp22 : 1.0;
        float tmp24 = tmp19.z > 0.0 ? tmp19.z : 0.0;
        float tmp25 = tmp24 < 1.0 ? tmp24 : 1.0;
        float3 tmp26 = state.geom_normal;
        float tmp27 = sret_ptr.k1.x;
        float tmp28 = sret_ptr.k1.y;
        float tmp29 = sret_ptr.k1.z;
        float3 tmp30 = float3(tmp27, tmp28, tmp29);
        float3 tmp31 = tmp30 * tmp26;
        float tmp32 = asfloat((asint(tmp31.x + tmp31.y + tmp31.z) & -2147483648) | 1065353216);
        float3 tmp33 = float3(tmp26.x * tmp32, tmp26.y * tmp32, tmp26.z * tmp32);
        float3 tmp34 = tmp33 * phi_out16;
        float tmp35 = asfloat((asint(tmp34.x + tmp34.y + tmp34.z) & -2147483648) | 1065353216);
        float tmp36 = phi_out16.x * tmp35;
        float tmp37 = phi_out16.y * tmp35;
        float tmp38 = phi_out16.z * tmp35;
        float3 tmp39 = float3(tmp36, tmp37, tmp38);
        float3 tmp40 = tmp39.zxy;
        float3 tmp41 = tmp39.yzx;
        float3 tmp42 = tmp40 * tmp2.yzx - tmp41 * tmp2.zxy;
        float3 tmp43 = tmp42 * tmp42;
        float tmp44 = tmp43.x + tmp43.y + tmp43.z;
        if (tmp44 < 1e-08) {
            sret_ptr.pdf = 0.0;
            sret_ptr.event_type = 0;
        } else {
            float tmp45 = 1.0 / sqrt(tmp44);
            float tmp46 = tmp45 * tmp42.x;
            float tmp47 = tmp45 * tmp42.y;
            float tmp48 = tmp45 * tmp42.z;
            float3 tmp49 = float3(tmp46, tmp47, tmp48);
            float3 tmp50 = tmp49.zxy * tmp41 - tmp49.yzx * tmp40;
            if (sret_ptr.ior1.x == -1.0) {
                sret_ptr.ior1.x = 1.0;
                sret_ptr.ior1.y = 1.0;
                sret_ptr.ior1.z = 1.0;
            }
            if (sret_ptr.ior2.x == -1.0) {
                sret_ptr.ior2.x = 1.0;
                sret_ptr.ior2.y = 1.0;
                sret_ptr.ior2.z = 1.0;
            }
            float3 tmp51 = tmp39 * tmp30;
            float tmp52 = abs(tmp51.x + tmp51.y + tmp51.z);
            float3 tmp53 = tmp50 * tmp30;
            float3 tmp54 = tmp49 * tmp30;
            float tmp55 = sret_ptr.xi.y;
            float tmp56 = sret_ptr.xi.x * 6.283185;
            float tmp57 = sin(tmp56);
            float tmp58 = cos(tmp56);
            float tmp59 = sqrt(0.09337474 / (tmp57 * tmp57 + tmp58 * tmp58)) * 3.272542;
            float tmp60 = tmp59 * tmp58;
            float tmp61 = tmp59 * tmp57;
            float tmp62 = tmp55 / ((tmp61 * tmp61 + tmp60 * tmp60) * (10.70953 - tmp55 * 10.70953));
            float tmp63 = tmp62 + 1.0;
            float tmp64 = sqrt(tmp62 / tmp63);
            float tmp65 = tmp64 * tmp61;
            float tmp66 = 1.0 / tmp63;
            float tmp67 = sqrt(tmp66);
            float tmp68 = tmp64 * tmp60;
            float tmp69 = tmp67 * tmp52;
            float tmp70 = tmp68 * (tmp54.x + tmp54.y + tmp54.z) + tmp65 * (tmp53.x + tmp53.y + tmp53.z);
            float tmp71 = tmp70 + tmp69;
            float tmp72 = tmp69 - tmp70;
            float tmp73 = tmp72 > 0.0 ? tmp72 : 0.0;
            float tmp74 = tmp73 / (tmp73 + (tmp71 > 0.0 ? tmp71 : 0.0));
            if (tmp0 < tmp74) {
                sret_ptr.xi.z = tmp0 / tmp74;
                float tmp75 = -tmp65;
                float tmp76 = -tmp68;
                phi_in77 = tmp75;
                phi_in79 = tmp76;
            } else {
                sret_ptr.xi.z = (tmp0 - tmp74) / (1.0 - tmp74);
                phi_in77 = tmp65;
                phi_in79 = tmp68;
            }
            phi_out78 = phi_in77;
            phi_out80 = phi_in79;
            if (tmp67 == 0.0) {
                sret_ptr.pdf = 0.0;
                sret_ptr.event_type = 0;
            } else {
                float tmp81 = phi_out78 * tmp50.x + tmp67 * tmp36 + phi_out80 * tmp46;
                float tmp82 = phi_out78 * tmp50.y + tmp67 * tmp37 + phi_out80 * tmp47;
                float tmp83 = phi_out78 * tmp50.z + tmp67 * tmp38 + phi_out80 * tmp48;
                float3 tmp84 = float3(tmp81, tmp82, tmp83);
                float3 tmp85 = tmp84 * tmp30;
                float tmp86 = tmp85.x + tmp85.y + tmp85.z;
                if (tmp86 > 0.0) {
                    float tmp87 = tmp86 * 2.0;
                    float tmp88 = tmp87 * tmp81 - tmp27;
                    float tmp89 = tmp87 * tmp82 - tmp28;
                    float tmp90 = tmp87 * tmp83 - tmp29;
                    sret_ptr.k2.x = tmp88;
                    sret_ptr.k2.y = tmp89;
                    sret_ptr.k2.z = tmp90;
                    sret_ptr.event_type = 10;
                    float3 tmp91 = float3(tmp88, tmp89, tmp90);
                    float3 tmp92 = tmp91 * tmp33;
                    if (tmp92.x + tmp92.y + tmp92.z > 0.0) {
                        sret_ptr.bsdf_over_pdf.x = 1.0;
                        sret_ptr.bsdf_over_pdf.y = 1.0;
                        sret_ptr.bsdf_over_pdf.z = 1.0;
                        float3 tmp93 = tmp91 * tmp39;
                        float3 tmp94 = tmp91 * tmp84;
                        float tmp95 = tmp67 * 2.0;
                        float tmp96 = tmp95 * tmp52 / tmp86;
                        float tmp97 = tmp96 > 1.0 ? 1.0 : tmp96;
                        float tmp98 = abs(tmp93.x + tmp93.y + tmp93.z) * tmp95 / abs(tmp94.x + tmp94.y + tmp94.z);
                        float tmp99 = tmp98 > 1.0 ? 1.0 : tmp98;
                        float tmp100 = tmp97 < tmp99 ? tmp97 : tmp99;
                        if (tmp100 > 0.0) {
                            float tmp101 = tmp100 / tmp97;
                            sret_ptr.bsdf_over_pdf.x = tmp101;
                            sret_ptr.bsdf_over_pdf.y = tmp101;
                            sret_ptr.bsdf_over_pdf.z = tmp101;
                            float tmp102 = phi_out78 * 3.272542;
                            float tmp103 = phi_out80 * 3.272542;
                            float tmp104 = tmp102 * tmp102 + tmp66 + tmp103 * tmp103;
                            sret_ptr.pdf = tmp67 * 3.408951 / (tmp104 * tmp104) * (0.25 / tmp69) * tmp97;
                            sret_ptr.handle = 0;
                        } else {
                            sret_ptr.pdf = 0.0;
                            sret_ptr.event_type = 0;
                        }
                    } else {
                        sret_ptr.pdf = 0.0;
                        sret_ptr.event_type = 0;
                    }
                } else {
                    sret_ptr.pdf = 0.0;
                    sret_ptr.event_type = 0;
                }
            }
            sret_ptr.bsdf_over_pdf.x = sret_ptr.bsdf_over_pdf.x * tmp21;
            sret_ptr.bsdf_over_pdf.y = sret_ptr.bsdf_over_pdf.y * tmp23;
            sret_ptr.bsdf_over_pdf.z = sret_ptr.bsdf_over_pdf.z * tmp25;
        }
    } else {
        sret_ptr.xi.z = (tmp0 + -1.0) / 0.0;
        sret_ptr.pdf = 0.0;
        sret_ptr.event_type = 0;
    }
    return;
}

void mdl_bsdf_scattering_1_evaluate(inout Bsdf_evaluate_data sret_ptr, in Shading_state_material state)
{
    int phi_in;
    int phi_out;
    float2 phi_in3;
    float2 phi_out4;
    float3 phi_in5;
    float3 phi_out6;
    float2 phi_in12;
    float2 phi_out13;
    float3 phi_in14;
    float3 phi_out15;
    int phi_in35;
    int phi_out36;
    float phi_in37;
    float phi_out38;
    sret_ptr.bsdf_diffuse = (float3)0;
    sret_ptr.bsdf_glossy = (float3)0;
    float3 tmp0 = state.text_coords[0];
    float3 tmp1 = state.tangent_u[0];
    float3 tmp2 = state.tangent_v[0];
    phi_in = 1;
    phi_in3 = float2(0.0, 0.0);
    phi_in5 = float3(0.0, 0.0, 0.0);
    if (tex_width_2d(4, int2(0, 0), 0.0) != 0) {
        phi_in = 1;
        phi_in3 = float2(0.0, 0.0);
        phi_in5 = float3(0.0, 0.0, 0.0);
        if (tex_height_2d(4, int2(0, 0), 0.0) != 0) {
            float2 tmp7 = tmp0.xy;
            float3 tmp8 = tex_lookup_float3_2d(4, tmp7, 1, 1, float2(0.0, 1.0), float2(0.0, 1.0), 0.0) * float3(2.0, 2.0, 2.0) + float3(-1.0, -1.0, -1.0);
            float3 tmp9 = tmp8.xxx * tmp1 + tmp8.yyy * tmp2 + tmp8.zzz * state.normal;
            float3 tmp10 = float3(sqrt(tmp9.x * tmp9.x + tmp9.y * tmp9.y + tmp9.z * tmp9.z), 0.0, 0.0);
            float3 tmp11 = tmp9 / tmp10.xxx;
            phi_in = 0;
            phi_in3 = tmp7;
            phi_in5 = tmp11;
        }
    }
    phi_out = phi_in;
    phi_out4 = phi_in3;
    phi_out6 = phi_in5;
    phi_in12 = phi_out4;
    phi_in14 = phi_out6;
    if (phi_out != 0) {
        float3 tmp16 = state.normal;
        float2 tmp17 = tmp0.xy;
        phi_in12 = tmp17;
        phi_in14 = tmp16;
    }
    phi_out13 = phi_in12;
    phi_out15 = phi_in14;
    float4 tmp18 = tex_lookup_float4_2d(3, phi_out13, 1, 1, float2(0.0, 1.0), float2(0.0, 1.0), 0.0);
    float3 tmp19 = state.geom_normal;
    float tmp20 = sret_ptr.k1.x;
    float tmp21 = sret_ptr.k1.y;
    float tmp22 = sret_ptr.k1.z;
    float3 tmp23 = float3(tmp20, tmp21, tmp22);
    float3 tmp24 = tmp23 * tmp19;
    float tmp25 = asfloat((asint(tmp24.x + tmp24.y + tmp24.z) & -2147483648) | 1065353216);
    float3 tmp26 = float3(tmp19.x * tmp25, tmp19.y * tmp25, tmp19.z * tmp25);
    float3 tmp27 = tmp26 * phi_out15;
    float tmp28 = asfloat((asint(tmp27.x + tmp27.y + tmp27.z) & -2147483648) | 1065353216);
    float3 tmp29 = float3(phi_out15.x * tmp28, phi_out15.y * tmp28, phi_out15.z * tmp28);
    float3 tmp30 = tmp29.zxy;
    float3 tmp31 = tmp29.yzx;
    float3 tmp32 = tmp30 * tmp1.yzx - tmp31 * tmp1.zxy;
    float3 tmp33 = tmp32 * tmp32;
    float tmp34 = tmp33.x + tmp33.y + tmp33.z;
    if (tmp34 < 1e-08) {
        sret_ptr.pdf = 0.0;
        phi_in35 = 0;
        phi_in37 = 0.0;
    } else {
        float tmp39 = 1.0 / sqrt(tmp34);
        float3 tmp40 = float3(tmp39 * tmp32.x, tmp39 * tmp32.y, tmp39 * tmp32.z);
        float3 tmp41 = tmp40.zxy * tmp31 - tmp40.yzx * tmp30;
        if (sret_ptr.ior1.x == -1.0) {
            sret_ptr.ior1.x = 1.0;
            sret_ptr.ior1.y = 1.0;
            sret_ptr.ior1.z = 1.0;
        }
        if (sret_ptr.ior2.x == -1.0) {
            sret_ptr.ior2.x = 1.0;
            sret_ptr.ior2.y = 1.0;
            sret_ptr.ior2.z = 1.0;
        }
        float tmp42 = sret_ptr.k2.x;
        float tmp43 = sret_ptr.k2.y;
        float tmp44 = sret_ptr.k2.z;
        float3 tmp45 = float3(tmp42, tmp43, tmp44);
        float3 tmp46 = tmp45 * tmp26;
        if (tmp46.x + tmp46.y + tmp46.z < 0.0) {
            sret_ptr.pdf = 0.0;
            phi_in35 = 0;
            phi_in37 = 0.0;
        } else {
            float3 tmp47 = tmp29 * tmp23;
            float tmp48 = abs(tmp47.x + tmp47.y + tmp47.z);
            float3 tmp49 = tmp45 * tmp29;
            float tmp50 = tmp42 + tmp20;
            float tmp51 = tmp43 + tmp21;
            float tmp52 = tmp44 + tmp22;
            float3 tmp53 = float3(sqrt(tmp51 * tmp51 + tmp50 * tmp50 + tmp52 * tmp52), 0.0, 0.0);
            float3 tmp54 = float3(tmp50, tmp51, tmp52) / tmp53.xxx;
            float3 tmp55 = tmp54 * tmp29;
            float tmp56 = tmp55.x + tmp55.y + tmp55.z;
            float3 tmp57 = tmp54 * tmp23;
            float tmp58 = tmp57.x + tmp57.y + tmp57.z;
            float3 tmp59 = tmp54 * tmp45;
            float tmp60 = tmp59.x + tmp59.y + tmp59.z;
            if (tmp60 < 0.0 || (tmp56 < 0.0 || tmp58 < 0.0)) {
                sret_ptr.pdf = 0.0;
                phi_in35 = 0;
                phi_in37 = 0.0;
            } else {
                float3 tmp61 = tmp54 * tmp41;
                float3 tmp62 = tmp54 * tmp40;
                float tmp63 = (tmp61.x + tmp61.y + tmp61.z) * 3.272542;
                float tmp64 = (tmp62.x + tmp62.y + tmp62.z) * 3.272542;
                float tmp65 = tmp63 * tmp63 + tmp56 * tmp56 + tmp64 * tmp64;
                float tmp66 = tmp56 * 2.0;
                float tmp67 = tmp66 * tmp48 / tmp58;
                float tmp68 = tmp67 > 1.0 ? 1.0 : tmp67;
                float tmp69 = tmp66 * abs(tmp49.x + tmp49.y + tmp49.z) / tmp60;
                float tmp70 = tmp69 > 1.0 ? 1.0 : tmp69;
                float tmp71 = tmp56 * 3.408951 / (tmp65 * tmp65) * (0.25 / (tmp56 * tmp48));
                float tmp72 = tmp71 * (tmp68 < tmp70 ? tmp68 : tmp70);
                float tmp73 = tmp71 * tmp68;
                sret_ptr.pdf = tmp73;
                int tmp74 = asint(tmp73);
                phi_in35 = tmp74;
                phi_in37 = tmp72;
            }
        }
    }
    phi_out36 = phi_in35;
    phi_out38 = phi_in37;
    float tmp75 = tmp18.x > 0.0 ? tmp18.x : 0.0;
    float tmp76 = tmp18.y > 0.0 ? tmp18.y : 0.0;
    float tmp77 = tmp18.z > 0.0 ? tmp18.z : 0.0;
    sret_ptr.bsdf_glossy.x = phi_out38 * (tmp75 < 1.0 ? tmp75 : 1.0);
    sret_ptr.bsdf_glossy.y = phi_out38 * (tmp76 < 1.0 ? tmp76 : 1.0);
    sret_ptr.bsdf_glossy.z = phi_out38 * (tmp77 < 1.0 ? tmp77 : 1.0);
    sret_ptr.pdf = asfloat(phi_out36);
    return;
}

void mdl_bsdf_scattering_1_pdf(inout Bsdf_pdf_data sret_ptr, in Shading_state_material state)
{
    int phi_in;
    int phi_out;
    float3 phi_in3;
    float3 phi_out4;
    float3 phi_in9;
    float3 phi_out10;
    int phi_in28;
    int phi_out29;
    float3 tmp0 = state.text_coords[0];
    float3 tmp1 = state.tangent_u[0];
    float3 tmp2 = state.tangent_v[0];
    phi_in = 1;
    phi_in3 = float3(0.0, 0.0, 0.0);
    if (tex_width_2d(4, int2(0, 0), 0.0) != 0) {
        phi_in = 1;
        phi_in3 = float3(0.0, 0.0, 0.0);
        if (tex_height_2d(4, int2(0, 0), 0.0) != 0) {
            float3 tmp5 = tex_lookup_float3_2d(4, tmp0.xy, 1, 1, float2(0.0, 1.0), float2(0.0, 1.0), 0.0) * float3(2.0, 2.0, 2.0) + float3(-1.0, -1.0, -1.0);
            float3 tmp6 = tmp5.xxx * tmp1 + tmp5.yyy * tmp2 + tmp5.zzz * state.normal;
            float3 tmp7 = float3(sqrt(tmp6.x * tmp6.x + tmp6.y * tmp6.y + tmp6.z * tmp6.z), 0.0, 0.0);
            float3 tmp8 = tmp6 / tmp7.xxx;
            phi_in = 0;
            phi_in3 = tmp8;
        }
    }
    phi_out = phi_in;
    phi_out4 = phi_in3;
    phi_in9 = phi_out4;
    if (phi_out != 0) {
        float3 tmp11 = state.normal;
        phi_in9 = tmp11;
    }
    phi_out10 = phi_in9;
    float3 tmp12 = state.geom_normal;
    float tmp13 = sret_ptr.k1.x;
    float tmp14 = sret_ptr.k1.y;
    float tmp15 = sret_ptr.k1.z;
    float3 tmp16 = float3(tmp13, tmp14, tmp15);
    float3 tmp17 = tmp16 * tmp12;
    float tmp18 = asfloat((asint(tmp17.x + tmp17.y + tmp17.z) & -2147483648) | 1065353216);
    float3 tmp19 = float3(tmp12.x * tmp18, tmp12.y * tmp18, tmp12.z * tmp18);
    float3 tmp20 = tmp19 * phi_out10;
    float tmp21 = asfloat((asint(tmp20.x + tmp20.y + tmp20.z) & -2147483648) | 1065353216);
    float3 tmp22 = float3(phi_out10.x * tmp21, phi_out10.y * tmp21, phi_out10.z * tmp21);
    float3 tmp23 = tmp22.zxy;
    float3 tmp24 = tmp22.yzx;
    float3 tmp25 = tmp23 * tmp1.yzx - tmp24 * tmp1.zxy;
    float3 tmp26 = tmp25 * tmp25;
    float tmp27 = tmp26.x + tmp26.y + tmp26.z;
    if (tmp27 < 1e-08) {
        sret_ptr.pdf = 0.0;
        phi_in28 = 0;
    } else {
        float tmp30 = 1.0 / sqrt(tmp27);
        float3 tmp31 = float3(tmp30 * tmp25.x, tmp30 * tmp25.y, tmp30 * tmp25.z);
        float3 tmp32 = tmp31.zxy * tmp24 - tmp31.yzx * tmp23;
        if (sret_ptr.ior1.x == -1.0) {
            sret_ptr.ior1.x = 1.0;
            sret_ptr.ior1.y = 1.0;
            sret_ptr.ior1.z = 1.0;
        }
        if (sret_ptr.ior2.x == -1.0) {
            sret_ptr.ior2.x = 1.0;
            sret_ptr.ior2.y = 1.0;
            sret_ptr.ior2.z = 1.0;
        }
        float tmp33 = sret_ptr.k2.x;
        float tmp34 = sret_ptr.k2.y;
        float tmp35 = sret_ptr.k2.z;
        float3 tmp36 = float3(tmp33, tmp34, tmp35);
        float3 tmp37 = tmp36 * tmp19;
        if (tmp37.x + tmp37.y + tmp37.z < 0.0) {
            sret_ptr.pdf = 0.0;
            phi_in28 = 0;
        } else {
            float3 tmp38 = tmp22 * tmp16;
            float tmp39 = tmp33 + tmp13;
            float tmp40 = tmp34 + tmp14;
            float tmp41 = tmp35 + tmp15;
            float3 tmp42 = float3(sqrt(tmp40 * tmp40 + tmp39 * tmp39 + tmp41 * tmp41), 0.0, 0.0);
            float3 tmp43 = float3(tmp39, tmp40, tmp41) / tmp42.xxx;
            float3 tmp44 = tmp43 * tmp22;
            float tmp45 = tmp44.x + tmp44.y + tmp44.z;
            float3 tmp46 = tmp43 * tmp16;
            float tmp47 = tmp46.x + tmp46.y + tmp46.z;
            float3 tmp48 = tmp43 * tmp36;
            if (tmp48.x + tmp48.y + tmp48.z < 0.0 || (tmp45 < 0.0 || tmp47 < 0.0)) {
                sret_ptr.pdf = 0.0;
                phi_in28 = 0;
            } else {
                float3 tmp49 = tmp43 * tmp32;
                float3 tmp50 = tmp43 * tmp31;
                float tmp51 = (tmp49.x + tmp49.y + tmp49.z) * 3.272542;
                float tmp52 = (tmp50.x + tmp50.y + tmp50.z) * 3.272542;
                float tmp53 = tmp51 * tmp51 + tmp45 * tmp45 + tmp52 * tmp52;
                float tmp54 = tmp45 * abs(tmp38.x + tmp38.y + tmp38.z);
                float tmp55 = tmp54 * 2.0 / tmp47;
                float tmp56 = (tmp55 > 1.0 ? 1.0 : tmp55) * (0.25 / tmp54) * (tmp45 * 3.408951 / (tmp53 * tmp53));
                sret_ptr.pdf = tmp56;
                int tmp57 = asint(tmp56);
                phi_in28 = tmp57;
            }
        }
    }
    phi_out29 = phi_in28;
    sret_ptr.pdf = asfloat(phi_out29);
    return;
}

void mdl_edf_emission_1_init(inout Shading_state_material state)
{
    return;
}

void mdl_edf_emission_1_sample(inout Edf_sample_data sret_ptr, in Shading_state_material state)
{
    float phi_in;
    float phi_out;
    float phi_in4;
    float phi_out5;
    float phi_in6;
    float phi_out7;
    float phi_in15;
    float phi_out16;
    float phi_in17;
    float phi_out18;
    float phi_in19;
    float phi_out20;
    int phi_in43;
    int phi_out44;
    float3 tmp0 = state.normal;
    float3 tmp1 = state.geom_normal;
    float tmp2 = sret_ptr.xi.x;
    float tmp3 = sret_ptr.xi.y;
    phi_in = 1.0;
    phi_in4 = 0.0;
    phi_in6 = 0.0;
    if (!(tmp2 == 0.0 && tmp3 == 0.0)) {
        float tmp8 = tmp2 * 2.0;
        float tmp9 = tmp3 * 2.0;
        float tmp10 = tmp8 < 1.0 ? tmp8 : tmp8 + -2.0;
        float tmp11 = tmp9 < 1.0 ? tmp9 : tmp9 + -2.0;
        float tmp12 = tmp10 * tmp10;
        float tmp13 = tmp11 * tmp11;
        if (tmp12 > tmp13) {
            float tmp14 = tmp11 / tmp10 * -0.7853982;
            phi_in15 = tmp10;
            phi_in17 = tmp14;
            phi_in19 = tmp12;
        } else {
            float tmp21 = tmp10 / tmp11 * 0.7853982 + -1.570796;
            phi_in15 = tmp11;
            phi_in17 = tmp21;
            phi_in19 = tmp13;
        }
        phi_out16 = phi_in15;
        phi_out18 = phi_in17;
        phi_out20 = phi_in19;
        float tmp22 = 1.0 - phi_out20;
        phi_in = 1.0;
        phi_in4 = 0.0;
        phi_in6 = 0.0;
        if (tmp22 > 0.0) {
            float tmp23 = sin(phi_out18) * phi_out16;
            float tmp24 = sqrt(tmp22);
            float tmp25 = cos(phi_out18) * phi_out16;
            phi_in = tmp24;
            phi_in4 = tmp23;
            phi_in6 = tmp25;
        }
    }
    phi_out = phi_in;
    phi_out5 = phi_in4;
    phi_out7 = phi_in6;
    float3 tmp26 = state.tangent_u[0];
    float3 tmp27 = tmp0.zxy;
    float3 tmp28 = tmp0.yzx;
    float3 tmp29 = tmp26.yzx * tmp27 - tmp26.zxy * tmp28;
    float3 tmp30 = tmp29 * tmp29;
    float tmp31 = tmp30.x + tmp30.y + tmp30.z;
    if (tmp31 < 1e-08) {
        sret_ptr.k1 = (float3)0;
        sret_ptr.pdf = float(0);
        sret_ptr.edf_over_pdf = (float3)0;
        sret_ptr.event_type = int(0);
    } else {
        float tmp32 = 1.0 / sqrt(tmp31);
        float tmp33 = tmp32 * tmp29.x;
        float tmp34 = tmp32 * tmp29.y;
        float tmp35 = tmp32 * tmp29.z;
        float3 tmp36 = float3(tmp33, tmp34, tmp35);
        float3 tmp37 = tmp36.zxy * tmp28 - tmp36.yzx * tmp27;
        float tmp38 = tmp33 * phi_out7 + phi_out * tmp0.x + tmp37.x * phi_out5;
        float tmp39 = tmp34 * phi_out7 + phi_out * tmp0.y + tmp37.y * phi_out5;
        float tmp40 = tmp35 * phi_out7 + phi_out * tmp0.z + tmp37.z * phi_out5;
        float3 tmp41 = float3(sqrt(tmp38 * tmp38 + tmp39 * tmp39 + tmp40 * tmp40), 0.0, 0.0);
        float3 tmp42 = float3(tmp38, tmp39, tmp40) / tmp41.xxx;
        sret_ptr.k1.x = tmp42.x;
        sret_ptr.k1.y = tmp42.y;
        sret_ptr.k1.z = tmp42.z;
        phi_in43 = 0;
        if (phi_out > 0.0) {
            float3 tmp45 = tmp42 * tmp1;
            phi_in43 = 0;
            if (tmp45.x + tmp45.y + tmp45.z > 0.0) {
                sret_ptr.pdf = phi_out * 0.3183099;
                sret_ptr.edf_over_pdf.x = 1.0;
                sret_ptr.edf_over_pdf.y = 1.0;
                sret_ptr.edf_over_pdf.z = 1.0;
                sret_ptr.event_type = 1;
                sret_ptr.handle = 0;
                phi_in43 = 1;
            }
        }
        phi_out44 = phi_in43;
        if (phi_out44 == 0) {
            sret_ptr.k1 = (float3)0;
            sret_ptr.pdf = float(0);
            sret_ptr.edf_over_pdf = (float3)0;
            sret_ptr.event_type = int(0);
        }
    }
    return;
}

void mdl_edf_emission_1_evaluate(inout Edf_evaluate_data sret_ptr, in Shading_state_material state)
{
    float3 tmp0 = state.normal;
    float3 tmp1 = state.geom_normal;
    float3 tmp2 = float3(sret_ptr.k1.x, sret_ptr.k1.y, sret_ptr.k1.z);
    float3 tmp3 = tmp2 * tmp1;
    float tmp4 = asfloat((asint(tmp3.x + tmp3.y + tmp3.z) & -2147483648) | 1065353216);
    float3 tmp5 = float3(tmp1.x * tmp4, tmp1.y * tmp4, tmp1.z * tmp4) * tmp0;
    float tmp6 = asfloat((asint(tmp5.x + tmp5.y + tmp5.z) & -2147483648) | 1065353216);
    float3 tmp7 = float3(tmp0.x * tmp6, tmp0.y * tmp6, tmp0.z * tmp6) * tmp2;
    float tmp8 = tmp7.x + tmp7.y + tmp7.z;
    float tmp9 = tmp8 > 0.0 ? tmp8 : 0.0;
    sret_ptr.cos = tmp9;
    sret_ptr.pdf = tmp9 * 0.3183099;
    sret_ptr.edf.x = 0.3183099;
    sret_ptr.edf.y = 0.3183099;
    sret_ptr.edf.z = 0.3183099;
    return;
}

void mdl_edf_emission_1_pdf(inout Edf_pdf_data sret_ptr, in Shading_state_material state)
{
    float3 tmp0 = state.normal;
    float3 tmp1 = state.geom_normal;
    float3 tmp2 = float3(sret_ptr.k1.x, sret_ptr.k1.y, sret_ptr.k1.z);
    float3 tmp3 = tmp2 * tmp1;
    float tmp4 = asfloat((asint(tmp3.x + tmp3.y + tmp3.z) & -2147483648) | 1065353216);
    float3 tmp5 = float3(tmp1.x * tmp4, tmp1.y * tmp4, tmp1.z * tmp4) * tmp0;
    float tmp6 = asfloat((asint(tmp5.x + tmp5.y + tmp5.z) & -2147483648) | 1065353216);
    float3 tmp7 = float3(tmp0.x * tmp6, tmp0.y * tmp6, tmp0.z * tmp6) * tmp2;
    float tmp8 = tmp7.x + tmp7.y + tmp7.z;
    sret_ptr.pdf = (tmp8 > 0.0 ? tmp8 : 0.0) * 0.3183099;
    return;
}

float3 mdl_edf_emission_intensity_1(in Shading_state_material state)
{
    return float3(0.0, 0.0, 0.0);
}

void mdl_bsdf_scattering_2_init(inout Shading_state_material state)
{
    return;
}

void gen_microfacet_ggx_vcavities_bsdf_pdf(inout Bsdf_pdf_data p_00, in Shading_state_material p_11, in float3 p_22)
{
    float3 tmp3 = p_11.tangent_u[0];
    float3 tmp4 = p_11.geom_normal;
    float tmp5 = p_00.k1.x;
    float tmp6 = p_00.k1.y;
    float tmp7 = p_00.k1.z;
    float3 tmp8 = float3(tmp5, tmp6, tmp7);
    float3 tmp9 = tmp8 * tmp4;
    float tmp10 = asfloat((asint(tmp9.x + tmp9.y + tmp9.z) & -2147483648) | 1065353216);
    float tmp11 = p_22.x;
    float tmp12 = p_22.y;
    float tmp13 = p_22.z;
    float3 tmp14 = float3(tmp4.x * tmp10, tmp4.y * tmp10, tmp4.z * tmp10);
    float3 tmp15 = tmp14 * float3(tmp11, tmp12, tmp13);
    float tmp16 = asfloat((asint(tmp15.x + tmp15.y + tmp15.z) & -2147483648) | 1065353216);
    float3 tmp17 = float3(tmp11 * tmp16, tmp12 * tmp16, tmp13 * tmp16);
    float3 tmp18 = tmp17.zxy;
    float3 tmp19 = tmp17.yzx;
    float3 tmp20 = tmp18 * tmp3.yzx - tmp19 * tmp3.zxy;
    float3 tmp21 = tmp20 * tmp20;
    float tmp22 = tmp21.x + tmp21.y + tmp21.z;
    if (tmp22 < 1e-08)
        p_00.pdf = 0.0;
    else {
        float tmp23 = 1.0 / sqrt(tmp22);
        float3 tmp24 = float3(tmp23 * tmp20.x, tmp23 * tmp20.y, tmp23 * tmp20.z);
        float3 tmp25 = tmp24.zxy * tmp19 - tmp24.yzx * tmp18;
        if (p_00.ior1.x == -1.0) {
            p_00.ior1.x = 1.0;
            p_00.ior1.y = 1.0;
            p_00.ior1.z = 1.0;
        }
        if (p_00.ior2.x == -1.0) {
            p_00.ior2.x = 1.0;
            p_00.ior2.y = 1.0;
            p_00.ior2.z = 1.0;
        }
        float tmp26 = p_00.k2.x;
        float tmp27 = p_00.k2.y;
        float tmp28 = p_00.k2.z;
        float3 tmp29 = float3(tmp26, tmp27, tmp28);
        float3 tmp30 = tmp29 * tmp14;
        if (tmp30.x + tmp30.y + tmp30.z < 0.0)
            p_00.pdf = 0.0;
        else {
            float3 tmp31 = tmp17 * tmp8;
            float tmp32 = tmp26 + tmp5;
            float tmp33 = tmp27 + tmp6;
            float tmp34 = tmp28 + tmp7;
            float3 tmp35 = float3(sqrt(tmp33 * tmp33 + tmp32 * tmp32 + tmp34 * tmp34), 0.0, 0.0);
            float3 tmp36 = float3(tmp32, tmp33, tmp34) / tmp35.xxx;
            float3 tmp37 = tmp36 * tmp17;
            float tmp38 = tmp37.x + tmp37.y + tmp37.z;
            float3 tmp39 = tmp36 * tmp8;
            float tmp40 = tmp39.x + tmp39.y + tmp39.z;
            float3 tmp41 = tmp36 * tmp29;
            if (tmp41.x + tmp41.y + tmp41.z < 0.0 || (tmp38 < 0.0 || tmp40 < 0.0))
                p_00.pdf = 0.0;
            else {
                float3 tmp42 = tmp36 * tmp25;
                float3 tmp43 = tmp36 * tmp24;
                float tmp44 = (tmp42.x + tmp42.y + tmp42.z) * 4.0;
                float tmp45 = (tmp43.x + tmp43.y + tmp43.z) * 4.0;
                float tmp46 = tmp44 * tmp44 + tmp38 * tmp38 + tmp45 * tmp45;
                float tmp47 = tmp38 * abs(tmp31.x + tmp31.y + tmp31.z);
                float tmp48 = tmp47 * 2.0 / tmp40;
                p_00.pdf = (tmp48 > 1.0 ? 1.0 : tmp48) * (0.25 / tmp47) * (tmp38 * 5.092958 / (tmp46 * tmp46));
            }
        }
    }
    return;
}

void mdl_bsdf_scattering_2_sample(inout Bsdf_sample_data sret_ptr, in Shading_state_material state)
{
    Bsdf_pdf_data tmp0;
    float3 tmp1;
    int phi_in;
    int phi_out;
    float3 phi_in40;
    float3 phi_out41;
    float phi_in42;
    float phi_out43;
    int phi_in44;
    int phi_out45;
    int phi_in46;
    int phi_out47;
    float phi_in48;
    float phi_out49;
    float phi_in50;
    float phi_out51;
    float phi_in52;
    float phi_out53;
    float phi_in54;
    float phi_out55;
    bool phi_in56;
    bool phi_out57;
    int phi_in66;
    int phi_out67;
    int phi_in70;
    int phi_out71;
    float phi_in98;
    float phi_out99;
    float phi_in100;
    float phi_out101;
    float phi_in141;
    float phi_out142;
    float phi_in143;
    float phi_out144;
    float phi_in145;
    float phi_out146;
    float phi_in154;
    float phi_out155;
    float phi_in156;
    float phi_out157;
    float phi_in158;
    float phi_out159;
    int phi_in170;
    int phi_out171;
    int phi_in172;
    int phi_out173;
    float3 phi_in174;
    float3 phi_out175;
    float phi_in176;
    float phi_out177;
    int phi_in178;
    int phi_out179;
    int phi_in180;
    int phi_out181;
    float phi_in182;
    float phi_out183;
    float phi_in184;
    float phi_out185;
    float phi_in186;
    float phi_out187;
    float phi_in188;
    float phi_out189;
    bool phi_in190;
    bool phi_out191;
    float phi_in210;
    float phi_out211;
    float phi_in214;
    float phi_out215;
    float phi_in216;
    float phi_out217;
    float phi_in218;
    float phi_out219;
    float3 tmp2 = state.normal;
    float3 tmp3 = state.geom_normal;
    float tmp4 = sret_ptr.k1.x;
    float tmp5 = sret_ptr.k1.y;
    float tmp6 = sret_ptr.k1.z;
    float3 tmp7 = float3(tmp4, tmp5, tmp6);
    float3 tmp8 = tmp7 * tmp3;
    float tmp9 = asfloat((asint(tmp8.x + tmp8.y + tmp8.z) & -2147483648) | 1065353216);
    float3 tmp10 = float3(tmp3.x * tmp9, tmp3.y * tmp9, tmp3.z * tmp9);
    float3 tmp11 = tmp10 * tmp2;
    float tmp12 = asfloat((asint(tmp11.x + tmp11.y + tmp11.z) & -2147483648) | 1065353216);
    float tmp13 = tmp2.x * tmp12;
    float tmp14 = tmp2.y * tmp12;
    float tmp15 = tmp2.z * tmp12;
    tmp1.x = tmp13;
    tmp1.y = tmp14;
    tmp1.z = tmp15;
    float3 tmp16 = float3(tmp13, tmp14, tmp15);
    float3 tmp17 = tmp16 * tmp7;
    float tmp18 = tmp17.x + tmp17.y + tmp17.z;
    float tmp19 = tmp18 > 0.0 ? tmp18 : 0.0;
    float tmp20 = tmp19 < 1.0 ? tmp19 : 1.0;
    float tmp21 = tmp20 > 0.0 ? tmp20 : 0.0;
    float tmp22 = 1.0 - (tmp21 < 1.0 ? tmp21 : 1.0);
    float tmp23 = tmp22 * tmp22;
    float tmp24 = tmp23 * tmp23 * (tmp22 * 0.46);
    float tmp25 = tmp24 + 0.04;
    float tmp26 = sret_ptr.xi.z;
    if (tmp26 < tmp25) {
        float tmp27 = tmp26 / tmp25;
        sret_ptr.xi.z = tmp27;
        float3 tmp28 = state.tangent_u[0];
        float3 tmp29 = tmp16 * tmp10;
        float tmp30 = asfloat((asint(tmp29.x + tmp29.y + tmp29.z) & -2147483648) | 1065353216);
        float tmp31 = tmp13 * tmp30;
        float tmp32 = tmp14 * tmp30;
        float tmp33 = tmp15 * tmp30;
        float3 tmp34 = float3(tmp31, tmp32, tmp33);
        float3 tmp35 = tmp34.zxy;
        float3 tmp36 = tmp34.yzx;
        float3 tmp37 = tmp28.yzx * tmp35 - tmp28.zxy * tmp36;
        float3 tmp38 = tmp37 * tmp37;
        float tmp39 = tmp38.x + tmp38.y + tmp38.z;
        if (tmp39 < 1e-08) {
            sret_ptr.pdf = 0.0;
            sret_ptr.event_type = 0;
            phi_in = 0;
            phi_in40 = float3(0.0, 0.0, 0.0);
            phi_in42 = 0.0;
            phi_in44 = 0;
            phi_in46 = 0;
            phi_in48 = 0.0;
            phi_in50 = 0.0;
            phi_in52 = 0.0;
            phi_in54 = 0.0;
            phi_in56 = false;
        } else {
            float tmp58 = 1.0 / sqrt(tmp39);
            float tmp59 = tmp58 * tmp37.x;
            float tmp60 = tmp58 * tmp37.y;
            float tmp61 = tmp58 * tmp37.z;
            float3 tmp62 = float3(tmp59, tmp60, tmp61);
            float3 tmp63 = tmp62.zxy * tmp36 - tmp62.yzx * tmp35;
            float tmp64 = sret_ptr.ior1.x;
            int tmp65 = asint(tmp64);
            phi_in66 = tmp65;
            if (tmp64 == -1.0) {
                sret_ptr.ior1.x = 1.0;
                sret_ptr.ior1.y = 1.0;
                sret_ptr.ior1.z = 1.0;
                phi_in66 = 1065353216;
            }
            phi_out67 = phi_in66;
            float tmp68 = sret_ptr.ior2.x;
            int tmp69 = asint(tmp68);
            phi_in70 = tmp69;
            if (tmp68 == -1.0) {
                sret_ptr.ior2.x = 1.0;
                sret_ptr.ior2.y = 1.0;
                sret_ptr.ior2.z = 1.0;
                phi_in70 = 1065353216;
            }
            phi_out71 = phi_in70;
            float3 tmp72 = tmp34 * tmp7;
            float tmp73 = abs(tmp72.x + tmp72.y + tmp72.z);
            float3 tmp74 = tmp63 * tmp7;
            float3 tmp75 = tmp62 * tmp7;
            float tmp76 = sret_ptr.xi.y;
            float tmp77 = sret_ptr.xi.x * 6.283185;
            float tmp78 = sin(tmp77);
            float tmp79 = cos(tmp77);
            float tmp80 = sqrt(0.0625 / (tmp78 * tmp78 + tmp79 * tmp79)) * 4.0;
            float tmp81 = tmp80 * tmp79;
            float tmp82 = tmp80 * tmp78;
            float tmp83 = tmp76 / ((tmp82 * tmp82 + tmp81 * tmp81) * (16.0 - tmp76 * 16.0));
            float tmp84 = tmp83 + 1.0;
            float tmp85 = sqrt(tmp83 / tmp84);
            float tmp86 = tmp85 * tmp82;
            float tmp87 = 1.0 / tmp84;
            float tmp88 = sqrt(tmp87);
            float tmp89 = tmp85 * tmp81;
            float tmp90 = tmp88 * tmp73;
            float tmp91 = tmp89 * (tmp75.x + tmp75.y + tmp75.z) + tmp86 * (tmp74.x + tmp74.y + tmp74.z);
            float tmp92 = tmp91 + tmp90;
            float tmp93 = tmp90 - tmp91;
            float tmp94 = tmp93 > 0.0 ? tmp93 : 0.0;
            float tmp95 = tmp94 / (tmp94 + (tmp92 > 0.0 ? tmp92 : 0.0));
            if (tmp27 < tmp95) {
                sret_ptr.xi.z = tmp27 / tmp95;
                float tmp96 = -tmp86;
                float tmp97 = -tmp89;
                phi_in98 = tmp96;
                phi_in100 = tmp97;
            } else {
                sret_ptr.xi.z = (tmp27 - tmp95) / (1.0 - tmp95);
                phi_in98 = tmp86;
                phi_in100 = tmp89;
            }
            phi_out99 = phi_in98;
            phi_out101 = phi_in100;
            if (tmp88 == 0.0) {
                sret_ptr.pdf = 0.0;
                sret_ptr.event_type = 0;
                phi_in = 0;
                phi_in40 = float3(0.0, 0.0, 0.0);
                phi_in42 = 0.0;
                phi_in44 = 0;
                phi_in46 = 0;
                phi_in48 = 0.0;
                phi_in50 = 0.0;
                phi_in52 = 0.0;
                phi_in54 = 0.0;
                phi_in56 = false;
            } else {
                float tmp102 = phi_out99 * tmp63.x + tmp88 * tmp31 + phi_out101 * tmp59;
                float tmp103 = phi_out99 * tmp63.y + tmp88 * tmp32 + phi_out101 * tmp60;
                float tmp104 = phi_out99 * tmp63.z + tmp88 * tmp33 + phi_out101 * tmp61;
                float3 tmp105 = float3(tmp102, tmp103, tmp104);
                float3 tmp106 = tmp105 * tmp7;
                float tmp107 = tmp106.x + tmp106.y + tmp106.z;
                if (tmp107 > 0.0) {
                    float tmp108 = tmp107 * 2.0;
                    float tmp109 = tmp108 * tmp102 - tmp4;
                    float tmp110 = tmp108 * tmp103 - tmp5;
                    float tmp111 = tmp108 * tmp104 - tmp6;
                    sret_ptr.k2.x = tmp109;
                    sret_ptr.k2.y = tmp110;
                    sret_ptr.k2.z = tmp111;
                    sret_ptr.event_type = 10;
                    float3 tmp112 = float3(tmp109, tmp110, tmp111);
                    float3 tmp113 = tmp112 * tmp10;
                    if (tmp113.x + tmp113.y + tmp113.z > 0.0) {
                        sret_ptr.bsdf_over_pdf.x = 1.0;
                        sret_ptr.bsdf_over_pdf.y = 1.0;
                        sret_ptr.bsdf_over_pdf.z = 1.0;
                        float3 tmp114 = tmp112 * tmp34;
                        float3 tmp115 = tmp112 * tmp105;
                        float tmp116 = tmp88 * 2.0;
                        float tmp117 = tmp116 * tmp73 / tmp107;
                        float tmp118 = tmp117 > 1.0 ? 1.0 : tmp117;
                        float tmp119 = abs(tmp114.x + tmp114.y + tmp114.z) * tmp116 / abs(tmp115.x + tmp115.y + tmp115.z);
                        float tmp120 = tmp119 > 1.0 ? 1.0 : tmp119;
                        float tmp121 = tmp118 < tmp120 ? tmp118 : tmp120;
                        if (tmp121 > 0.0) {
                            float tmp122 = tmp121 / tmp118;
                            sret_ptr.bsdf_over_pdf.x = tmp122;
                            sret_ptr.bsdf_over_pdf.y = tmp122;
                            sret_ptr.bsdf_over_pdf.z = tmp122;
                            float tmp123 = phi_out99 * 4.0;
                            float tmp124 = phi_out101 * 4.0;
                            float tmp125 = tmp123 * tmp123 + tmp87 + tmp124 * tmp124;
                            float tmp126 = tmp88 * 5.092958 / (tmp125 * tmp125) * (0.25 / tmp90) * tmp118;
                            sret_ptr.pdf = tmp126;
                            sret_ptr.handle = 0;
                            phi_in = 1;
                            phi_in40 = tmp112;
                            phi_in42 = tmp122;
                            phi_in44 = phi_out71;
                            phi_in46 = phi_out67;
                            phi_in48 = tmp111;
                            phi_in50 = tmp110;
                            phi_in52 = tmp109;
                            phi_in54 = tmp126;
                            phi_in56 = true;
                        } else {
                            sret_ptr.pdf = 0.0;
                            sret_ptr.event_type = 0;
                            phi_in = 0;
                            phi_in40 = float3(0.0, 0.0, 0.0);
                            phi_in42 = 0.0;
                            phi_in44 = 0;
                            phi_in46 = 0;
                            phi_in48 = 0.0;
                            phi_in50 = 0.0;
                            phi_in52 = 0.0;
                            phi_in54 = 0.0;
                            phi_in56 = false;
                        }
                    } else {
                        sret_ptr.pdf = 0.0;
                        sret_ptr.event_type = 0;
                        phi_in = 0;
                        phi_in40 = float3(0.0, 0.0, 0.0);
                        phi_in42 = 0.0;
                        phi_in44 = 0;
                        phi_in46 = 0;
                        phi_in48 = 0.0;
                        phi_in50 = 0.0;
                        phi_in52 = 0.0;
                        phi_in54 = 0.0;
                        phi_in56 = false;
                    }
                } else {
                    sret_ptr.pdf = 0.0;
                    sret_ptr.event_type = 0;
                    phi_in = 0;
                    phi_in40 = float3(0.0, 0.0, 0.0);
                    phi_in42 = 0.0;
                    phi_in44 = 0;
                    phi_in46 = 0;
                    phi_in48 = 0.0;
                    phi_in50 = 0.0;
                    phi_in52 = 0.0;
                    phi_in54 = 0.0;
                    phi_in56 = false;
                }
            }
        }
    } else {
        sret_ptr.xi.z = (1.0 - tmp26) / (0.96 - tmp24);
        float3 tmp127 = state.tangent_u[0];
        float3 tmp128 = tmp16.zxy;
        float3 tmp129 = tmp16.yzx;
        float3 tmp130 = tmp127.yzx * tmp128 - tmp127.zxy * tmp129;
        float3 tmp131 = tmp130 * tmp130;
        float tmp132 = tmp131.x + tmp131.y + tmp131.z;
        if (tmp132 < 1e-08) {
            sret_ptr.pdf = 0.0;
            sret_ptr.event_type = 0;
            phi_in = 0;
            phi_in40 = float3(0.0, 0.0, 0.0);
            phi_in42 = 0.0;
            phi_in44 = 0;
            phi_in46 = 0;
            phi_in48 = 0.0;
            phi_in50 = 0.0;
            phi_in52 = 0.0;
            phi_in54 = 0.0;
            phi_in56 = false;
        } else {
            float tmp133 = 1.0 / sqrt(tmp132);
            float tmp134 = tmp133 * tmp130.x;
            float tmp135 = tmp133 * tmp130.y;
            float tmp136 = tmp133 * tmp130.z;
            float3 tmp137 = float3(tmp134, tmp135, tmp136);
            float3 tmp138 = tmp137.zxy * tmp129 - tmp137.yzx * tmp128;
            float tmp139 = sret_ptr.xi.x;
            float tmp140 = sret_ptr.xi.y;
            phi_in141 = 1.0;
            phi_in143 = 0.0;
            phi_in145 = 0.0;
            if (!(tmp139 == 0.0 && tmp140 == 0.0)) {
                float tmp147 = tmp139 * 2.0;
                float tmp148 = tmp140 * 2.0;
                float tmp149 = tmp147 < 1.0 ? tmp147 : tmp147 + -2.0;
                float tmp150 = tmp148 < 1.0 ? tmp148 : tmp148 + -2.0;
                float tmp151 = tmp149 * tmp149;
                float tmp152 = tmp150 * tmp150;
                if (tmp151 > tmp152) {
                    float tmp153 = tmp150 / tmp149 * -0.7853982;
                    phi_in154 = tmp149;
                    phi_in156 = tmp153;
                    phi_in158 = tmp151;
                } else {
                    float tmp160 = tmp149 / tmp150 * 0.7853982 + -1.570796;
                    phi_in154 = tmp150;
                    phi_in156 = tmp160;
                    phi_in158 = tmp152;
                }
                phi_out155 = phi_in154;
                phi_out157 = phi_in156;
                phi_out159 = phi_in158;
                float tmp161 = 1.0 - phi_out159;
                phi_in141 = 1.0;
                phi_in143 = 0.0;
                phi_in145 = 0.0;
                if (tmp161 > 0.0) {
                    float tmp162 = sin(phi_out157) * phi_out155;
                    float tmp163 = sqrt(tmp161);
                    float tmp164 = cos(phi_out157) * phi_out155;
                    phi_in141 = tmp163;
                    phi_in143 = tmp162;
                    phi_in145 = tmp164;
                }
            }
            phi_out142 = phi_in141;
            phi_out144 = phi_in143;
            phi_out146 = phi_in145;
            float tmp165 = phi_out144 * tmp138.x + phi_out142 * tmp13 + phi_out146 * tmp134;
            float tmp166 = phi_out144 * tmp138.y + phi_out142 * tmp14 + phi_out146 * tmp135;
            float tmp167 = phi_out144 * tmp138.z + phi_out142 * tmp15 + phi_out146 * tmp136;
            float3 tmp168 = float3(sqrt(tmp165 * tmp165 + tmp166 * tmp166 + tmp167 * tmp167), 0.0, 0.0);
            float3 tmp169 = float3(tmp165, tmp166, tmp167) / tmp168.xxx;
            sret_ptr.k2.x = tmp169.x;
            sret_ptr.k2.y = tmp169.y;
            sret_ptr.k2.z = tmp169.z;
            phi_in170 = 0;
            phi_in172 = 0;
            phi_in174 = float3(0.0, 0.0, 0.0);
            phi_in176 = 0.0;
            phi_in178 = 0;
            phi_in180 = 0;
            phi_in182 = 0.0;
            phi_in184 = 0.0;
            phi_in186 = 0.0;
            phi_in188 = 0.0;
            phi_in190 = false;
            if (phi_out142 > 0.0) {
                float3 tmp192 = tmp169 * tmp10;
                phi_in170 = 0;
                phi_in172 = 0;
                phi_in174 = float3(0.0, 0.0, 0.0);
                phi_in176 = 0.0;
                phi_in178 = 0;
                phi_in180 = 0;
                phi_in182 = 0.0;
                phi_in184 = 0.0;
                phi_in186 = 0.0;
                phi_in188 = 0.0;
                phi_in190 = false;
                if (tmp192.x + tmp192.y + tmp192.z > 0.0) {
                    sret_ptr.bsdf_over_pdf.x = 1.0;
                    sret_ptr.bsdf_over_pdf.y = 1.0;
                    sret_ptr.bsdf_over_pdf.z = 1.0;
                    float tmp193 = phi_out142 * 0.3183099;
                    sret_ptr.pdf = tmp193;
                    sret_ptr.event_type = 9;
                    sret_ptr.handle = 0;
                    int tmp194 = asint(sret_ptr.ior1.x);
                    int tmp195 = asint(sret_ptr.ior2.x);
                    phi_in170 = 1;
                    phi_in172 = 1;
                    phi_in174 = tmp169;
                    phi_in176 = 1.0;
                    phi_in178 = tmp195;
                    phi_in180 = tmp194;
                    phi_in182 = tmp169.z;
                    phi_in184 = tmp169.y;
                    phi_in186 = tmp169.x;
                    phi_in188 = tmp193;
                    phi_in190 = false;
                }
            }
            phi_out171 = phi_in170;
            phi_out173 = phi_in172;
            phi_out175 = phi_in174;
            phi_out177 = phi_in176;
            phi_out179 = phi_in178;
            phi_out181 = phi_in180;
            phi_out183 = phi_in182;
            phi_out185 = phi_in184;
            phi_out187 = phi_in186;
            phi_out189 = phi_in188;
            phi_out191 = phi_in190;
            phi_in = phi_out173;
            phi_in40 = phi_out175;
            phi_in42 = phi_out177;
            phi_in44 = phi_out179;
            phi_in46 = phi_out181;
            phi_in48 = phi_out183;
            phi_in50 = phi_out185;
            phi_in52 = phi_out187;
            phi_in54 = phi_out189;
            phi_in56 = phi_out191;
            if (phi_out171 == 0) {
                sret_ptr.pdf = 0.0;
                sret_ptr.event_type = 0;
                phi_in = 0;
                phi_in40 = float3(0.0, 0.0, 0.0);
                phi_in42 = 0.0;
                phi_in44 = 0;
                phi_in46 = 0;
                phi_in48 = 0.0;
                phi_in50 = 0.0;
                phi_in52 = 0.0;
                phi_in54 = 0.0;
                phi_in56 = false;
            }
        }
    }
    phi_out = phi_in;
    phi_out41 = phi_in40;
    phi_out43 = phi_in42;
    phi_out45 = phi_in44;
    phi_out47 = phi_in46;
    phi_out49 = phi_in48;
    phi_out51 = phi_in50;
    phi_out53 = phi_in52;
    phi_out55 = phi_in54;
    phi_out57 = phi_in56;
    if (phi_out != 0) {
        float3 tmp196 = phi_out41 * tmp16;
        float tmp197 = tmp196.x + tmp196.y + tmp196.z;
        float tmp198 = abs(tmp197);
        float tmp199 = phi_out51 + tmp5;
        float tmp200 = phi_out53 + tmp4;
        float tmp201 = phi_out49 + tmp6;
        float3 tmp202 = float3(sqrt(tmp199 * tmp199 + tmp200 * tmp200 + tmp201 * tmp201), 0.0, 0.0);
        float3 tmp203 = float3(tmp200, tmp199, tmp201) / tmp202.xxx * tmp7;
        float tmp204 = abs(tmp203.x + tmp203.y + tmp203.z);
        tmp0.ior1.x = asfloat(phi_out47);
        tmp0.ior1.y = asfloat(asint(sret_ptr.ior1.y));
        tmp0.ior1.z = asfloat(asint(sret_ptr.ior1.z));
        tmp0.ior2.x = asfloat(phi_out45);
        tmp0.ior2.y = asfloat(asint(sret_ptr.ior2.y));
        tmp0.ior2.z = asfloat(asint(sret_ptr.ior2.z));
        tmp0.k1.x = tmp4;
        tmp0.k1.y = tmp5;
        tmp0.k1.z = tmp6;
        tmp0.k2.x = phi_out53;
        tmp0.k2.y = phi_out51;
        tmp0.k2.z = phi_out49;
        if (phi_out57) {
            float tmp205 = tmp204 > 0.0 ? tmp204 : 0.0;
            float tmp206 = 1.0 - (tmp205 < 1.0 ? tmp205 : 1.0);
            float tmp207 = tmp206 * tmp206;
            float tmp208 = phi_out43 * ((tmp207 * tmp207 * (tmp206 * 0.46) + 0.04) * (1.0 / tmp25));
            sret_ptr.bsdf_over_pdf.x = tmp208;
            sret_ptr.bsdf_over_pdf.y = tmp208;
            sret_ptr.bsdf_over_pdf.z = tmp208;
            float3 tmp209 = phi_out41 * tmp10;
            phi_in210 = 0.0;
            if (tmp209.x + tmp209.y + tmp209.z > 0.0) {
                float tmp212 = (tmp197 > 0.0 ? tmp197 : 0.0) * 0.3183099;
                phi_in210 = tmp212;
            }
            phi_out211 = phi_in210;
            tmp0.pdf = phi_out211;
            float tmp213 = phi_out211 * (0.96 - tmp24);
            phi_in214 = phi_out55;
            phi_in216 = tmp25;
            phi_in218 = tmp213;
        } else {
            float tmp220 = tmp198 > 0.0 ? tmp198 : 0.0;
            float tmp221 = 1.0 - (tmp220 < 1.0 ? tmp220 : 1.0);
            float tmp222 = tmp221 * tmp221;
            float tmp223 = tmp222 * tmp222 * (tmp221 * 0.46) + 0.04;
            float tmp224 = 0.96 - tmp24;
            float tmp225 = phi_out43 * ((1.0 - (tmp25 > tmp223 ? tmp25 : tmp223)) * (1.0 / tmp224));
            sret_ptr.bsdf_over_pdf.x = tmp225;
            sret_ptr.bsdf_over_pdf.y = tmp225;
            sret_ptr.bsdf_over_pdf.z = tmp225;
            gen_microfacet_ggx_vcavities_bsdf_pdf(tmp0, state, tmp1);
            float tmp226 = tmp0.pdf;
            float tmp227 = tmp226 * tmp25;
            float tmp228 = sret_ptr.pdf;
            phi_in214 = tmp228;
            phi_in216 = tmp224;
            phi_in218 = tmp227;
        }
        phi_out215 = phi_in214;
        phi_out217 = phi_in216;
        phi_out219 = phi_in218;
        sret_ptr.pdf = phi_out217 * phi_out215 + phi_out219;
    }
    return;
}

void mdl_bsdf_scattering_2_evaluate(inout Bsdf_evaluate_data sret_ptr, in Shading_state_material state)
{
    float phi_in;
    float phi_out;
    float phi_in29;
    float phi_out30;
    float phi_in31;
    float phi_out32;
    float phi_in65;
    float phi_out66;
    float phi_in67;
    float phi_out68;
    float phi_in101;
    float phi_out102;
    float3 tmp0 = state.normal;
    sret_ptr.bsdf_diffuse = (float3)0;
    sret_ptr.bsdf_glossy = (float3)0;
    float3 tmp1 = state.geom_normal;
    float tmp2 = sret_ptr.k1.x;
    float tmp3 = sret_ptr.k1.y;
    float tmp4 = sret_ptr.k1.z;
    float3 tmp5 = float3(tmp2, tmp3, tmp4);
    float3 tmp6 = tmp5 * tmp1;
    float tmp7 = asfloat((asint(tmp6.x + tmp6.y + tmp6.z) & -2147483648) | 1065353216);
    float3 tmp8 = float3(tmp1.x * tmp7, tmp1.y * tmp7, tmp1.z * tmp7);
    float3 tmp9 = tmp8 * tmp0;
    float tmp10 = asfloat((asint(tmp9.x + tmp9.y + tmp9.z) & -2147483648) | 1065353216);
    float tmp11 = tmp0.x * tmp10;
    float tmp12 = tmp0.y * tmp10;
    float tmp13 = tmp0.z * tmp10;
    float3 tmp14 = float3(tmp11, tmp12, tmp13);
    float3 tmp15 = tmp14 * tmp5;
    float tmp16 = tmp15.x + tmp15.y + tmp15.z;
    float tmp17 = tmp16 > 0.0 ? tmp16 : 0.0;
    float tmp18 = tmp17 < 1.0 ? tmp17 : 1.0;
    float tmp19 = sret_ptr.k2.x;
    float tmp20 = sret_ptr.k2.y;
    float tmp21 = sret_ptr.k2.z;
    float3 tmp22 = float3(tmp19, tmp20, tmp21);
    float3 tmp23 = tmp14 * tmp22;
    float tmp24 = tmp23.x + tmp23.y + tmp23.z;
    float tmp25 = abs(tmp24);
    float3 tmp26 = tmp8 * tmp22;
    float tmp27 = tmp26.x + tmp26.y + tmp26.z;
    bool tmp28 = tmp27 < 0.0;
    phi_in = tmp21;
    phi_in29 = tmp19;
    phi_in31 = tmp20;
    if (tmp28) {
        float tmp33 = tmp25 * 2.0;
        float tmp34 = tmp33 * tmp11 + tmp19;
        float tmp35 = tmp33 * tmp12 + tmp20;
        float tmp36 = tmp33 * tmp13 + tmp21;
        phi_in = tmp36;
        phi_in29 = tmp34;
        phi_in31 = tmp35;
    }
    phi_out = phi_in;
    phi_out30 = phi_in29;
    phi_out32 = phi_in31;
    float tmp37 = phi_out32 + tmp3;
    float tmp38 = phi_out30 + tmp2;
    float tmp39 = phi_out + tmp4;
    float3 tmp40 = float3(sqrt(tmp38 * tmp38 + tmp39 * tmp39 + tmp37 * tmp37), 0.0, 0.0);
    float3 tmp41 = float3(tmp38, tmp37, tmp39) / tmp40.xxx * tmp5;
    float tmp42 = abs(tmp41.x + tmp41.y + tmp41.z);
    float tmp43 = tmp42 > 0.0 ? tmp42 : 0.0;
    float tmp44 = 1.0 - (tmp43 < 1.0 ? tmp43 : 1.0);
    float tmp45 = tmp44 * tmp44;
    float tmp46 = tmp45 * tmp45 * (tmp44 * 0.46) + 0.04;
    float tmp47 = tmp18 > 0.0 ? tmp18 : 0.0;
    float tmp48 = 1.0 - (tmp47 < 1.0 ? tmp47 : 1.0);
    float tmp49 = tmp48 * tmp48;
    float tmp50 = tmp49 * tmp49 * (tmp48 * 0.46);
    float tmp51 = tmp50 + 0.04;
    float tmp52 = tmp25 > 0.0 ? tmp25 : 0.0;
    float tmp53 = 1.0 - (tmp52 < 1.0 ? tmp52 : 1.0);
    float tmp54 = tmp53 * tmp53;
    float tmp55 = tmp54 * tmp54 * (tmp53 * 0.46) + 0.04;
    float3 tmp56 = state.tangent_u[0];
    float3 tmp57 = tmp14 * tmp8;
    float tmp58 = asfloat((asint(tmp57.x + tmp57.y + tmp57.z) & -2147483648) | 1065353216);
    float3 tmp59 = float3(tmp11 * tmp58, tmp12 * tmp58, tmp13 * tmp58);
    float3 tmp60 = tmp59.zxy;
    float3 tmp61 = tmp59.yzx;
    float3 tmp62 = tmp56.yzx * tmp60 - tmp56.zxy * tmp61;
    float3 tmp63 = tmp62 * tmp62;
    float tmp64 = tmp63.x + tmp63.y + tmp63.z;
    if (tmp64 < 1e-08) {
        sret_ptr.pdf = 0.0;
        phi_in65 = 0.0;
        phi_in67 = 0.0;
    } else {
        float tmp69 = 1.0 / sqrt(tmp64);
        float3 tmp70 = float3(tmp69 * tmp62.x, tmp69 * tmp62.y, tmp69 * tmp62.z);
        float3 tmp71 = tmp70.zxy * tmp61 - tmp70.yzx * tmp60;
        if (sret_ptr.ior1.x == -1.0) {
            sret_ptr.ior1.x = 1.0;
            sret_ptr.ior1.y = 1.0;
            sret_ptr.ior1.z = 1.0;
        }
        if (sret_ptr.ior2.x == -1.0) {
            sret_ptr.ior2.x = 1.0;
            sret_ptr.ior2.y = 1.0;
            sret_ptr.ior2.z = 1.0;
        }
        if (tmp28) {
            sret_ptr.pdf = 0.0;
            phi_in65 = 0.0;
            phi_in67 = 0.0;
        } else {
            float3 tmp72 = tmp59 * tmp5;
            float tmp73 = abs(tmp72.x + tmp72.y + tmp72.z);
            float3 tmp74 = tmp59 * tmp22;
            float tmp75 = tmp19 + tmp2;
            float tmp76 = tmp20 + tmp3;
            float tmp77 = tmp21 + tmp4;
            float3 tmp78 = float3(sqrt(tmp76 * tmp76 + tmp75 * tmp75 + tmp77 * tmp77), 0.0, 0.0);
            float3 tmp79 = float3(tmp75, tmp76, tmp77) / tmp78.xxx;
            float3 tmp80 = tmp59 * tmp79;
            float tmp81 = tmp80.x + tmp80.y + tmp80.z;
            float3 tmp82 = tmp79 * tmp5;
            float tmp83 = tmp82.x + tmp82.y + tmp82.z;
            float3 tmp84 = tmp79 * tmp22;
            float tmp85 = tmp84.x + tmp84.y + tmp84.z;
            if (tmp85 < 0.0 || (tmp83 < 0.0 || tmp81 < 0.0)) {
                sret_ptr.pdf = 0.0;
                phi_in65 = 0.0;
                phi_in67 = 0.0;
            } else {
                float3 tmp86 = tmp71 * tmp79;
                float3 tmp87 = tmp70 * tmp79;
                float tmp88 = (tmp86.x + tmp86.y + tmp86.z) * 4.0;
                float tmp89 = (tmp87.x + tmp87.y + tmp87.z) * 4.0;
                float tmp90 = tmp89 * tmp89 + tmp81 * tmp81 + tmp88 * tmp88;
                float tmp91 = tmp81 * 2.0;
                float tmp92 = tmp91 * tmp73 / tmp83;
                float tmp93 = tmp92 > 1.0 ? 1.0 : tmp92;
                float tmp94 = tmp91 * abs(tmp74.x + tmp74.y + tmp74.z) / tmp85;
                float tmp95 = tmp94 > 1.0 ? 1.0 : tmp94;
                float tmp96 = tmp81 * 5.092958 / (tmp90 * tmp90) * (0.25 / (tmp73 * tmp81));
                float tmp97 = tmp96 * (tmp93 < tmp95 ? tmp93 : tmp95);
                float tmp98 = tmp96 * tmp93;
                sret_ptr.pdf = tmp98;
                phi_in65 = tmp98;
                phi_in67 = tmp97;
            }
        }
    }
    phi_out66 = phi_in65;
    phi_out68 = phi_in67;
    float tmp99 = phi_out68 * tmp46;
    sret_ptr.bsdf_glossy.x = tmp99;
    sret_ptr.bsdf_glossy.y = tmp99;
    sret_ptr.bsdf_glossy.z = tmp99;
    float tmp100 = phi_out66 * tmp51;
    phi_in101 = 0.0;
    if (tmp27 > 0.0) {
        float tmp103 = (tmp24 > 0.0 ? tmp24 : 0.0) * 0.3183099;
        float tmp104 = (1.0 - (tmp51 > tmp55 ? tmp51 : tmp55)) * tmp103;
        sret_ptr.bsdf_diffuse.x = tmp104;
        sret_ptr.bsdf_diffuse.y = tmp104;
        sret_ptr.bsdf_diffuse.z = tmp104;
        phi_in101 = tmp103;
    }
    phi_out102 = phi_in101;
    sret_ptr.pdf = phi_out102 * (0.96 - tmp50) + tmp100;
    return;
}

void mdl_bsdf_scattering_2_pdf(inout Bsdf_pdf_data sret_ptr, in Shading_state_material state)
{
    float3 tmp0;
    float phi_in;
    float phi_out;
    float3 tmp1 = state.normal;
    float3 tmp2 = state.geom_normal;
    float tmp3 = sret_ptr.k1.x;
    float tmp4 = sret_ptr.k1.y;
    float tmp5 = sret_ptr.k1.z;
    float3 tmp6 = float3(tmp3, tmp4, tmp5) * tmp2;
    float tmp7 = asfloat((asint(tmp6.x + tmp6.y + tmp6.z) & -2147483648) | 1065353216);
    float3 tmp8 = float3(tmp2.x * tmp7, tmp2.y * tmp7, tmp2.z * tmp7) * tmp1;
    float tmp9 = asfloat((asint(tmp8.x + tmp8.y + tmp8.z) & -2147483648) | 1065353216);
    float tmp10 = tmp1.x * tmp9;
    float tmp11 = tmp1.y * tmp9;
    float tmp12 = tmp1.z * tmp9;
    tmp0.x = tmp10;
    tmp0.y = tmp11;
    tmp0.z = tmp12;
    gen_microfacet_ggx_vcavities_bsdf_pdf(sret_ptr, state, tmp0);
    float tmp13 = sret_ptr.k1.x;
    float tmp14 = sret_ptr.k1.y;
    float tmp15 = sret_ptr.k1.z;
    float3 tmp16 = float3(tmp13, tmp14, tmp15);
    float3 tmp17 = float3(tmp10, tmp11, tmp12) * tmp16;
    float tmp18 = tmp17.x + tmp17.y + tmp17.z;
    float tmp19 = tmp18 > 0.0 ? tmp18 : 0.0;
    float tmp20 = tmp19 < 1.0 ? tmp19 : 1.0;
    float tmp21 = tmp20 > 0.0 ? tmp20 : 0.0;
    float tmp22 = 1.0 - (tmp21 < 1.0 ? tmp21 : 1.0);
    float tmp23 = tmp22 * tmp22;
    float tmp24 = tmp23 * tmp23 * (tmp22 * 0.46);
    float tmp25 = sret_ptr.pdf;
    float tmp26 = (tmp24 + 0.04) * tmp25;
    float3 tmp27 = tmp16 * tmp2;
    float tmp28 = asfloat((asint(tmp27.x + tmp27.y + tmp27.z) & -2147483648) | 1065353216);
    float3 tmp29 = float3(tmp2.x * tmp28, tmp2.y * tmp28, tmp2.z * tmp28);
    float3 tmp30 = tmp29 * tmp1;
    float tmp31 = asfloat((asint(tmp30.x + tmp30.y + tmp30.z) & -2147483648) | 1065353216);
    float tmp32 = sret_ptr.k2.x;
    float tmp33 = sret_ptr.k2.y;
    float tmp34 = sret_ptr.k2.z;
    float3 tmp35 = float3(tmp32, tmp33, tmp34);
    float3 tmp36 = tmp29 * tmp35;
    phi_in = 0.0;
    if (tmp36.x + tmp36.y + tmp36.z > 0.0) {
        float3 tmp37 = float3(tmp1.x * tmp31, tmp1.y * tmp31, tmp1.z * tmp31) * tmp35;
        float tmp38 = tmp37.x + tmp37.y + tmp37.z;
        float tmp39 = (tmp38 > 0.0 ? tmp38 : 0.0) * 0.3183099;
        phi_in = tmp39;
    }
    phi_out = phi_in;
    sret_ptr.pdf = phi_out * (0.96 - tmp24) + tmp26;
    return;
}

void mdl_edf_emission_2_init(inout Shading_state_material state)
{
    return;
}

void mdl_edf_emission_2_sample(inout Edf_sample_data sret_ptr, in Shading_state_material state)
{
    float phi_in;
    float phi_out;
    float phi_in4;
    float phi_out5;
    float phi_in6;
    float phi_out7;
    float phi_in15;
    float phi_out16;
    float phi_in17;
    float phi_out18;
    float phi_in19;
    float phi_out20;
    int phi_in43;
    int phi_out44;
    float3 tmp0 = state.normal;
    float3 tmp1 = state.geom_normal;
    float tmp2 = sret_ptr.xi.x;
    float tmp3 = sret_ptr.xi.y;
    phi_in = 1.0;
    phi_in4 = 0.0;
    phi_in6 = 0.0;
    if (!(tmp2 == 0.0 && tmp3 == 0.0)) {
        float tmp8 = tmp2 * 2.0;
        float tmp9 = tmp3 * 2.0;
        float tmp10 = tmp8 < 1.0 ? tmp8 : tmp8 + -2.0;
        float tmp11 = tmp9 < 1.0 ? tmp9 : tmp9 + -2.0;
        float tmp12 = tmp10 * tmp10;
        float tmp13 = tmp11 * tmp11;
        if (tmp12 > tmp13) {
            float tmp14 = tmp11 / tmp10 * -0.7853982;
            phi_in15 = tmp10;
            phi_in17 = tmp14;
            phi_in19 = tmp12;
        } else {
            float tmp21 = tmp10 / tmp11 * 0.7853982 + -1.570796;
            phi_in15 = tmp11;
            phi_in17 = tmp21;
            phi_in19 = tmp13;
        }
        phi_out16 = phi_in15;
        phi_out18 = phi_in17;
        phi_out20 = phi_in19;
        float tmp22 = 1.0 - phi_out20;
        phi_in = 1.0;
        phi_in4 = 0.0;
        phi_in6 = 0.0;
        if (tmp22 > 0.0) {
            float tmp23 = sin(phi_out18) * phi_out16;
            float tmp24 = sqrt(tmp22);
            float tmp25 = cos(phi_out18) * phi_out16;
            phi_in = tmp24;
            phi_in4 = tmp23;
            phi_in6 = tmp25;
        }
    }
    phi_out = phi_in;
    phi_out5 = phi_in4;
    phi_out7 = phi_in6;
    float3 tmp26 = state.tangent_u[0];
    float3 tmp27 = tmp0.zxy;
    float3 tmp28 = tmp0.yzx;
    float3 tmp29 = tmp26.yzx * tmp27 - tmp26.zxy * tmp28;
    float3 tmp30 = tmp29 * tmp29;
    float tmp31 = tmp30.x + tmp30.y + tmp30.z;
    if (tmp31 < 1e-08) {
        sret_ptr.k1 = (float3)0;
        sret_ptr.pdf = float(0);
        sret_ptr.edf_over_pdf = (float3)0;
        sret_ptr.event_type = int(0);
    } else {
        float tmp32 = 1.0 / sqrt(tmp31);
        float tmp33 = tmp32 * tmp29.x;
        float tmp34 = tmp32 * tmp29.y;
        float tmp35 = tmp32 * tmp29.z;
        float3 tmp36 = float3(tmp33, tmp34, tmp35);
        float3 tmp37 = tmp36.zxy * tmp28 - tmp36.yzx * tmp27;
        float tmp38 = tmp33 * phi_out7 + phi_out * tmp0.x + tmp37.x * phi_out5;
        float tmp39 = tmp34 * phi_out7 + phi_out * tmp0.y + tmp37.y * phi_out5;
        float tmp40 = tmp35 * phi_out7 + phi_out * tmp0.z + tmp37.z * phi_out5;
        float3 tmp41 = float3(sqrt(tmp38 * tmp38 + tmp39 * tmp39 + tmp40 * tmp40), 0.0, 0.0);
        float3 tmp42 = float3(tmp38, tmp39, tmp40) / tmp41.xxx;
        sret_ptr.k1.x = tmp42.x;
        sret_ptr.k1.y = tmp42.y;
        sret_ptr.k1.z = tmp42.z;
        phi_in43 = 0;
        if (phi_out > 0.0) {
            float3 tmp45 = tmp42 * tmp1;
            phi_in43 = 0;
            if (tmp45.x + tmp45.y + tmp45.z > 0.0) {
                sret_ptr.pdf = phi_out * 0.3183099;
                sret_ptr.edf_over_pdf.x = 1.0;
                sret_ptr.edf_over_pdf.y = 1.0;
                sret_ptr.edf_over_pdf.z = 1.0;
                sret_ptr.event_type = 1;
                sret_ptr.handle = 0;
                phi_in43 = 1;
            }
        }
        phi_out44 = phi_in43;
        if (phi_out44 == 0) {
            sret_ptr.k1 = (float3)0;
            sret_ptr.pdf = float(0);
            sret_ptr.edf_over_pdf = (float3)0;
            sret_ptr.event_type = int(0);
        }
    }
    return;
}

void mdl_edf_emission_2_evaluate(inout Edf_evaluate_data sret_ptr, in Shading_state_material state)
{
    float3 tmp0 = state.normal;
    float3 tmp1 = state.geom_normal;
    float3 tmp2 = float3(sret_ptr.k1.x, sret_ptr.k1.y, sret_ptr.k1.z);
    float3 tmp3 = tmp2 * tmp1;
    float tmp4 = asfloat((asint(tmp3.x + tmp3.y + tmp3.z) & -2147483648) | 1065353216);
    float3 tmp5 = float3(tmp1.x * tmp4, tmp1.y * tmp4, tmp1.z * tmp4) * tmp0;
    float tmp6 = asfloat((asint(tmp5.x + tmp5.y + tmp5.z) & -2147483648) | 1065353216);
    float3 tmp7 = float3(tmp0.x * tmp6, tmp0.y * tmp6, tmp0.z * tmp6) * tmp2;
    float tmp8 = tmp7.x + tmp7.y + tmp7.z;
    float tmp9 = tmp8 > 0.0 ? tmp8 : 0.0;
    sret_ptr.cos = tmp9;
    sret_ptr.pdf = tmp9 * 0.3183099;
    sret_ptr.edf.x = 0.3183099;
    sret_ptr.edf.y = 0.3183099;
    sret_ptr.edf.z = 0.3183099;
    return;
}

void mdl_edf_emission_2_pdf(inout Edf_pdf_data sret_ptr, in Shading_state_material state)
{
    float3 tmp0 = state.normal;
    float3 tmp1 = state.geom_normal;
    float3 tmp2 = float3(sret_ptr.k1.x, sret_ptr.k1.y, sret_ptr.k1.z);
    float3 tmp3 = tmp2 * tmp1;
    float tmp4 = asfloat((asint(tmp3.x + tmp3.y + tmp3.z) & -2147483648) | 1065353216);
    float3 tmp5 = float3(tmp1.x * tmp4, tmp1.y * tmp4, tmp1.z * tmp4) * tmp0;
    float tmp6 = asfloat((asint(tmp5.x + tmp5.y + tmp5.z) & -2147483648) | 1065353216);
    float3 tmp7 = float3(tmp0.x * tmp6, tmp0.y * tmp6, tmp0.z * tmp6) * tmp2;
    float tmp8 = tmp7.x + tmp7.y + tmp7.z;
    sret_ptr.pdf = (tmp8 > 0.0 ? tmp8 : 0.0) * 0.3183099;
    return;
}

float3 mdl_edf_emission_intensity_2(in Shading_state_material state)
{
    return float3(0.0, 0.0, 0.0);
}

void mdl_bsdf_scattering_3_init(inout Shading_state_material state)
{
    return;
}

void gen_microfacet_ggx_vcavities_bsdf_pdf0(inout Bsdf_pdf_data p_00, in Shading_state_material p_11, in float3 p_22)
{
    float3 tmp3 = p_11.tangent_u[0];
    float3 tmp4 = p_11.geom_normal;
    float tmp5 = p_00.k1.x;
    float tmp6 = p_00.k1.y;
    float tmp7 = p_00.k1.z;
    float3 tmp8 = float3(tmp5, tmp6, tmp7);
    float3 tmp9 = tmp8 * tmp4;
    float tmp10 = asfloat((asint(tmp9.x + tmp9.y + tmp9.z) & -2147483648) | 1065353216);
    float tmp11 = p_22.x;
    float tmp12 = p_22.y;
    float tmp13 = p_22.z;
    float3 tmp14 = float3(tmp4.x * tmp10, tmp4.y * tmp10, tmp4.z * tmp10);
    float3 tmp15 = tmp14 * float3(tmp11, tmp12, tmp13);
    float tmp16 = asfloat((asint(tmp15.x + tmp15.y + tmp15.z) & -2147483648) | 1065353216);
    float3 tmp17 = float3(tmp11 * tmp16, tmp12 * tmp16, tmp13 * tmp16);
    float3 tmp18 = tmp17.zxy;
    float3 tmp19 = tmp17.yzx;
    float3 tmp20 = tmp18 * tmp3.yzx - tmp19 * tmp3.zxy;
    float3 tmp21 = tmp20 * tmp20;
    float tmp22 = tmp21.x + tmp21.y + tmp21.z;
    if (tmp22 < 1e-08)
        p_00.pdf = 0.0;
    else {
        float tmp23 = 1.0 / sqrt(tmp22);
        float3 tmp24 = float3(tmp23 * tmp20.x, tmp23 * tmp20.y, tmp23 * tmp20.z);
        float3 tmp25 = tmp24.zxy * tmp19 - tmp24.yzx * tmp18;
        if (p_00.ior1.x == -1.0) {
            p_00.ior1.x = 1.0;
            p_00.ior1.y = 1.0;
            p_00.ior1.z = 1.0;
        }
        if (p_00.ior2.x == -1.0) {
            p_00.ior2.x = 1.0;
            p_00.ior2.y = 1.0;
            p_00.ior2.z = 1.0;
        }
        float tmp26 = p_00.k2.x;
        float tmp27 = p_00.k2.y;
        float tmp28 = p_00.k2.z;
        float3 tmp29 = float3(tmp26, tmp27, tmp28);
        float3 tmp30 = tmp29 * tmp14;
        if (tmp30.x + tmp30.y + tmp30.z < 0.0)
            p_00.pdf = 0.0;
        else {
            float3 tmp31 = tmp17 * tmp8;
            float tmp32 = tmp26 + tmp5;
            float tmp33 = tmp27 + tmp6;
            float tmp34 = tmp28 + tmp7;
            float3 tmp35 = float3(sqrt(tmp33 * tmp33 + tmp32 * tmp32 + tmp34 * tmp34), 0.0, 0.0);
            float3 tmp36 = float3(tmp32, tmp33, tmp34) / tmp35.xxx;
            float3 tmp37 = tmp36 * tmp17;
            float tmp38 = tmp37.x + tmp37.y + tmp37.z;
            float3 tmp39 = tmp36 * tmp8;
            float tmp40 = tmp39.x + tmp39.y + tmp39.z;
            float3 tmp41 = tmp36 * tmp29;
            if (tmp41.x + tmp41.y + tmp41.z < 0.0 || (tmp38 < 0.0 || tmp40 < 0.0))
                p_00.pdf = 0.0;
            else {
                float3 tmp42 = tmp36 * tmp25;
                float3 tmp43 = tmp36 * tmp24;
                float tmp44 = (tmp42.x + tmp42.y + tmp42.z) * 10000.0;
                float tmp45 = (tmp43.x + tmp43.y + tmp43.z) * 10000.0;
                float tmp46 = tmp44 * tmp44 + tmp38 * tmp38 + tmp45 * tmp45;
                float tmp47 = tmp38 * abs(tmp31.x + tmp31.y + tmp31.z);
                float tmp48 = tmp47 * 2.0 / tmp40;
                p_00.pdf = (tmp48 > 1.0 ? 1.0 : tmp48) * (0.25 / tmp47) * (tmp38 * 3.183099e+07 / (tmp46 * tmp46));
            }
        }
    }
    return;
}

void gen_custom_curve_layer_pdf(inout Bsdf_pdf_data p_00, in Shading_state_material p_11, in float3 p_22)
{
    float3 tmp3;
    float phi_in;
    float phi_out;
    float3 tmp4 = p_11.normal;
    float3 tmp5 = p_11.geom_normal;
    float tmp6 = p_00.k1.x;
    float tmp7 = p_00.k1.y;
    float tmp8 = p_00.k1.z;
    float3 tmp9 = float3(tmp6, tmp7, tmp8) * tmp5;
    float tmp10 = asfloat((asint(tmp9.x + tmp9.y + tmp9.z) & -2147483648) | 1065353216);
    float3 tmp11 = float3(tmp5.x * tmp10, tmp5.y * tmp10, tmp5.z * tmp10) * tmp4;
    float tmp12 = asfloat((asint(tmp11.x + tmp11.y + tmp11.z) & -2147483648) | 1065353216);
    float tmp13 = tmp4.x * tmp12;
    float tmp14 = tmp4.y * tmp12;
    float tmp15 = tmp4.z * tmp12;
    tmp3.x = tmp13;
    tmp3.y = tmp14;
    tmp3.z = tmp15;
    gen_microfacet_ggx_vcavities_bsdf_pdf0(p_00, p_11, tmp3);
    float tmp16 = p_00.k1.x;
    float tmp17 = p_00.k1.y;
    float tmp18 = p_00.k1.z;
    float3 tmp19 = float3(tmp16, tmp17, tmp18);
    float3 tmp20 = float3(tmp13, tmp14, tmp15) * tmp19;
    float tmp21 = tmp20.x + tmp20.y + tmp20.z;
    float tmp22 = tmp21 > 0.0 ? tmp21 : 0.0;
    float tmp23 = tmp22 < 1.0 ? tmp22 : 1.0;
    float tmp24 = tmp23 > 0.0 ? tmp23 : 0.0;
    float tmp25 = 1.0 - (tmp24 < 1.0 ? tmp24 : 1.0);
    float tmp26 = tmp25 * tmp25;
    float tmp27 = tmp26 * tmp26 * (tmp25 * 0.95);
    float tmp28 = p_00.pdf;
    float tmp29 = (tmp27 + 0.04) * tmp28;
    float3 tmp30 = p_11.geom_normal;
    float3 tmp31 = tmp19 * tmp30;
    float tmp32 = asfloat((asint(tmp31.x + tmp31.y + tmp31.z) & -2147483648) | 1065353216);
    float tmp33 = p_22.x;
    float tmp34 = p_22.y;
    float tmp35 = p_22.z;
    float3 tmp36 = float3(tmp30.x * tmp32, tmp30.y * tmp32, tmp30.z * tmp32);
    float3 tmp37 = tmp36 * float3(tmp33, tmp34, tmp35);
    float tmp38 = asfloat((asint(tmp37.x + tmp37.y + tmp37.z) & -2147483648) | 1065353216);
    float tmp39 = p_00.k2.x;
    float tmp40 = p_00.k2.y;
    float tmp41 = p_00.k2.z;
    float3 tmp42 = float3(tmp39, tmp40, tmp41);
    float3 tmp43 = tmp36 * tmp42;
    phi_in = 0.0;
    if (tmp43.x + tmp43.y + tmp43.z > 0.0) {
        float3 tmp44 = float3(tmp33 * tmp38, tmp34 * tmp38, tmp35 * tmp38) * tmp42;
        float tmp45 = tmp44.x + tmp44.y + tmp44.z;
        float tmp46 = (tmp45 > 0.0 ? tmp45 : 0.0) * 0.3183099;
        phi_in = tmp46;
    }
    phi_out = phi_in;
    p_00.pdf = phi_out * (0.96 - tmp27) + tmp29;
    return;
}

void mdl_bsdf_scattering_3_sample(inout Bsdf_sample_data sret_ptr, in Shading_state_material state)
{
    Bsdf_pdf_data tmp0;
    float3 tmp1;
    Bsdf_pdf_data tmp2;
    float3 normal_buf;
    int phi_in;
    int phi_out;
    int phi_in36;
    int phi_out37;
    float phi_in64;
    float phi_out65;
    float phi_in66;
    float phi_out67;
    int phi_in129;
    int phi_out130;
    float3 phi_in131;
    float3 phi_out132;
    float phi_in133;
    float phi_out134;
    int phi_in135;
    int phi_out136;
    int phi_in137;
    int phi_out138;
    float phi_in139;
    float phi_out140;
    float phi_in141;
    float phi_out142;
    float phi_in143;
    float phi_out144;
    float phi_in145;
    float phi_out146;
    bool phi_in147;
    bool phi_out148;
    int phi_in157;
    int phi_out158;
    int phi_in161;
    int phi_out162;
    float phi_in189;
    float phi_out190;
    float phi_in191;
    float phi_out192;
    float phi_in232;
    float phi_out233;
    float phi_in234;
    float phi_out235;
    float phi_in236;
    float phi_out237;
    float phi_in245;
    float phi_out246;
    float phi_in247;
    float phi_out248;
    float phi_in249;
    float phi_out250;
    int phi_in261;
    int phi_out262;
    int phi_in263;
    int phi_out264;
    float3 phi_in265;
    float3 phi_out266;
    float phi_in267;
    float phi_out268;
    int phi_in269;
    int phi_out270;
    int phi_in271;
    int phi_out272;
    float phi_in273;
    float phi_out274;
    float phi_in275;
    float phi_out276;
    float phi_in277;
    float phi_out278;
    float phi_in279;
    float phi_out280;
    bool phi_in281;
    bool phi_out282;
    float phi_in301;
    float phi_out302;
    float phi_in305;
    float phi_out306;
    float phi_in307;
    float phi_out308;
    float phi_in309;
    float phi_out310;
    float3 tmp3 = state.normal;
    normal_buf.x = tmp3.x;
    normal_buf.y = tmp3.y;
    normal_buf.z = tmp3.z;
    float tmp4 = sret_ptr.xi.z;
    if (tmp4 < 0.0) {
        float tmp5 = tmp4 / 0.0;
        sret_ptr.xi.z = tmp5;
        float3 tmp6 = state.tangent_u[0];
        float3 tmp7 = state.geom_normal;
        float tmp8 = sret_ptr.k1.x;
        float tmp9 = sret_ptr.k1.y;
        float tmp10 = sret_ptr.k1.z;
        float3 tmp11 = float3(tmp8, tmp9, tmp10);
        float3 tmp12 = tmp11 * tmp7;
        float tmp13 = asfloat((asint(tmp12.x + tmp12.y + tmp12.z) & -2147483648) | 1065353216);
        float3 tmp14 = float3(tmp7.x * tmp13, tmp7.y * tmp13, tmp7.z * tmp13);
        float3 tmp15 = tmp14 * tmp3;
        float tmp16 = asfloat((asint(tmp15.x + tmp15.y + tmp15.z) & -2147483648) | 1065353216);
        float tmp17 = tmp3.x * tmp16;
        float tmp18 = tmp3.y * tmp16;
        float tmp19 = tmp3.z * tmp16;
        float3 tmp20 = float3(tmp17, tmp18, tmp19);
        float3 tmp21 = tmp20.zxy;
        float3 tmp22 = tmp20.yzx;
        float3 tmp23 = tmp21 * tmp6.yzx - tmp22 * tmp6.zxy;
        float3 tmp24 = tmp23 * tmp23;
        float tmp25 = tmp24.x + tmp24.y + tmp24.z;
        if (tmp25 < 1e-08) {
            sret_ptr.pdf = 0.0;
            sret_ptr.event_type = 0;
        } else {
            float tmp26 = 1.0 / sqrt(tmp25);
            float tmp27 = tmp26 * tmp23.x;
            float tmp28 = tmp26 * tmp23.y;
            float tmp29 = tmp26 * tmp23.z;
            float3 tmp30 = float3(tmp27, tmp28, tmp29);
            float3 tmp31 = tmp30.zxy * tmp22 - tmp30.yzx * tmp21;
            float tmp32 = sret_ptr.ior1.x;
            int tmp33 = asint(tmp32);
            phi_in = tmp33;
            if (tmp32 == -1.0) {
                sret_ptr.ior1.x = 1.0;
                sret_ptr.ior1.y = 1.0;
                sret_ptr.ior1.z = 1.0;
                phi_in = 1065353216;
            }
            phi_out = phi_in;
            float tmp34 = sret_ptr.ior2.x;
            int tmp35 = asint(tmp34);
            phi_in36 = tmp35;
            if (tmp34 == -1.0) {
                sret_ptr.ior2.x = 1.0;
                sret_ptr.ior2.y = 1.0;
                sret_ptr.ior2.z = 1.0;
                phi_in36 = 1065353216;
            }
            phi_out37 = phi_in36;
            float3 tmp38 = tmp20 * tmp11;
            float tmp39 = abs(tmp38.x + tmp38.y + tmp38.z);
            float3 tmp40 = tmp31 * tmp11;
            float3 tmp41 = tmp30 * tmp11;
            float tmp42 = sret_ptr.xi.y;
            float tmp43 = sret_ptr.xi.x * 6.283185;
            float tmp44 = sin(tmp43);
            float tmp45 = cos(tmp43);
            float tmp46 = sqrt(1e-08 / (tmp44 * tmp44 + tmp45 * tmp45)) * 10000.0;
            float tmp47 = tmp46 * tmp45;
            float tmp48 = tmp46 * tmp44;
            float tmp49 = tmp42 / ((tmp48 * tmp48 + tmp47 * tmp47) * (1e+08 - tmp42 * 1e+08));
            float tmp50 = tmp49 + 1.0;
            float tmp51 = sqrt(tmp49 / tmp50);
            float tmp52 = tmp51 * tmp48;
            float tmp53 = 1.0 / tmp50;
            float tmp54 = sqrt(tmp53);
            float tmp55 = tmp51 * tmp47;
            float tmp56 = tmp54 * tmp39;
            float tmp57 = tmp55 * (tmp41.x + tmp41.y + tmp41.z) + tmp52 * (tmp40.x + tmp40.y + tmp40.z);
            float tmp58 = tmp57 + tmp56;
            float tmp59 = tmp56 - tmp57;
            float tmp60 = tmp59 > 0.0 ? tmp59 : 0.0;
            float tmp61 = tmp60 / (tmp60 + (tmp58 > 0.0 ? tmp58 : 0.0));
            if (tmp5 < tmp61) {
                sret_ptr.xi.z = tmp5 / tmp61;
                float tmp62 = -tmp52;
                float tmp63 = -tmp55;
                phi_in64 = tmp62;
                phi_in66 = tmp63;
            } else {
                sret_ptr.xi.z = (tmp5 - tmp61) / (1.0 - tmp61);
                phi_in64 = tmp52;
                phi_in66 = tmp55;
            }
            phi_out65 = phi_in64;
            phi_out67 = phi_in66;
            if (tmp54 == 0.0) {
                sret_ptr.pdf = 0.0;
                sret_ptr.event_type = 0;
            } else {
                float tmp68 = phi_out65 * tmp31.x + tmp54 * tmp17 + phi_out67 * tmp27;
                float tmp69 = phi_out65 * tmp31.y + tmp54 * tmp18 + phi_out67 * tmp28;
                float tmp70 = phi_out65 * tmp31.z + tmp54 * tmp19 + phi_out67 * tmp29;
                float3 tmp71 = float3(tmp68, tmp69, tmp70);
                float3 tmp72 = tmp71 * tmp11;
                float tmp73 = tmp72.x + tmp72.y + tmp72.z;
                if (tmp73 > 0.0) {
                    float tmp74 = tmp73 * 2.0;
                    float tmp75 = tmp74 * tmp68 - tmp8;
                    float tmp76 = tmp74 * tmp69 - tmp9;
                    float tmp77 = tmp74 * tmp70 - tmp10;
                    sret_ptr.k2.x = tmp75;
                    sret_ptr.k2.y = tmp76;
                    sret_ptr.k2.z = tmp77;
                    sret_ptr.event_type = 10;
                    float3 tmp78 = float3(tmp75, tmp76, tmp77);
                    float3 tmp79 = tmp78 * tmp14;
                    if (tmp79.x + tmp79.y + tmp79.z > 0.0) {
                        sret_ptr.bsdf_over_pdf.x = 1.0;
                        sret_ptr.bsdf_over_pdf.y = 1.0;
                        sret_ptr.bsdf_over_pdf.z = 1.0;
                        float3 tmp80 = tmp78 * tmp20;
                        float3 tmp81 = tmp78 * tmp71;
                        float tmp82 = tmp54 * 2.0;
                        float tmp83 = tmp82 * tmp39 / tmp73;
                        float tmp84 = tmp83 > 1.0 ? 1.0 : tmp83;
                        float tmp85 = abs(tmp80.x + tmp80.y + tmp80.z) * tmp82 / abs(tmp81.x + tmp81.y + tmp81.z);
                        float tmp86 = tmp85 > 1.0 ? 1.0 : tmp85;
                        float tmp87 = tmp84 < tmp86 ? tmp84 : tmp86;
                        if (tmp87 > 0.0) {
                            float tmp88 = tmp87 / tmp84;
                            sret_ptr.bsdf_over_pdf.x = tmp88;
                            sret_ptr.bsdf_over_pdf.y = tmp88;
                            sret_ptr.bsdf_over_pdf.z = tmp88;
                            float tmp89 = phi_out65 * 10000.0;
                            float tmp90 = phi_out67 * 10000.0;
                            float tmp91 = tmp89 * tmp89 + tmp53 + tmp90 * tmp90;
                            sret_ptr.pdf = tmp54 * 3.183099e+07 / (tmp91 * tmp91) * (0.25 / tmp56) * tmp84;
                            sret_ptr.handle = 0;
                            tmp2.ior1.x = asfloat(phi_out);
                            tmp2.ior1.y = asfloat(asint(sret_ptr.ior1.y));
                            tmp2.ior1.z = asfloat(asint(sret_ptr.ior1.z));
                            tmp2.ior2.x = asfloat(phi_out37);
                            tmp2.ior2.y = asfloat(asint(sret_ptr.ior2.y));
                            tmp2.ior2.z = asfloat(asint(sret_ptr.ior2.z));
                            tmp2.k1.x = tmp8;
                            tmp2.k1.y = tmp9;
                            tmp2.k1.z = tmp10;
                            tmp2.k2.x = tmp75;
                            tmp2.k2.y = tmp76;
                            tmp2.k2.z = tmp77;
                            gen_custom_curve_layer_pdf(tmp2, state, normal_buf);
                            int tmp92 = asint(tmp2.pdf);
                            sret_ptr.pdf = asfloat(tmp92);
                        } else {
                            sret_ptr.pdf = 0.0;
                            sret_ptr.event_type = 0;
                        }
                    } else {
                        sret_ptr.pdf = 0.0;
                        sret_ptr.event_type = 0;
                    }
                } else {
                    sret_ptr.pdf = 0.0;
                    sret_ptr.event_type = 0;
                }
            }
        }
    } else {
        float3 tmp93 = state.geom_normal;
        float tmp94 = sret_ptr.k1.x;
        float tmp95 = sret_ptr.k1.y;
        float tmp96 = sret_ptr.k1.z;
        float3 tmp97 = float3(tmp94, tmp95, tmp96);
        float3 tmp98 = tmp97 * tmp93;
        float tmp99 = asfloat((asint(tmp98.x + tmp98.y + tmp98.z) & -2147483648) | 1065353216);
        float3 tmp100 = float3(tmp93.x * tmp99, tmp93.y * tmp99, tmp93.z * tmp99);
        float3 tmp101 = tmp100 * tmp3;
        float tmp102 = asfloat((asint(tmp101.x + tmp101.y + tmp101.z) & -2147483648) | 1065353216);
        float tmp103 = tmp3.x * tmp102;
        float tmp104 = tmp3.y * tmp102;
        float tmp105 = tmp3.z * tmp102;
        tmp1.x = tmp103;
        tmp1.y = tmp104;
        tmp1.z = tmp105;
        float3 tmp106 = float3(tmp103, tmp104, tmp105);
        float3 tmp107 = tmp106 * tmp97;
        float tmp108 = tmp107.x + tmp107.y + tmp107.z;
        float tmp109 = tmp108 > 0.0 ? tmp108 : 0.0;
        float tmp110 = tmp109 < 1.0 ? tmp109 : 1.0;
        float tmp111 = tmp110 > 0.0 ? tmp110 : 0.0;
        float tmp112 = 1.0 - (tmp111 < 1.0 ? tmp111 : 1.0);
        float tmp113 = tmp112 * tmp112;
        float tmp114 = tmp113 * tmp113 * (tmp112 * 0.95);
        float tmp115 = tmp114 + 0.04;
        if (tmp4 < tmp115) {
            float tmp116 = tmp4 / tmp115;
            sret_ptr.xi.z = tmp116;
            float3 tmp117 = state.tangent_u[0];
            float3 tmp118 = tmp106 * tmp100;
            float tmp119 = asfloat((asint(tmp118.x + tmp118.y + tmp118.z) & -2147483648) | 1065353216);
            float tmp120 = tmp103 * tmp119;
            float tmp121 = tmp104 * tmp119;
            float tmp122 = tmp105 * tmp119;
            float3 tmp123 = float3(tmp120, tmp121, tmp122);
            float3 tmp124 = tmp123.zxy;
            float3 tmp125 = tmp123.yzx;
            float3 tmp126 = tmp117.yzx * tmp124 - tmp117.zxy * tmp125;
            float3 tmp127 = tmp126 * tmp126;
            float tmp128 = tmp127.x + tmp127.y + tmp127.z;
            if (tmp128 < 1e-08) {
                sret_ptr.pdf = 0.0;
                sret_ptr.event_type = 0;
                phi_in129 = 0;
                phi_in131 = float3(0.0, 0.0, 0.0);
                phi_in133 = 0.0;
                phi_in135 = 0;
                phi_in137 = 0;
                phi_in139 = 0.0;
                phi_in141 = 0.0;
                phi_in143 = 0.0;
                phi_in145 = 0.0;
                phi_in147 = false;
            } else {
                float tmp149 = 1.0 / sqrt(tmp128);
                float tmp150 = tmp149 * tmp126.x;
                float tmp151 = tmp149 * tmp126.y;
                float tmp152 = tmp149 * tmp126.z;
                float3 tmp153 = float3(tmp150, tmp151, tmp152);
                float3 tmp154 = tmp153.zxy * tmp125 - tmp153.yzx * tmp124;
                float tmp155 = sret_ptr.ior1.x;
                int tmp156 = asint(tmp155);
                phi_in157 = tmp156;
                if (tmp155 == -1.0) {
                    sret_ptr.ior1.x = 1.0;
                    sret_ptr.ior1.y = 1.0;
                    sret_ptr.ior1.z = 1.0;
                    phi_in157 = 1065353216;
                }
                phi_out158 = phi_in157;
                float tmp159 = sret_ptr.ior2.x;
                int tmp160 = asint(tmp159);
                phi_in161 = tmp160;
                if (tmp159 == -1.0) {
                    sret_ptr.ior2.x = 1.0;
                    sret_ptr.ior2.y = 1.0;
                    sret_ptr.ior2.z = 1.0;
                    phi_in161 = 1065353216;
                }
                phi_out162 = phi_in161;
                float3 tmp163 = tmp123 * tmp97;
                float tmp164 = abs(tmp163.x + tmp163.y + tmp163.z);
                float3 tmp165 = tmp154 * tmp97;
                float3 tmp166 = tmp153 * tmp97;
                float tmp167 = sret_ptr.xi.y;
                float tmp168 = sret_ptr.xi.x * 6.283185;
                float tmp169 = sin(tmp168);
                float tmp170 = cos(tmp168);
                float tmp171 = sqrt(1e-08 / (tmp169 * tmp169 + tmp170 * tmp170)) * 10000.0;
                float tmp172 = tmp171 * tmp170;
                float tmp173 = tmp171 * tmp169;
                float tmp174 = tmp167 / ((tmp173 * tmp173 + tmp172 * tmp172) * (1e+08 - tmp167 * 1e+08));
                float tmp175 = tmp174 + 1.0;
                float tmp176 = sqrt(tmp174 / tmp175);
                float tmp177 = tmp176 * tmp173;
                float tmp178 = 1.0 / tmp175;
                float tmp179 = sqrt(tmp178);
                float tmp180 = tmp176 * tmp172;
                float tmp181 = tmp179 * tmp164;
                float tmp182 = tmp180 * (tmp166.x + tmp166.y + tmp166.z) + tmp177 * (tmp165.x + tmp165.y + tmp165.z);
                float tmp183 = tmp182 + tmp181;
                float tmp184 = tmp181 - tmp182;
                float tmp185 = tmp184 > 0.0 ? tmp184 : 0.0;
                float tmp186 = tmp185 / (tmp185 + (tmp183 > 0.0 ? tmp183 : 0.0));
                if (tmp116 < tmp186) {
                    sret_ptr.xi.z = tmp116 / tmp186;
                    float tmp187 = -tmp177;
                    float tmp188 = -tmp180;
                    phi_in189 = tmp187;
                    phi_in191 = tmp188;
                } else {
                    sret_ptr.xi.z = (tmp116 - tmp186) / (1.0 - tmp186);
                    phi_in189 = tmp177;
                    phi_in191 = tmp180;
                }
                phi_out190 = phi_in189;
                phi_out192 = phi_in191;
                if (tmp179 == 0.0) {
                    sret_ptr.pdf = 0.0;
                    sret_ptr.event_type = 0;
                    phi_in129 = 0;
                    phi_in131 = float3(0.0, 0.0, 0.0);
                    phi_in133 = 0.0;
                    phi_in135 = 0;
                    phi_in137 = 0;
                    phi_in139 = 0.0;
                    phi_in141 = 0.0;
                    phi_in143 = 0.0;
                    phi_in145 = 0.0;
                    phi_in147 = false;
                } else {
                    float tmp193 = phi_out190 * tmp154.x + tmp179 * tmp120 + phi_out192 * tmp150;
                    float tmp194 = phi_out190 * tmp154.y + tmp179 * tmp121 + phi_out192 * tmp151;
                    float tmp195 = phi_out190 * tmp154.z + tmp179 * tmp122 + phi_out192 * tmp152;
                    float3 tmp196 = float3(tmp193, tmp194, tmp195);
                    float3 tmp197 = tmp196 * tmp97;
                    float tmp198 = tmp197.x + tmp197.y + tmp197.z;
                    if (tmp198 > 0.0) {
                        float tmp199 = tmp198 * 2.0;
                        float tmp200 = tmp199 * tmp193 - tmp94;
                        float tmp201 = tmp199 * tmp194 - tmp95;
                        float tmp202 = tmp199 * tmp195 - tmp96;
                        sret_ptr.k2.x = tmp200;
                        sret_ptr.k2.y = tmp201;
                        sret_ptr.k2.z = tmp202;
                        sret_ptr.event_type = 10;
                        float3 tmp203 = float3(tmp200, tmp201, tmp202);
                        float3 tmp204 = tmp203 * tmp100;
                        if (tmp204.x + tmp204.y + tmp204.z > 0.0) {
                            sret_ptr.bsdf_over_pdf.x = 1.0;
                            sret_ptr.bsdf_over_pdf.y = 1.0;
                            sret_ptr.bsdf_over_pdf.z = 1.0;
                            float3 tmp205 = tmp203 * tmp123;
                            float3 tmp206 = tmp203 * tmp196;
                            float tmp207 = tmp179 * 2.0;
                            float tmp208 = tmp207 * tmp164 / tmp198;
                            float tmp209 = tmp208 > 1.0 ? 1.0 : tmp208;
                            float tmp210 = abs(tmp205.x + tmp205.y + tmp205.z) * tmp207 / abs(tmp206.x + tmp206.y + tmp206.z);
                            float tmp211 = tmp210 > 1.0 ? 1.0 : tmp210;
                            float tmp212 = tmp209 < tmp211 ? tmp209 : tmp211;
                            if (tmp212 > 0.0) {
                                float tmp213 = tmp212 / tmp209;
                                sret_ptr.bsdf_over_pdf.x = tmp213;
                                sret_ptr.bsdf_over_pdf.y = tmp213;
                                sret_ptr.bsdf_over_pdf.z = tmp213;
                                float tmp214 = phi_out190 * 10000.0;
                                float tmp215 = phi_out192 * 10000.0;
                                float tmp216 = tmp214 * tmp214 + tmp178 + tmp215 * tmp215;
                                float tmp217 = tmp179 * 3.183099e+07 / (tmp216 * tmp216) * (0.25 / tmp181) * tmp209;
                                sret_ptr.pdf = tmp217;
                                sret_ptr.handle = 0;
                                phi_in129 = 1;
                                phi_in131 = tmp203;
                                phi_in133 = tmp213;
                                phi_in135 = phi_out162;
                                phi_in137 = phi_out158;
                                phi_in139 = tmp202;
                                phi_in141 = tmp201;
                                phi_in143 = tmp200;
                                phi_in145 = tmp217;
                                phi_in147 = true;
                            } else {
                                sret_ptr.pdf = 0.0;
                                sret_ptr.event_type = 0;
                                phi_in129 = 0;
                                phi_in131 = float3(0.0, 0.0, 0.0);
                                phi_in133 = 0.0;
                                phi_in135 = 0;
                                phi_in137 = 0;
                                phi_in139 = 0.0;
                                phi_in141 = 0.0;
                                phi_in143 = 0.0;
                                phi_in145 = 0.0;
                                phi_in147 = false;
                            }
                        } else {
                            sret_ptr.pdf = 0.0;
                            sret_ptr.event_type = 0;
                            phi_in129 = 0;
                            phi_in131 = float3(0.0, 0.0, 0.0);
                            phi_in133 = 0.0;
                            phi_in135 = 0;
                            phi_in137 = 0;
                            phi_in139 = 0.0;
                            phi_in141 = 0.0;
                            phi_in143 = 0.0;
                            phi_in145 = 0.0;
                            phi_in147 = false;
                        }
                    } else {
                        sret_ptr.pdf = 0.0;
                        sret_ptr.event_type = 0;
                        phi_in129 = 0;
                        phi_in131 = float3(0.0, 0.0, 0.0);
                        phi_in133 = 0.0;
                        phi_in135 = 0;
                        phi_in137 = 0;
                        phi_in139 = 0.0;
                        phi_in141 = 0.0;
                        phi_in143 = 0.0;
                        phi_in145 = 0.0;
                        phi_in147 = false;
                    }
                }
            }
        } else {
            sret_ptr.xi.z = (1.0 - tmp4) / (0.96 - tmp114);
            float3 tmp218 = state.tangent_u[0];
            float3 tmp219 = tmp106.zxy;
            float3 tmp220 = tmp106.yzx;
            float3 tmp221 = tmp218.yzx * tmp219 - tmp218.zxy * tmp220;
            float3 tmp222 = tmp221 * tmp221;
            float tmp223 = tmp222.x + tmp222.y + tmp222.z;
            if (tmp223 < 1e-08) {
                sret_ptr.pdf = 0.0;
                sret_ptr.event_type = 0;
                phi_in129 = 0;
                phi_in131 = float3(0.0, 0.0, 0.0);
                phi_in133 = 0.0;
                phi_in135 = 0;
                phi_in137 = 0;
                phi_in139 = 0.0;
                phi_in141 = 0.0;
                phi_in143 = 0.0;
                phi_in145 = 0.0;
                phi_in147 = false;
            } else {
                float tmp224 = 1.0 / sqrt(tmp223);
                float tmp225 = tmp224 * tmp221.x;
                float tmp226 = tmp224 * tmp221.y;
                float tmp227 = tmp224 * tmp221.z;
                float3 tmp228 = float3(tmp225, tmp226, tmp227);
                float3 tmp229 = tmp228.zxy * tmp220 - tmp228.yzx * tmp219;
                float tmp230 = sret_ptr.xi.x;
                float tmp231 = sret_ptr.xi.y;
                phi_in232 = 1.0;
                phi_in234 = 0.0;
                phi_in236 = 0.0;
                if (!(tmp230 == 0.0 && tmp231 == 0.0)) {
                    float tmp238 = tmp230 * 2.0;
                    float tmp239 = tmp231 * 2.0;
                    float tmp240 = tmp238 < 1.0 ? tmp238 : tmp238 + -2.0;
                    float tmp241 = tmp239 < 1.0 ? tmp239 : tmp239 + -2.0;
                    float tmp242 = tmp240 * tmp240;
                    float tmp243 = tmp241 * tmp241;
                    if (tmp242 > tmp243) {
                        float tmp244 = tmp241 / tmp240 * -0.7853982;
                        phi_in245 = tmp240;
                        phi_in247 = tmp244;
                        phi_in249 = tmp242;
                    } else {
                        float tmp251 = tmp240 / tmp241 * 0.7853982 + -1.570796;
                        phi_in245 = tmp241;
                        phi_in247 = tmp251;
                        phi_in249 = tmp243;
                    }
                    phi_out246 = phi_in245;
                    phi_out248 = phi_in247;
                    phi_out250 = phi_in249;
                    float tmp252 = 1.0 - phi_out250;
                    phi_in232 = 1.0;
                    phi_in234 = 0.0;
                    phi_in236 = 0.0;
                    if (tmp252 > 0.0) {
                        float tmp253 = sin(phi_out248) * phi_out246;
                        float tmp254 = sqrt(tmp252);
                        float tmp255 = cos(phi_out248) * phi_out246;
                        phi_in232 = tmp254;
                        phi_in234 = tmp253;
                        phi_in236 = tmp255;
                    }
                }
                phi_out233 = phi_in232;
                phi_out235 = phi_in234;
                phi_out237 = phi_in236;
                float tmp256 = phi_out235 * tmp229.x + phi_out233 * tmp103 + phi_out237 * tmp225;
                float tmp257 = phi_out235 * tmp229.y + phi_out233 * tmp104 + phi_out237 * tmp226;
                float tmp258 = phi_out235 * tmp229.z + phi_out233 * tmp105 + phi_out237 * tmp227;
                float3 tmp259 = float3(sqrt(tmp256 * tmp256 + tmp257 * tmp257 + tmp258 * tmp258), 0.0, 0.0);
                float3 tmp260 = float3(tmp256, tmp257, tmp258) / tmp259.xxx;
                sret_ptr.k2.x = tmp260.x;
                sret_ptr.k2.y = tmp260.y;
                sret_ptr.k2.z = tmp260.z;
                phi_in261 = 0;
                phi_in263 = 0;
                phi_in265 = float3(0.0, 0.0, 0.0);
                phi_in267 = 0.0;
                phi_in269 = 0;
                phi_in271 = 0;
                phi_in273 = 0.0;
                phi_in275 = 0.0;
                phi_in277 = 0.0;
                phi_in279 = 0.0;
                phi_in281 = false;
                if (phi_out233 > 0.0) {
                    float3 tmp283 = tmp260 * tmp100;
                    phi_in261 = 0;
                    phi_in263 = 0;
                    phi_in265 = float3(0.0, 0.0, 0.0);
                    phi_in267 = 0.0;
                    phi_in269 = 0;
                    phi_in271 = 0;
                    phi_in273 = 0.0;
                    phi_in275 = 0.0;
                    phi_in277 = 0.0;
                    phi_in279 = 0.0;
                    phi_in281 = false;
                    if (tmp283.x + tmp283.y + tmp283.z > 0.0) {
                        sret_ptr.bsdf_over_pdf.x = 1.0;
                        sret_ptr.bsdf_over_pdf.y = 1.0;
                        sret_ptr.bsdf_over_pdf.z = 1.0;
                        float tmp284 = phi_out233 * 0.3183099;
                        sret_ptr.pdf = tmp284;
                        sret_ptr.event_type = 9;
                        sret_ptr.handle = 0;
                        int tmp285 = asint(sret_ptr.ior1.x);
                        int tmp286 = asint(sret_ptr.ior2.x);
                        phi_in261 = 1;
                        phi_in263 = 1;
                        phi_in265 = tmp260;
                        phi_in267 = 1.0;
                        phi_in269 = tmp286;
                        phi_in271 = tmp285;
                        phi_in273 = tmp260.z;
                        phi_in275 = tmp260.y;
                        phi_in277 = tmp260.x;
                        phi_in279 = tmp284;
                        phi_in281 = false;
                    }
                }
                phi_out262 = phi_in261;
                phi_out264 = phi_in263;
                phi_out266 = phi_in265;
                phi_out268 = phi_in267;
                phi_out270 = phi_in269;
                phi_out272 = phi_in271;
                phi_out274 = phi_in273;
                phi_out276 = phi_in275;
                phi_out278 = phi_in277;
                phi_out280 = phi_in279;
                phi_out282 = phi_in281;
                phi_in129 = phi_out264;
                phi_in131 = phi_out266;
                phi_in133 = phi_out268;
                phi_in135 = phi_out270;
                phi_in137 = phi_out272;
                phi_in139 = phi_out274;
                phi_in141 = phi_out276;
                phi_in143 = phi_out278;
                phi_in145 = phi_out280;
                phi_in147 = phi_out282;
                if (phi_out262 == 0) {
                    sret_ptr.pdf = 0.0;
                    sret_ptr.event_type = 0;
                    phi_in129 = 0;
                    phi_in131 = float3(0.0, 0.0, 0.0);
                    phi_in133 = 0.0;
                    phi_in135 = 0;
                    phi_in137 = 0;
                    phi_in139 = 0.0;
                    phi_in141 = 0.0;
                    phi_in143 = 0.0;
                    phi_in145 = 0.0;
                    phi_in147 = false;
                }
            }
        }
        phi_out130 = phi_in129;
        phi_out132 = phi_in131;
        phi_out134 = phi_in133;
        phi_out136 = phi_in135;
        phi_out138 = phi_in137;
        phi_out140 = phi_in139;
        phi_out142 = phi_in141;
        phi_out144 = phi_in143;
        phi_out146 = phi_in145;
        phi_out148 = phi_in147;
        if (phi_out130 != 0) {
            float3 tmp287 = phi_out132 * tmp106;
            float tmp288 = tmp287.x + tmp287.y + tmp287.z;
            float tmp289 = abs(tmp288);
            float tmp290 = phi_out142 + tmp95;
            float tmp291 = phi_out144 + tmp94;
            float tmp292 = phi_out140 + tmp96;
            float3 tmp293 = float3(sqrt(tmp290 * tmp290 + tmp292 * tmp292 + tmp291 * tmp291), 0.0, 0.0);
            float3 tmp294 = float3(tmp291, tmp290, tmp292) / tmp293.xxx * tmp97;
            float tmp295 = abs(tmp294.x + tmp294.y + tmp294.z);
            tmp0.ior1.x = asfloat(phi_out138);
            tmp0.ior1.y = asfloat(asint(sret_ptr.ior1.y));
            tmp0.ior1.z = asfloat(asint(sret_ptr.ior1.z));
            tmp0.ior2.x = asfloat(phi_out136);
            tmp0.ior2.y = asfloat(asint(sret_ptr.ior2.y));
            tmp0.ior2.z = asfloat(asint(sret_ptr.ior2.z));
            tmp0.k1.x = tmp94;
            tmp0.k1.y = tmp95;
            tmp0.k1.z = tmp96;
            tmp0.k2.x = phi_out144;
            tmp0.k2.y = phi_out142;
            tmp0.k2.z = phi_out140;
            if (phi_out148) {
                float tmp296 = tmp295 > 0.0 ? tmp295 : 0.0;
                float tmp297 = 1.0 - (tmp296 < 1.0 ? tmp296 : 1.0);
                float tmp298 = tmp297 * tmp297;
                float tmp299 = phi_out134 * (1.0 / tmp115) * (tmp298 * tmp298 * (tmp297 * 0.95) + 0.04);
                sret_ptr.bsdf_over_pdf.x = tmp299;
                sret_ptr.bsdf_over_pdf.y = tmp299;
                sret_ptr.bsdf_over_pdf.z = tmp299;
                float3 tmp300 = phi_out132 * tmp100;
                phi_in301 = 0.0;
                if (tmp300.x + tmp300.y + tmp300.z > 0.0) {
                    float tmp303 = (tmp288 > 0.0 ? tmp288 : 0.0) * 0.3183099;
                    phi_in301 = tmp303;
                }
                phi_out302 = phi_in301;
                tmp0.pdf = phi_out302;
                float tmp304 = phi_out302 * (0.96 - tmp114);
                phi_in305 = phi_out146;
                phi_in307 = tmp115;
                phi_in309 = tmp304;
            } else {
                float tmp311 = tmp289 > 0.0 ? tmp289 : 0.0;
                float tmp312 = 1.0 - (tmp311 < 1.0 ? tmp311 : 1.0);
                float tmp313 = tmp312 * tmp312;
                float tmp314 = tmp313 * tmp313 * (tmp312 * 0.95) + 0.04;
                float tmp315 = 0.96 - tmp114;
                float tmp316 = phi_out134 * (1.0 / tmp315) * (1.0 - (tmp115 > tmp314 ? tmp115 : tmp314));
                sret_ptr.bsdf_over_pdf.x = tmp316;
                sret_ptr.bsdf_over_pdf.y = tmp316;
                sret_ptr.bsdf_over_pdf.z = tmp316;
                gen_microfacet_ggx_vcavities_bsdf_pdf0(tmp0, state, tmp1);
                float tmp317 = tmp0.pdf;
                float tmp318 = tmp317 * tmp115;
                float tmp319 = sret_ptr.pdf;
                phi_in305 = tmp319;
                phi_in307 = tmp315;
                phi_in309 = tmp318;
            }
            phi_out306 = phi_in305;
            phi_out308 = phi_in307;
            phi_out310 = phi_in309;
            sret_ptr.pdf = phi_out308 * phi_out306 + phi_out310;
        }
    }
    return;
}

void mdl_bsdf_scattering_3_evaluate(inout Bsdf_evaluate_data sret_ptr, in Shading_state_material state)
{
    float phi_in;
    float phi_out;
    float phi_in29;
    float phi_out30;
    float phi_in31;
    float phi_out32;
    float phi_in65;
    float phi_out66;
    float phi_in67;
    float phi_out68;
    float phi_in101;
    float phi_out102;
    float3 tmp0 = state.normal;
    sret_ptr.bsdf_diffuse = (float3)0;
    sret_ptr.bsdf_glossy = (float3)0;
    float3 tmp1 = state.geom_normal;
    float tmp2 = sret_ptr.k1.x;
    float tmp3 = sret_ptr.k1.y;
    float tmp4 = sret_ptr.k1.z;
    float3 tmp5 = float3(tmp2, tmp3, tmp4);
    float3 tmp6 = tmp5 * tmp1;
    float tmp7 = asfloat((asint(tmp6.x + tmp6.y + tmp6.z) & -2147483648) | 1065353216);
    float3 tmp8 = float3(tmp1.x * tmp7, tmp1.y * tmp7, tmp1.z * tmp7);
    float3 tmp9 = tmp8 * tmp0;
    float tmp10 = asfloat((asint(tmp9.x + tmp9.y + tmp9.z) & -2147483648) | 1065353216);
    float tmp11 = tmp0.x * tmp10;
    float tmp12 = tmp0.y * tmp10;
    float tmp13 = tmp0.z * tmp10;
    float3 tmp14 = float3(tmp11, tmp12, tmp13);
    float3 tmp15 = tmp14 * tmp5;
    float tmp16 = tmp15.x + tmp15.y + tmp15.z;
    float tmp17 = tmp16 > 0.0 ? tmp16 : 0.0;
    float tmp18 = tmp17 < 1.0 ? tmp17 : 1.0;
    float tmp19 = sret_ptr.k2.x;
    float tmp20 = sret_ptr.k2.y;
    float tmp21 = sret_ptr.k2.z;
    float3 tmp22 = float3(tmp19, tmp20, tmp21);
    float3 tmp23 = tmp14 * tmp22;
    float tmp24 = tmp23.x + tmp23.y + tmp23.z;
    float tmp25 = abs(tmp24);
    float3 tmp26 = tmp8 * tmp22;
    float tmp27 = tmp26.x + tmp26.y + tmp26.z;
    bool tmp28 = tmp27 < 0.0;
    phi_in = tmp21;
    phi_in29 = tmp19;
    phi_in31 = tmp20;
    if (tmp28) {
        float tmp33 = tmp25 * 2.0;
        float tmp34 = tmp33 * tmp11 + tmp19;
        float tmp35 = tmp33 * tmp12 + tmp20;
        float tmp36 = tmp33 * tmp13 + tmp21;
        phi_in = tmp36;
        phi_in29 = tmp34;
        phi_in31 = tmp35;
    }
    phi_out = phi_in;
    phi_out30 = phi_in29;
    phi_out32 = phi_in31;
    float tmp37 = phi_out32 + tmp3;
    float tmp38 = phi_out30 + tmp2;
    float tmp39 = phi_out + tmp4;
    float3 tmp40 = float3(sqrt(tmp38 * tmp38 + tmp39 * tmp39 + tmp37 * tmp37), 0.0, 0.0);
    float3 tmp41 = float3(tmp38, tmp37, tmp39) / tmp40.xxx * tmp5;
    float tmp42 = abs(tmp41.x + tmp41.y + tmp41.z);
    float tmp43 = tmp42 > 0.0 ? tmp42 : 0.0;
    float tmp44 = 1.0 - (tmp43 < 1.0 ? tmp43 : 1.0);
    float tmp45 = tmp44 * tmp44;
    float tmp46 = tmp45 * tmp45 * (tmp44 * 0.95) + 0.04;
    float tmp47 = tmp18 > 0.0 ? tmp18 : 0.0;
    float tmp48 = 1.0 - (tmp47 < 1.0 ? tmp47 : 1.0);
    float tmp49 = tmp48 * tmp48;
    float tmp50 = tmp49 * tmp49 * (tmp48 * 0.95);
    float tmp51 = tmp50 + 0.04;
    float tmp52 = tmp25 > 0.0 ? tmp25 : 0.0;
    float tmp53 = 1.0 - (tmp52 < 1.0 ? tmp52 : 1.0);
    float tmp54 = tmp53 * tmp53;
    float tmp55 = tmp54 * tmp54 * (tmp53 * 0.95) + 0.04;
    float3 tmp56 = state.tangent_u[0];
    float3 tmp57 = tmp14 * tmp8;
    float tmp58 = asfloat((asint(tmp57.x + tmp57.y + tmp57.z) & -2147483648) | 1065353216);
    float3 tmp59 = float3(tmp11 * tmp58, tmp12 * tmp58, tmp13 * tmp58);
    float3 tmp60 = tmp59.zxy;
    float3 tmp61 = tmp59.yzx;
    float3 tmp62 = tmp56.yzx * tmp60 - tmp56.zxy * tmp61;
    float3 tmp63 = tmp62 * tmp62;
    float tmp64 = tmp63.x + tmp63.y + tmp63.z;
    if (tmp64 < 1e-08) {
        sret_ptr.pdf = 0.0;
        phi_in65 = 0.0;
        phi_in67 = 0.0;
    } else {
        float tmp69 = 1.0 / sqrt(tmp64);
        float3 tmp70 = float3(tmp69 * tmp62.x, tmp69 * tmp62.y, tmp69 * tmp62.z);
        float3 tmp71 = tmp70.zxy * tmp61 - tmp70.yzx * tmp60;
        if (sret_ptr.ior1.x == -1.0) {
            sret_ptr.ior1.x = 1.0;
            sret_ptr.ior1.y = 1.0;
            sret_ptr.ior1.z = 1.0;
        }
        if (sret_ptr.ior2.x == -1.0) {
            sret_ptr.ior2.x = 1.0;
            sret_ptr.ior2.y = 1.0;
            sret_ptr.ior2.z = 1.0;
        }
        if (tmp28) {
            sret_ptr.pdf = 0.0;
            phi_in65 = 0.0;
            phi_in67 = 0.0;
        } else {
            float3 tmp72 = tmp59 * tmp5;
            float tmp73 = abs(tmp72.x + tmp72.y + tmp72.z);
            float3 tmp74 = tmp59 * tmp22;
            float tmp75 = tmp19 + tmp2;
            float tmp76 = tmp20 + tmp3;
            float tmp77 = tmp21 + tmp4;
            float3 tmp78 = float3(sqrt(tmp76 * tmp76 + tmp75 * tmp75 + tmp77 * tmp77), 0.0, 0.0);
            float3 tmp79 = float3(tmp75, tmp76, tmp77) / tmp78.xxx;
            float3 tmp80 = tmp59 * tmp79;
            float tmp81 = tmp80.x + tmp80.y + tmp80.z;
            float3 tmp82 = tmp79 * tmp5;
            float tmp83 = tmp82.x + tmp82.y + tmp82.z;
            float3 tmp84 = tmp79 * tmp22;
            float tmp85 = tmp84.x + tmp84.y + tmp84.z;
            if (tmp85 < 0.0 || (tmp83 < 0.0 || tmp81 < 0.0)) {
                sret_ptr.pdf = 0.0;
                phi_in65 = 0.0;
                phi_in67 = 0.0;
            } else {
                float3 tmp86 = tmp71 * tmp79;
                float3 tmp87 = tmp70 * tmp79;
                float tmp88 = (tmp86.x + tmp86.y + tmp86.z) * 10000.0;
                float tmp89 = (tmp87.x + tmp87.y + tmp87.z) * 10000.0;
                float tmp90 = tmp89 * tmp89 + tmp81 * tmp81 + tmp88 * tmp88;
                float tmp91 = tmp81 * 2.0;
                float tmp92 = tmp91 * tmp73 / tmp83;
                float tmp93 = tmp92 > 1.0 ? 1.0 : tmp92;
                float tmp94 = tmp91 * abs(tmp74.x + tmp74.y + tmp74.z) / tmp85;
                float tmp95 = tmp94 > 1.0 ? 1.0 : tmp94;
                float tmp96 = tmp81 * 3.183099e+07 / (tmp90 * tmp90) * (0.25 / (tmp73 * tmp81));
                float tmp97 = tmp96 * (tmp93 < tmp95 ? tmp93 : tmp95);
                float tmp98 = tmp96 * tmp93;
                sret_ptr.pdf = tmp98;
                phi_in65 = tmp98;
                phi_in67 = tmp97;
            }
        }
    }
    phi_out66 = phi_in65;
    phi_out68 = phi_in67;
    float tmp99 = phi_out68 * tmp46;
    sret_ptr.bsdf_glossy.x = tmp99;
    sret_ptr.bsdf_glossy.y = tmp99;
    sret_ptr.bsdf_glossy.z = tmp99;
    float tmp100 = phi_out66 * tmp51;
    phi_in101 = 0.0;
    if (tmp27 > 0.0) {
        float tmp103 = (tmp24 > 0.0 ? tmp24 : 0.0) * 0.3183099;
        float tmp104 = (1.0 - (tmp51 > tmp55 ? tmp51 : tmp55)) * tmp103;
        sret_ptr.bsdf_diffuse.x = tmp104;
        sret_ptr.bsdf_diffuse.y = tmp104;
        sret_ptr.bsdf_diffuse.z = tmp104;
        phi_in101 = tmp103;
    }
    phi_out102 = phi_in101;
    sret_ptr.pdf = phi_out102 * (0.96 - tmp50) + tmp100;
    return;
}

void mdl_bsdf_scattering_3_pdf(inout Bsdf_pdf_data sret_ptr, in Shading_state_material state)
{
    float3 normal_buf;
    float3 tmp0 = state.normal;
    normal_buf.x = tmp0.x;
    normal_buf.y = tmp0.y;
    normal_buf.z = tmp0.z;
    gen_custom_curve_layer_pdf(sret_ptr, state, normal_buf);
    return;
}

void mdl_edf_emission_3_init(inout Shading_state_material state)
{
    return;
}

void mdl_edf_emission_3_sample(inout Edf_sample_data sret_ptr, in Shading_state_material state)
{
    float phi_in;
    float phi_out;
    float phi_in4;
    float phi_out5;
    float phi_in6;
    float phi_out7;
    float phi_in15;
    float phi_out16;
    float phi_in17;
    float phi_out18;
    float phi_in19;
    float phi_out20;
    int phi_in43;
    int phi_out44;
    float3 tmp0 = state.normal;
    float3 tmp1 = state.geom_normal;
    float tmp2 = sret_ptr.xi.x;
    float tmp3 = sret_ptr.xi.y;
    phi_in = 1.0;
    phi_in4 = 0.0;
    phi_in6 = 0.0;
    if (!(tmp2 == 0.0 && tmp3 == 0.0)) {
        float tmp8 = tmp2 * 2.0;
        float tmp9 = tmp3 * 2.0;
        float tmp10 = tmp8 < 1.0 ? tmp8 : tmp8 + -2.0;
        float tmp11 = tmp9 < 1.0 ? tmp9 : tmp9 + -2.0;
        float tmp12 = tmp10 * tmp10;
        float tmp13 = tmp11 * tmp11;
        if (tmp12 > tmp13) {
            float tmp14 = tmp11 / tmp10 * -0.7853982;
            phi_in15 = tmp10;
            phi_in17 = tmp14;
            phi_in19 = tmp12;
        } else {
            float tmp21 = tmp10 / tmp11 * 0.7853982 + -1.570796;
            phi_in15 = tmp11;
            phi_in17 = tmp21;
            phi_in19 = tmp13;
        }
        phi_out16 = phi_in15;
        phi_out18 = phi_in17;
        phi_out20 = phi_in19;
        float tmp22 = 1.0 - phi_out20;
        phi_in = 1.0;
        phi_in4 = 0.0;
        phi_in6 = 0.0;
        if (tmp22 > 0.0) {
            float tmp23 = sin(phi_out18) * phi_out16;
            float tmp24 = sqrt(tmp22);
            float tmp25 = cos(phi_out18) * phi_out16;
            phi_in = tmp24;
            phi_in4 = tmp23;
            phi_in6 = tmp25;
        }
    }
    phi_out = phi_in;
    phi_out5 = phi_in4;
    phi_out7 = phi_in6;
    float3 tmp26 = state.tangent_u[0];
    float3 tmp27 = tmp0.zxy;
    float3 tmp28 = tmp0.yzx;
    float3 tmp29 = tmp26.yzx * tmp27 - tmp26.zxy * tmp28;
    float3 tmp30 = tmp29 * tmp29;
    float tmp31 = tmp30.x + tmp30.y + tmp30.z;
    if (tmp31 < 1e-08) {
        sret_ptr.k1 = (float3)0;
        sret_ptr.pdf = float(0);
        sret_ptr.edf_over_pdf = (float3)0;
        sret_ptr.event_type = int(0);
    } else {
        float tmp32 = 1.0 / sqrt(tmp31);
        float tmp33 = tmp32 * tmp29.x;
        float tmp34 = tmp32 * tmp29.y;
        float tmp35 = tmp32 * tmp29.z;
        float3 tmp36 = float3(tmp33, tmp34, tmp35);
        float3 tmp37 = tmp36.zxy * tmp28 - tmp36.yzx * tmp27;
        float tmp38 = tmp33 * phi_out7 + phi_out * tmp0.x + tmp37.x * phi_out5;
        float tmp39 = tmp34 * phi_out7 + phi_out * tmp0.y + tmp37.y * phi_out5;
        float tmp40 = tmp35 * phi_out7 + phi_out * tmp0.z + tmp37.z * phi_out5;
        float3 tmp41 = float3(sqrt(tmp38 * tmp38 + tmp39 * tmp39 + tmp40 * tmp40), 0.0, 0.0);
        float3 tmp42 = float3(tmp38, tmp39, tmp40) / tmp41.xxx;
        sret_ptr.k1.x = tmp42.x;
        sret_ptr.k1.y = tmp42.y;
        sret_ptr.k1.z = tmp42.z;
        phi_in43 = 0;
        if (phi_out > 0.0) {
            float3 tmp45 = tmp42 * tmp1;
            phi_in43 = 0;
            if (tmp45.x + tmp45.y + tmp45.z > 0.0) {
                sret_ptr.pdf = phi_out * 0.3183099;
                sret_ptr.edf_over_pdf.x = 1.0;
                sret_ptr.edf_over_pdf.y = 1.0;
                sret_ptr.edf_over_pdf.z = 1.0;
                sret_ptr.event_type = 1;
                sret_ptr.handle = 0;
                phi_in43 = 1;
            }
        }
        phi_out44 = phi_in43;
        if (phi_out44 == 0) {
            sret_ptr.k1 = (float3)0;
            sret_ptr.pdf = float(0);
            sret_ptr.edf_over_pdf = (float3)0;
            sret_ptr.event_type = int(0);
        }
    }
    return;
}

void mdl_edf_emission_3_evaluate(inout Edf_evaluate_data sret_ptr, in Shading_state_material state)
{
    float3 tmp0 = state.normal;
    float3 tmp1 = state.geom_normal;
    float3 tmp2 = float3(sret_ptr.k1.x, sret_ptr.k1.y, sret_ptr.k1.z);
    float3 tmp3 = tmp2 * tmp1;
    float tmp4 = asfloat((asint(tmp3.x + tmp3.y + tmp3.z) & -2147483648) | 1065353216);
    float3 tmp5 = float3(tmp1.x * tmp4, tmp1.y * tmp4, tmp1.z * tmp4) * tmp0;
    float tmp6 = asfloat((asint(tmp5.x + tmp5.y + tmp5.z) & -2147483648) | 1065353216);
    float3 tmp7 = float3(tmp0.x * tmp6, tmp0.y * tmp6, tmp0.z * tmp6) * tmp2;
    float tmp8 = tmp7.x + tmp7.y + tmp7.z;
    float tmp9 = tmp8 > 0.0 ? tmp8 : 0.0;
    sret_ptr.cos = tmp9;
    sret_ptr.pdf = tmp9 * 0.3183099;
    sret_ptr.edf.x = 0.3183099;
    sret_ptr.edf.y = 0.3183099;
    sret_ptr.edf.z = 0.3183099;
    return;
}

void mdl_edf_emission_3_pdf(inout Edf_pdf_data sret_ptr, in Shading_state_material state)
{
    float3 tmp0 = state.normal;
    float3 tmp1 = state.geom_normal;
    float3 tmp2 = float3(sret_ptr.k1.x, sret_ptr.k1.y, sret_ptr.k1.z);
    float3 tmp3 = tmp2 * tmp1;
    float tmp4 = asfloat((asint(tmp3.x + tmp3.y + tmp3.z) & -2147483648) | 1065353216);
    float3 tmp5 = float3(tmp1.x * tmp4, tmp1.y * tmp4, tmp1.z * tmp4) * tmp0;
    float tmp6 = asfloat((asint(tmp5.x + tmp5.y + tmp5.z) & -2147483648) | 1065353216);
    float3 tmp7 = float3(tmp0.x * tmp6, tmp0.y * tmp6, tmp0.z * tmp6) * tmp2;
    float tmp8 = tmp7.x + tmp7.y + tmp7.z;
    sret_ptr.pdf = (tmp8 > 0.0 ? tmp8 : 0.0) * 0.3183099;
    return;
}

float3 mdl_edf_emission_intensity_3(in Shading_state_material state)
{
    return float3(-3.373259e+08, -3.373259e+08, -3.373259e+08);
}

void mdl_bsdf_scattering_4_init(inout Shading_state_material state)
{
    return;
}

void gen_microfacet_ggx_vcavities_bsdf_pdf1(inout Bsdf_pdf_data p_00, in Shading_state_material p_11, in float3 p_22)
{
    float3 tmp3 = p_11.tangent_u[0];
    float3 tmp4 = p_11.geom_normal;
    float tmp5 = p_00.k1.x;
    float tmp6 = p_00.k1.y;
    float tmp7 = p_00.k1.z;
    float3 tmp8 = float3(tmp5, tmp6, tmp7);
    float3 tmp9 = tmp8 * tmp4;
    float tmp10 = asfloat((asint(tmp9.x + tmp9.y + tmp9.z) & -2147483648) | 1065353216);
    float tmp11 = p_22.x;
    float tmp12 = p_22.y;
    float tmp13 = p_22.z;
    float3 tmp14 = float3(tmp4.x * tmp10, tmp4.y * tmp10, tmp4.z * tmp10);
    float3 tmp15 = tmp14 * float3(tmp11, tmp12, tmp13);
    float tmp16 = asfloat((asint(tmp15.x + tmp15.y + tmp15.z) & -2147483648) | 1065353216);
    float3 tmp17 = float3(tmp11 * tmp16, tmp12 * tmp16, tmp13 * tmp16);
    float3 tmp18 = tmp17.zxy;
    float3 tmp19 = tmp17.yzx;
    float3 tmp20 = tmp18 * tmp3.yzx - tmp19 * tmp3.zxy;
    float3 tmp21 = tmp20 * tmp20;
    float tmp22 = tmp21.x + tmp21.y + tmp21.z;
    if (tmp22 < 1e-08)
        p_00.pdf = 0.0;
    else {
        float tmp23 = 1.0 / sqrt(tmp22);
        float3 tmp24 = float3(tmp23 * tmp20.x, tmp23 * tmp20.y, tmp23 * tmp20.z);
        float3 tmp25 = tmp24.zxy * tmp19 - tmp24.yzx * tmp18;
        if (p_00.ior1.x == -1.0) {
            p_00.ior1.x = 1.0;
            p_00.ior1.y = 1.0;
            p_00.ior1.z = 1.0;
        }
        if (p_00.ior2.x == -1.0) {
            p_00.ior2.x = 1.0;
            p_00.ior2.y = 1.0;
            p_00.ior2.z = 1.0;
        }
        float tmp26 = p_00.k2.x;
        float tmp27 = p_00.k2.y;
        float tmp28 = p_00.k2.z;
        float3 tmp29 = float3(tmp26, tmp27, tmp28);
        float3 tmp30 = tmp29 * tmp14;
        if (tmp30.x + tmp30.y + tmp30.z < 0.0)
            p_00.pdf = 0.0;
        else {
            float3 tmp31 = tmp17 * tmp8;
            float tmp32 = tmp26 + tmp5;
            float tmp33 = tmp27 + tmp6;
            float tmp34 = tmp28 + tmp7;
            float3 tmp35 = float3(sqrt(tmp33 * tmp33 + tmp32 * tmp32 + tmp34 * tmp34), 0.0, 0.0);
            float3 tmp36 = float3(tmp32, tmp33, tmp34) / tmp35.xxx;
            float3 tmp37 = tmp36 * tmp17;
            float tmp38 = tmp37.x + tmp37.y + tmp37.z;
            float3 tmp39 = tmp36 * tmp8;
            float tmp40 = tmp39.x + tmp39.y + tmp39.z;
            float3 tmp41 = tmp36 * tmp29;
            if (tmp41.x + tmp41.y + tmp41.z < 0.0 || (tmp38 < 0.0 || tmp40 < 0.0))
                p_00.pdf = 0.0;
            else {
                float3 tmp42 = tmp36 * tmp25;
                float3 tmp43 = tmp36 * tmp24;
                float tmp44 = (tmp42.x + tmp42.y + tmp42.z) * 10000.0;
                float tmp45 = (tmp43.x + tmp43.y + tmp43.z) * 10000.0;
                float tmp46 = tmp44 * tmp44 + tmp38 * tmp38 + tmp45 * tmp45;
                float tmp47 = tmp38 * abs(tmp31.x + tmp31.y + tmp31.z);
                float tmp48 = tmp47 * 2.0 / tmp40;
                p_00.pdf = (tmp48 > 1.0 ? 1.0 : tmp48) * (0.25 / tmp47) * (tmp38 * 3.183099e+07 / (tmp46 * tmp46));
            }
        }
    }
    return;
}

void gen_custom_curve_layer_pdf2(inout Bsdf_pdf_data p_00, in Shading_state_material p_11, in float3 p_22)
{
    float3 tmp3;
    float phi_in;
    float phi_out;
    float3 tmp4 = p_11.normal;
    float3 tmp5 = p_11.geom_normal;
    float tmp6 = p_00.k1.x;
    float tmp7 = p_00.k1.y;
    float tmp8 = p_00.k1.z;
    float3 tmp9 = float3(tmp6, tmp7, tmp8) * tmp5;
    float tmp10 = asfloat((asint(tmp9.x + tmp9.y + tmp9.z) & -2147483648) | 1065353216);
    float3 tmp11 = float3(tmp5.x * tmp10, tmp5.y * tmp10, tmp5.z * tmp10) * tmp4;
    float tmp12 = asfloat((asint(tmp11.x + tmp11.y + tmp11.z) & -2147483648) | 1065353216);
    float tmp13 = tmp4.x * tmp12;
    float tmp14 = tmp4.y * tmp12;
    float tmp15 = tmp4.z * tmp12;
    tmp3.x = tmp13;
    tmp3.y = tmp14;
    tmp3.z = tmp15;
    gen_microfacet_ggx_vcavities_bsdf_pdf1(p_00, p_11, tmp3);
    float tmp16 = p_00.k1.x;
    float tmp17 = p_00.k1.y;
    float tmp18 = p_00.k1.z;
    float3 tmp19 = float3(tmp16, tmp17, tmp18);
    float3 tmp20 = float3(tmp13, tmp14, tmp15) * tmp19;
    float tmp21 = tmp20.x + tmp20.y + tmp20.z;
    float tmp22 = tmp21 > 0.0 ? tmp21 : 0.0;
    float tmp23 = tmp22 < 1.0 ? tmp22 : 1.0;
    float tmp24 = tmp23 > 0.0 ? tmp23 : 0.0;
    float tmp25 = 1.0 - (tmp24 < 1.0 ? tmp24 : 1.0);
    float tmp26 = tmp25 * tmp25;
    float tmp27 = tmp26 * tmp26 * (tmp25 * 0.95);
    float tmp28 = p_00.pdf;
    float tmp29 = (tmp27 + 0.04) * tmp28;
    float3 tmp30 = p_11.geom_normal;
    float3 tmp31 = tmp19 * tmp30;
    float tmp32 = asfloat((asint(tmp31.x + tmp31.y + tmp31.z) & -2147483648) | 1065353216);
    float tmp33 = p_22.x;
    float tmp34 = p_22.y;
    float tmp35 = p_22.z;
    float3 tmp36 = float3(tmp30.x * tmp32, tmp30.y * tmp32, tmp30.z * tmp32);
    float3 tmp37 = tmp36 * float3(tmp33, tmp34, tmp35);
    float tmp38 = asfloat((asint(tmp37.x + tmp37.y + tmp37.z) & -2147483648) | 1065353216);
    float tmp39 = p_00.k2.x;
    float tmp40 = p_00.k2.y;
    float tmp41 = p_00.k2.z;
    float3 tmp42 = float3(tmp39, tmp40, tmp41);
    float3 tmp43 = tmp36 * tmp42;
    phi_in = 0.0;
    if (tmp43.x + tmp43.y + tmp43.z > 0.0) {
        float3 tmp44 = float3(tmp33 * tmp38, tmp34 * tmp38, tmp35 * tmp38) * tmp42;
        float tmp45 = tmp44.x + tmp44.y + tmp44.z;
        float tmp46 = (tmp45 > 0.0 ? tmp45 : 0.0) * 0.3183099;
        phi_in = tmp46;
    }
    phi_out = phi_in;
    p_00.pdf = phi_out * (0.96 - tmp27) + tmp29;
    return;
}

void mdl_bsdf_scattering_4_sample(inout Bsdf_sample_data sret_ptr, in Shading_state_material state)
{
    Bsdf_pdf_data tmp0;
    float3 tmp1;
    Bsdf_pdf_data tmp2;
    float3 normal_buf;
    int phi_in;
    int phi_out;
    int phi_in36;
    int phi_out37;
    float phi_in64;
    float phi_out65;
    float phi_in66;
    float phi_out67;
    int phi_in129;
    int phi_out130;
    float3 phi_in131;
    float3 phi_out132;
    float phi_in133;
    float phi_out134;
    int phi_in135;
    int phi_out136;
    int phi_in137;
    int phi_out138;
    float phi_in139;
    float phi_out140;
    float phi_in141;
    float phi_out142;
    float phi_in143;
    float phi_out144;
    float phi_in145;
    float phi_out146;
    bool phi_in147;
    bool phi_out148;
    int phi_in157;
    int phi_out158;
    int phi_in161;
    int phi_out162;
    float phi_in189;
    float phi_out190;
    float phi_in191;
    float phi_out192;
    float phi_in232;
    float phi_out233;
    float phi_in234;
    float phi_out235;
    float phi_in236;
    float phi_out237;
    float phi_in245;
    float phi_out246;
    float phi_in247;
    float phi_out248;
    float phi_in249;
    float phi_out250;
    int phi_in261;
    int phi_out262;
    int phi_in263;
    int phi_out264;
    float3 phi_in265;
    float3 phi_out266;
    float phi_in267;
    float phi_out268;
    int phi_in269;
    int phi_out270;
    int phi_in271;
    int phi_out272;
    float phi_in273;
    float phi_out274;
    float phi_in275;
    float phi_out276;
    float phi_in277;
    float phi_out278;
    float phi_in279;
    float phi_out280;
    bool phi_in281;
    bool phi_out282;
    float phi_in301;
    float phi_out302;
    float phi_in305;
    float phi_out306;
    float phi_in307;
    float phi_out308;
    float phi_in309;
    float phi_out310;
    float3 tmp3 = state.normal;
    normal_buf.x = tmp3.x;
    normal_buf.y = tmp3.y;
    normal_buf.z = tmp3.z;
    float tmp4 = sret_ptr.xi.z;
    if (tmp4 < 0.0) {
        float tmp5 = tmp4 / 0.0;
        sret_ptr.xi.z = tmp5;
        float3 tmp6 = state.tangent_u[0];
        float3 tmp7 = state.geom_normal;
        float tmp8 = sret_ptr.k1.x;
        float tmp9 = sret_ptr.k1.y;
        float tmp10 = sret_ptr.k1.z;
        float3 tmp11 = float3(tmp8, tmp9, tmp10);
        float3 tmp12 = tmp11 * tmp7;
        float tmp13 = asfloat((asint(tmp12.x + tmp12.y + tmp12.z) & -2147483648) | 1065353216);
        float3 tmp14 = float3(tmp7.x * tmp13, tmp7.y * tmp13, tmp7.z * tmp13);
        float3 tmp15 = tmp14 * tmp3;
        float tmp16 = asfloat((asint(tmp15.x + tmp15.y + tmp15.z) & -2147483648) | 1065353216);
        float tmp17 = tmp3.x * tmp16;
        float tmp18 = tmp3.y * tmp16;
        float tmp19 = tmp3.z * tmp16;
        float3 tmp20 = float3(tmp17, tmp18, tmp19);
        float3 tmp21 = tmp20.zxy;
        float3 tmp22 = tmp20.yzx;
        float3 tmp23 = tmp21 * tmp6.yzx - tmp22 * tmp6.zxy;
        float3 tmp24 = tmp23 * tmp23;
        float tmp25 = tmp24.x + tmp24.y + tmp24.z;
        if (tmp25 < 1e-08) {
            sret_ptr.pdf = 0.0;
            sret_ptr.event_type = 0;
        } else {
            float tmp26 = 1.0 / sqrt(tmp25);
            float tmp27 = tmp26 * tmp23.x;
            float tmp28 = tmp26 * tmp23.y;
            float tmp29 = tmp26 * tmp23.z;
            float3 tmp30 = float3(tmp27, tmp28, tmp29);
            float3 tmp31 = tmp30.zxy * tmp22 - tmp30.yzx * tmp21;
            float tmp32 = sret_ptr.ior1.x;
            int tmp33 = asint(tmp32);
            phi_in = tmp33;
            if (tmp32 == -1.0) {
                sret_ptr.ior1.x = 1.0;
                sret_ptr.ior1.y = 1.0;
                sret_ptr.ior1.z = 1.0;
                phi_in = 1065353216;
            }
            phi_out = phi_in;
            float tmp34 = sret_ptr.ior2.x;
            int tmp35 = asint(tmp34);
            phi_in36 = tmp35;
            if (tmp34 == -1.0) {
                sret_ptr.ior2.x = 1.0;
                sret_ptr.ior2.y = 1.0;
                sret_ptr.ior2.z = 1.0;
                phi_in36 = 1065353216;
            }
            phi_out37 = phi_in36;
            float3 tmp38 = tmp20 * tmp11;
            float tmp39 = abs(tmp38.x + tmp38.y + tmp38.z);
            float3 tmp40 = tmp31 * tmp11;
            float3 tmp41 = tmp30 * tmp11;
            float tmp42 = sret_ptr.xi.y;
            float tmp43 = sret_ptr.xi.x * 6.283185;
            float tmp44 = sin(tmp43);
            float tmp45 = cos(tmp43);
            float tmp46 = sqrt(1e-08 / (tmp44 * tmp44 + tmp45 * tmp45)) * 10000.0;
            float tmp47 = tmp46 * tmp45;
            float tmp48 = tmp46 * tmp44;
            float tmp49 = tmp42 / ((tmp48 * tmp48 + tmp47 * tmp47) * (1e+08 - tmp42 * 1e+08));
            float tmp50 = tmp49 + 1.0;
            float tmp51 = sqrt(tmp49 / tmp50);
            float tmp52 = tmp51 * tmp48;
            float tmp53 = 1.0 / tmp50;
            float tmp54 = sqrt(tmp53);
            float tmp55 = tmp51 * tmp47;
            float tmp56 = tmp54 * tmp39;
            float tmp57 = tmp55 * (tmp41.x + tmp41.y + tmp41.z) + tmp52 * (tmp40.x + tmp40.y + tmp40.z);
            float tmp58 = tmp57 + tmp56;
            float tmp59 = tmp56 - tmp57;
            float tmp60 = tmp59 > 0.0 ? tmp59 : 0.0;
            float tmp61 = tmp60 / (tmp60 + (tmp58 > 0.0 ? tmp58 : 0.0));
            if (tmp5 < tmp61) {
                sret_ptr.xi.z = tmp5 / tmp61;
                float tmp62 = -tmp52;
                float tmp63 = -tmp55;
                phi_in64 = tmp62;
                phi_in66 = tmp63;
            } else {
                sret_ptr.xi.z = (tmp5 - tmp61) / (1.0 - tmp61);
                phi_in64 = tmp52;
                phi_in66 = tmp55;
            }
            phi_out65 = phi_in64;
            phi_out67 = phi_in66;
            if (tmp54 == 0.0) {
                sret_ptr.pdf = 0.0;
                sret_ptr.event_type = 0;
            } else {
                float tmp68 = phi_out65 * tmp31.x + tmp54 * tmp17 + phi_out67 * tmp27;
                float tmp69 = phi_out65 * tmp31.y + tmp54 * tmp18 + phi_out67 * tmp28;
                float tmp70 = phi_out65 * tmp31.z + tmp54 * tmp19 + phi_out67 * tmp29;
                float3 tmp71 = float3(tmp68, tmp69, tmp70);
                float3 tmp72 = tmp71 * tmp11;
                float tmp73 = tmp72.x + tmp72.y + tmp72.z;
                if (tmp73 > 0.0) {
                    float tmp74 = tmp73 * 2.0;
                    float tmp75 = tmp74 * tmp68 - tmp8;
                    float tmp76 = tmp74 * tmp69 - tmp9;
                    float tmp77 = tmp74 * tmp70 - tmp10;
                    sret_ptr.k2.x = tmp75;
                    sret_ptr.k2.y = tmp76;
                    sret_ptr.k2.z = tmp77;
                    sret_ptr.event_type = 10;
                    float3 tmp78 = float3(tmp75, tmp76, tmp77);
                    float3 tmp79 = tmp78 * tmp14;
                    if (tmp79.x + tmp79.y + tmp79.z > 0.0) {
                        sret_ptr.bsdf_over_pdf.x = 1.0;
                        sret_ptr.bsdf_over_pdf.y = 1.0;
                        sret_ptr.bsdf_over_pdf.z = 1.0;
                        float3 tmp80 = tmp78 * tmp20;
                        float3 tmp81 = tmp78 * tmp71;
                        float tmp82 = tmp54 * 2.0;
                        float tmp83 = tmp82 * tmp39 / tmp73;
                        float tmp84 = tmp83 > 1.0 ? 1.0 : tmp83;
                        float tmp85 = abs(tmp80.x + tmp80.y + tmp80.z) * tmp82 / abs(tmp81.x + tmp81.y + tmp81.z);
                        float tmp86 = tmp85 > 1.0 ? 1.0 : tmp85;
                        float tmp87 = tmp84 < tmp86 ? tmp84 : tmp86;
                        if (tmp87 > 0.0) {
                            float tmp88 = tmp87 / tmp84;
                            sret_ptr.bsdf_over_pdf.x = tmp88;
                            sret_ptr.bsdf_over_pdf.y = tmp88;
                            sret_ptr.bsdf_over_pdf.z = tmp88;
                            float tmp89 = phi_out65 * 10000.0;
                            float tmp90 = phi_out67 * 10000.0;
                            float tmp91 = tmp89 * tmp89 + tmp53 + tmp90 * tmp90;
                            sret_ptr.pdf = tmp54 * 3.183099e+07 / (tmp91 * tmp91) * (0.25 / tmp56) * tmp84;
                            sret_ptr.handle = 0;
                            tmp2.ior1.x = asfloat(phi_out);
                            tmp2.ior1.y = asfloat(asint(sret_ptr.ior1.y));
                            tmp2.ior1.z = asfloat(asint(sret_ptr.ior1.z));
                            tmp2.ior2.x = asfloat(phi_out37);
                            tmp2.ior2.y = asfloat(asint(sret_ptr.ior2.y));
                            tmp2.ior2.z = asfloat(asint(sret_ptr.ior2.z));
                            tmp2.k1.x = tmp8;
                            tmp2.k1.y = tmp9;
                            tmp2.k1.z = tmp10;
                            tmp2.k2.x = tmp75;
                            tmp2.k2.y = tmp76;
                            tmp2.k2.z = tmp77;
                            gen_custom_curve_layer_pdf2(tmp2, state, normal_buf);
                            int tmp92 = asint(tmp2.pdf);
                            sret_ptr.pdf = asfloat(tmp92);
                        } else {
                            sret_ptr.pdf = 0.0;
                            sret_ptr.event_type = 0;
                        }
                    } else {
                        sret_ptr.pdf = 0.0;
                        sret_ptr.event_type = 0;
                    }
                } else {
                    sret_ptr.pdf = 0.0;
                    sret_ptr.event_type = 0;
                }
            }
        }
    } else {
        float3 tmp93 = state.geom_normal;
        float tmp94 = sret_ptr.k1.x;
        float tmp95 = sret_ptr.k1.y;
        float tmp96 = sret_ptr.k1.z;
        float3 tmp97 = float3(tmp94, tmp95, tmp96);
        float3 tmp98 = tmp97 * tmp93;
        float tmp99 = asfloat((asint(tmp98.x + tmp98.y + tmp98.z) & -2147483648) | 1065353216);
        float3 tmp100 = float3(tmp93.x * tmp99, tmp93.y * tmp99, tmp93.z * tmp99);
        float3 tmp101 = tmp100 * tmp3;
        float tmp102 = asfloat((asint(tmp101.x + tmp101.y + tmp101.z) & -2147483648) | 1065353216);
        float tmp103 = tmp3.x * tmp102;
        float tmp104 = tmp3.y * tmp102;
        float tmp105 = tmp3.z * tmp102;
        tmp1.x = tmp103;
        tmp1.y = tmp104;
        tmp1.z = tmp105;
        float3 tmp106 = float3(tmp103, tmp104, tmp105);
        float3 tmp107 = tmp106 * tmp97;
        float tmp108 = tmp107.x + tmp107.y + tmp107.z;
        float tmp109 = tmp108 > 0.0 ? tmp108 : 0.0;
        float tmp110 = tmp109 < 1.0 ? tmp109 : 1.0;
        float tmp111 = tmp110 > 0.0 ? tmp110 : 0.0;
        float tmp112 = 1.0 - (tmp111 < 1.0 ? tmp111 : 1.0);
        float tmp113 = tmp112 * tmp112;
        float tmp114 = tmp113 * tmp113 * (tmp112 * 0.95);
        float tmp115 = tmp114 + 0.04;
        if (tmp4 < tmp115) {
            float tmp116 = tmp4 / tmp115;
            sret_ptr.xi.z = tmp116;
            float3 tmp117 = state.tangent_u[0];
            float3 tmp118 = tmp106 * tmp100;
            float tmp119 = asfloat((asint(tmp118.x + tmp118.y + tmp118.z) & -2147483648) | 1065353216);
            float tmp120 = tmp103 * tmp119;
            float tmp121 = tmp104 * tmp119;
            float tmp122 = tmp105 * tmp119;
            float3 tmp123 = float3(tmp120, tmp121, tmp122);
            float3 tmp124 = tmp123.zxy;
            float3 tmp125 = tmp123.yzx;
            float3 tmp126 = tmp117.yzx * tmp124 - tmp117.zxy * tmp125;
            float3 tmp127 = tmp126 * tmp126;
            float tmp128 = tmp127.x + tmp127.y + tmp127.z;
            if (tmp128 < 1e-08) {
                sret_ptr.pdf = 0.0;
                sret_ptr.event_type = 0;
                phi_in129 = 0;
                phi_in131 = float3(0.0, 0.0, 0.0);
                phi_in133 = 0.0;
                phi_in135 = 0;
                phi_in137 = 0;
                phi_in139 = 0.0;
                phi_in141 = 0.0;
                phi_in143 = 0.0;
                phi_in145 = 0.0;
                phi_in147 = false;
            } else {
                float tmp149 = 1.0 / sqrt(tmp128);
                float tmp150 = tmp149 * tmp126.x;
                float tmp151 = tmp149 * tmp126.y;
                float tmp152 = tmp149 * tmp126.z;
                float3 tmp153 = float3(tmp150, tmp151, tmp152);
                float3 tmp154 = tmp153.zxy * tmp125 - tmp153.yzx * tmp124;
                float tmp155 = sret_ptr.ior1.x;
                int tmp156 = asint(tmp155);
                phi_in157 = tmp156;
                if (tmp155 == -1.0) {
                    sret_ptr.ior1.x = 1.0;
                    sret_ptr.ior1.y = 1.0;
                    sret_ptr.ior1.z = 1.0;
                    phi_in157 = 1065353216;
                }
                phi_out158 = phi_in157;
                float tmp159 = sret_ptr.ior2.x;
                int tmp160 = asint(tmp159);
                phi_in161 = tmp160;
                if (tmp159 == -1.0) {
                    sret_ptr.ior2.x = 1.0;
                    sret_ptr.ior2.y = 1.0;
                    sret_ptr.ior2.z = 1.0;
                    phi_in161 = 1065353216;
                }
                phi_out162 = phi_in161;
                float3 tmp163 = tmp123 * tmp97;
                float tmp164 = abs(tmp163.x + tmp163.y + tmp163.z);
                float3 tmp165 = tmp154 * tmp97;
                float3 tmp166 = tmp153 * tmp97;
                float tmp167 = sret_ptr.xi.y;
                float tmp168 = sret_ptr.xi.x * 6.283185;
                float tmp169 = sin(tmp168);
                float tmp170 = cos(tmp168);
                float tmp171 = sqrt(1e-08 / (tmp169 * tmp169 + tmp170 * tmp170)) * 10000.0;
                float tmp172 = tmp171 * tmp170;
                float tmp173 = tmp171 * tmp169;
                float tmp174 = tmp167 / ((tmp173 * tmp173 + tmp172 * tmp172) * (1e+08 - tmp167 * 1e+08));
                float tmp175 = tmp174 + 1.0;
                float tmp176 = sqrt(tmp174 / tmp175);
                float tmp177 = tmp176 * tmp173;
                float tmp178 = 1.0 / tmp175;
                float tmp179 = sqrt(tmp178);
                float tmp180 = tmp176 * tmp172;
                float tmp181 = tmp179 * tmp164;
                float tmp182 = tmp180 * (tmp166.x + tmp166.y + tmp166.z) + tmp177 * (tmp165.x + tmp165.y + tmp165.z);
                float tmp183 = tmp182 + tmp181;
                float tmp184 = tmp181 - tmp182;
                float tmp185 = tmp184 > 0.0 ? tmp184 : 0.0;
                float tmp186 = tmp185 / (tmp185 + (tmp183 > 0.0 ? tmp183 : 0.0));
                if (tmp116 < tmp186) {
                    sret_ptr.xi.z = tmp116 / tmp186;
                    float tmp187 = -tmp177;
                    float tmp188 = -tmp180;
                    phi_in189 = tmp187;
                    phi_in191 = tmp188;
                } else {
                    sret_ptr.xi.z = (tmp116 - tmp186) / (1.0 - tmp186);
                    phi_in189 = tmp177;
                    phi_in191 = tmp180;
                }
                phi_out190 = phi_in189;
                phi_out192 = phi_in191;
                if (tmp179 == 0.0) {
                    sret_ptr.pdf = 0.0;
                    sret_ptr.event_type = 0;
                    phi_in129 = 0;
                    phi_in131 = float3(0.0, 0.0, 0.0);
                    phi_in133 = 0.0;
                    phi_in135 = 0;
                    phi_in137 = 0;
                    phi_in139 = 0.0;
                    phi_in141 = 0.0;
                    phi_in143 = 0.0;
                    phi_in145 = 0.0;
                    phi_in147 = false;
                } else {
                    float tmp193 = phi_out190 * tmp154.x + tmp179 * tmp120 + phi_out192 * tmp150;
                    float tmp194 = phi_out190 * tmp154.y + tmp179 * tmp121 + phi_out192 * tmp151;
                    float tmp195 = phi_out190 * tmp154.z + tmp179 * tmp122 + phi_out192 * tmp152;
                    float3 tmp196 = float3(tmp193, tmp194, tmp195);
                    float3 tmp197 = tmp196 * tmp97;
                    float tmp198 = tmp197.x + tmp197.y + tmp197.z;
                    if (tmp198 > 0.0) {
                        float tmp199 = tmp198 * 2.0;
                        float tmp200 = tmp199 * tmp193 - tmp94;
                        float tmp201 = tmp199 * tmp194 - tmp95;
                        float tmp202 = tmp199 * tmp195 - tmp96;
                        sret_ptr.k2.x = tmp200;
                        sret_ptr.k2.y = tmp201;
                        sret_ptr.k2.z = tmp202;
                        sret_ptr.event_type = 10;
                        float3 tmp203 = float3(tmp200, tmp201, tmp202);
                        float3 tmp204 = tmp203 * tmp100;
                        if (tmp204.x + tmp204.y + tmp204.z > 0.0) {
                            sret_ptr.bsdf_over_pdf.x = 1.0;
                            sret_ptr.bsdf_over_pdf.y = 1.0;
                            sret_ptr.bsdf_over_pdf.z = 1.0;
                            float3 tmp205 = tmp203 * tmp123;
                            float3 tmp206 = tmp203 * tmp196;
                            float tmp207 = tmp179 * 2.0;
                            float tmp208 = tmp207 * tmp164 / tmp198;
                            float tmp209 = tmp208 > 1.0 ? 1.0 : tmp208;
                            float tmp210 = abs(tmp205.x + tmp205.y + tmp205.z) * tmp207 / abs(tmp206.x + tmp206.y + tmp206.z);
                            float tmp211 = tmp210 > 1.0 ? 1.0 : tmp210;
                            float tmp212 = tmp209 < tmp211 ? tmp209 : tmp211;
                            if (tmp212 > 0.0) {
                                float tmp213 = tmp212 / tmp209;
                                sret_ptr.bsdf_over_pdf.x = tmp213;
                                sret_ptr.bsdf_over_pdf.y = tmp213;
                                sret_ptr.bsdf_over_pdf.z = tmp213;
                                float tmp214 = phi_out190 * 10000.0;
                                float tmp215 = phi_out192 * 10000.0;
                                float tmp216 = tmp214 * tmp214 + tmp178 + tmp215 * tmp215;
                                float tmp217 = tmp179 * 3.183099e+07 / (tmp216 * tmp216) * (0.25 / tmp181) * tmp209;
                                sret_ptr.pdf = tmp217;
                                sret_ptr.handle = 0;
                                phi_in129 = 1;
                                phi_in131 = tmp203;
                                phi_in133 = tmp213;
                                phi_in135 = phi_out162;
                                phi_in137 = phi_out158;
                                phi_in139 = tmp202;
                                phi_in141 = tmp201;
                                phi_in143 = tmp200;
                                phi_in145 = tmp217;
                                phi_in147 = true;
                            } else {
                                sret_ptr.pdf = 0.0;
                                sret_ptr.event_type = 0;
                                phi_in129 = 0;
                                phi_in131 = float3(0.0, 0.0, 0.0);
                                phi_in133 = 0.0;
                                phi_in135 = 0;
                                phi_in137 = 0;
                                phi_in139 = 0.0;
                                phi_in141 = 0.0;
                                phi_in143 = 0.0;
                                phi_in145 = 0.0;
                                phi_in147 = false;
                            }
                        } else {
                            sret_ptr.pdf = 0.0;
                            sret_ptr.event_type = 0;
                            phi_in129 = 0;
                            phi_in131 = float3(0.0, 0.0, 0.0);
                            phi_in133 = 0.0;
                            phi_in135 = 0;
                            phi_in137 = 0;
                            phi_in139 = 0.0;
                            phi_in141 = 0.0;
                            phi_in143 = 0.0;
                            phi_in145 = 0.0;
                            phi_in147 = false;
                        }
                    } else {
                        sret_ptr.pdf = 0.0;
                        sret_ptr.event_type = 0;
                        phi_in129 = 0;
                        phi_in131 = float3(0.0, 0.0, 0.0);
                        phi_in133 = 0.0;
                        phi_in135 = 0;
                        phi_in137 = 0;
                        phi_in139 = 0.0;
                        phi_in141 = 0.0;
                        phi_in143 = 0.0;
                        phi_in145 = 0.0;
                        phi_in147 = false;
                    }
                }
            }
        } else {
            sret_ptr.xi.z = (1.0 - tmp4) / (0.96 - tmp114);
            float3 tmp218 = state.tangent_u[0];
            float3 tmp219 = tmp106.zxy;
            float3 tmp220 = tmp106.yzx;
            float3 tmp221 = tmp218.yzx * tmp219 - tmp218.zxy * tmp220;
            float3 tmp222 = tmp221 * tmp221;
            float tmp223 = tmp222.x + tmp222.y + tmp222.z;
            if (tmp223 < 1e-08) {
                sret_ptr.pdf = 0.0;
                sret_ptr.event_type = 0;
                phi_in129 = 0;
                phi_in131 = float3(0.0, 0.0, 0.0);
                phi_in133 = 0.0;
                phi_in135 = 0;
                phi_in137 = 0;
                phi_in139 = 0.0;
                phi_in141 = 0.0;
                phi_in143 = 0.0;
                phi_in145 = 0.0;
                phi_in147 = false;
            } else {
                float tmp224 = 1.0 / sqrt(tmp223);
                float tmp225 = tmp224 * tmp221.x;
                float tmp226 = tmp224 * tmp221.y;
                float tmp227 = tmp224 * tmp221.z;
                float3 tmp228 = float3(tmp225, tmp226, tmp227);
                float3 tmp229 = tmp228.zxy * tmp220 - tmp228.yzx * tmp219;
                float tmp230 = sret_ptr.xi.x;
                float tmp231 = sret_ptr.xi.y;
                phi_in232 = 1.0;
                phi_in234 = 0.0;
                phi_in236 = 0.0;
                if (!(tmp230 == 0.0 && tmp231 == 0.0)) {
                    float tmp238 = tmp230 * 2.0;
                    float tmp239 = tmp231 * 2.0;
                    float tmp240 = tmp238 < 1.0 ? tmp238 : tmp238 + -2.0;
                    float tmp241 = tmp239 < 1.0 ? tmp239 : tmp239 + -2.0;
                    float tmp242 = tmp240 * tmp240;
                    float tmp243 = tmp241 * tmp241;
                    if (tmp242 > tmp243) {
                        float tmp244 = tmp241 / tmp240 * -0.7853982;
                        phi_in245 = tmp240;
                        phi_in247 = tmp244;
                        phi_in249 = tmp242;
                    } else {
                        float tmp251 = tmp240 / tmp241 * 0.7853982 + -1.570796;
                        phi_in245 = tmp241;
                        phi_in247 = tmp251;
                        phi_in249 = tmp243;
                    }
                    phi_out246 = phi_in245;
                    phi_out248 = phi_in247;
                    phi_out250 = phi_in249;
                    float tmp252 = 1.0 - phi_out250;
                    phi_in232 = 1.0;
                    phi_in234 = 0.0;
                    phi_in236 = 0.0;
                    if (tmp252 > 0.0) {
                        float tmp253 = sin(phi_out248) * phi_out246;
                        float tmp254 = sqrt(tmp252);
                        float tmp255 = cos(phi_out248) * phi_out246;
                        phi_in232 = tmp254;
                        phi_in234 = tmp253;
                        phi_in236 = tmp255;
                    }
                }
                phi_out233 = phi_in232;
                phi_out235 = phi_in234;
                phi_out237 = phi_in236;
                float tmp256 = phi_out235 * tmp229.x + phi_out233 * tmp103 + phi_out237 * tmp225;
                float tmp257 = phi_out235 * tmp229.y + phi_out233 * tmp104 + phi_out237 * tmp226;
                float tmp258 = phi_out235 * tmp229.z + phi_out233 * tmp105 + phi_out237 * tmp227;
                float3 tmp259 = float3(sqrt(tmp256 * tmp256 + tmp257 * tmp257 + tmp258 * tmp258), 0.0, 0.0);
                float3 tmp260 = float3(tmp256, tmp257, tmp258) / tmp259.xxx;
                sret_ptr.k2.x = tmp260.x;
                sret_ptr.k2.y = tmp260.y;
                sret_ptr.k2.z = tmp260.z;
                phi_in261 = 0;
                phi_in263 = 0;
                phi_in265 = float3(0.0, 0.0, 0.0);
                phi_in267 = 0.0;
                phi_in269 = 0;
                phi_in271 = 0;
                phi_in273 = 0.0;
                phi_in275 = 0.0;
                phi_in277 = 0.0;
                phi_in279 = 0.0;
                phi_in281 = false;
                if (phi_out233 > 0.0) {
                    float3 tmp283 = tmp260 * tmp100;
                    phi_in261 = 0;
                    phi_in263 = 0;
                    phi_in265 = float3(0.0, 0.0, 0.0);
                    phi_in267 = 0.0;
                    phi_in269 = 0;
                    phi_in271 = 0;
                    phi_in273 = 0.0;
                    phi_in275 = 0.0;
                    phi_in277 = 0.0;
                    phi_in279 = 0.0;
                    phi_in281 = false;
                    if (tmp283.x + tmp283.y + tmp283.z > 0.0) {
                        sret_ptr.bsdf_over_pdf.x = 1.0;
                        sret_ptr.bsdf_over_pdf.y = 1.0;
                        sret_ptr.bsdf_over_pdf.z = 1.0;
                        float tmp284 = phi_out233 * 0.3183099;
                        sret_ptr.pdf = tmp284;
                        sret_ptr.event_type = 9;
                        sret_ptr.handle = 0;
                        int tmp285 = asint(sret_ptr.ior1.x);
                        int tmp286 = asint(sret_ptr.ior2.x);
                        phi_in261 = 1;
                        phi_in263 = 1;
                        phi_in265 = tmp260;
                        phi_in267 = 1.0;
                        phi_in269 = tmp286;
                        phi_in271 = tmp285;
                        phi_in273 = tmp260.z;
                        phi_in275 = tmp260.y;
                        phi_in277 = tmp260.x;
                        phi_in279 = tmp284;
                        phi_in281 = false;
                    }
                }
                phi_out262 = phi_in261;
                phi_out264 = phi_in263;
                phi_out266 = phi_in265;
                phi_out268 = phi_in267;
                phi_out270 = phi_in269;
                phi_out272 = phi_in271;
                phi_out274 = phi_in273;
                phi_out276 = phi_in275;
                phi_out278 = phi_in277;
                phi_out280 = phi_in279;
                phi_out282 = phi_in281;
                phi_in129 = phi_out264;
                phi_in131 = phi_out266;
                phi_in133 = phi_out268;
                phi_in135 = phi_out270;
                phi_in137 = phi_out272;
                phi_in139 = phi_out274;
                phi_in141 = phi_out276;
                phi_in143 = phi_out278;
                phi_in145 = phi_out280;
                phi_in147 = phi_out282;
                if (phi_out262 == 0) {
                    sret_ptr.pdf = 0.0;
                    sret_ptr.event_type = 0;
                    phi_in129 = 0;
                    phi_in131 = float3(0.0, 0.0, 0.0);
                    phi_in133 = 0.0;
                    phi_in135 = 0;
                    phi_in137 = 0;
                    phi_in139 = 0.0;
                    phi_in141 = 0.0;
                    phi_in143 = 0.0;
                    phi_in145 = 0.0;
                    phi_in147 = false;
                }
            }
        }
        phi_out130 = phi_in129;
        phi_out132 = phi_in131;
        phi_out134 = phi_in133;
        phi_out136 = phi_in135;
        phi_out138 = phi_in137;
        phi_out140 = phi_in139;
        phi_out142 = phi_in141;
        phi_out144 = phi_in143;
        phi_out146 = phi_in145;
        phi_out148 = phi_in147;
        if (phi_out130 != 0) {
            float3 tmp287 = phi_out132 * tmp106;
            float tmp288 = tmp287.x + tmp287.y + tmp287.z;
            float tmp289 = abs(tmp288);
            float tmp290 = phi_out142 + tmp95;
            float tmp291 = phi_out144 + tmp94;
            float tmp292 = phi_out140 + tmp96;
            float3 tmp293 = float3(sqrt(tmp290 * tmp290 + tmp292 * tmp292 + tmp291 * tmp291), 0.0, 0.0);
            float3 tmp294 = float3(tmp291, tmp290, tmp292) / tmp293.xxx * tmp97;
            float tmp295 = abs(tmp294.x + tmp294.y + tmp294.z);
            tmp0.ior1.x = asfloat(phi_out138);
            tmp0.ior1.y = asfloat(asint(sret_ptr.ior1.y));
            tmp0.ior1.z = asfloat(asint(sret_ptr.ior1.z));
            tmp0.ior2.x = asfloat(phi_out136);
            tmp0.ior2.y = asfloat(asint(sret_ptr.ior2.y));
            tmp0.ior2.z = asfloat(asint(sret_ptr.ior2.z));
            tmp0.k1.x = tmp94;
            tmp0.k1.y = tmp95;
            tmp0.k1.z = tmp96;
            tmp0.k2.x = phi_out144;
            tmp0.k2.y = phi_out142;
            tmp0.k2.z = phi_out140;
            if (phi_out148) {
                float tmp296 = tmp295 > 0.0 ? tmp295 : 0.0;
                float tmp297 = 1.0 - (tmp296 < 1.0 ? tmp296 : 1.0);
                float tmp298 = tmp297 * tmp297;
                float tmp299 = phi_out134 * (1.0 / tmp115) * (tmp298 * tmp298 * (tmp297 * 0.95) + 0.04);
                sret_ptr.bsdf_over_pdf.x = tmp299;
                sret_ptr.bsdf_over_pdf.y = tmp299;
                sret_ptr.bsdf_over_pdf.z = tmp299;
                float3 tmp300 = phi_out132 * tmp100;
                phi_in301 = 0.0;
                if (tmp300.x + tmp300.y + tmp300.z > 0.0) {
                    float tmp303 = (tmp288 > 0.0 ? tmp288 : 0.0) * 0.3183099;
                    phi_in301 = tmp303;
                }
                phi_out302 = phi_in301;
                tmp0.pdf = phi_out302;
                float tmp304 = phi_out302 * (0.96 - tmp114);
                phi_in305 = phi_out146;
                phi_in307 = tmp115;
                phi_in309 = tmp304;
            } else {
                float tmp311 = tmp289 > 0.0 ? tmp289 : 0.0;
                float tmp312 = 1.0 - (tmp311 < 1.0 ? tmp311 : 1.0);
                float tmp313 = tmp312 * tmp312;
                float tmp314 = tmp313 * tmp313 * (tmp312 * 0.95) + 0.04;
                float tmp315 = 0.96 - tmp114;
                float tmp316 = phi_out134 * (1.0 / tmp315) * (1.0 - (tmp115 > tmp314 ? tmp115 : tmp314));
                sret_ptr.bsdf_over_pdf.x = tmp316;
                sret_ptr.bsdf_over_pdf.y = tmp316;
                sret_ptr.bsdf_over_pdf.z = tmp316;
                gen_microfacet_ggx_vcavities_bsdf_pdf1(tmp0, state, tmp1);
                float tmp317 = tmp0.pdf;
                float tmp318 = tmp317 * tmp115;
                float tmp319 = sret_ptr.pdf;
                phi_in305 = tmp319;
                phi_in307 = tmp315;
                phi_in309 = tmp318;
            }
            phi_out306 = phi_in305;
            phi_out308 = phi_in307;
            phi_out310 = phi_in309;
            sret_ptr.pdf = phi_out308 * phi_out306 + phi_out310;
        }
    }
    return;
}

void mdl_bsdf_scattering_4_evaluate(inout Bsdf_evaluate_data sret_ptr, in Shading_state_material state)
{
    float phi_in;
    float phi_out;
    float phi_in29;
    float phi_out30;
    float phi_in31;
    float phi_out32;
    float phi_in65;
    float phi_out66;
    float phi_in67;
    float phi_out68;
    float phi_in101;
    float phi_out102;
    float3 tmp0 = state.normal;
    sret_ptr.bsdf_diffuse = (float3)0;
    sret_ptr.bsdf_glossy = (float3)0;
    float3 tmp1 = state.geom_normal;
    float tmp2 = sret_ptr.k1.x;
    float tmp3 = sret_ptr.k1.y;
    float tmp4 = sret_ptr.k1.z;
    float3 tmp5 = float3(tmp2, tmp3, tmp4);
    float3 tmp6 = tmp5 * tmp1;
    float tmp7 = asfloat((asint(tmp6.x + tmp6.y + tmp6.z) & -2147483648) | 1065353216);
    float3 tmp8 = float3(tmp1.x * tmp7, tmp1.y * tmp7, tmp1.z * tmp7);
    float3 tmp9 = tmp8 * tmp0;
    float tmp10 = asfloat((asint(tmp9.x + tmp9.y + tmp9.z) & -2147483648) | 1065353216);
    float tmp11 = tmp0.x * tmp10;
    float tmp12 = tmp0.y * tmp10;
    float tmp13 = tmp0.z * tmp10;
    float3 tmp14 = float3(tmp11, tmp12, tmp13);
    float3 tmp15 = tmp14 * tmp5;
    float tmp16 = tmp15.x + tmp15.y + tmp15.z;
    float tmp17 = tmp16 > 0.0 ? tmp16 : 0.0;
    float tmp18 = tmp17 < 1.0 ? tmp17 : 1.0;
    float tmp19 = sret_ptr.k2.x;
    float tmp20 = sret_ptr.k2.y;
    float tmp21 = sret_ptr.k2.z;
    float3 tmp22 = float3(tmp19, tmp20, tmp21);
    float3 tmp23 = tmp14 * tmp22;
    float tmp24 = tmp23.x + tmp23.y + tmp23.z;
    float tmp25 = abs(tmp24);
    float3 tmp26 = tmp8 * tmp22;
    float tmp27 = tmp26.x + tmp26.y + tmp26.z;
    bool tmp28 = tmp27 < 0.0;
    phi_in = tmp21;
    phi_in29 = tmp19;
    phi_in31 = tmp20;
    if (tmp28) {
        float tmp33 = tmp25 * 2.0;
        float tmp34 = tmp33 * tmp11 + tmp19;
        float tmp35 = tmp33 * tmp12 + tmp20;
        float tmp36 = tmp33 * tmp13 + tmp21;
        phi_in = tmp36;
        phi_in29 = tmp34;
        phi_in31 = tmp35;
    }
    phi_out = phi_in;
    phi_out30 = phi_in29;
    phi_out32 = phi_in31;
    float tmp37 = phi_out32 + tmp3;
    float tmp38 = phi_out30 + tmp2;
    float tmp39 = phi_out + tmp4;
    float3 tmp40 = float3(sqrt(tmp38 * tmp38 + tmp39 * tmp39 + tmp37 * tmp37), 0.0, 0.0);
    float3 tmp41 = float3(tmp38, tmp37, tmp39) / tmp40.xxx * tmp5;
    float tmp42 = abs(tmp41.x + tmp41.y + tmp41.z);
    float tmp43 = tmp42 > 0.0 ? tmp42 : 0.0;
    float tmp44 = 1.0 - (tmp43 < 1.0 ? tmp43 : 1.0);
    float tmp45 = tmp44 * tmp44;
    float tmp46 = tmp45 * tmp45 * (tmp44 * 0.95) + 0.04;
    float tmp47 = tmp18 > 0.0 ? tmp18 : 0.0;
    float tmp48 = 1.0 - (tmp47 < 1.0 ? tmp47 : 1.0);
    float tmp49 = tmp48 * tmp48;
    float tmp50 = tmp49 * tmp49 * (tmp48 * 0.95);
    float tmp51 = tmp50 + 0.04;
    float tmp52 = tmp25 > 0.0 ? tmp25 : 0.0;
    float tmp53 = 1.0 - (tmp52 < 1.0 ? tmp52 : 1.0);
    float tmp54 = tmp53 * tmp53;
    float tmp55 = tmp54 * tmp54 * (tmp53 * 0.95) + 0.04;
    float3 tmp56 = state.tangent_u[0];
    float3 tmp57 = tmp14 * tmp8;
    float tmp58 = asfloat((asint(tmp57.x + tmp57.y + tmp57.z) & -2147483648) | 1065353216);
    float3 tmp59 = float3(tmp11 * tmp58, tmp12 * tmp58, tmp13 * tmp58);
    float3 tmp60 = tmp59.zxy;
    float3 tmp61 = tmp59.yzx;
    float3 tmp62 = tmp56.yzx * tmp60 - tmp56.zxy * tmp61;
    float3 tmp63 = tmp62 * tmp62;
    float tmp64 = tmp63.x + tmp63.y + tmp63.z;
    if (tmp64 < 1e-08) {
        sret_ptr.pdf = 0.0;
        phi_in65 = 0.0;
        phi_in67 = 0.0;
    } else {
        float tmp69 = 1.0 / sqrt(tmp64);
        float3 tmp70 = float3(tmp69 * tmp62.x, tmp69 * tmp62.y, tmp69 * tmp62.z);
        float3 tmp71 = tmp70.zxy * tmp61 - tmp70.yzx * tmp60;
        if (sret_ptr.ior1.x == -1.0) {
            sret_ptr.ior1.x = 1.0;
            sret_ptr.ior1.y = 1.0;
            sret_ptr.ior1.z = 1.0;
        }
        if (sret_ptr.ior2.x == -1.0) {
            sret_ptr.ior2.x = 1.0;
            sret_ptr.ior2.y = 1.0;
            sret_ptr.ior2.z = 1.0;
        }
        if (tmp28) {
            sret_ptr.pdf = 0.0;
            phi_in65 = 0.0;
            phi_in67 = 0.0;
        } else {
            float3 tmp72 = tmp59 * tmp5;
            float tmp73 = abs(tmp72.x + tmp72.y + tmp72.z);
            float3 tmp74 = tmp59 * tmp22;
            float tmp75 = tmp19 + tmp2;
            float tmp76 = tmp20 + tmp3;
            float tmp77 = tmp21 + tmp4;
            float3 tmp78 = float3(sqrt(tmp76 * tmp76 + tmp75 * tmp75 + tmp77 * tmp77), 0.0, 0.0);
            float3 tmp79 = float3(tmp75, tmp76, tmp77) / tmp78.xxx;
            float3 tmp80 = tmp59 * tmp79;
            float tmp81 = tmp80.x + tmp80.y + tmp80.z;
            float3 tmp82 = tmp79 * tmp5;
            float tmp83 = tmp82.x + tmp82.y + tmp82.z;
            float3 tmp84 = tmp79 * tmp22;
            float tmp85 = tmp84.x + tmp84.y + tmp84.z;
            if (tmp85 < 0.0 || (tmp83 < 0.0 || tmp81 < 0.0)) {
                sret_ptr.pdf = 0.0;
                phi_in65 = 0.0;
                phi_in67 = 0.0;
            } else {
                float3 tmp86 = tmp71 * tmp79;
                float3 tmp87 = tmp70 * tmp79;
                float tmp88 = (tmp86.x + tmp86.y + tmp86.z) * 10000.0;
                float tmp89 = (tmp87.x + tmp87.y + tmp87.z) * 10000.0;
                float tmp90 = tmp89 * tmp89 + tmp81 * tmp81 + tmp88 * tmp88;
                float tmp91 = tmp81 * 2.0;
                float tmp92 = tmp91 * tmp73 / tmp83;
                float tmp93 = tmp92 > 1.0 ? 1.0 : tmp92;
                float tmp94 = tmp91 * abs(tmp74.x + tmp74.y + tmp74.z) / tmp85;
                float tmp95 = tmp94 > 1.0 ? 1.0 : tmp94;
                float tmp96 = tmp81 * 3.183099e+07 / (tmp90 * tmp90) * (0.25 / (tmp73 * tmp81));
                float tmp97 = tmp96 * (tmp93 < tmp95 ? tmp93 : tmp95);
                float tmp98 = tmp96 * tmp93;
                sret_ptr.pdf = tmp98;
                phi_in65 = tmp98;
                phi_in67 = tmp97;
            }
        }
    }
    phi_out66 = phi_in65;
    phi_out68 = phi_in67;
    float tmp99 = phi_out68 * tmp46;
    sret_ptr.bsdf_glossy.x = tmp99;
    sret_ptr.bsdf_glossy.y = tmp99;
    sret_ptr.bsdf_glossy.z = tmp99;
    float tmp100 = phi_out66 * tmp51;
    phi_in101 = 0.0;
    if (tmp27 > 0.0) {
        float tmp103 = (tmp24 > 0.0 ? tmp24 : 0.0) * 0.3183099;
        float tmp104 = (1.0 - (tmp51 > tmp55 ? tmp51 : tmp55)) * tmp103;
        sret_ptr.bsdf_diffuse.x = tmp104;
        sret_ptr.bsdf_diffuse.y = tmp104;
        sret_ptr.bsdf_diffuse.z = tmp104;
        phi_in101 = tmp103;
    }
    phi_out102 = phi_in101;
    sret_ptr.pdf = phi_out102 * (0.96 - tmp50) + tmp100;
    return;
}

void mdl_bsdf_scattering_4_pdf(inout Bsdf_pdf_data sret_ptr, in Shading_state_material state)
{
    float3 normal_buf;
    float3 tmp0 = state.normal;
    normal_buf.x = tmp0.x;
    normal_buf.y = tmp0.y;
    normal_buf.z = tmp0.z;
    gen_custom_curve_layer_pdf2(sret_ptr, state, normal_buf);
    return;
}

void mdl_edf_emission_4_init(inout Shading_state_material state)
{
    return;
}

void mdl_edf_emission_4_sample(inout Edf_sample_data sret_ptr, in Shading_state_material state)
{
    float phi_in;
    float phi_out;
    float phi_in4;
    float phi_out5;
    float phi_in6;
    float phi_out7;
    float phi_in15;
    float phi_out16;
    float phi_in17;
    float phi_out18;
    float phi_in19;
    float phi_out20;
    int phi_in43;
    int phi_out44;
    float3 tmp0 = state.normal;
    float3 tmp1 = state.geom_normal;
    float tmp2 = sret_ptr.xi.x;
    float tmp3 = sret_ptr.xi.y;
    phi_in = 1.0;
    phi_in4 = 0.0;
    phi_in6 = 0.0;
    if (!(tmp2 == 0.0 && tmp3 == 0.0)) {
        float tmp8 = tmp2 * 2.0;
        float tmp9 = tmp3 * 2.0;
        float tmp10 = tmp8 < 1.0 ? tmp8 : tmp8 + -2.0;
        float tmp11 = tmp9 < 1.0 ? tmp9 : tmp9 + -2.0;
        float tmp12 = tmp10 * tmp10;
        float tmp13 = tmp11 * tmp11;
        if (tmp12 > tmp13) {
            float tmp14 = tmp11 / tmp10 * -0.7853982;
            phi_in15 = tmp10;
            phi_in17 = tmp14;
            phi_in19 = tmp12;
        } else {
            float tmp21 = tmp10 / tmp11 * 0.7853982 + -1.570796;
            phi_in15 = tmp11;
            phi_in17 = tmp21;
            phi_in19 = tmp13;
        }
        phi_out16 = phi_in15;
        phi_out18 = phi_in17;
        phi_out20 = phi_in19;
        float tmp22 = 1.0 - phi_out20;
        phi_in = 1.0;
        phi_in4 = 0.0;
        phi_in6 = 0.0;
        if (tmp22 > 0.0) {
            float tmp23 = sin(phi_out18) * phi_out16;
            float tmp24 = sqrt(tmp22);
            float tmp25 = cos(phi_out18) * phi_out16;
            phi_in = tmp24;
            phi_in4 = tmp23;
            phi_in6 = tmp25;
        }
    }
    phi_out = phi_in;
    phi_out5 = phi_in4;
    phi_out7 = phi_in6;
    float3 tmp26 = state.tangent_u[0];
    float3 tmp27 = tmp0.zxy;
    float3 tmp28 = tmp0.yzx;
    float3 tmp29 = tmp26.yzx * tmp27 - tmp26.zxy * tmp28;
    float3 tmp30 = tmp29 * tmp29;
    float tmp31 = tmp30.x + tmp30.y + tmp30.z;
    if (tmp31 < 1e-08) {
        sret_ptr.k1 = (float3)0;
        sret_ptr.pdf = float(0);
        sret_ptr.edf_over_pdf = (float3)0;
        sret_ptr.event_type = int(0);
    } else {
        float tmp32 = 1.0 / sqrt(tmp31);
        float tmp33 = tmp32 * tmp29.x;
        float tmp34 = tmp32 * tmp29.y;
        float tmp35 = tmp32 * tmp29.z;
        float3 tmp36 = float3(tmp33, tmp34, tmp35);
        float3 tmp37 = tmp36.zxy * tmp28 - tmp36.yzx * tmp27;
        float tmp38 = tmp33 * phi_out7 + phi_out * tmp0.x + tmp37.x * phi_out5;
        float tmp39 = tmp34 * phi_out7 + phi_out * tmp0.y + tmp37.y * phi_out5;
        float tmp40 = tmp35 * phi_out7 + phi_out * tmp0.z + tmp37.z * phi_out5;
        float3 tmp41 = float3(sqrt(tmp38 * tmp38 + tmp39 * tmp39 + tmp40 * tmp40), 0.0, 0.0);
        float3 tmp42 = float3(tmp38, tmp39, tmp40) / tmp41.xxx;
        sret_ptr.k1.x = tmp42.x;
        sret_ptr.k1.y = tmp42.y;
        sret_ptr.k1.z = tmp42.z;
        phi_in43 = 0;
        if (phi_out > 0.0) {
            float3 tmp45 = tmp42 * tmp1;
            phi_in43 = 0;
            if (tmp45.x + tmp45.y + tmp45.z > 0.0) {
                sret_ptr.pdf = phi_out * 0.3183099;
                sret_ptr.edf_over_pdf.x = 1.0;
                sret_ptr.edf_over_pdf.y = 1.0;
                sret_ptr.edf_over_pdf.z = 1.0;
                sret_ptr.event_type = 1;
                sret_ptr.handle = 0;
                phi_in43 = 1;
            }
        }
        phi_out44 = phi_in43;
        if (phi_out44 == 0) {
            sret_ptr.k1 = (float3)0;
            sret_ptr.pdf = float(0);
            sret_ptr.edf_over_pdf = (float3)0;
            sret_ptr.event_type = int(0);
        }
    }
    return;
}

void mdl_edf_emission_4_evaluate(inout Edf_evaluate_data sret_ptr, in Shading_state_material state)
{
    float3 tmp0 = state.normal;
    float3 tmp1 = state.geom_normal;
    float3 tmp2 = float3(sret_ptr.k1.x, sret_ptr.k1.y, sret_ptr.k1.z);
    float3 tmp3 = tmp2 * tmp1;
    float tmp4 = asfloat((asint(tmp3.x + tmp3.y + tmp3.z) & -2147483648) | 1065353216);
    float3 tmp5 = float3(tmp1.x * tmp4, tmp1.y * tmp4, tmp1.z * tmp4) * tmp0;
    float tmp6 = asfloat((asint(tmp5.x + tmp5.y + tmp5.z) & -2147483648) | 1065353216);
    float3 tmp7 = float3(tmp0.x * tmp6, tmp0.y * tmp6, tmp0.z * tmp6) * tmp2;
    float tmp8 = tmp7.x + tmp7.y + tmp7.z;
    float tmp9 = tmp8 > 0.0 ? tmp8 : 0.0;
    sret_ptr.cos = tmp9;
    sret_ptr.pdf = tmp9 * 0.3183099;
    sret_ptr.edf.x = 0.3183099;
    sret_ptr.edf.y = 0.3183099;
    sret_ptr.edf.z = 0.3183099;
    return;
}

void mdl_edf_emission_4_pdf(inout Edf_pdf_data sret_ptr, in Shading_state_material state)
{
    float3 tmp0 = state.normal;
    float3 tmp1 = state.geom_normal;
    float3 tmp2 = float3(sret_ptr.k1.x, sret_ptr.k1.y, sret_ptr.k1.z);
    float3 tmp3 = tmp2 * tmp1;
    float tmp4 = asfloat((asint(tmp3.x + tmp3.y + tmp3.z) & -2147483648) | 1065353216);
    float3 tmp5 = float3(tmp1.x * tmp4, tmp1.y * tmp4, tmp1.z * tmp4) * tmp0;
    float tmp6 = asfloat((asint(tmp5.x + tmp5.y + tmp5.z) & -2147483648) | 1065353216);
    float3 tmp7 = float3(tmp0.x * tmp6, tmp0.y * tmp6, tmp0.z * tmp6) * tmp2;
    float tmp8 = tmp7.x + tmp7.y + tmp7.z;
    sret_ptr.pdf = (tmp8 > 0.0 ? tmp8 : 0.0) * 0.3183099;
    return;
}

float3 mdl_edf_emission_intensity_4(in Shading_state_material state)
{
    return float3(-3.373259e+08, -3.373259e+08, -3.373259e+08);
}
void mdl_bsdf_scattering_sample(in int idx, inout Bsdf_sample_data sInOut, in Shading_state_material sIn)
{
	switch(idx)
	{
		case 0: mdl_bsdf_scattering_0_sample(sInOut, sIn); return;
		case 1: mdl_bsdf_scattering_1_sample(sInOut, sIn); return;
		case 2: mdl_bsdf_scattering_2_sample(sInOut, sIn); return;
		case 3: mdl_bsdf_scattering_3_sample(sInOut, sIn); return;
		case 4: mdl_bsdf_scattering_4_sample(sInOut, sIn); return;
	}
}
void mdl_bsdf_scattering_init(in int idx, in Shading_state_material sIn)
{
	switch(idx)
	{
		case 0: mdl_bsdf_scattering_0_init(sIn); return;
		case 1: mdl_bsdf_scattering_1_init(sIn); return;
		case 2: mdl_bsdf_scattering_2_init(sIn); return;
		case 3: mdl_bsdf_scattering_3_init(sIn); return;
		case 4: mdl_bsdf_scattering_4_init(sIn); return;
	}
}
void mdl_bsdf_scattering_pdf(in int idx, inout Bsdf_pdf_data sInOut, in Shading_state_material sIn)
{
	switch(idx)
	{
		case 0: mdl_bsdf_scattering_0_pdf(sInOut, sIn); return;
		case 1: mdl_bsdf_scattering_1_pdf(sInOut, sIn); return;
		case 2: mdl_bsdf_scattering_2_pdf(sInOut, sIn); return;
		case 3: mdl_bsdf_scattering_3_pdf(sInOut, sIn); return;
		case 4: mdl_bsdf_scattering_4_pdf(sInOut, sIn); return;
	}
}
void mdl_bsdf_scattering_evaluate(in int idx, inout Bsdf_evaluate_data sInOut, in Shading_state_material sIn)
{
	switch(idx)
	{
		case 0: mdl_bsdf_scattering_0_evaluate(sInOut, sIn); return;
		case 1: mdl_bsdf_scattering_1_evaluate(sInOut, sIn); return;
		case 2: mdl_bsdf_scattering_2_evaluate(sInOut, sIn); return;
		case 3: mdl_bsdf_scattering_3_evaluate(sInOut, sIn); return;
		case 4: mdl_bsdf_scattering_4_evaluate(sInOut, sIn); return;
	}
}
void mdl_edf_emission_evaluate(in int idx, inout Edf_evaluate_data sInOut, in Shading_state_material sIn)
{
	switch(idx)
	{
		case 0: mdl_edf_emission_0_evaluate(sInOut, sIn); return;
		case 1: mdl_edf_emission_1_evaluate(sInOut, sIn); return;
		case 2: mdl_edf_emission_2_evaluate(sInOut, sIn); return;
		case 3: mdl_edf_emission_3_evaluate(sInOut, sIn); return;
		case 4: mdl_edf_emission_4_evaluate(sInOut, sIn); return;
	}
}
void mdl_edf_emission_init(in int idx, in Shading_state_material sIn)
{
	switch(idx)
	{
		case 0: mdl_edf_emission_0_init(sIn); return;
		case 1: mdl_edf_emission_1_init(sIn); return;
		case 2: mdl_edf_emission_2_init(sIn); return;
		case 3: mdl_edf_emission_3_init(sIn); return;
		case 4: mdl_edf_emission_4_init(sIn); return;
	}
}
float3 mdl_edf_emission_intensity(in int idx, in Shading_state_material sIn)
{
	switch(idx)
	{
		case 0: return mdl_edf_emission_intensity_0(sIn);
		case 1: return mdl_edf_emission_intensity_1(sIn);
		case 2: return mdl_edf_emission_intensity_2(sIn);
		case 3: return mdl_edf_emission_intensity_3(sIn);
		case 4: return mdl_edf_emission_intensity_4(sIn);
	}
	return float3(0.0, 0.0, 0.0);
}

#include "helper.h"
#include "materials.h"
#include "pack.h"
#include "lights.h"
#include "helper.h"
#include "raytracing.h"
#include "pathtracerparam.h"
#include "instanceconstants.h"

ConstantBuffer<PathTracerParam> ubo;

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;
Texture2D<float4> gbTangent;
Texture2D<int> gbInstId;
Texture2D<float2> gbUV;

TextureCube<float4> cubeMap;
SamplerState cubeMapSampler;

StructuredBuffer<BVHNode> bvhNodes;
StructuredBuffer<Vertex> vb;
StructuredBuffer<uint> ib;
StructuredBuffer<InstanceConstants> instanceConstants;
StructuredBuffer<Material> materials;
StructuredBuffer<MdlMaterial> mdlMaterials;
StructuredBuffer<RectLight> lights;

RWTexture2D<float4> output;

float3 UniformSampleRect(in RectLight l, float2 u)
{
    float3 e1 = l.points[1].xyz - l.points[0].xyz;
    float3 e2 = l.points[3].xyz - l.points[0].xyz;
    return l.points[0].xyz + e1 * u.x + e2 * u.y;
}

float3 calcLightNormal(in RectLight l)
{
    float3 e1 = l.points[1].xyz - l.points[0].xyz;
    float3 e2 = l.points[3].xyz - l.points[0].xyz;
    return normalize(cross(e1, e2));
}

float calcLightArea(in RectLight l)
{
    float3 e1 = l.points[1].xyz - l.points[0].xyz;
    float3 e2 = l.points[3].xyz - l.points[0].xyz;
    return length(cross(e1, e2));    
}

float3 estimateDirectLighting(inout uint rngState, in Accel accel, in RectLight light, in Shading_state_material state, out float3 toLight, out float lightPdf)
{
    const float3 pointOnLight = UniformSampleRect(light, float2(rand(rngState), rand(rngState)));
    float3 L = normalize(pointOnLight - state.position);
    float3 lightNormal = calcLightNormal(light);
    float3 Li = light.color.rgb;

    if (dot(state.normal, L) > 0 && -dot(L, lightNormal) > 0.0 && all(Li))
    {
        float distToLight = distance(pointOnLight, state.position);
        float lightArea = calcLightArea(light);
        float lightPDF = distToLight * distToLight / (-dot(L, lightNormal) * lightArea);

        Ray shadowRay;
        shadowRay.d = float4(L, 0.0);
        const float3 offset = state.normal * 1e-6; // need to add small offset to fix self-collision
        shadowRay.o = float4(state.position + offset, distToLight - 1e-5);

        Hit shadowHit;
        shadowHit.t = 0.0;
        float visibility = anyHit(accel, shadowRay, shadowHit) ? 0.0f : 1.0f;

        toLight = L;
        lightPdf = lightPDF;
        return visibility * Li * saturate(dot(state.normal, L)) / (lightPDF + 1e-5);
    }

    return float3(0.0);
}

float3 sampleLights(inout uint rngState, in Accel accel, in Shading_state_material state, out float3 toLight, out float lightPdf)
{
    uint lightId = (uint) (ubo.numLights * rand(rngState));
    float lightSelectionPdf = 1.0f / (ubo.numLights + 1e-6);
    RectLight currLight = lights[lightId];
    float3 r = estimateDirectLighting(rngState, accel, currLight, state, toLight, lightPdf);
    lightPdf *= lightSelectionPdf;

    return r / (lightPdf + 1e-5);
}

float3 CalcBumpedNormal(float3 normal, float3 tangent, float2 uv, uint32_t texId, uint32_t sampId)
{
    float3 Normal = normalize(normal);
    float3 Tangent = -normalize(tangent);
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    float3 Bitangent = cross(Normal, Tangent);

    float3 BumpMapNormal = mdl_textures_2d[NonUniformResourceIndex(texId)].Sample(mdl_sampler_tex, uv).xyz;
    BumpMapNormal = BumpMapNormal * 2.0 - 1.0;

    float3x3 TBN = transpose(float3x3(Tangent, Bitangent, Normal));
    float3 NewNormal = normalize(mul(TBN, BumpMapNormal));

    return NewNormal;
}

float toRad(float a)
{
    return a * PI / 180;
}

// Matrices version
Ray generateCameraRay(uint2 pixelIndex)
{
    float2 pixelPos = float2(pixelIndex) + 0.5f;
    float2 pixelNDC = (pixelPos / ubo.dimension) * 2.0f - 1.0f;

    float4 clip = float4(pixelNDC, 1.0, 1.0);
    float4 viewSpace = mul(ubo.clipToView, clip);

    float4 wpos = mul(ubo.viewToWorld, float4(viewSpace.xyz, 0.0f));

    Ray ray = (Ray) 0;
    //ray.o = ubo.camPos;
    ray.o = mul(ubo.viewToWorld, float4(0.0f, 0.0f, 0.0f, 1.0f));;
    ray.o.w = 1e9;
    ray.d.xyz = normalize(wpos.xyz);

    return ray;
}

// Generates a primary ray for pixel given in NDC space using pinhole camera
Ray generateCameraRays(uint2 pixelIndex)
{
    float2 pixelPos = float2(pixelIndex) + 0.5;
    float2 pixelNDC = pixelPos / ubo.dimension * 2.0 - 1.0;

    // Setup the ray
    Ray ray;
    ray.o.xyz = ubo.worldToView[3].xyz;

    // Extract the aspect ratio and field of view from the projection matrix
    float aspect = ubo.viewToClip[1][1] / ubo.viewToClip[0][0];
    float tanHalfFovY = 1.0f / ubo.viewToClip[1][1];

    // Compute the ray direction for this pixel
    ray.d.xyz = normalize(
        (pixelNDC.x * ubo.worldToView[0].xyz * tanHalfFovY * aspect) -
            (pixelNDC.y * ubo.worldToView[1].xyz * tanHalfFovY) +
            ubo.worldToView[2].xyz);

    ray.o.w = 1e9;

    return ray;
}

float3 pathTrace1(uint2 pixelIndex)
{
    Accel accel;
    accel.bvhNodes = bvhNodes;
    accel.instanceConstants = instanceConstants;
    accel.vb = vb;
    accel.ib = ib;

    uint rngState = initRNG(pixelIndex, ubo.dimension, ubo.frameNumber);

    float3 finalColor = float3(0.0f);
    float3 throughput = float3(1.0f);

    int depth = 0;
    const int maxDepth = ubo.maxDepth;

    Ray ray = generateCameraRay(pixelIndex);

    while (depth < maxDepth)
    {
        Hit hit;
        hit.t = 0.0;
        if (closestHit(accel, ray, hit))
        {
            InstanceConstants instConst = accel.instanceConstants[hit.instId];
            Material material = materials[instConst.materialId];

            if (material.isLight)
            {
                finalColor += throughput * float3(1.0f);
                //depth = maxDepth;
                break;
            }
            else
            {
                uint i0 = accel.ib[instConst.indexOffset + hit.primId * 3 + 0];
                uint i1 = accel.ib[instConst.indexOffset + hit.primId * 3 + 1];
                uint i2 = accel.ib[instConst.indexOffset + hit.primId * 3 + 2];

                float3 p0 = mul(instConst.objectToWorld, float4(accel.vb[i0].position, 1.0f)).xyz;
                float3 p1 = mul(instConst.objectToWorld, float4(accel.vb[i1].position, 1.0f)).xyz;
                float3 p2 = mul(instConst.objectToWorld, float4(accel.vb[i2].position, 1.0f)).xyz;

                float3 geom_normal = normalize(cross(p1 - p0, p2 - p0));

                float3 n0 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i0].normal));
                float3 n1 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i1].normal));
                float3 n2 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i2].normal));

                float3 t0 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i0].tangent));
                float3 t1 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i1].tangent));
                float3 t2 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i2].tangent));

                const float2 bcoords = hit.bary;
                float3 world_position = interpolateAttrib(p0, p1, p2, bcoords);
                float3 world_normal = normalize(interpolateAttrib(n0, n1, n2, bcoords));
                float3 world_tangent = normalize(interpolateAttrib(t0, t1, t2, bcoords));
                float3 world_binormal = cross(world_normal, world_tangent);

                if (ubo.debug == 1)
                {
                    float3 debugN = (world_normal + 1.0) * 0.5;
                    finalColor = debugN;
                    break;
                }

                float2 uv0 = unpackUV(accel.vb[i0].uv);
                float2 uv1 = unpackUV(accel.vb[i1].uv);
                float2 uv2 = unpackUV(accel.vb[i2].uv);

                float2 uvCoord = interpolateAttrib(uv0, uv1, uv2, bcoords);

                MdlMaterial currMdlMaterial = mdlMaterials[instConst.materialId];

                // setup MDL state
                Shading_state_material mdlState;
                mdlState.normal = world_normal;
                mdlState.geom_normal = geom_normal;
                mdlState.position = world_position; // hit position
                mdlState.animation_time = 0.0f;
                mdlState.tangent_u[0] = world_tangent;
                mdlState.tangent_v[0] = world_binormal;
                mdlState.ro_data_segment_offset = currMdlMaterial.ro_data_segment_offset;
                mdlState.world_to_object = instConst.worldToObject;
                mdlState.object_to_world = instConst.objectToWorld; // TODO: replace on precalc
                mdlState.object_id = 0;
                mdlState.meters_per_scene_unit = 1.0f;
                mdlState.arg_block_offset = currMdlMaterial.arg_block_offset;
                mdlState.text_coords[0] = float3(uvCoord, 0);

                int scatteringFunctionIndex = currMdlMaterial.functionId;
                mdl_bsdf_scattering_init(scatteringFunctionIndex, mdlState);

                float3 toLight; //return value for sampleLights()
                float lightPdf = 0.0f; //return value for sampleLights()
                float3 radianceOverPdf = sampleLights(rngState, accel, mdlState, toLight, lightPdf);

                Bsdf_evaluate_data evalData = (Bsdf_evaluate_data) 0;
                evalData.ior1 = 1.5;       // IOR current medium // 1.2
                evalData.ior2 = 1.5;       // IOR other side
                evalData.k1 = -ray.d.xyz;   // outgoing direction
                evalData.k2 = toLight;      // incoming direction

                mdl_bsdf_scattering_evaluate(scatteringFunctionIndex, evalData, mdlState);

                if (evalData.pdf > 0.0f)
                {
                    const float mis_weight = lightPdf / (lightPdf + evalData.pdf + 1e-5);
                    const float3 w = throughput * radianceOverPdf * mis_weight;
                    finalColor += w * evalData.bsdf_diffuse;
                    finalColor += w * evalData.bsdf_glossy;
                }

                float4 rndSample = float4(rand(rngState), rand(rngState), rand(rngState), rand(rngState));

                Bsdf_sample_data sampleData = (Bsdf_sample_data) 0;
                sampleData.ior1 = 1.5;
                sampleData.ior2 = 1.5;
                sampleData.k1 = -ray.d.xyz;
                sampleData.xi = rndSample;

                mdl_bsdf_scattering_sample(scatteringFunctionIndex, sampleData, mdlState);

                throughput *= sampleData.bsdf_over_pdf;

                if (depth > 3)
                {
                    float p = max(throughput.r, max(throughput.g, throughput.b));
                    if (rand(rngState) > p)
                    {
                        // break
                        depth = maxDepth;
                    }
                    throughput *= 1.0 / p;
                }

                const float3 offset = world_normal * 1e-6; // need to add small offset to fix self-collision
                // add check and flip offset for transmission event
                ray.o = float4(mdlState.position + offset, 1e9);
                ray.d = float4(sampleData.k2, 0.0);
            }
        }
        else
        {
            // miss - add background color and exit
            finalColor += (throughput + 1e-5) * cubeMap.Sample(cubeMapSampler, ray.d.xyz).rgb;
            //depth = maxDepth;
            break;
        }
        ++depth;
    }
    return finalColor;
}

float3 pathTrace(uint2 pixelIndex)
{
    float4 gbWorldPos = gbWPos[pixelIndex];
    // early out - miss on camera ray
    if (gbWorldPos.w == 0.0)
        return 0;

    InstanceConstants instConst = instanceConstants[gbInstId[pixelIndex]];
    Material material = materials[instConst.materialId];
    float2 matUV = gbUV[pixelIndex];
    if (material.isLight)
    {
        return getBaseColor(material, matUV, mdl_textures_2d, mdl_sampler_tex);
    }
    float3 wpos = gbWPos[pixelIndex].xyz;

    float3 N = normalize(gbNormal[pixelIndex].xyz);
    float3 world_normal = N;// normalize(mul(float4(N, 0), instConst.worldToObject).xyz);

    float4 tangent0 = gbTangent[pixelIndex];
    tangent0.xyz = normalize(tangent0.xyz);
    float3 world_tangent = tangent0.xyz;//normalize(mul(instConst.objectToWorld, float4(tangent0.xyz, 0)).xyz);
    world_tangent = normalize(world_tangent - dot(world_tangent, world_normal) * world_normal);
    float3 world_binormal = cross(world_normal, world_tangent);

    uint rngState = initRNG(pixelIndex, ubo.dimension, ubo.frameNumber);

    MdlMaterial currMdlMaterial = mdlMaterials[instConst.materialId];

    // setup MDL state
    Shading_state_material mdlState = (Shading_state_material) 0;
    mdlState.normal = world_normal;
    mdlState.geom_normal = world_normal;
    mdlState.position = wpos;
    mdlState.animation_time = 0.0f;
    mdlState.tangent_u[0] = world_tangent;
    mdlState.tangent_v[0] = world_binormal;
    mdlState.world_to_object = instConst.objectToWorld;
    mdlState.object_to_world = instConst.worldToObject;
    mdlState.object_id = 0;
    mdlState.meters_per_scene_unit = 1.0f;
    //fill from MDL material struct
    mdlState.arg_block_offset = currMdlMaterial.arg_block_offset;
    mdlState.ro_data_segment_offset = currMdlMaterial.ro_data_segment_offset;
    
    mdlState.text_coords[0] = float3(matUV, 0);

    int scatteringFunctionIndex = currMdlMaterial.functionId;
    mdl_bsdf_scattering_init(scatteringFunctionIndex, mdlState);

    Accel accel;
    accel.bvhNodes = bvhNodes;
    accel.instanceConstants = instanceConstants;
    accel.vb = vb;
    accel.ib = ib;

    if (ubo.debug == 1)
    {
        //float3 debugN = (world_normal + 1.0) * 0.5;
        if (scatteringFunctionIndex == 0)
        {
            return float3(1.0, 0.0, 0.0);
        }
        else if (scatteringFunctionIndex == 1)
        {
            return float3(0.0, 1.0, 0.0);
        }
        else
        {
            return float3(0.0, 0.0, 1.0);
        }
        //return debugN;
    }

    float3 V = normalize(wpos - ubo.camPos.xyz);

    float3 toLight;
    float lightPdf = 0;
    float3 radianceOverPdf = sampleLights(rngState, accel, mdlState, toLight, lightPdf);

    const float ior1 = 1.5f;
    const float ior2 = 1.5f;

    Bsdf_evaluate_data evalData = (Bsdf_evaluate_data) 0;
    evalData.ior1 = ior1;    // IOR current medium
    evalData.ior2 = ior2;    // IOR other side
    evalData.k1 = -V;        // outgoing direction
    evalData.k2 = toLight;   // incoming direction
    
    mdl_bsdf_scattering_evaluate(scatteringFunctionIndex, evalData, mdlState);

    float3 finalColor = float3(0.0f);
    if (evalData.pdf > 0.0f)
    {
        const float mis_weight = lightPdf / (lightPdf + evalData.pdf + 1e-5);
        const float3 w = float3(1.0f) * radianceOverPdf * mis_weight;
        finalColor += w * evalData.bsdf_diffuse;
        finalColor += w * evalData.bsdf_glossy;
    }

    float4 rndSample = float4(rand(rngState), rand(rngState), rand(rngState), rand(rngState));

    Bsdf_sample_data sampleData = (Bsdf_sample_data) 0;
    sampleData.ior1 = ior1;
    sampleData.ior2 = ior2;
    sampleData.k1 = -V;
    sampleData.xi = rndSample;

    mdl_bsdf_scattering_sample(scatteringFunctionIndex, sampleData, mdlState);

    // generate new ray
    Ray ray;
    const float3 offset = world_normal * 1e-6; // need to add small offset to fix self-collision
    // add check and flip offset for transmission event
    ray.o = float4(mdlState.position + offset, 1e9);
    ray.d = float4(sampleData.k2, 0.0);

    Hit hit;
    hit.t = 0.0;
    
    float3 throughput = sampleData.bsdf_over_pdf;
    
    int depth = 1;
    int maxDepth = ubo.maxDepth;
    while (depth < maxDepth)
    {
        if (closestHit(accel, ray, hit))
        {
            instConst = accel.instanceConstants[hit.instId];
            material = materials[instConst.materialId];

            if (material.isLight)
            {
                finalColor += throughput * float3(1.0f);
                //break;
                depth = maxDepth;
            }
            else
            {
                uint i0 = accel.ib[instConst.indexOffset + hit.primId * 3 + 0];
                uint i1 = accel.ib[instConst.indexOffset + hit.primId * 3 + 1];
                uint i2 = accel.ib[instConst.indexOffset + hit.primId * 3 + 2];

                float3 p0 = mul(instConst.objectToWorld, float4(accel.vb[i0].position, 1.0f)).xyz;
                float3 p1 = mul(instConst.objectToWorld, float4(accel.vb[i1].position, 1.0f)).xyz;
                float3 p2 = mul(instConst.objectToWorld, float4(accel.vb[i2].position, 1.0f)).xyz;

                float3 geom_normal = normalize(cross(p1 - p0, p2 - p0));

                float3 n0 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i0].normal));
                float3 n1 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i1].normal));
                float3 n2 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i2].normal));

                float3 t0 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i0].tangent));
                float3 t1 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i1].tangent));
                float3 t2 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i2].tangent));

                const float2 bcoords = hit.bary;

                float3 world_normal = normalize(interpolateAttrib(n0, n1, n2, bcoords));
                float3 world_tangent = normalize(interpolateAttrib(t0, t1, t2, bcoords));
                float3 world_binormal = cross(world_normal, world_tangent);

                float2 uv0 = unpackUV(accel.vb[i0].uv);
                float2 uv1 = unpackUV(accel.vb[i1].uv);
                float2 uv2 = unpackUV(accel.vb[i2].uv);

                float2 uvCoord = interpolateAttrib(uv0, uv1, uv2, bcoords);

               currMdlMaterial = mdlMaterials[instConst.materialId];

                // setup MDL state
                mdlState.normal = world_normal;
                mdlState.geom_normal = geom_normal;
                mdlState.position = ray.o.xyz + ray.d.xyz * hit.t; // hit position
                mdlState.animation_time = 0.0f;
                mdlState.tangent_u[0] = world_tangent;
                mdlState.tangent_v[0] = world_binormal;
                mdlState.ro_data_segment_offset = currMdlMaterial.ro_data_segment_offset;
                mdlState.world_to_object = instConst.objectToWorld;
                mdlState.object_to_world = instConst.worldToObject; // TODO: replace on precalc
                mdlState.object_id = 0;
                mdlState.meters_per_scene_unit = 1.0f;
                mdlState.arg_block_offset = currMdlMaterial.arg_block_offset;
                mdlState.text_coords[0] = float3(uvCoord, 0);

                int scatteringFunctionIndex = currMdlMaterial.functionId;
                mdl_bsdf_scattering_init(scatteringFunctionIndex, mdlState);

                radianceOverPdf = sampleLights(rngState, accel, mdlState, toLight, lightPdf);

                Bsdf_evaluate_data evalData = (Bsdf_evaluate_data) 0;
                evalData.ior1 = ior1;       // IOR current medium // 1.2
                evalData.ior2 = ior2;       // IOR other side
                evalData.k1 = -ray.d.xyz; // outgoing direction
                evalData.k2 = toLight;      // incoming direction
                
                mdl_bsdf_scattering_evaluate(scatteringFunctionIndex, evalData, mdlState);

                if (evalData.pdf > 0.0f)
                {
                    const float mis_weight = lightPdf / (lightPdf + evalData.pdf + 1e-5);
                    const float3 w = throughput * radianceOverPdf * mis_weight;
                    finalColor += w * evalData.bsdf_diffuse;
                    finalColor += w * evalData.bsdf_glossy;
                }

                rndSample = float4(rand(rngState), rand(rngState), rand(rngState), rand(rngState));

                Bsdf_sample_data sampleData = (Bsdf_sample_data) 0;
                sampleData.ior1 = ior1;
                sampleData.ior2 = ior2;
                sampleData.k1 = -ray.d.xyz;
                sampleData.xi = rndSample;

                mdl_bsdf_scattering_sample(scatteringFunctionIndex, sampleData, mdlState);

                throughput *= sampleData.bsdf_over_pdf;

                if (depth > 3)
                {
                    float p = max(throughput.r, max(throughput.g, throughput.b));
                    if (rand(rngState) > p)
                    {
                        // break
                        depth = maxDepth;
                    }
                    throughput *= 1.0 / p;
                }

                const float3 offset = world_normal * 1e-6; // need to add small offset to fix self-collision
                // add check and flip offset for transmission event
                ray.o = float4(mdlState.position + offset, 1e9);
                ray.d = float4(sampleData.k2, 0.0);
            }
        }
        else
        {
            // miss - add background color and exit
            // finalColor += throughput * float3(0.f);
            finalColor += (throughput + 1e-5) * cubeMap.Sample(cubeMapSampler, ray.d.xyz).rgb;

            //break;
            depth = maxDepth;
        }
        ++depth;
    }
    return finalColor;
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }

    float3 color = 0.f;

   color = pathTrace1(pixelIndex);

    output[pixelIndex] = float4(color, 1.0);
}
