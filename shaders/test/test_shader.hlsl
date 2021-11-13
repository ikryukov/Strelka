// shaders.slang

//
// This file provides a simple vertex and fragment shader that can be compiled
// using Slang. This code should also be valid as HLSL, and thus it does not
// use any of the new language features supported by Slang.
//
#include "mdl_types.hlsl"
#include "mdl_runtime.hlsl"
// Uniform data to be passed from application -> shader.
cbuffer Uniforms
{
    float4x4 modelViewProjection;
}
Texture2D<float> shadow;

// Per-vertex attributes to be assembled from bound vertex buffers.
struct AssembledVertex
{
    float3	position : POSITION;
    float3	color    : COLOR;
};

// Output of the vertex shader, and input to the fragment shader.
struct CoarseVertex
{
    float3 color;
};

// Output of the fragment shader
struct Fragment
{
    float4 color;
};

// Vertex  Shader

struct VertexStageOutput
{
    CoarseVertex    coarseVertex    : CoarseVertex;
    float4          sv_position     : SV_Position;
};


  struct structtype0 {
  float3 m_0;
  float3 m_1;
  float3 m_2;
  };

  structtype0 constr_structtype0(float3 m_0, float3 m_1, float3 m_2)
  {
  structtype0 res;
  res.m_0 = m_0;
  res.m_1 = m_1;
  res.m_2 = m_2;
  return res;
  }


static const int glob_cnst30[256] = { 151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180 };
static const int glob_cnst33[256] = { 120, 73, 105, 71, 106, 25, 159, 92, 184, 93, 179, 181, 51, 168, 252, 235, 114, 143, 108, 82, 4, 72, 9, 192, 214, 112, 12, 200, 188, 8, 187, 117, 157, 88, 70, 87, 56, 38, 115, 96, 59, 24, 215, 231, 123, 44, 144, 119, 243, 245, 212, 249, 197, 109, 76, 66, 183, 58, 232, 113, 86, 234, 203, 80, 163, 254, 140, 62, 174, 118, 167, 18, 55, 99, 126, 170, 52, 149, 156, 142, 89, 189, 178, 240, 255, 251, 169, 226, 236, 153, 223, 219, 54, 130, 133, 7, 173, 77, 242, 190, 1, 122, 27, 147, 196, 180, 238, 124, 145, 101, 195, 57, 61, 39, 53, 154, 45, 47, 107, 233, 20, 176, 83, 36, 136, 138, 15, 230, 246, 94, 69, 6, 135, 132, 29, 131, 172, 199, 228, 177, 182, 216, 134, 151, 95, 17, 218, 155, 19, 175, 41, 191, 85, 23, 160, 125, 248, 42, 209, 165, 110, 102, 79, 221, 32, 26, 217, 2, 46, 81, 74, 16, 63, 202, 220, 37, 75, 205, 13, 21, 68, 148, 40, 253, 241, 250, 141, 164, 35, 104, 49, 50, 224, 166, 247, 208, 162, 193, 150, 30, 14, 10, 60, 98, 207, 84, 11, 239, 100, 116, 152, 90, 225, 129, 128, 64, 28, 158, 186, 127, 97, 204, 206, 65, 5, 194, 91, 67, 198, 48, 111, 211, 227, 229, 237, 244, 43, 146, 139, 222, 137, 201, 22, 103, 210, 34, 213, 31, 0, 33, 185, 161, 78, 121, 171, 3 };
static const int glob_cnst36[256] = { 88, 147, 251, 140, 206, 77, 107, 169, 5, 222, 84, 97, 116, 234, 24, 90, 201, 133, 40, 216, 229, 34, 37, 158, 186, 49, 98, 61, 47, 9, 124, 123, 65, 249, 2, 66, 76, 172, 198, 179, 218, 210, 78, 170, 132, 32, 60, 110, 213, 223, 151, 225, 115, 83, 180, 161, 42, 41, 164, 250, 233, 70, 231, 27, 217, 244, 114, 96, 183, 228, 63, 195, 236, 192, 177, 209, 246, 109, 171, 72, 101, 23, 35, 112, 182, 162, 57, 69, 146, 81, 248, 215, 7, 154, 178, 252, 136, 55, 150, 8, 1, 142, 167, 199, 39, 3, 14, 135, 152, 74, 33, 243, 121, 13, 181, 26, 211, 19, 64, 168, 58, 67, 52, 143, 113, 43, 25, 240, 166, 59, 4, 187, 53, 238, 103, 159, 220, 204, 208, 245, 85, 122, 62, 120, 93, 191, 224, 16, 68, 80, 226, 207, 134, 188, 73, 232, 102, 125, 196, 254, 253, 130, 241, 46, 119, 38, 94, 221, 153, 100, 163, 175, 242, 131, 255, 214, 87, 139, 92, 12, 203, 117, 219, 21, 239, 6, 31, 20, 44, 50, 28, 111, 141, 18, 157, 145, 11, 30, 237, 82, 129, 200, 89, 148, 95, 194, 144, 128, 176, 45, 79, 106, 235, 75, 0, 230, 160, 126, 138, 227, 247, 91, 17, 173, 29, 51, 71, 22, 36, 118, 149, 189, 155, 156, 197, 54, 202, 99, 174, 137, 105, 184, 205, 108, 10, 193, 165, 127, 56, 212, 104, 190, 185, 48, 86, 15 };
static const int glob_cnst39[256] = { 249, 199, 162, 114, 17, 55, 64, 57, 29, 137, 194, 247, 45, 70, 210, 106, 184, 178, 219, 122, 156, 193, 214, 126, 138, 28, 148, 121, 19, 37, 135, 132, 173, 225, 221, 161, 180, 220, 36, 84, 224, 10, 185, 209, 238, 119, 89, 253, 165, 248, 120, 235, 198, 52, 3, 190, 125, 226, 20, 6, 168, 2, 170, 167, 94, 54, 201, 179, 42, 208, 242, 33, 146, 158, 245, 196, 166, 83, 86, 34, 74, 188, 44, 49, 85, 11, 243, 4, 99, 100, 102, 78, 252, 26, 35, 24, 113, 236, 237, 9, 1, 72, 63, 204, 13, 15, 129, 53, 79, 163, 212, 213, 48, 92, 97, 38, 189, 68, 230, 234, 41, 22, 246, 133, 250, 202, 77, 112, 18, 218, 229, 124, 181, 14, 108, 107, 255, 91, 145, 223, 134, 142, 96, 88, 222, 207, 141, 175, 203, 27, 69, 95, 62, 47, 147, 164, 130, 232, 39, 244, 21, 154, 239, 153, 110, 172, 59, 46, 81, 176, 217, 80, 150, 159, 182, 186, 66, 174, 169, 98, 231, 123, 215, 12, 128, 187, 127, 58, 32, 111, 160, 149, 31, 195, 65, 152, 144, 82, 197, 216, 75, 61, 101, 117, 93, 51, 60, 0, 67, 211, 241, 206, 90, 87, 56, 240, 73, 177, 43, 155, 157, 71, 191, 136, 103, 116, 140, 205, 143, 228, 109, 23, 16, 171, 115, 7, 131, 192, 183, 105, 251, 5, 139, 40, 200, 30, 254, 227, 76, 151, 50, 8, 104, 118, 233, 25 };

void mdl_bsdf_scattering_0_init(inout Shading_state_material state)
{
    return;
}

float3 expr_lambda_12_(in Shading_state_material state)
{
    float tmp0[2];
    float tmp1[4];
    float4x3 tmp2;
    float3 phi_in;
    float3 phi_out;
    int phi_in9;
    int phi_out10;
    int phi_in48;
    int phi_out49;
    int phi_in50;
    int phi_out51;
    float4 phi_in52;
    float4 phi_out53;
    float3 tmp3 = state.text_coords[0];
    float3 tmp4 = state.tangent_u[0];
    float3 tmp5 = state.tangent_v[0];
    float3 tmp6 = float3(tmp3.x * 33.33333, tmp3.y * 33.33333, tmp3.z * 3.333333);
    float3 tmp7 = state.normal;
    tmp2[0] = float3(0.0, 0.0, 0.0);
    tmp2[1] = float3(0.0001, 0.0, 0.0);
    tmp2[2] = float3(0.0, 0.0001, 0.0);
    tmp2[3] = float3(0.0, 0.0, 0.0001);
    tmp1[0] = float(0);
    tmp1[1] = float(0);
    tmp1[2] = float(0);
    tmp1[3] = float(0);
    int tmp8 = state.ro_data_segment_offset;
    phi_in = float3(0.0, 0.0, 0.0);
    phi_in9 = 0;
    do {
        phi_out = phi_in;
        phi_out10 = phi_in9;
        float3 tmp11 = phi_out + tmp6;
        float tmp12 = floor(tmp11.x);
        int tmp13 = int(tmp12);
        float tmp14 = floor(tmp11.y);
        int tmp15 = int(tmp14);
        float tmp16 = floor(tmp11.z);
        int tmp17 = int(tmp16);
        float tmp18 = floor(0.0);
        int tmp19 = int(tmp18);
        float tmp20 = tmp11.x - tmp12;
        float tmp21 = tmp11.y - tmp14;
        float tmp22 = tmp11.z - tmp16;
        float tmp23 = -tmp18;
        float4 tmp24 = float4(tmp20, tmp21, tmp22, tmp23);
        float tmp25 = tmp20 * tmp20 * tmp20 * ((tmp20 * 6.0 + -15.0) * tmp20 + 10.0);
        float tmp26 = tmp21 * tmp21 * tmp21 * ((tmp21 * 6.0 + -15.0) * tmp21 + 10.0);
        float tmp27 = tmp22 * tmp22 * tmp22 * ((tmp22 * 6.0 + -15.0) * tmp22 + 10.0);
        float tmp28 = tmp18 * 6.0;
        int tmp29 = (glob_cnst30[tmp13 & 255]) & 127;
        int tmp31 = (glob_cnst30[(tmp13 + 1) & 255]) & 127;
        int tmp32 = (glob_cnst33[tmp15 & 255]) & 127;
        int tmp34 = (glob_cnst33[(tmp15 + 1) & 255]) & 127;
        int tmp35 = (glob_cnst36[tmp17 & 255]) & 127;
        int tmp37 = (glob_cnst36[(tmp17 + 1) & 255]) & 127;
        int tmp38 = glob_cnst39[tmp19 & 255];
        tmp0[0] = 0.0;
        tmp0[1] = 0.0;
        int tmp40 = tmp32 ^ tmp29;
        int tmp41 = tmp32 ^ tmp31;
        float tmp42 = 1.0 - tmp25;
        int tmp43 = tmp34 ^ tmp29;
        int tmp44 = tmp34 ^ tmp31;
        float tmp45 = 1.0 - tmp26;
        float tmp46 = 1.0 - tmp27;
        int tmp47 = glob_cnst39[(tmp19 + 1) & 255];
        phi_in48 = 0;
        phi_in50 = tmp38;
        phi_in52 = tmp24;
        do {
            phi_out49 = phi_in48;
            phi_out51 = phi_in50;
            phi_out53 = phi_in52;
            int tmp54 = phi_out51 & 127;
            int tmp55 = tmp54 ^ tmp35;
            int tmp56 = ((tmp55 ^ tmp40) << 4) + tmp8;
            float tmp57 = phi_out53.x + -1.0;
            int tmp58 = ((tmp55 ^ tmp41) << 4) + tmp8;
            float tmp59 = phi_out53.y + -1.0;
            int tmp60 = ((tmp55 ^ tmp43) << 4) + tmp8;
            int tmp61 = ((tmp55 ^ tmp44) << 4) + tmp8;
            int tmp62 = tmp54 ^ tmp37;
            float tmp63 = phi_out53.z + -1.0;
            int tmp64 = ((tmp62 ^ tmp40) << 4) + tmp8;
            int tmp65 = ((tmp62 ^ tmp41) << 4) + tmp8;
            int tmp66 = ((tmp62 ^ tmp43) << 4) + tmp8;
            int tmp67 = ((tmp62 ^ tmp44) << 4) + tmp8;
            tmp0[phi_out49] = (((mdl_read_rodata_as_float(tmp67 + 4) * tmp59 + mdl_read_rodata_as_float(tmp67) * tmp57 + mdl_read_rodata_as_float(tmp67 + 8) * tmp63 + mdl_read_rodata_as_float(tmp67 + 12) * phi_out53.w) * tmp25 + (mdl_read_rodata_as_float(tmp66 + 4) * tmp59 + mdl_read_rodata_as_float(tmp66) * phi_out53.x + mdl_read_rodata_as_float(tmp66 + 8) * tmp63 + mdl_read_rodata_as_float(tmp66 + 12) * phi_out53.w) * tmp42) * tmp26 + ((mdl_read_rodata_as_float(tmp65 + 4) * phi_out53.y + mdl_read_rodata_as_float(tmp65) * tmp57 + mdl_read_rodata_as_float(tmp65 + 8) * tmp63 + mdl_read_rodata_as_float(tmp65 + 12) * phi_out53.w) * tmp25 + (mdl_read_rodata_as_float(tmp64 + 4) * phi_out53.y + mdl_read_rodata_as_float(tmp64) * phi_out53.x + mdl_read_rodata_as_float(tmp64 + 8) * tmp63 + mdl_read_rodata_as_float(tmp64 + 12) * phi_out53.w) * tmp42) * tmp45) * tmp27 + (((mdl_read_rodata_as_float(tmp61 + 4) * tmp59 + mdl_read_rodata_as_float(tmp61) * tmp57 + mdl_read_rodata_as_float(tmp61 + 8) * phi_out53.z + mdl_read_rodata_as_float(tmp61 + 12) * phi_out53.w) * tmp25 + (mdl_read_rodata_as_float(tmp60 + 4) * tmp59 + mdl_read_rodata_as_float(tmp60) * phi_out53.x + mdl_read_rodata_as_float(tmp60 + 8) * phi_out53.z + mdl_read_rodata_as_float(tmp60 + 12) * phi_out53.w) * tmp42) * tmp26 + ((mdl_read_rodata_as_float(tmp58 + 4) * phi_out53.y + mdl_read_rodata_as_float(tmp58) * tmp57 + mdl_read_rodata_as_float(tmp58 + 8) * phi_out53.z + mdl_read_rodata_as_float(tmp58 + 12) * phi_out53.w) * tmp25 + (mdl_read_rodata_as_float(tmp56 + 4) * phi_out53.y + mdl_read_rodata_as_float(tmp56) * phi_out53.x + mdl_read_rodata_as_float(tmp56 + 8) * phi_out53.z + mdl_read_rodata_as_float(tmp56 + 12) * phi_out53.w) * tmp42) * tmp45) * tmp46;
            int tmp68 = phi_out49 + 1;
            phi_in50 = tmp47;
            phi_in52 = float4(phi_out53.x, phi_out53.y, phi_out53.z, phi_out53.w + -1.0);
            if (tmp68 == 2)
                break;
        } while (true);
        float tmp69 = tmp18 * tmp18 * tmp23 * ((-15.0 - tmp28) * tmp23 + 10.0);
        float tmp70 = tmp0[0];
        float tmp71 = tmp0[1];
        float tmp72 = (tmp71 * tmp69 + tmp70 * (1.0 - tmp69)) * 0.5 + 0.5;
        float tmp73 = tmp72 > 0.0 ? tmp72 : 0.0;
        tmp1[phi_out10] = tmp73 < 1.0 ? tmp73 : 1.0;
        int tmp74 = phi_out10 + 1;
        if (tmp74 == 4)
            break;
        else {
            float3 tmp75 = tmp2[tmp74];
            phi_in = tmp75;
            phi_in9 = tmp74;
        }
    } while (true);
    float tmp76 = tmp1[3];
    float tmp77 = tmp1[0];
    float3 tmp78 = float3(abs((tmp76 - tmp77) * -100.0) + 1.0, 0.0, 0.0);
    float tmp79 = tmp1[1];
    float3 tmp80 = float3(tmp79 - tmp77, 0.0, 0.0);
    float tmp81 = tmp1[2];
    float3 tmp82 = float3(tmp81 - tmp77, 0.0, 0.0);
    float3 tmp83 = (tmp82.xxx * tmp5 + tmp80.xxx * tmp4) * float3(-100.0, -100.0, -100.0) + tmp78.xxx * tmp7;
    float3 tmp84 = float3(sqrt(tmp83.x * tmp83.x + tmp83.y * tmp83.y + tmp83.z * tmp83.z), 0.0, 0.0);
    return tmp83 / tmp84.xxx;
}

float3 expr_lambda_11_(in Shading_state_material state)
{
    float3 phi_in;
    float3 phi_out;
    int phi_in13;
    int phi_out14;
    int phi_in20;
    int phi_out21;
    int phi_in26;
    int phi_out27;
    int phi_in32;
    int phi_out33;
    int phi_in38;
    int phi_out39;
    int phi_in43;
    int phi_out44;
    int phi_in49;
    int phi_out50;
    int phi_in54;
    int phi_out55;
    float3 tmp0 = state.text_coords[0];
    float3 tmp1 = state.tangent_u[0];
    float3 tmp2 = state.tangent_v[0];
    float3 tmp3 = state.normal;
    int tmp4 = tex_width_2d(3, int2(0, 0), 0.0);
    int tmp5 = tex_height_2d(3, int2(0, 0), 0.0);
    phi_in = tmp3;
    if (!(tmp4 == 0 || tmp5 == 0)) {
        int tmp6 = int(float(tmp4));
        int tmp7 = int(float(tmp5));
        float tmp8 = tmp0.x * 10.0 * float(tmp6);
        float tmp9 = tmp0.y * 10.0 * float(tmp7);
        float tmp10 = tmp9 + -1.0;
        int tmp11 = int(tmp8 + -1.0);
        bool tmp12 = tmp6 == 0;
        phi_in13 = 0;
        if (!tmp12) {
            int tmp15 = tmp11 % tmp6;
            phi_in13 = tmp15;
        }
        phi_out14 = phi_in13;
        int tmp16 = tmp6 + -1;
        int tmp17 = phi_out14 + (tmp11 > -1 ? 0 : tmp16);
        int tmp18 = int(tmp10);
        bool tmp19 = tmp7 == 0;
        phi_in20 = 0;
        if (!tmp19) {
            int tmp22 = tmp18 % tmp7;
            phi_in20 = tmp22;
        }
        phi_out21 = phi_in20;
        int tmp23 = tmp7 + -1;
        int tmp24 = phi_out21 + (tmp18 > -1 ? 0 : tmp23);
        int tmp25 = int(tmp8);
        phi_in26 = 0;
        if (!tmp12) {
            int tmp28 = tmp25 % tmp6;
            phi_in26 = tmp28;
        }
        phi_out27 = phi_in26;
        int tmp29 = phi_out27 + (tmp25 > -1 ? 0 : tmp16);
        int4 tmp30 = int4(tmp29, 0, 0, 0);
        int tmp31 = int(tmp9);
        phi_in32 = 0;
        if (!tmp19) {
            int tmp34 = tmp31 % tmp7;
            phi_in32 = tmp34;
        }
        phi_out33 = phi_in32;
        int tmp35 = phi_out33 + (tmp31 > -1 ? 0 : tmp23);
        float tmp36 = tmp9 + 1.0;
        int tmp37 = int(tmp8 + 1.0);
        phi_in38 = 0;
        if (!tmp12) {
            int tmp40 = tmp37 % tmp6;
            phi_in38 = tmp40;
        }
        phi_out39 = phi_in38;
        int tmp41 = phi_out39 + (tmp37 > -1 ? 0 : tmp16);
        int tmp42 = int(tmp36);
        phi_in43 = 0;
        if (!tmp19) {
            int tmp45 = tmp42 % tmp7;
            phi_in43 = tmp45;
        }
        phi_out44 = phi_in43;
        int tmp46 = phi_out44 + (tmp42 > -1 ? 0 : tmp23);
        float tmp47 = tmp9 + 2.0;
        int tmp48 = int(tmp8 + 2.0);
        phi_in49 = 0;
        if (!tmp12) {
            int tmp51 = tmp48 % tmp6;
            phi_in49 = tmp51;
        }
        phi_out50 = phi_in49;
        int tmp52 = phi_out50 + (tmp48 > -1 ? 0 : tmp16);
        int tmp53 = int(tmp47);
        phi_in54 = 0;
        if (!tmp19) {
            int tmp56 = tmp53 % tmp7;
            phi_in54 = tmp56;
        }
        phi_out55 = phi_in54;
        int tmp57 = phi_out55 + (tmp53 > -1 ? 0 : tmp23);
        float tmp58 = tmp8 - floor(tmp8);
        float tmp59 = tmp9 - floor(tmp9);
        float tmp60 = tmp58 * tmp58 * tmp58 * ((tmp58 * 6.0 + -15.0) * tmp58 + 10.0);
        float tmp61 = tmp59 * tmp59 * tmp59 * ((tmp59 * 6.0 + -15.0) * tmp59 + 10.0);
        float tmp62 = tmp61 * tmp60;
        float tmp63 = 1.0 - tmp61;
        float tmp64 = tmp63 * tmp60;
        float tmp65 = 1.0 - tmp60;
        float tmp66 = tmp63 * tmp65;
        float tmp67 = tmp65 * tmp61;
        int4 tmp68 = int4(tmp41, tmp35, tmp52, tmp46);
        int2 tmp69 = tmp68.xx;
        float3 tmp70 = tex_texel_color_2d(3, int2(tmp69.x, tmp35), int2(0, 0), 0.0);
        int2 tmp71 = tmp68.zz;
        float3 tmp72 = tex_texel_color_2d(3, int2(tmp71.x, tmp35), int2(0, 0), 0.0);
        float3 tmp73 = tex_texel_color_2d(3, int2(tmp69.x, tmp46), int2(0, 0), 0.0);
        float3 tmp74 = tex_texel_color_2d(3, int2(tmp71.x, tmp46), int2(0, 0), 0.0);
        int4 tmp75 = int4(tmp17, tmp35, tmp29, tmp46);
        int2 tmp76 = tmp75.xx;
        float3 tmp77 = tex_texel_color_2d(3, int2(tmp76.x, tmp35), int2(0, 0), 0.0);
        int2 tmp78 = tmp75.zz;
        float3 tmp79 = tex_texel_color_2d(3, int2(tmp78.x, tmp35), int2(0, 0), 0.0);
        float3 tmp80 = tex_texel_color_2d(3, int2(tmp76.x, tmp46), int2(0, 0), 0.0);
        float3 tmp81 = tex_texel_color_2d(3, int2(tmp78.x, tmp46), int2(0, 0), 0.0);
        int4 tmp82 = int4(tmp30.x, tmp46, tmp41, tmp57);
        int2 tmp83 = tmp82.xx;
        float3 tmp84 = tex_texel_color_2d(3, int2(tmp83.x, tmp46), int2(0, 0), 0.0);
        int2 tmp85 = tmp82.zz;
        float3 tmp86 = tex_texel_color_2d(3, int2(tmp85.x, tmp46), int2(0, 0), 0.0);
        float3 tmp87 = tex_texel_color_2d(3, int2(tmp83.x, tmp57), int2(0, 0), 0.0);
        float3 tmp88 = tex_texel_color_2d(3, int2(tmp85.x, tmp57), int2(0, 0), 0.0);
        int4 tmp89 = int4(tmp30.x, tmp24, tmp41, tmp35);
        int2 tmp90 = tmp89.xx;
        float3 tmp91 = tex_texel_color_2d(3, int2(tmp90.x, tmp24), int2(0, 0), 0.0);
        int2 tmp92 = tmp89.zz;
        float3 tmp93 = tex_texel_color_2d(3, int2(tmp92.x, tmp24), int2(0, 0), 0.0);
        float3 tmp94 = tex_texel_color_2d(3, int2(tmp90.x, tmp35), int2(0, 0), 0.0);
        float3 tmp95 = tex_texel_color_2d(3, int2(tmp92.x, tmp35), int2(0, 0), 0.0);
        float3 tmp96 = float3((tmp62 * (tmp74.x + tmp74.y + tmp74.z) + tmp67 * (tmp73.x + tmp73.y + tmp73.z) + tmp66 * (tmp70.x + tmp70.y + tmp70.z) + tmp64 * (tmp72.x + tmp72.y + tmp72.z) - ((tmp79.x + tmp79.y + tmp79.z) * tmp64 + tmp66 * (tmp77.x + tmp77.y + tmp77.z) + (tmp80.x + tmp80.y + tmp80.z) * tmp67 + (tmp81.x + tmp81.y + tmp81.z) * tmp62)) * -3.333333, 0.0, 0.0);
        float3 tmp97 = float3(((tmp86.x + tmp86.y + tmp86.z) * tmp64 + (tmp84.x + tmp84.y + tmp84.z) * tmp66 + (tmp87.x + tmp87.y + tmp87.z) * tmp67 + (tmp88.x + tmp88.y + tmp88.z) * tmp62 - ((tmp93.x + tmp93.y + tmp93.z) * tmp64 + (tmp91.x + tmp91.y + tmp91.z) * tmp66 + (tmp94.x + tmp94.y + tmp94.z) * tmp67 + (tmp95.x + tmp95.y + tmp95.z) * tmp62)) * -3.333333, 0.0, 0.0);
        float3 tmp98 = tmp96.xxx * tmp1 + tmp3 + tmp97.xxx * tmp2;
        float3 tmp99 = float3(sqrt(tmp98.x * tmp98.x + tmp98.y * tmp98.y + tmp98.z * tmp98.z), 0.0, 0.0);
        float3 tmp100 = tmp98 / tmp99.xxx;
        phi_in = tmp100;
    }
    phi_out = phi_in;
    return phi_out;
}

void gen_weighted_layer_pdf(inout Bsdf_pdf_data p_00, in Shading_state_material p_11)
{
    float3 phi_in;
    float3 phi_out;
    int phi_in28;
    int phi_out29;
    float3 tmp2 = expr_lambda_11_(p_11);
    float3 tmp3 = p_11.text_coords[0];
    float3 tmp4 = p_11.tangent_u[0];
    float3 tmp5 = tex_lookup_color_2d(1, float2(tmp3.x * 10.0, tmp3.y * 10.0), 1, 1, float2(0.0, 1.0), float2(0.0, 1.0), 0.0);
    float tmp6 = tmp5.x + tmp5.y + tmp5.z;
    phi_in = tmp4;
    if (tmp6 * 0.3333333 != 0.0) {
        float tmp7 = tmp6 * 2.094395;
        float3 tmp8 = p_11.normal;
        float3 tmp9 = float3(cos(tmp7), 0.0, 0.0);
        float3 tmp10 = float3(sin(tmp7), 0.0, 0.0);
        float3 tmp11 = tmp9.xxx * tmp4 - (tmp8.yzx * tmp4.zxy - tmp8.zxy * tmp4.yzx) * tmp10.xxx;
        phi_in = tmp11;
    }
    phi_out = phi_in;
    float3 tmp12 = p_11.geom_normal;
    float tmp13 = p_00.k1.x;
    float tmp14 = p_00.k1.y;
    float tmp15 = p_00.k1.z;
    float3 tmp16 = float3(tmp13, tmp14, tmp15);
    float3 tmp17 = tmp16 * tmp12;
    float tmp18 = asfloat((asint(tmp17.x + tmp17.y + tmp17.z) & -2147483648) | 1065353216);
    float3 tmp19 = float3(tmp12.x * tmp18, tmp12.y * tmp18, tmp12.z * tmp18);
    float3 tmp20 = tmp19 * tmp2;
    float tmp21 = asfloat((asint(tmp20.x + tmp20.y + tmp20.z) & -2147483648) | 1065353216);
    float3 tmp22 = float3(tmp2.x * tmp21, tmp2.y * tmp21, tmp2.z * tmp21);
    float3 tmp23 = tmp22.zxy;
    float3 tmp24 = tmp22.yzx;
    float3 tmp25 = tmp23 * phi_out.yzx - tmp24 * phi_out.zxy;
    float3 tmp26 = tmp25 * tmp25;
    float tmp27 = tmp26.x + tmp26.y + tmp26.z;
    if (tmp27 < 1e-08) {
        p_00.pdf = 0.0;
        phi_in28 = 0;
    } else {
        float tmp30 = 1.0 / sqrt(tmp27);
        float3 tmp31 = float3(tmp30 * tmp25.x, tmp30 * tmp25.y, tmp30 * tmp25.z);
        float3 tmp32 = tmp31.zxy * tmp24 - tmp31.yzx * tmp23;
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
        float tmp33 = p_00.k2.x;
        float tmp34 = p_00.k2.y;
        float tmp35 = p_00.k2.z;
        float3 tmp36 = float3(tmp33, tmp34, tmp35);
        float3 tmp37 = tmp36 * tmp19;
        if (tmp37.x + tmp37.y + tmp37.z < 0.0) {
            p_00.pdf = 0.0;
            phi_in28 = 0;
        } else {
            float3 tmp38 = tmp22 * tmp16;
            float tmp39 = tmp38.x + tmp38.y + tmp38.z;
            float tmp40 = tmp33 + tmp13;
            float tmp41 = tmp34 + tmp14;
            float tmp42 = tmp35 + tmp15;
            float3 tmp43 = float3(sqrt(tmp41 * tmp41 + tmp40 * tmp40 + tmp42 * tmp42), 0.0, 0.0);
            float3 tmp44 = float3(tmp40, tmp41, tmp42) / tmp43.xxx;
            float3 tmp45 = tmp44 * tmp22;
            float tmp46 = tmp45.x + tmp45.y + tmp45.z;
            float3 tmp47 = tmp44 * tmp16;
            float3 tmp48 = tmp44 * tmp36;
            if (tmp48.x + tmp48.y + tmp48.z < 0.0 || (tmp46 < 0.0 || tmp47.x + tmp47.y + tmp47.z < 0.0)) {
                p_00.pdf = 0.0;
                phi_in28 = 0;
            } else {
                float3 tmp49 = tmp44 * tmp32;
                float3 tmp50 = tmp44 * tmp31;
                float tmp51 = (tmp49.x + tmp49.y + tmp49.z) * 25.0;
                float tmp52 = (tmp50.x + tmp50.y + tmp50.z) * 2.5;
                float tmp53 = tmp51 * tmp51 + tmp46 * tmp46 + tmp52 * tmp52;
                float3 tmp54 = tmp32 * tmp16;
                float3 tmp55 = tmp31 * tmp16;
                float tmp56 = (tmp54.x + tmp54.y + tmp54.z) * 0.04;
                float tmp57 = (tmp55.x + tmp55.y + tmp55.z) * 0.3999999;
                float tmp58 = 0.25 / (tmp46 * abs(tmp39)) * (2.0 / (sqrt((tmp56 * tmp56 + tmp57 * tmp57) / (tmp39 * tmp39) + 1.0) + 1.0)) * (tmp46 * 19.89437 / (tmp53 * tmp53));
                p_00.pdf = tmp58;
                int tmp59 = asint(tmp58);
                phi_in28 = tmp59;
            }
        }
    }
    phi_out29 = phi_in28;
    p_00.pdf = asfloat(phi_out29);
    return;
}

void gen_microfacet_ggx_smith_bsdf_pdf(inout Bsdf_pdf_data p_00, in Shading_state_material p_11, in float3 p_22)
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
            float tmp32 = tmp31.x + tmp31.y + tmp31.z;
            float tmp33 = tmp26 + tmp5;
            float tmp34 = tmp27 + tmp6;
            float tmp35 = tmp28 + tmp7;
            float3 tmp36 = float3(sqrt(tmp34 * tmp34 + tmp33 * tmp33 + tmp35 * tmp35), 0.0, 0.0);
            float3 tmp37 = float3(tmp33, tmp34, tmp35) / tmp36.xxx;
            float3 tmp38 = tmp37 * tmp17;
            float tmp39 = tmp38.x + tmp38.y + tmp38.z;
            float3 tmp40 = tmp37 * tmp8;
            float3 tmp41 = tmp37 * tmp29;
            if (tmp41.x + tmp41.y + tmp41.z < 0.0 || (tmp39 < 0.0 || tmp40.x + tmp40.y + tmp40.z < 0.0))
                p_00.pdf = 0.0;
            else {
                float3 tmp42 = tmp37 * tmp25;
                float3 tmp43 = tmp37 * tmp24;
                float tmp44 = (tmp42.x + tmp42.y + tmp42.z) * 1e+07;
                float tmp45 = (tmp43.x + tmp43.y + tmp43.z) * 1e+07;
                float tmp46 = tmp44 * tmp44 + tmp39 * tmp39 + tmp45 * tmp45;
                float3 tmp47 = tmp25 * tmp8;
                float3 tmp48 = tmp24 * tmp8;
                float tmp49 = (tmp47.x + tmp47.y + tmp47.z) * 1e-07;
                float tmp50 = (tmp48.x + tmp48.y + tmp48.z) * 1e-07;
                p_00.pdf = 0.25 / (tmp39 * abs(tmp32)) * (2.0 / (sqrt((tmp49 * tmp49 + tmp50 * tmp50) / (tmp32 * tmp32) + 1.0) + 1.0)) * (tmp39 * 3.183099e+13 / (tmp46 * tmp46));
            }
        }
    }
    return;
}

void mdl_bsdf_scattering_0_sample(inout Bsdf_sample_data sret_ptr, in Shading_state_material state)
{
    Bsdf_pdf_data tmp0;
    float3 tmp1;
    int phi_in;
    int phi_out;
    float phi_in18;
    float phi_out19;
    int phi_in21;
    int phi_out22;
    int phi_in23;
    int phi_out24;
    float phi_in25;
    float phi_out26;
    float phi_in27;
    float phi_out28;
    float phi_in29;
    float phi_out30;
    float phi_in36;
    float phi_out37;
    float phi_in38;
    float phi_out39;
    float phi_in51;
    float phi_out52;
    int phi_in74;
    int phi_out75;
    float3 phi_in76;
    float3 phi_out77;
    float phi_in78;
    float phi_out79;
    float phi_in80;
    float phi_out81;
    float phi_in82;
    float phi_out83;
    float phi_in84;
    float phi_out85;
    float phi_in86;
    float phi_out87;
    float phi_in88;
    float phi_out89;
    bool phi_in90;
    bool phi_out91;
    int phi_in92;
    int phi_out93;
    int phi_in94;
    int phi_out95;
    int phi_in96;
    int phi_out97;
    int phi_in104;
    int phi_out105;
    int phi_in106;
    int phi_out107;
    int phi_in108;
    int phi_out109;
    float phi_in124;
    float phi_out125;
    float phi_in126;
    float phi_out127;
    float phi_in128;
    float phi_out129;
    float phi_in141;
    float phi_out142;
    float3 phi_in181;
    float3 phi_out182;
    int phi_in212;
    int phi_out213;
    int phi_in214;
    int phi_out215;
    int phi_in216;
    int phi_out217;
    float phi_in232;
    float phi_out233;
    float phi_in234;
    float phi_out235;
    float phi_in236;
    float phi_out237;
    float phi_in249;
    float phi_out250;
    bool phi_in262;
    bool phi_out263;
    float phi_in302;
    float phi_out303;
    float phi_in315;
    float phi_out316;
    float phi_in317;
    float phi_out318;
    float phi_in319;
    float phi_out320;
    float phi_in330;
    float phi_out331;
    float3 tmp2 = expr_lambda_12_(state);
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
    float tmp16 = sret_ptr.ior1.x;
    int tmp17 = asint(tmp16);
    phi_in = tmp17;
    phi_in18 = tmp16;
    if (tmp16 == -1.0) {
        sret_ptr.ior1.x = 1.0;
        sret_ptr.ior1.y = 1.0;
        sret_ptr.ior1.z = 1.0;
        phi_in = 1065353216;
        phi_in18 = 1.0;
    }
    phi_out = phi_in;
    phi_out19 = phi_in18;
    float tmp20 = sret_ptr.ior2.x;
    if (tmp20 == -1.0) {
        sret_ptr.ior2.x = 1.0;
        sret_ptr.ior2.y = 1.0;
        sret_ptr.ior2.z = 1.0;
        phi_in21 = 1065353216;
        phi_in23 = 1065353216;
        phi_in25 = 1.0;
        phi_in27 = 1.0;
        phi_in29 = 1.0;
    } else {
        int tmp31 = asint(tmp20);
        float tmp32 = sret_ptr.ior2.y;
        float tmp33 = sret_ptr.ior2.z;
        int tmp34 = asint(tmp33);
        phi_in21 = tmp34;
        phi_in23 = tmp31;
        phi_in25 = tmp33;
        phi_in27 = tmp32;
        phi_in29 = tmp20;
    }
    phi_out22 = phi_in21;
    phi_out24 = phi_in23;
    phi_out26 = phi_in25;
    phi_out28 = phi_in27;
    phi_out30 = phi_in29;
    if (phi_out26 == 1.0 && (phi_out28 == 1.0 && phi_out30 == 1.0)) {
        float tmp35 = (sret_ptr.ior1.y + phi_out19 + sret_ptr.ior1.z) * 0.3333333;
        phi_in36 = tmp35;
        phi_in38 = 1.5;
    } else {
        float tmp40 = (phi_out28 + phi_out26 + phi_out30) * 0.3333333;
        phi_in36 = 1.5;
        phi_in38 = tmp40;
    }
    phi_out37 = phi_in36;
    phi_out39 = phi_in38;
    float tmp41 = phi_out39 - phi_out37;
    float tmp42 = (abs(tmp41) < 0.0001 ? phi_out37 + asfloat((asint(tmp41) & -2147483648) | 953267991) : phi_out39) / phi_out37;
    float3 tmp43 = float3(tmp13, tmp14, tmp15);
    float3 tmp44 = tmp43 * tmp7;
    float tmp45 = tmp44.x + tmp44.y + tmp44.z;
    float tmp46 = tmp45 > 0.0 ? tmp45 : 0.0;
    float tmp47 = tmp46 < 1.0 ? tmp46 : 1.0;
    float tmp48 = tmp42 * tmp42;
    float tmp49 = 1.0 - (1.0 - tmp47 * tmp47) / tmp48;
    bool tmp50 = tmp49 < 0.0;
    phi_in51 = 1.0;
    if (!tmp50) {
        float tmp53 = sqrt(tmp49);
        float tmp54 = tmp42 * tmp47;
        float tmp55 = tmp53 * tmp42;
        float tmp56 = (tmp53 - tmp54) / (tmp53 + tmp54);
        float tmp57 = (tmp47 - tmp55) / (tmp55 + tmp47);
        float tmp58 = (tmp57 * tmp57 + tmp56 * tmp56) * 0.5;
        float tmp59 = tmp58 > 0.0 ? tmp58 : 0.0;
        float tmp60 = tmp59 < 1.0 ? tmp59 : 1.0;
        phi_in51 = tmp60;
    }
    phi_out52 = phi_in51;
    float tmp61 = sret_ptr.xi.z;
    if (tmp61 < phi_out52) {
        sret_ptr.xi.z = tmp61 / phi_out52;
        float3 tmp62 = state.tangent_u[0];
        float3 tmp63 = tmp43 * tmp10;
        float tmp64 = asfloat((asint(tmp63.x + tmp63.y + tmp63.z) & -2147483648) | 1065353216);
        float tmp65 = tmp13 * tmp64;
        float tmp66 = tmp14 * tmp64;
        float tmp67 = tmp15 * tmp64;
        float3 tmp68 = float3(tmp65, tmp66, tmp67);
        float3 tmp69 = tmp68.zxy;
        float3 tmp70 = tmp68.yzx;
        float3 tmp71 = tmp62.yzx * tmp69 - tmp62.zxy * tmp70;
        float3 tmp72 = tmp71 * tmp71;
        float tmp73 = tmp72.x + tmp72.y + tmp72.z;
        if (tmp73 < 1e-08) {
            sret_ptr.pdf = 0.0;
            sret_ptr.event_type = 0;
            phi_in74 = 0;
            phi_in76 = float3(0.0, 0.0, 0.0);
            phi_in78 = 0.0;
            phi_in80 = 0.0;
            phi_in82 = 0.0;
            phi_in84 = 0.0;
            phi_in86 = 0.0;
            phi_in88 = 0.0;
            phi_in90 = false;
            phi_in92 = 0;
            phi_in94 = 0;
            phi_in96 = 0;
        } else {
            float tmp98 = 1.0 / sqrt(tmp73);
            float tmp99 = tmp98 * tmp71.x;
            float tmp100 = tmp98 * tmp71.y;
            float tmp101 = tmp98 * tmp71.z;
            float3 tmp102 = float3(tmp99, tmp100, tmp101);
            float3 tmp103 = tmp102.zxy * tmp70 - tmp102.yzx * tmp69;
            phi_in104 = phi_out;
            if (phi_out19 == -1.0) {
                sret_ptr.ior1.x = 1.0;
                sret_ptr.ior1.y = 1.0;
                sret_ptr.ior1.z = 1.0;
                phi_in104 = 1065353216;
            }
            phi_out105 = phi_in104;
            phi_in106 = phi_out22;
            phi_in108 = phi_out24;
            if (phi_out30 == -1.0) {
                sret_ptr.ior2.x = 1.0;
                sret_ptr.ior2.y = 1.0;
                sret_ptr.ior2.z = 1.0;
                phi_in106 = 1065353216;
                phi_in108 = 1065353216;
            }
            phi_out107 = phi_in106;
            phi_out109 = phi_in108;
            float3 tmp110 = tmp68 * tmp7;
            float tmp111 = tmp110.x + tmp110.y + tmp110.z;
            float tmp112 = abs(tmp111);
            float3 tmp113 = tmp103 * tmp7;
            float3 tmp114 = tmp102 * tmp7;
            float tmp115 = sret_ptr.xi.x;
            float tmp116 = sret_ptr.xi.y;
            float tmp117 = (tmp113.x + tmp113.y + tmp113.z) * 1e-07;
            float tmp118 = (tmp114.x + tmp114.y + tmp114.z) * 1e-07;
            float tmp119 = tmp111 * tmp111;
            float tmp120 = tmp118 * tmp118;
            float tmp121 = tmp117 * tmp117 + tmp120;
            float3 tmp122 = float3(sqrt(tmp121 + tmp119), 0.0, 0.0);
            float3 tmp123 = float3(tmp117, tmp112, tmp118) / tmp122.xxx;
            phi_in124 = 1.0;
            phi_in126 = 0.0;
            phi_in128 = 0.0;
            if (tmp123.y < 0.99999) {
                float3 tmp130 = tmp123 * float3(1.0, 0.0, 0.0);
                float3 tmp131 = tmp123 * float3(0.0, 0.0, 1.0);
                float3 tmp132 = tmp130.yzx - tmp131.zxy;
                float3 tmp133 = float3(sqrt(tmp132.x * tmp132.x + tmp132.y * tmp132.y + tmp132.z * tmp132.z), 0.0, 0.0);
                float3 tmp134 = tmp132 / tmp133.xxx;
                phi_in124 = tmp134.x;
                phi_in126 = tmp134.y;
                phi_in128 = tmp134.z;
            }
            phi_out125 = phi_in124;
            phi_out127 = phi_in126;
            phi_out129 = phi_in128;
            float3 tmp135 = float3(phi_out125, phi_out127, phi_out129);
            float3 tmp136 = tmp135.yzx * tmp123.zxy - tmp135.zxy * tmp123.yzx;
            float tmp137 = 1.0 / (tmp123.y + 1.0);
            float tmp138 = sqrt(tmp115);
            bool tmp139 = tmp116 < tmp137;
            if (tmp139) {
                float tmp140 = tmp116 / tmp137 * 3.141593;
                phi_in141 = tmp140;
            } else {
                float tmp143 = (tmp116 - tmp137) / (1.0 - tmp137) * 3.141593 + 3.141593;
                phi_in141 = tmp143;
            }
            phi_out142 = phi_in141;
            float tmp144 = cos(phi_out142) * tmp138;
            float tmp145 = (tmp139 ? 1.0 : tmp123.y) * tmp138 * sin(phi_out142);
            float tmp146 = 1.0 - (tmp145 * tmp145 + tmp144 * tmp144);
            float tmp147 = sqrt(tmp146 < 0.0 ? 0.0 : tmp146);
            float tmp148 = tmp145 * tmp136.y + tmp144 * phi_out127 + tmp147 * tmp123.y;
            float tmp149 = (tmp145 * tmp136.x + tmp144 * phi_out125 + tmp147 * tmp123.x) * 1e-07;
            float tmp150 = tmp148 < 0.0 ? 0.0 : tmp148;
            float tmp151 = (tmp145 * tmp136.z + tmp144 * phi_out129 + tmp147 * tmp123.z) * 1e-07;
            float3 tmp152 = float3(sqrt(tmp151 * tmp151 + tmp149 * tmp149 + tmp150 * tmp150), 0.0, 0.0);
            float3 tmp153 = float3(tmp149, tmp150, tmp151) / tmp152.xxx;
            if (tmp153.y == 0.0) {
                sret_ptr.pdf = 0.0;
                sret_ptr.event_type = 0;
                phi_in74 = 0;
                phi_in76 = float3(0.0, 0.0, 0.0);
                phi_in78 = 0.0;
                phi_in80 = 0.0;
                phi_in82 = 0.0;
                phi_in84 = 0.0;
                phi_in86 = 0.0;
                phi_in88 = 0.0;
                phi_in90 = false;
                phi_in92 = 0;
                phi_in94 = 0;
                phi_in96 = 0;
            } else {
                float tmp154 = tmp153.x * tmp103.x + tmp153.y * tmp65 + tmp153.z * tmp99;
                float tmp155 = tmp153.x * tmp103.y + tmp153.y * tmp66 + tmp153.z * tmp100;
                float tmp156 = tmp153.x * tmp103.z + tmp153.y * tmp67 + tmp153.z * tmp101;
                float3 tmp157 = float3(tmp154, tmp155, tmp156) * tmp7;
                float tmp158 = tmp157.x + tmp157.y + tmp157.z;
                if (tmp158 > 0.0) {
                    float tmp159 = tmp158 * 2.0;
                    float tmp160 = tmp159 * tmp154 - tmp4;
                    float tmp161 = tmp159 * tmp155 - tmp5;
                    float tmp162 = tmp159 * tmp156 - tmp6;
                    sret_ptr.k2.x = tmp160;
                    sret_ptr.k2.y = tmp161;
                    sret_ptr.k2.z = tmp162;
                    sret_ptr.event_type = 10;
                    float3 tmp163 = float3(tmp160, tmp161, tmp162);
                    float3 tmp164 = tmp163 * tmp10;
                    if (tmp164.x + tmp164.y + tmp164.z > 0.0) {
                        sret_ptr.bsdf_over_pdf.x = 1.0;
                        sret_ptr.bsdf_over_pdf.y = 1.0;
                        sret_ptr.bsdf_over_pdf.z = 1.0;
                        float3 tmp165 = tmp163 * tmp68;
                        float tmp166 = tmp165.x + tmp165.y + tmp165.z;
                        float3 tmp167 = tmp163 * tmp103;
                        float tmp168 = 2.0 / (sqrt(tmp121 / tmp119 + 1.0) + 1.0);
                        float tmp169 = (tmp167.x + tmp167.y + tmp167.z) * 1e-07;
                        float tmp170 = 2.0 / (sqrt((tmp169 * tmp169 + tmp120) / (tmp166 * tmp166) + 1.0) + 1.0);
                        if (tmp170 * tmp168 > 0.0) {
                            sret_ptr.bsdf_over_pdf.x = tmp170;
                            sret_ptr.bsdf_over_pdf.y = tmp170;
                            sret_ptr.bsdf_over_pdf.z = tmp170;
                            float tmp171 = tmp153.x * 1e+07;
                            float tmp172 = tmp153.z * 1e+07;
                            float tmp173 = tmp171 * tmp171 + tmp153.y * tmp153.y + tmp172 * tmp172;
                            sret_ptr.pdf = 0.25 / (tmp153.y * tmp112) * tmp168 * (tmp153.y * 3.183099e+13 / (tmp173 * tmp173));
                            sret_ptr.handle = 0;
                            phi_in74 = 1;
                            phi_in76 = tmp163;
                            phi_in78 = tmp170;
                            phi_in80 = tmp170;
                            phi_in82 = tmp170;
                            phi_in84 = tmp162;
                            phi_in86 = tmp161;
                            phi_in88 = tmp160;
                            phi_in90 = true;
                            phi_in92 = phi_out105;
                            phi_in94 = phi_out109;
                            phi_in96 = phi_out107;
                        } else {
                            sret_ptr.pdf = 0.0;
                            sret_ptr.event_type = 0;
                            phi_in74 = 0;
                            phi_in76 = float3(0.0, 0.0, 0.0);
                            phi_in78 = 0.0;
                            phi_in80 = 0.0;
                            phi_in82 = 0.0;
                            phi_in84 = 0.0;
                            phi_in86 = 0.0;
                            phi_in88 = 0.0;
                            phi_in90 = false;
                            phi_in92 = 0;
                            phi_in94 = 0;
                            phi_in96 = 0;
                        }
                    } else {
                        sret_ptr.pdf = 0.0;
                        sret_ptr.event_type = 0;
                        phi_in74 = 0;
                        phi_in76 = float3(0.0, 0.0, 0.0);
                        phi_in78 = 0.0;
                        phi_in80 = 0.0;
                        phi_in82 = 0.0;
                        phi_in84 = 0.0;
                        phi_in86 = 0.0;
                        phi_in88 = 0.0;
                        phi_in90 = false;
                        phi_in92 = 0;
                        phi_in94 = 0;
                        phi_in96 = 0;
                    }
                } else {
                    sret_ptr.pdf = 0.0;
                    sret_ptr.event_type = 0;
                    phi_in74 = 0;
                    phi_in76 = float3(0.0, 0.0, 0.0);
                    phi_in78 = 0.0;
                    phi_in80 = 0.0;
                    phi_in82 = 0.0;
                    phi_in84 = 0.0;
                    phi_in86 = 0.0;
                    phi_in88 = 0.0;
                    phi_in90 = false;
                    phi_in92 = 0;
                    phi_in94 = 0;
                    phi_in96 = 0;
                }
            }
        }
    } else {
        float tmp174 = (1.0 - tmp61) / (1.0 - phi_out52);
        sret_ptr.xi.z = tmp174;
        float3 tmp175 = expr_lambda_11_(state);
        if (tmp174 < 1.0) {
            float3 tmp176 = state.text_coords[0];
            float3 tmp177 = state.tangent_u[0];
            float2 tmp178 = float2(tmp176.x * 10.0, tmp176.y * 10.0);
            float3 tmp179 = tex_lookup_color_2d(1, tmp178, 1, 1, float2(0.0, 1.0), float2(0.0, 1.0), 0.0);
            float tmp180 = tmp179.x + tmp179.y + tmp179.z;
            phi_in181 = tmp177;
            if (tmp180 * 0.3333333 != 0.0) {
                float tmp183 = tmp180 * 2.094395;
                float3 tmp184 = state.normal;
                float3 tmp185 = float3(cos(tmp183), 0.0, 0.0);
                float3 tmp186 = float3(sin(tmp183), 0.0, 0.0);
                float3 tmp187 = tmp185.xxx * tmp177 - (tmp184.yzx * tmp177.zxy - tmp184.zxy * tmp177.yzx) * tmp186.xxx;
                phi_in181 = tmp187;
            }
            phi_out182 = phi_in181;
            float3 tmp188 = tex_lookup_color_2d(2, tmp178, 1, 1, float2(0.0, 1.0), float2(0.0, 1.0), 0.0) * float3(0.1, 0.1, 0.1);
            float tmp189 = tmp188.x > 0.0 ? tmp188.x : 0.0;
            float tmp190 = tmp189 < 1.0 ? tmp189 : 1.0;
            float tmp191 = tmp188.y > 0.0 ? tmp188.y : 0.0;
            float tmp192 = tmp191 < 1.0 ? tmp191 : 1.0;
            float tmp193 = tmp188.z > 0.0 ? tmp188.z : 0.0;
            float tmp194 = tmp193 < 1.0 ? tmp193 : 1.0;
            float3 tmp195 = tmp175 * tmp10;
            float tmp196 = asfloat((asint(tmp195.x + tmp195.y + tmp195.z) & -2147483648) | 1065353216);
            float tmp197 = tmp175.x * tmp196;
            float tmp198 = tmp175.y * tmp196;
            float tmp199 = tmp175.z * tmp196;
            float3 tmp200 = float3(tmp197, tmp198, tmp199);
            float3 tmp201 = tmp200.zxy;
            float3 tmp202 = tmp200.yzx;
            float3 tmp203 = phi_out182.yzx * tmp201 - phi_out182.zxy * tmp202;
            float3 tmp204 = tmp203 * tmp203;
            float tmp205 = tmp204.x + tmp204.y + tmp204.z;
            if (tmp205 < 1e-08) {
                sret_ptr.pdf = 0.0;
                sret_ptr.event_type = 0;
                phi_in74 = 0;
                phi_in76 = float3(0.0, 0.0, 0.0);
                phi_in78 = 0.0;
                phi_in80 = 0.0;
                phi_in82 = 0.0;
                phi_in84 = 0.0;
                phi_in86 = 0.0;
                phi_in88 = 0.0;
                phi_in90 = false;
                phi_in92 = 0;
                phi_in94 = 0;
                phi_in96 = 0;
            } else {
                float tmp206 = 1.0 / sqrt(tmp205);
                float tmp207 = tmp206 * tmp203.x;
                float tmp208 = tmp206 * tmp203.y;
                float tmp209 = tmp206 * tmp203.z;
                float3 tmp210 = float3(tmp207, tmp208, tmp209);
                float3 tmp211 = tmp210.zxy * tmp202 - tmp210.yzx * tmp201;
                phi_in212 = phi_out;
                if (phi_out19 == -1.0) {
                    sret_ptr.ior1.x = 1.0;
                    sret_ptr.ior1.y = 1.0;
                    sret_ptr.ior1.z = 1.0;
                    phi_in212 = 1065353216;
                }
                phi_out213 = phi_in212;
                phi_in214 = phi_out22;
                phi_in216 = phi_out24;
                if (phi_out30 == -1.0) {
                    sret_ptr.ior2.x = 1.0;
                    sret_ptr.ior2.y = 1.0;
                    sret_ptr.ior2.z = 1.0;
                    phi_in214 = 1065353216;
                    phi_in216 = 1065353216;
                }
                phi_out215 = phi_in214;
                phi_out217 = phi_in216;
                float3 tmp218 = tmp200 * tmp7;
                float tmp219 = tmp218.x + tmp218.y + tmp218.z;
                float tmp220 = abs(tmp219);
                float3 tmp221 = tmp211 * tmp7;
                float3 tmp222 = tmp210 * tmp7;
                float tmp223 = sret_ptr.xi.x;
                float tmp224 = sret_ptr.xi.y;
                float tmp225 = (tmp221.x + tmp221.y + tmp221.z) * 0.04;
                float tmp226 = (tmp222.x + tmp222.y + tmp222.z) * 0.3999999;
                float tmp227 = tmp219 * tmp219;
                float tmp228 = tmp226 * tmp226;
                float tmp229 = tmp225 * tmp225 + tmp228;
                float3 tmp230 = float3(sqrt(tmp229 + tmp227), 0.0, 0.0);
                float3 tmp231 = float3(tmp225, tmp220, tmp226) / tmp230.xxx;
                phi_in232 = 1.0;
                phi_in234 = 0.0;
                phi_in236 = 0.0;
                if (tmp231.y < 0.99999) {
                    float3 tmp238 = tmp231 * float3(1.0, 0.0, 0.0);
                    float3 tmp239 = tmp231 * float3(0.0, 0.0, 1.0);
                    float3 tmp240 = tmp238.yzx - tmp239.zxy;
                    float3 tmp241 = float3(sqrt(tmp240.x * tmp240.x + tmp240.y * tmp240.y + tmp240.z * tmp240.z), 0.0, 0.0);
                    float3 tmp242 = tmp240 / tmp241.xxx;
                    phi_in232 = tmp242.x;
                    phi_in234 = tmp242.y;
                    phi_in236 = tmp242.z;
                }
                phi_out233 = phi_in232;
                phi_out235 = phi_in234;
                phi_out237 = phi_in236;
                float3 tmp243 = float3(phi_out233, phi_out235, phi_out237);
                float3 tmp244 = tmp243.yzx * tmp231.zxy - tmp243.zxy * tmp231.yzx;
                float tmp245 = 1.0 / (tmp231.y + 1.0);
                float tmp246 = sqrt(tmp223);
                bool tmp247 = tmp224 < tmp245;
                if (tmp247) {
                    float tmp248 = tmp224 / tmp245 * 3.141593;
                    phi_in249 = tmp248;
                } else {
                    float tmp251 = (tmp224 - tmp245) / (1.0 - tmp245) * 3.141593 + 3.141593;
                    phi_in249 = tmp251;
                }
                phi_out250 = phi_in249;
                float tmp252 = cos(phi_out250) * tmp246;
                float tmp253 = (tmp247 ? 1.0 : tmp231.y) * tmp246 * sin(phi_out250);
                float tmp254 = 1.0 - (tmp253 * tmp253 + tmp252 * tmp252);
                float tmp255 = sqrt(tmp254 < 0.0 ? 0.0 : tmp254);
                float tmp256 = tmp253 * tmp244.y + tmp252 * phi_out235 + tmp255 * tmp231.y;
                float tmp257 = (tmp253 * tmp244.x + tmp252 * phi_out233 + tmp255 * tmp231.x) * 0.04;
                float tmp258 = tmp256 < 0.0 ? 0.0 : tmp256;
                float tmp259 = (tmp253 * tmp244.z + tmp252 * phi_out237 + tmp255 * tmp231.z) * 0.3999999;
                float3 tmp260 = float3(sqrt(tmp259 * tmp259 + tmp257 * tmp257 + tmp258 * tmp258), 0.0, 0.0);
                float3 tmp261 = float3(tmp257, tmp258, tmp259) / tmp260.xxx;
                if (tmp261.y == 0.0) {
                    sret_ptr.pdf = 0.0;
                    sret_ptr.event_type = 0;
                    phi_in262 = true;
                } else {
                    float tmp264 = tmp261.x * tmp211.x + tmp261.y * tmp197 + tmp261.z * tmp207;
                    float tmp265 = tmp261.x * tmp211.y + tmp261.y * tmp198 + tmp261.z * tmp208;
                    float tmp266 = tmp261.x * tmp211.z + tmp261.y * tmp199 + tmp261.z * tmp209;
                    float3 tmp267 = float3(tmp264, tmp265, tmp266) * tmp7;
                    float tmp268 = tmp267.x + tmp267.y + tmp267.z;
                    if (tmp268 > 0.0) {
                        float tmp269 = tmp268 * 2.0;
                        float tmp270 = tmp269 * tmp264 - tmp4;
                        float tmp271 = tmp269 * tmp265 - tmp5;
                        float tmp272 = tmp269 * tmp266 - tmp6;
                        sret_ptr.k2.x = tmp270;
                        sret_ptr.k2.y = tmp271;
                        sret_ptr.k2.z = tmp272;
                        sret_ptr.event_type = 10;
                        float3 tmp273 = float3(tmp270, tmp271, tmp272);
                        float3 tmp274 = tmp273 * tmp10;
                        if (tmp274.x + tmp274.y + tmp274.z > 0.0) {
                            sret_ptr.bsdf_over_pdf.x = 1.0;
                            sret_ptr.bsdf_over_pdf.y = 1.0;
                            sret_ptr.bsdf_over_pdf.z = 1.0;
                            float3 tmp275 = tmp273 * tmp200;
                            float tmp276 = tmp275.x + tmp275.y + tmp275.z;
                            float3 tmp277 = tmp273 * tmp211;
                            float tmp278 = 2.0 / (sqrt(tmp229 / tmp227 + 1.0) + 1.0);
                            float tmp279 = (tmp277.x + tmp277.y + tmp277.z) * 0.04;
                            float tmp280 = 2.0 / (sqrt((tmp279 * tmp279 + tmp228) / (tmp276 * tmp276) + 1.0) + 1.0);
                            if (tmp280 * tmp278 > 0.0) {
                                sret_ptr.bsdf_over_pdf.x = tmp280;
                                sret_ptr.bsdf_over_pdf.y = tmp280;
                                sret_ptr.bsdf_over_pdf.z = tmp280;
                                float tmp281 = tmp261.x * 25.0;
                                float tmp282 = tmp261.z * 2.5;
                                float tmp283 = tmp281 * tmp281 + tmp261.y * tmp261.y + tmp282 * tmp282;
                                sret_ptr.pdf = 0.25 / (tmp261.y * tmp220) * tmp278 * (tmp261.y * 19.89437 / (tmp283 * tmp283));
                                sret_ptr.handle = 0;
                                phi_in262 = false;
                            } else {
                                sret_ptr.pdf = 0.0;
                                sret_ptr.event_type = 0;
                                phi_in262 = true;
                            }
                        } else {
                            sret_ptr.pdf = 0.0;
                            sret_ptr.event_type = 0;
                            phi_in262 = true;
                        }
                    } else {
                        sret_ptr.pdf = 0.0;
                        sret_ptr.event_type = 0;
                        phi_in262 = true;
                    }
                }
                phi_out263 = phi_in262;
                float tmp284 = sret_ptr.bsdf_over_pdf.x * tmp190;
                sret_ptr.bsdf_over_pdf.x = tmp284;
                float tmp285 = sret_ptr.bsdf_over_pdf.y * tmp192;
                sret_ptr.bsdf_over_pdf.y = tmp285;
                float tmp286 = sret_ptr.bsdf_over_pdf.z * tmp194;
                sret_ptr.bsdf_over_pdf.z = tmp286;
                phi_in74 = 0;
                phi_in76 = float3(0.0, 0.0, 0.0);
                phi_in78 = 0.0;
                phi_in80 = 0.0;
                phi_in82 = 0.0;
                phi_in84 = 0.0;
                phi_in86 = 0.0;
                phi_in88 = 0.0;
                phi_in90 = false;
                phi_in92 = 0;
                phi_in94 = 0;
                phi_in96 = 0;
                if (!phi_out263) {
                    float tmp287 = sret_ptr.k2.x;
                    float tmp288 = sret_ptr.k2.y;
                    float tmp289 = sret_ptr.k2.z;
                    float3 tmp290 = float3(tmp287, tmp288, tmp289);
                    phi_in74 = 1;
                    phi_in76 = tmp290;
                    phi_in78 = tmp286;
                    phi_in80 = tmp285;
                    phi_in82 = tmp284;
                    phi_in84 = tmp289;
                    phi_in86 = tmp288;
                    phi_in88 = tmp287;
                    phi_in90 = false;
                    phi_in92 = phi_out213;
                    phi_in94 = phi_out217;
                    phi_in96 = phi_out215;
                }
            }
        } else {
            sret_ptr.xi.z = (tmp174 + -1.0) / 0.0;
            sret_ptr.pdf = 0.0;
            sret_ptr.event_type = 0;
            phi_in74 = 0;
            phi_in76 = float3(0.0, 0.0, 0.0);
            phi_in78 = 0.0;
            phi_in80 = 0.0;
            phi_in82 = 0.0;
            phi_in84 = 0.0;
            phi_in86 = 0.0;
            phi_in88 = 0.0;
            phi_in90 = false;
            phi_in92 = 0;
            phi_in94 = 0;
            phi_in96 = 0;
        }
    }
    phi_out75 = phi_in74;
    phi_out77 = phi_in76;
    phi_out79 = phi_in78;
    phi_out81 = phi_in80;
    phi_out83 = phi_in82;
    phi_out85 = phi_in84;
    phi_out87 = phi_in86;
    phi_out89 = phi_in88;
    phi_out91 = phi_in90;
    phi_out93 = phi_in92;
    phi_out95 = phi_in94;
    phi_out97 = phi_in96;
    if (phi_out75 != 0) {
        float3 tmp291 = phi_out77 * tmp43;
        float tmp292 = tmp291.x + tmp291.y + tmp291.z;
        float tmp293 = abs(tmp292);
        float tmp294 = phi_out89 + tmp4;
        float tmp295 = phi_out87 + tmp5;
        float tmp296 = phi_out85 + tmp6;
        float3 tmp297 = float3(sqrt(tmp295 * tmp295 + tmp294 * tmp294 + tmp296 * tmp296), 0.0, 0.0);
        float3 tmp298 = float3(tmp294, tmp295, tmp296) / tmp297.xxx * tmp7;
        float tmp299 = tmp298.x + tmp298.y + tmp298.z;
        float tmp300 = abs(tmp299);
        tmp0.ior1.x = asfloat(phi_out93);
        tmp0.ior1.y = asfloat(asint(sret_ptr.ior1.y));
        tmp0.ior1.z = asfloat(asint(sret_ptr.ior1.z));
        tmp0.ior2.x = asfloat(phi_out95);
        tmp0.ior2.y = asfloat(asint(sret_ptr.ior2.y));
        tmp0.ior2.z = asfloat(phi_out97);
        tmp0.k1.x = tmp4;
        tmp0.k1.y = tmp5;
        tmp0.k1.z = tmp6;
        tmp0.k2.x = phi_out89;
        tmp0.k2.y = phi_out87;
        tmp0.k2.z = phi_out85;
        if (phi_out91) {
            float tmp301 = 1.0 - (1.0 - tmp299 * tmp299) / tmp48;
            phi_in302 = 1.0;
            if (!(tmp301 < 0.0)) {
                float tmp304 = sqrt(tmp301);
                float tmp305 = tmp300 * tmp42;
                float tmp306 = tmp304 * tmp42;
                float tmp307 = (tmp304 - tmp305) / (tmp304 + tmp305);
                float tmp308 = (tmp300 - tmp306) / (tmp306 + tmp300);
                float tmp309 = (tmp308 * tmp308 + tmp307 * tmp307) * 0.5;
                float tmp310 = tmp309 > 0.0 ? tmp309 : 0.0;
                float tmp311 = tmp310 < 1.0 ? tmp310 : 1.0;
                phi_in302 = tmp311;
            }
            phi_out303 = phi_in302;
            float tmp312 = phi_out303 * (1.0 / phi_out52);
            sret_ptr.bsdf_over_pdf.x = tmp312 * phi_out83;
            sret_ptr.bsdf_over_pdf.y = phi_out81 * tmp312;
            sret_ptr.bsdf_over_pdf.z = phi_out79 * tmp312;
            gen_weighted_layer_pdf(tmp0, state);
            float tmp313 = tmp0.pdf;
            float tmp314 = tmp313 * (1.0 - phi_out52);
            phi_in315 = phi_out52;
            phi_in317 = tmp314;
        } else {
            phi_in319 = 1.0;
            if (!tmp50) {
                float tmp321 = sqrt(tmp49);
                float tmp322 = tmp42 * tmp47;
                float tmp323 = tmp321 * tmp42;
                float tmp324 = (tmp321 - tmp322) / (tmp321 + tmp322);
                float tmp325 = (tmp47 - tmp323) / (tmp323 + tmp47);
                float tmp326 = (tmp325 * tmp325 + tmp324 * tmp324) * 0.5;
                float tmp327 = tmp326 > 0.0 ? tmp326 : 0.0;
                float tmp328 = tmp327 < 1.0 ? tmp327 : 1.0;
                phi_in319 = tmp328;
            }
            phi_out320 = phi_in319;
            float tmp329 = 1.0 - (1.0 - tmp292 * tmp292) / tmp48;
            phi_in330 = 1.0;
            if (!(tmp329 < 0.0)) {
                float tmp332 = sqrt(tmp329);
                float tmp333 = tmp293 * tmp42;
                float tmp334 = tmp332 * tmp42;
                float tmp335 = (tmp332 - tmp333) / (tmp332 + tmp333);
                float tmp336 = (tmp293 - tmp334) / (tmp334 + tmp293);
                float tmp337 = (tmp336 * tmp336 + tmp335 * tmp335) * 0.5;
                float tmp338 = tmp337 > 0.0 ? tmp337 : 0.0;
                float tmp339 = tmp338 < 1.0 ? tmp338 : 1.0;
                phi_in330 = tmp339;
            }
            phi_out331 = phi_in330;
            float tmp340 = 1.0 - phi_out52;
            float tmp341 = (1.0 - (phi_out320 > phi_out331 ? phi_out320 : phi_out331)) * (1.0 / tmp340);
            sret_ptr.bsdf_over_pdf.x = tmp341 * phi_out83;
            sret_ptr.bsdf_over_pdf.y = tmp341 * phi_out81;
            sret_ptr.bsdf_over_pdf.z = phi_out79 * tmp341;
            gen_microfacet_ggx_smith_bsdf_pdf(tmp0, state, tmp1);
            float tmp342 = tmp0.pdf;
            float tmp343 = tmp342 * phi_out52;
            phi_in315 = tmp340;
            phi_in317 = tmp343;
        }
        phi_out316 = phi_in315;
        phi_out318 = phi_in317;
        sret_ptr.pdf = sret_ptr.pdf * phi_out316 + phi_out318;
    }
    return;
}

void mdl_bsdf_scattering_0_evaluate(inout Bsdf_evaluate_data sret_ptr, in Shading_state_material state)
{
    float phi_in;
    float phi_out;
    float phi_in16;
    float phi_out17;
    float phi_in18;
    float phi_out19;
    float phi_in20;
    float phi_out21;
    float phi_in25;
    float phi_out26;
    float phi_in27;
    float phi_out28;
    float phi_in51;
    float phi_out52;
    float phi_in53;
    float phi_out54;
    float phi_in55;
    float phi_out56;
    float phi_in73;
    float phi_out74;
    float phi_in85;
    float phi_out86;
    float phi_in95;
    float phi_out96;
    float phi_in100;
    float phi_out101;
    float phi_in121;
    float phi_out122;
    float phi_in123;
    float phi_out124;
    float phi_in125;
    float phi_out126;
    float phi_in127;
    float phi_out128;
    float phi_in132;
    float phi_out133;
    float phi_in134;
    float phi_out135;
    float phi_in165;
    float phi_out166;
    float3 phi_in182;
    float3 phi_out183;
    float3 phi_in184;
    float3 phi_out185;
    float phi_in202;
    float phi_out203;
    float phi_in204;
    float phi_out205;
    sret_ptr.bsdf_diffuse = (float3)0;
    sret_ptr.bsdf_glossy = (float3)0;
    float3 tmp0 = expr_lambda_12_(state);
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
    float tmp14 = sret_ptr.ior1.x;
    phi_in = tmp14;
    if (tmp14 == -1.0) {
        sret_ptr.ior1.x = 1.0;
        sret_ptr.ior1.y = 1.0;
        sret_ptr.ior1.z = 1.0;
        phi_in = 1.0;
    }
    phi_out = phi_in;
    float tmp15 = sret_ptr.ior2.x;
    if (tmp15 == -1.0) {
        sret_ptr.ior2.x = 1.0;
        sret_ptr.ior2.y = 1.0;
        sret_ptr.ior2.z = 1.0;
        phi_in16 = 1.0;
        phi_in18 = 1.0;
        phi_in20 = 1.0;
    } else {
        float tmp22 = sret_ptr.ior2.y;
        float tmp23 = sret_ptr.ior2.z;
        phi_in16 = tmp23;
        phi_in18 = tmp22;
        phi_in20 = tmp15;
    }
    phi_out17 = phi_in16;
    phi_out19 = phi_in18;
    phi_out21 = phi_in20;
    if (phi_out17 == 1.0 && (phi_out19 == 1.0 && phi_out21 == 1.0)) {
        float tmp24 = (sret_ptr.ior1.y + phi_out + sret_ptr.ior1.z) * 0.3333333;
        phi_in25 = tmp24;
        phi_in27 = 1.5;
    } else {
        float tmp29 = (phi_out19 + phi_out17 + phi_out21) * 0.3333333;
        phi_in25 = 1.5;
        phi_in27 = tmp29;
    }
    phi_out26 = phi_in25;
    phi_out28 = phi_in27;
    float tmp30 = phi_out28 - phi_out26;
    float tmp31 = abs(tmp30) < 0.0001 ? phi_out26 + asfloat((asint(tmp30) & -2147483648) | 953267991) : phi_out28;
    float tmp32 = tmp31 / phi_out26;
    float3 tmp33 = float3(tmp11, tmp12, tmp13);
    float3 tmp34 = tmp33 * tmp5;
    float tmp35 = tmp34.x + tmp34.y + tmp34.z;
    float tmp36 = tmp35 > 0.0 ? tmp35 : 0.0;
    float tmp37 = tmp36 < 1.0 ? tmp36 : 1.0;
    float tmp38 = sret_ptr.k2.x;
    float tmp39 = sret_ptr.k2.y;
    float tmp40 = sret_ptr.k2.z;
    float3 tmp41 = float3(tmp38, tmp39, tmp40);
    float3 tmp42 = tmp41 * tmp33;
    float tmp43 = abs(tmp42.x + tmp42.y + tmp42.z);
    float3 tmp44 = tmp41 * tmp8;
    bool tmp45 = tmp44.x + tmp44.y + tmp44.z < 0.0;
    bool tmp46 = phi_out26 < 0.0 || tmp45 != true;
    if (tmp45) {
        if (tmp46) {
            float tmp47 = tmp43 * 2.0;
            float tmp48 = tmp38 + tmp2 + tmp47 * tmp11;
            float tmp49 = tmp39 + tmp3 + tmp47 * tmp12;
            float tmp50 = tmp40 + tmp4 + tmp47 * tmp13;
            phi_in51 = tmp48;
            phi_in53 = tmp49;
            phi_in55 = tmp50;
        } else {
            float tmp57 = tmp31 * tmp38 + phi_out26 * tmp2;
            float tmp58 = tmp31 * tmp39 + phi_out26 * tmp3;
            float tmp59 = tmp31 * tmp40 + phi_out26 * tmp4;
            phi_in51 = tmp57;
            phi_in53 = tmp58;
            phi_in55 = tmp59;
            if (tmp31 > phi_out26) {
                float tmp60 = -tmp59;
                float tmp61 = -tmp58;
                float tmp62 = -tmp57;
                phi_in51 = tmp62;
                phi_in53 = tmp61;
                phi_in55 = tmp60;
            }
        }
    }
    else {
        float tmp63 = tmp38 + tmp2;
        float tmp64 = tmp39 + tmp3;
        float tmp65 = tmp40 + tmp4;
        phi_in51 = tmp63;
        phi_in53 = tmp64;
        phi_in55 = tmp65;
    }
    phi_out52 = phi_in51;
    phi_out54 = phi_in53;
    phi_out56 = phi_in55;
    float3 tmp66 = float3(sqrt(phi_out54 * phi_out54 + phi_out52 * phi_out52 + phi_out56 * phi_out56), 0.0, 0.0);
    float3 tmp67 = float3(phi_out52, phi_out54, phi_out56) / tmp66.xxx;
    float3 tmp68 = tmp67 * tmp5;
    float tmp69 = tmp68.x + tmp68.y + tmp68.z;
    float tmp70 = abs(tmp69);
    float tmp71 = tmp32 * tmp32;
    float tmp72 = 1.0 - (1.0 - tmp69 * tmp69) / tmp71;
    phi_in73 = 1.0;
    if (!(tmp72 < 0.0)) {
        float tmp75 = sqrt(tmp72);
        float tmp76 = tmp70 * tmp32;
        float tmp77 = tmp75 * tmp32;
        float tmp78 = (tmp75 - tmp76) / (tmp75 + tmp76);
        float tmp79 = (tmp70 - tmp77) / (tmp77 + tmp70);
        float tmp80 = (tmp79 * tmp79 + tmp78 * tmp78) * 0.5;
        float tmp81 = tmp80 > 0.0 ? tmp80 : 0.0;
        float tmp82 = tmp81 < 1.0 ? tmp81 : 1.0;
        phi_in73 = tmp82;
    }
    phi_out74 = phi_in73;
    float tmp83 = 1.0 - (1.0 - tmp37 * tmp37) / tmp71;
    bool tmp84 = tmp83 < 0.0;
    phi_in85 = 1.0;
    if (!tmp84) {
        float tmp87 = sqrt(tmp83);
        float tmp88 = tmp32 * tmp37;
        float tmp89 = tmp87 * tmp32;
        float tmp90 = (tmp87 - tmp88) / (tmp87 + tmp88);
        float tmp91 = (tmp37 - tmp89) / (tmp89 + tmp37);
        float tmp92 = (tmp91 * tmp91 + tmp90 * tmp90) * 0.5;
        float tmp93 = tmp92 > 0.0 ? tmp92 : 0.0;
        float tmp94 = tmp93 < 1.0 ? tmp93 : 1.0;
        phi_in85 = tmp94;
    }
    phi_out86 = phi_in85;
    phi_in95 = tmp43;
    if (!tmp46) {
        float3 tmp97 = tmp67 * tmp33;
        float tmp98 = (tmp97.x + tmp97.y + tmp97.z) * 2.0 * tmp70 - tmp37;
        phi_in95 = tmp98;
    }
    phi_out96 = phi_in95;
    float tmp99 = 1.0 - (1.0 - phi_out96 * phi_out96) / tmp71;
    phi_in100 = 1.0;
    if (!(tmp99 < 0.0)) {
        float tmp102 = sqrt(tmp99);
        float tmp103 = phi_out96 * tmp32;
        float tmp104 = tmp102 * tmp32;
        float tmp105 = (tmp102 - tmp103) / (tmp102 + tmp103);
        float tmp106 = (phi_out96 - tmp104) / (tmp104 + phi_out96);
        float tmp107 = (tmp106 * tmp106 + tmp105 * tmp105) * 0.5;
        float tmp108 = tmp107 > 0.0 ? tmp107 : 0.0;
        float tmp109 = tmp108 < 1.0 ? tmp108 : 1.0;
        phi_in100 = tmp109;
    }
    phi_out101 = phi_in100;
    float3 tmp110 = state.tangent_u[0];
    float3 tmp111 = tmp33 * tmp8;
    float tmp112 = asfloat((asint(tmp111.x + tmp111.y + tmp111.z) & -2147483648) | 1065353216);
    float3 tmp113 = float3(tmp11 * tmp112, tmp12 * tmp112, tmp13 * tmp112);
    float3 tmp114 = tmp110.yzx;
    float3 tmp115 = tmp113.zxy;
    float3 tmp116 = tmp110.zxy;
    float3 tmp117 = tmp113.yzx;
    float3 tmp118 = tmp114 * tmp115 - tmp116 * tmp117;
    float3 tmp119 = tmp118 * tmp118;
    float tmp120 = tmp119.x + tmp119.y + tmp119.z;
    if (tmp120 < 1e-08) {
        sret_ptr.pdf = 0.0;
        phi_in121 = phi_out21;
        phi_in123 = phi_out;
        phi_in125 = 0.0;
        phi_in127 = 0.0;
    } else {
        float tmp129 = 1.0 / sqrt(tmp120);
        float3 tmp130 = float3(tmp129 * tmp118.x, tmp129 * tmp118.y, tmp129 * tmp118.z);
        float3 tmp131 = tmp130.zxy * tmp117 - tmp130.yzx * tmp115;
        phi_in132 = phi_out;
        if (phi_out == -1.0) {
            sret_ptr.ior1.x = 1.0;
            sret_ptr.ior1.y = 1.0;
            sret_ptr.ior1.z = 1.0;
            phi_in132 = 1.0;
        }
        phi_out133 = phi_in132;
        phi_in134 = phi_out21;
        if (phi_out21 == -1.0) {
            sret_ptr.ior2.x = 1.0;
            sret_ptr.ior2.y = 1.0;
            sret_ptr.ior2.z = 1.0;
            phi_in134 = 1.0;
        }
        phi_out135 = phi_in134;
        if (tmp45) {
            sret_ptr.pdf = 0.0;
            phi_in121 = phi_out135;
            phi_in123 = phi_out133;
            phi_in125 = 0.0;
            phi_in127 = 0.0;
        } else {
            float3 tmp136 = tmp113 * tmp5;
            float tmp137 = tmp136.x + tmp136.y + tmp136.z;
            float3 tmp138 = tmp41 * tmp113;
            float tmp139 = tmp138.x + tmp138.y + tmp138.z;
            float tmp140 = tmp38 + tmp2;
            float tmp141 = tmp39 + tmp3;
            float tmp142 = tmp40 + tmp4;
            float3 tmp143 = float3(sqrt(tmp141 * tmp141 + tmp140 * tmp140 + tmp142 * tmp142), 0.0, 0.0);
            float3 tmp144 = float3(tmp140, tmp141, tmp142) / tmp143.xxx;
            float3 tmp145 = tmp144 * tmp113;
            float tmp146 = tmp145.x + tmp145.y + tmp145.z;
            float3 tmp147 = tmp144 * tmp5;
            float3 tmp148 = tmp144 * tmp41;
            if (tmp148.x + tmp148.y + tmp148.z < 0.0 || (tmp146 < 0.0 || tmp147.x + tmp147.y + tmp147.z < 0.0)) {
                sret_ptr.pdf = 0.0;
                phi_in121 = phi_out135;
                phi_in123 = phi_out133;
                phi_in125 = 0.0;
                phi_in127 = 0.0;
            } else {
                float3 tmp149 = tmp131 * tmp144;
                float3 tmp150 = tmp130 * tmp144;
                float tmp151 = (tmp149.x + tmp149.y + tmp149.z) * 1e+07;
                float tmp152 = (tmp150.x + tmp150.y + tmp150.z) * 1e+07;
                float tmp153 = tmp152 * tmp152 + tmp146 * tmp146 + tmp151 * tmp151;
                float3 tmp154 = tmp131 * tmp5;
                float3 tmp155 = tmp130 * tmp5;
                float3 tmp156 = tmp131 * tmp41;
                float3 tmp157 = tmp130 * tmp41;
                float tmp158 = (tmp154.x + tmp154.y + tmp154.z) * 1e-07;
                float tmp159 = (tmp155.x + tmp155.y + tmp155.z) * 1e-07;
                float tmp160 = (tmp156.x + tmp156.y + tmp156.z) * 1e-07;
                float tmp161 = (tmp157.x + tmp157.y + tmp157.z) * 1e-07;
                float tmp162 = tmp146 * 3.183099e+13 / (tmp153 * tmp153) * (0.25 / (tmp146 * abs(tmp137))) * (2.0 / (sqrt((tmp158 * tmp158 + tmp159 * tmp159) / (tmp137 * tmp137) + 1.0) + 1.0));
                float tmp163 = tmp162 * (2.0 / (sqrt((tmp160 * tmp160 + tmp161 * tmp161) / (tmp139 * tmp139) + 1.0) + 1.0));
                sret_ptr.pdf = tmp162;
                phi_in121 = phi_out135;
                phi_in123 = phi_out133;
                phi_in125 = tmp162;
                phi_in127 = tmp163;
            }
        }
    }
    phi_out122 = phi_in121;
    phi_out124 = phi_in123;
    phi_out126 = phi_in125;
    phi_out128 = phi_in127;
    float tmp164 = phi_out128 * phi_out74;
    sret_ptr.bsdf_glossy.x = tmp164;
    sret_ptr.bsdf_glossy.y = tmp164;
    sret_ptr.bsdf_glossy.z = tmp164;
    phi_in165 = 1.0;
    if (!tmp84) {
        float tmp167 = sqrt(tmp83);
        float tmp168 = tmp32 * tmp37;
        float tmp169 = tmp167 * tmp32;
        float tmp170 = (tmp167 - tmp168) / (tmp167 + tmp168);
        float tmp171 = (tmp37 - tmp169) / (tmp169 + tmp37);
        float tmp172 = (tmp171 * tmp171 + tmp170 * tmp170) * 0.5;
        float tmp173 = tmp172 > 0.0 ? tmp172 : 0.0;
        float tmp174 = tmp173 < 1.0 ? tmp173 : 1.0;
        phi_in165 = tmp174;
    }
    phi_out166 = phi_in165;
    float tmp175 = phi_out166 * phi_out126;
    float tmp176 = 1.0 - (phi_out86 > phi_out101 ? phi_out86 : phi_out101);
    float3 tmp177 = expr_lambda_11_(state);
    float3 tmp178 = state.text_coords[0];
    float2 tmp179 = float2(tmp178.x * 10.0, tmp178.y * 10.0);
    float3 tmp180 = tex_lookup_color_2d(1, tmp179, 1, 1, float2(0.0, 1.0), float2(0.0, 1.0), 0.0);
    float tmp181 = tmp180.x + tmp180.y + tmp180.z;
    phi_in182 = tmp116;
    phi_in184 = tmp114;
    if (tmp181 * 0.3333333 != 0.0) {
        float tmp186 = tmp181 * 2.094395;
        float3 tmp187 = state.normal;
        float3 tmp188 = float3(cos(tmp186), 0.0, 0.0);
        float3 tmp189 = float3(sin(tmp186), 0.0, 0.0);
        float3 tmp190 = tmp188.xxx * tmp110 - (tmp187.yzx * tmp116 - tmp187.zxy * tmp114) * tmp189.xxx;
        float3 tmp191 = tmp190.yzx;
        float3 tmp192 = tmp190.zxy;
        phi_in182 = tmp192;
        phi_in184 = tmp191;
    }
    phi_out183 = phi_in182;
    phi_out185 = phi_in184;
    float3 tmp193 = tex_lookup_color_2d(2, tmp179, 1, 1, float2(0.0, 1.0), float2(0.0, 1.0), 0.0) * float3(0.1, 0.1, 0.1);
    float3 tmp194 = tmp177 * tmp8;
    float tmp195 = asfloat((asint(tmp194.x + tmp194.y + tmp194.z) & -2147483648) | 1065353216);
    float3 tmp196 = float3(tmp177.x * tmp195, tmp177.y * tmp195, tmp177.z * tmp195);
    float3 tmp197 = tmp196.zxy;
    float3 tmp198 = tmp196.yzx;
    float3 tmp199 = phi_out185 * tmp197 - phi_out183 * tmp198;
    float3 tmp200 = tmp199 * tmp199;
    float tmp201 = tmp200.x + tmp200.y + tmp200.z;
    if (tmp201 < 1e-08) {
        sret_ptr.pdf = 0.0;
        phi_in202 = 0.0;
        phi_in204 = 0.0;
    } else {
        float tmp206 = 1.0 / sqrt(tmp201);
        float3 tmp207 = float3(tmp206 * tmp199.x, tmp206 * tmp199.y, tmp206 * tmp199.z);
        float3 tmp208 = tmp207.zxy * tmp198 - tmp207.yzx * tmp197;
        if (phi_out124 == -1.0) {
            sret_ptr.ior1.x = 1.0;
            sret_ptr.ior1.y = 1.0;
            sret_ptr.ior1.z = 1.0;
        }
        if (phi_out122 == -1.0) {
            sret_ptr.ior2.x = 1.0;
            sret_ptr.ior2.y = 1.0;
            sret_ptr.ior2.z = 1.0;
        }
        if (tmp45) {
            sret_ptr.pdf = 0.0;
            phi_in202 = 0.0;
            phi_in204 = 0.0;
        } else {
            float3 tmp209 = tmp196 * tmp5;
            float tmp210 = tmp209.x + tmp209.y + tmp209.z;
            float3 tmp211 = tmp196 * tmp41;
            float tmp212 = tmp211.x + tmp211.y + tmp211.z;
            float tmp213 = tmp38 + tmp2;
            float tmp214 = tmp39 + tmp3;
            float tmp215 = tmp40 + tmp4;
            float3 tmp216 = float3(sqrt(tmp214 * tmp214 + tmp213 * tmp213 + tmp215 * tmp215), 0.0, 0.0);
            float3 tmp217 = float3(tmp213, tmp214, tmp215) / tmp216.xxx;
            float3 tmp218 = tmp196 * tmp217;
            float tmp219 = tmp218.x + tmp218.y + tmp218.z;
            float3 tmp220 = tmp217 * tmp5;
            float3 tmp221 = tmp217 * tmp41;
            if (tmp221.x + tmp221.y + tmp221.z < 0.0 || (tmp220.x + tmp220.y + tmp220.z < 0.0 || tmp219 < 0.0)) {
                sret_ptr.pdf = 0.0;
                phi_in202 = 0.0;
                phi_in204 = 0.0;
            } else {
                float3 tmp222 = tmp208 * tmp217;
                float3 tmp223 = tmp207 * tmp217;
                float tmp224 = (tmp222.x + tmp222.y + tmp222.z) * 25.0;
                float tmp225 = (tmp223.x + tmp223.y + tmp223.z) * 2.5;
                float tmp226 = tmp225 * tmp225 + tmp219 * tmp219 + tmp224 * tmp224;
                float3 tmp227 = tmp208 * tmp5;
                float3 tmp228 = tmp207 * tmp5;
                float3 tmp229 = tmp208 * tmp41;
                float3 tmp230 = tmp207 * tmp41;
                float tmp231 = (tmp227.x + tmp227.y + tmp227.z) * 0.04;
                float tmp232 = (tmp228.x + tmp228.y + tmp228.z) * 0.3999999;
                float tmp233 = (tmp229.x + tmp229.y + tmp229.z) * 0.04;
                float tmp234 = (tmp230.x + tmp230.y + tmp230.z) * 0.3999999;
                float tmp235 = tmp219 * 19.89437 / (tmp226 * tmp226) * (0.25 / (abs(tmp210) * tmp219)) * (2.0 / (sqrt((tmp231 * tmp231 + tmp232 * tmp232) / (tmp210 * tmp210) + 1.0) + 1.0));
                float tmp236 = tmp235 * (2.0 / (sqrt((tmp233 * tmp233 + tmp234 * tmp234) / (tmp212 * tmp212) + 1.0) + 1.0));
                sret_ptr.pdf = tmp235;
                phi_in202 = tmp235;
                phi_in204 = tmp236;
            }
        }
    }
    phi_out203 = phi_in202;
    phi_out205 = phi_in204;
    float tmp237 = tmp193.x > 0.0 ? tmp193.x : 0.0;
    float tmp238 = tmp193.y > 0.0 ? tmp193.y : 0.0;
    float tmp239 = tmp193.z > 0.0 ? tmp193.z : 0.0;
    float tmp240 = phi_out205 * tmp176;
    sret_ptr.bsdf_glossy.x = tmp240 * (tmp237 < 1.0 ? tmp237 : 1.0) + tmp164;
    sret_ptr.bsdf_glossy.y = tmp240 * (tmp238 < 1.0 ? tmp238 : 1.0) + tmp164;
    sret_ptr.bsdf_glossy.z = tmp240 * (tmp239 < 1.0 ? tmp239 : 1.0) + tmp164;
    sret_ptr.pdf = phi_out203 * (1.0 - phi_out166) + tmp175;
    return;
}

void mdl_bsdf_scattering_0_pdf(inout Bsdf_pdf_data sret_ptr, in Shading_state_material state)
{
    float3 tmp0;
    float phi_in;
    float phi_out;
    float phi_in12;
    float phi_out13;
    float phi_in14;
    float phi_out15;
    float phi_in16;
    float phi_out17;
    float phi_in21;
    float phi_out22;
    float phi_in23;
    float phi_out24;
    float phi_in36;
    float phi_out37;
    float3 tmp1 = expr_lambda_12_(state);
    float3 tmp2 = state.geom_normal;
    float3 tmp3 = float3(sret_ptr.k1.x, sret_ptr.k1.y, sret_ptr.k1.z) * tmp2;
    float tmp4 = asfloat((asint(tmp3.x + tmp3.y + tmp3.z) & -2147483648) | 1065353216);
    float3 tmp5 = float3(tmp2.x * tmp4, tmp2.y * tmp4, tmp2.z * tmp4) * tmp1;
    float tmp6 = asfloat((asint(tmp5.x + tmp5.y + tmp5.z) & -2147483648) | 1065353216);
    float tmp7 = tmp1.x * tmp6;
    float tmp8 = tmp1.y * tmp6;
    float tmp9 = tmp1.z * tmp6;
    tmp0.x = tmp7;
    tmp0.y = tmp8;
    tmp0.z = tmp9;
    float tmp10 = sret_ptr.ior1.x;
    phi_in = tmp10;
    if (tmp10 == -1.0) {
        sret_ptr.ior1.x = 1.0;
        sret_ptr.ior1.y = 1.0;
        sret_ptr.ior1.z = 1.0;
        phi_in = 1.0;
    }
    phi_out = phi_in;
    float tmp11 = sret_ptr.ior2.x;
    if (tmp11 == -1.0) {
        sret_ptr.ior2.x = 1.0;
        sret_ptr.ior2.y = 1.0;
        sret_ptr.ior2.z = 1.0;
        phi_in12 = 1.0;
        phi_in14 = 1.0;
        phi_in16 = 1.0;
    } else {
        float tmp18 = sret_ptr.ior2.y;
        float tmp19 = sret_ptr.ior2.z;
        phi_in12 = tmp19;
        phi_in14 = tmp18;
        phi_in16 = tmp11;
    }
    phi_out13 = phi_in12;
    phi_out15 = phi_in14;
    phi_out17 = phi_in16;
    if (phi_out13 == 1.0 && (phi_out15 == 1.0 && phi_out17 == 1.0)) {
        float tmp20 = (sret_ptr.ior1.y + phi_out + sret_ptr.ior1.z) * 0.3333333;
        phi_in21 = tmp20;
        phi_in23 = 1.5;
    } else {
        float tmp25 = (phi_out15 + phi_out13 + phi_out17) * 0.3333333;
        phi_in21 = 1.5;
        phi_in23 = tmp25;
    }
    phi_out22 = phi_in21;
    phi_out24 = phi_in23;
    float tmp26 = phi_out24 - phi_out22;
    float tmp27 = (abs(tmp26) < 0.0001 ? phi_out22 + asfloat((asint(tmp26) & -2147483648) | 953267991) : phi_out24) / phi_out22;
    gen_microfacet_ggx_smith_bsdf_pdf(sret_ptr, state, tmp0);
    float tmp28 = sret_ptr.k1.x;
    float tmp29 = sret_ptr.k1.y;
    float tmp30 = sret_ptr.k1.z;
    float3 tmp31 = float3(tmp28, tmp29, tmp30) * float3(tmp7, tmp8, tmp9);
    float tmp32 = tmp31.x + tmp31.y + tmp31.z;
    float tmp33 = tmp32 > 0.0 ? tmp32 : 0.0;
    float tmp34 = tmp33 < 1.0 ? tmp33 : 1.0;
    float tmp35 = 1.0 - (1.0 - tmp34 * tmp34) / (tmp27 * tmp27);
    phi_in36 = 1.0;
    if (!(tmp35 < 0.0)) {
        float tmp38 = sqrt(tmp35);
        float tmp39 = tmp34 * tmp27;
        float tmp40 = tmp38 * tmp27;
        float tmp41 = (tmp38 - tmp39) / (tmp38 + tmp39);
        float tmp42 = (tmp34 - tmp40) / (tmp40 + tmp34);
        float tmp43 = (tmp42 * tmp42 + tmp41 * tmp41) * 0.5;
        float tmp44 = tmp43 > 0.0 ? tmp43 : 0.0;
        float tmp45 = tmp44 < 1.0 ? tmp44 : 1.0;
        phi_in36 = tmp45;
    }
    phi_out37 = phi_in36;
    float tmp46 = sret_ptr.pdf;
    gen_weighted_layer_pdf(sret_ptr, state);
    float tmp47 = sret_ptr.pdf;
    sret_ptr.pdf = tmp47 * (1.0 - phi_out37) + tmp46 * phi_out37;
    return;
}

void mdl_edf_emission_0_init(inout Shading_state_material state)
{
    return;
}

void mdl_edf_emission_0_sample(inout Edf_sample_data sret_ptr, in Shading_state_material state)
{
    sret_ptr.k1 = (float3)0;
    sret_ptr.pdf = float(0);
    sret_ptr.edf_over_pdf = (float3)0;
    sret_ptr.event_type = int(0);
    return;
}

void mdl_edf_emission_0_evaluate(inout Edf_evaluate_data sret_ptr, in Shading_state_material state)
{
    sret_ptr.edf = (float3)0;
    sret_ptr.pdf = float(0);
    return;
}

void mdl_edf_emission_0_pdf(inout Edf_pdf_data sret_ptr, in Shading_state_material state)
{
    sret_ptr.pdf = 0.0;
    return;
}

float3 mdl_edf_emission_intensity_0(in Shading_state_material state)
{
    return float3(0.0, 0.0, 0.0);
}
void mdl_bsdf_scattering_sample(in int idx, inout Bsdf_sample_data sInOut, in Shading_state_material sIn)
{
    switch(idx)
    {
    case 0: mdl_bsdf_scattering_0_sample(sInOut, sIn); return;
    }
}
void mdl_bsdf_scattering_init(in int idx, in Shading_state_material sIn)
{
    switch(idx)
    {
    case 0: mdl_bsdf_scattering_0_init(sIn); return;
    }
}
void mdl_edf_emission_evaluate(in int idx, inout Edf_evaluate_data sInOut, in Shading_state_material sIn)
{
    switch(idx)
    {
    case 0: mdl_edf_emission_0_evaluate(sInOut, sIn); return;
    }
}
void mdl_edf_emission_init(in int idx, in Shading_state_material sIn)
{
    switch(idx)
    {
    case 0: mdl_edf_emission_0_init(sIn); return;
    }
}
float3 mdl_edf_emission_intensity(in int idx, in Shading_state_material sIn)
{
    switch(idx)
    {
    case 0: return mdl_edf_emission_intensity_0(sIn);
    }
    return float3(0.0, 0.0, 0.0);
}




[shader("vertex")]
VertexStageOutput vertexMain(
    AssembledVertex assembledVertex)
{
    VertexStageOutput output;

    float3 position = assembledVertex.position;
    float3 color    = assembledVertex.color;

    output.coarseVertex.color = color;
    output.sv_position = mul(modelViewProjection, float4(position, 1.0));

    return output;
}

// Fragment Shader
[shader("fragment")]
float4 fragmentMain(
    CoarseVertex coarseVertex : CoarseVertex) : SV_Target
{
    float3 color = coarseVertex.color;

    return float4(color, 1.0);
}
