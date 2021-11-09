// shaders.slang

//
// This file provides a simple vertex and fragment shader that can be compiled
// using Slang. This code should also be valid as HLSL, and thus it does not
// use any of the new language features supported by Slang.
//
#include "mdl_types.hlsl"
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

  static const int glob_cnst21[256] = { 88, 147, 251, 140, 206, 77, 107, 169, 5, 222, 84, 97, 116, 234, 24, 90, 201, 133, 40, 216, 229, 34, 37, 158, 186, 49, 98, 61, 47, 9, 124, 123, 65, 249, 2, 66, 76, 172, 198, 179, 218, 210, 78, 170, 132, 32, 60, 110, 213, 223, 151, 225, 115, 83, 180, 161, 42, 41, 164, 250, 233, 70, 231, 27, 217, 244, 114, 96, 183, 228, 63, 195, 236, 192, 177, 209, 246, 109, 171, 72, 101, 23, 35, 112, 182, 162, 57, 69, 146, 81, 248, 215, 7, 154, 178, 252, 136, 55, 150, 8, 1, 142, 167, 199, 39, 3, 14, 135, 152, 74, 33, 243, 121, 13, 181, 26, 211, 19, 64, 168, 58, 67, 52, 143, 113, 43, 25, 240, 166, 59, 4, 187, 53, 238, 103, 159, 220, 204, 208, 245, 85, 122, 62, 120, 93, 191, 224, 16, 68, 80, 226, 207, 134, 188, 73, 232, 102, 125, 196, 254, 253, 130, 241, 46, 119, 38, 94, 221, 153, 100, 163, 175, 242, 131, 255, 214, 87, 139, 92, 12, 203, 117, 219, 21, 239, 6, 31, 20, 44, 50, 28, 111, 141, 18, 157, 145, 11, 30, 237, 82, 129, 200, 89, 148, 95, 194, 144, 128, 176, 45, 79, 106, 235, 75, 0, 230, 160, 126, 138, 227, 247, 91, 17, 173, 29, 51, 71, 22, 36, 118, 149, 189, 155, 156, 197, 54, 202, 99, 174, 137, 105, 184, 205, 108, 10, 193, 165, 127, 56, 212, 104, 190, 185, 48, 86, 15 };
  static const int glob_cnst23[256] = { 151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180 };
  static const int glob_cnst25[256] = { 120, 73, 105, 71, 106, 25, 159, 92, 184, 93, 179, 181, 51, 168, 252, 235, 114, 143, 108, 82, 4, 72, 9, 192, 214, 112, 12, 200, 188, 8, 187, 117, 157, 88, 70, 87, 56, 38, 115, 96, 59, 24, 215, 231, 123, 44, 144, 119, 243, 245, 212, 249, 197, 109, 76, 66, 183, 58, 232, 113, 86, 234, 203, 80, 163, 254, 140, 62, 174, 118, 167, 18, 55, 99, 126, 170, 52, 149, 156, 142, 89, 189, 178, 240, 255, 251, 169, 226, 236, 153, 223, 219, 54, 130, 133, 7, 173, 77, 242, 190, 1, 122, 27, 147, 196, 180, 238, 124, 145, 101, 195, 57, 61, 39, 53, 154, 45, 47, 107, 233, 20, 176, 83, 36, 136, 138, 15, 230, 246, 94, 69, 6, 135, 132, 29, 131, 172, 199, 228, 177, 182, 216, 134, 151, 95, 17, 218, 155, 19, 175, 41, 191, 85, 23, 160, 125, 248, 42, 209, 165, 110, 102, 79, 221, 32, 26, 217, 2, 46, 81, 74, 16, 63, 202, 220, 37, 75, 205, 13, 21, 68, 148, 40, 253, 241, 250, 141, 164, 35, 104, 49, 50, 224, 166, 247, 208, 162, 193, 150, 30, 14, 10, 60, 98, 207, 84, 11, 239, 100, 116, 152, 90, 225, 129, 128, 64, 28, 158, 186, 127, 97, 204, 206, 65, 5, 194, 91, 67, 198, 48, 111, 211, 227, 229, 237, 244, 43, 146, 139, 222, 137, 201, 22, 103, 210, 34, 213, 31, 0, 33, 185, 161, 78, 121, 171, 3 };
  static const int glob_cnst29[256] = { 39, 244, 56, 240, 73, 135, 254, 227, 196, 35, 166, 195, 65, 132, 173, 225, 177, 41, 158, 245, 22, 199, 214, 126, 138, 28, 200, 148, 121, 19, 37, 221, 161, 180, 220, 36, 84, 224, 10, 143, 185, 209, 249, 248, 120, 235, 198, 52, 3, 190, 125, 226, 20, 6, 168, 2, 170, 167, 94, 54, 201, 5, 139, 40, 30, 76, 179, 42, 49, 85, 11, 243, 4, 99, 100, 182, 97, 26, 186, 206, 90, 175, 203, 113, 144, 236, 130, 232, 7, 131, 192, 228, 109, 23, 21, 159, 183, 105, 251, 151, 50, 8, 104, 89, 253, 142, 222, 207, 27, 69, 95, 189, 68, 230, 234, 162, 114, 246, 133, 250, 202, 77, 112, 18, 218, 229, 124, 43, 155, 157, 71, 191, 154, 239, 153, 66, 174, 169, 98, 231, 75, 115, 17, 145, 61, 101, 123, 215, 128, 187, 127, 102, 110, 78, 252, 14, 108, 223, 62, 47, 147, 164, 13, 96, 88, 93, 51, 15, 129, 53, 79, 163, 212, 213, 48, 92, 24, 55, 64, 57, 29, 137, 194, 247, 45, 70, 210, 106, 184, 178, 219, 122, 156, 193, 181, 107, 255, 91, 12, 172, 59, 46, 81, 176, 217, 80, 150, 58, 32, 111, 160, 149, 31, 82, 197, 216, 117, 165, 134, 60, 0, 67, 211, 87, 16, 171, 241, 136, 103, 116, 140, 205, 237, 9, 72, 63, 204, 38, 238, 119, 118, 1, 233, 25, 141, 208, 242, 33, 146, 152, 83, 86, 34, 74, 188, 44 };
  static const int glob_cnst30[256] = { 249, 199, 162, 114, 17, 55, 64, 57, 29, 137, 194, 247, 45, 70, 210, 106, 184, 178, 219, 122, 156, 193, 214, 126, 138, 28, 148, 121, 19, 37, 135, 132, 173, 225, 221, 161, 180, 220, 36, 84, 224, 10, 185, 209, 238, 119, 89, 253, 165, 248, 120, 235, 198, 52, 3, 190, 125, 226, 20, 6, 168, 2, 170, 167, 94, 54, 201, 179, 42, 208, 242, 33, 146, 158, 245, 196, 166, 83, 86, 34, 74, 188, 44, 49, 85, 11, 243, 4, 99, 100, 102, 78, 252, 26, 35, 24, 113, 236, 237, 9, 1, 72, 63, 204, 13, 15, 129, 53, 79, 163, 212, 213, 48, 92, 97, 38, 189, 68, 230, 234, 41, 22, 246, 133, 250, 202, 77, 112, 18, 218, 229, 124, 181, 14, 108, 107, 255, 91, 145, 223, 134, 142, 96, 88, 222, 207, 141, 175, 203, 27, 69, 95, 62, 47, 147, 164, 130, 232, 39, 244, 21, 154, 239, 153, 110, 172, 59, 46, 81, 176, 217, 80, 150, 159, 182, 186, 66, 174, 169, 98, 231, 123, 215, 12, 128, 187, 127, 58, 32, 111, 160, 149, 31, 195, 65, 152, 144, 82, 197, 216, 75, 61, 101, 117, 93, 51, 60, 0, 67, 211, 241, 206, 90, 87, 56, 240, 73, 177, 43, 155, 157, 71, 191, 136, 103, 116, 140, 205, 143, 228, 109, 23, 16, 171, 115, 7, 131, 192, 183, 105, 251, 5, 139, 40, 200, 30, 254, 227, 76, 151, 50, 8, 104, 118, 233, 25 };

  void mdl_bsdf_scattering_0_init(inout Shading_state_material state)
  {
  return;
  }

  structtype0 expr_lambda_2_(in Shading_state_material state)
  {
  int phi_in;
  int phi_out;
  float3 phi_in14;
  float3 phi_out15;
  float3 phi_in16;
  float3 phi_out17;
  float3 phi_in29;
  float3 phi_out30;
  float3 phi_in31;
  float3 phi_out32;
  float phi_in44;
  float phi_out45;
  float3 phi_in46;
  float3 phi_out47;
  float3 tmp0 = state.position;
  float4 tmp1 = state.world_to_object[0];
  float4 tmp2 = state.world_to_object[1];
  float4 tmp3 = state.world_to_object[2];
  float3 tmp4 = float3(tmp1.y * tmp0.y + tmp1.w + tmp1.x * tmp0.x + tmp1.z * tmp0.z, tmp2.y * tmp0.y + tmp2.w + tmp2.x * tmp0.x + tmp2.z * tmp0.z, tmp3.y * tmp0.y + tmp3.w + tmp3.x * tmp0.x + tmp3.z * tmp0.z);
  float4 tmp5 = state.object_to_world[0];
  float4 tmp6 = state.object_to_world[1];
  float4 tmp7 = state.object_to_world[2];
  float3 tmp8 = float3(sqrt(tmp6.x * tmp6.x + tmp5.x * tmp5.x + tmp7.x * tmp7.x), 0.0, 0.0);
  float3 tmp9 = float3(tmp5.x, tmp6.x, tmp7.x) / tmp8.xxx;
  float3 tmp10 = float3(sqrt(tmp6.y * tmp6.y + tmp5.y * tmp5.y + tmp7.y * tmp7.y), 0.0, 0.0);
  float3 tmp11 = float3(tmp5.y, tmp6.y, tmp7.y) / tmp10.xxx;
  float3 tmp12 = state.normal;
  float3 tmp13 = tmp11 * tmp12;
  phi_in = 1;
  phi_in14 = float3(0.0, 0.0, 0.0);
  phi_in16 = float3(0.0, 0.0, 0.0);
  if (!(abs(tmp13.x + tmp13.y + tmp13.z) > 0.999)) {
  float3 tmp18 = tmp9 * tmp12;
  phi_in = 1;
  phi_in14 = float3(0.0, 0.0, 0.0);
  phi_in16 = float3(0.0, 0.0, 0.0);
  if (!(abs(tmp18.x + tmp18.y + tmp18.z) > 0.999)) {
  float3 tmp19 = tmp12.zxy;
  float3 tmp20 = tmp12.yzx;
  float3 tmp21 = tmp11.yzx * tmp19 - tmp11.zxy * tmp20;
  float3 tmp22 = float3(sqrt(tmp21.x * tmp21.x + tmp21.y * tmp21.y + tmp21.z * tmp21.z), 0.0, 0.0);
  float3 tmp23 = tmp21 / tmp22.xxx;
  float3 tmp24 = tmp23.zxy * tmp20 - tmp23.yzx * tmp19;
  float3 tmp25 = float3(sqrt(tmp24.x * tmp24.x + tmp24.y * tmp24.y + tmp24.z * tmp24.z), 0.0, 0.0);
  float3 tmp26 = tmp24 / tmp25.xxx;
  float3 tmp27 = tmp23 * tmp9;
  phi_in = 0;
  phi_in14 = tmp23;
  phi_in16 = tmp26;
  if (tmp27.x + tmp27.y + tmp27.z < 0.0) {
  float3 tmp28 = -tmp23;
  phi_in = 0;
  phi_in14 = tmp28;
  phi_in16 = tmp26;
  }
  }
  }
  phi_out = phi_in;
  phi_out15 = phi_in14;
  phi_out17 = phi_in16;
  phi_in29 = phi_out15;
  phi_in31 = phi_out17;
  if (phi_out != 0) {
  float tmp33 = tmp12.x * tmp1.x + tmp12.y * tmp1.y + tmp12.z * tmp1.z;
  float tmp34 = tmp12.x * tmp2.x + tmp12.y * tmp2.y + tmp12.z * tmp2.z;
  float tmp35 = tmp12.x * tmp3.x + tmp12.y * tmp3.y + tmp12.z * tmp3.z;
  float3 tmp36 = float3(tmp33, tmp34, tmp35);
  float tmp37 = tmp34 * tmp35;
  float tmp38 = -tmp37;
  float tmp39 = -tmp33;
  if (abs(tmp35) > 0.99999) {
  float tmp40 = tmp34 * tmp39;
  float tmp41 = 1.0 - tmp34 * tmp34;
  float3 tmp42 = float3(tmp40, tmp41, tmp38);
  float tmp43 = tmp37 * tmp37 + tmp40 * tmp40 + tmp41 * tmp41;
  phi_in44 = tmp43;
  phi_in46 = tmp42;
  } else {
  float tmp48 = tmp35 * tmp39;
  float tmp49 = 1.0 - tmp35 * tmp35;
  float3 tmp50 = float3(tmp48, tmp38, tmp49);
  float tmp51 = tmp48 * tmp48 + tmp37 * tmp37 + tmp49 * tmp49;
  phi_in44 = tmp51;
  phi_in46 = tmp50;
  }
  phi_out45 = phi_in44;
  phi_out47 = phi_in46;
  float3 tmp52 = float3(sqrt(phi_out45), 0.0, 0.0);
  float3 tmp53 = phi_out47 / tmp52.xxx;
  float3 tmp54 = tmp53.yzx * tmp36.zxy - tmp53.zxy * tmp36.yzx;
  float3 tmp55 = float3(sqrt(tmp54.x * tmp54.x + tmp54.y * tmp54.y + tmp54.z * tmp54.z), 0.0, 0.0);
  float3 tmp56 = tmp54 / tmp55.xxx;
  float tmp57 = tmp53.x * tmp5.x + tmp53.y * tmp5.y + tmp53.z * tmp5.z;
  float tmp58 = tmp53.x * tmp6.x + tmp53.y * tmp6.y + tmp53.z * tmp6.z;
  float tmp59 = tmp53.x * tmp7.x + tmp53.y * tmp7.y + tmp53.z * tmp7.z;
  float3 tmp60 = float3(sqrt(tmp57 * tmp57 + tmp58 * tmp58 + tmp59 * tmp59), 0.0, 0.0);
  float3 tmp61 = float3(tmp57, tmp58, tmp59) / tmp60.xxx;
  float tmp62 = tmp56.x * tmp5.x + tmp56.y * tmp5.y + tmp56.z * tmp5.z;
  float tmp63 = tmp56.x * tmp6.x + tmp56.y * tmp6.y + tmp56.z * tmp6.z;
  float tmp64 = tmp56.x * tmp7.x + tmp56.y * tmp7.y + tmp56.z * tmp7.z;
  float3 tmp65 = float3(sqrt(tmp62 * tmp62 + tmp63 * tmp63 + tmp64 * tmp64), 0.0, 0.0);
  float3 tmp66 = float3(tmp62, tmp63, tmp64) / tmp65.xxx;
  phi_in29 = tmp66;
  phi_in31 = tmp61;
  }
  phi_out30 = phi_in29;
  phi_out32 = phi_in31;
  return constr_structtype0(tmp4, phi_out30, phi_out32);
  }

  float3 expr_lambda_5_(in Shading_state_material state)
  {
  structtype0 tmp0 = expr_lambda_2_(state);
  float3 tmp1 = tmp0.m_0 * float3(125000.0, 125000.0, 125000.0);
  float tmp2 = floor(tmp1.z);
  float tmp3 = tmp1.z - tmp2;
  float tmp4 = tmp3 * tmp3;
  float tmp5 = tmp4 * 0.5;
  float tmp6 = tmp5 - tmp3 + 0.5;
  float tmp7 = floor(tmp1.y);
  float tmp8 = tmp1.y - tmp7;
  float tmp9 = tmp8 * tmp8;
  float tmp10 = tmp9 * 0.5;
  float tmp11 = floor(tmp1.x);
  float tmp12 = tmp1.x - tmp11;
  float tmp13 = tmp12 * 0.00390625;
  int tmp14 = int(tmp2);
  int tmp15 = tmp14 & 255;
  int tmp16 = int(tmp11);
  int tmp17 = (tmp16 + 2) & 255;
  int tmp18 = int(tmp7);
  int tmp19 = (tmp18 + 2) & 255;
  int tmp20 = tmp17 ^ tmp19;
  int tmp21 = glob_cnst21[tmp15];
  int tmp22 = glob_cnst23[tmp17];
  int tmp23 = glob_cnst25[tmp19];
  int tmp24 = tmp23 ^ tmp22;
  int tmp25 = tmp24 ^ tmp21;
  float tmp26 = float(glob_cnst29[uint(tmp25) < uint(256) ? tmp25 : 0] ^ glob_cnst30[tmp20 ^ tmp15]);
  float tmp27 = 0.00390625 - tmp13 - tmp13;
  int tmp28 = (tmp16 + 1) & 255;
  int tmp29 = tmp28 ^ tmp19;
  int tmp30 = glob_cnst23[tmp28];
  int tmp31 = tmp30 ^ tmp23;
  int tmp32 = tmp31 ^ tmp21;
  float tmp33 = float(glob_cnst29[uint(tmp32) < uint(256) ? tmp32 : 0] ^ glob_cnst30[tmp29 ^ tmp15]);
  float tmp34 = tmp13 + -0.00390625;
  int tmp35 = tmp16 & 255;
  int tmp36 = tmp35 ^ tmp19;
  int tmp37 = glob_cnst23[tmp35];
  int tmp38 = tmp37 ^ tmp23;
  int tmp39 = tmp38 ^ tmp21;
  float tmp40 = float(glob_cnst29[uint(tmp39) < uint(256) ? tmp39 : 0] ^ glob_cnst30[tmp36 ^ tmp15]);
  float tmp41 = tmp8 + 0.5 - tmp9;
  int tmp42 = (tmp18 + 1) & 255;
  int tmp43 = tmp17 ^ tmp42;
  int tmp44 = glob_cnst25[tmp42];
  int tmp45 = tmp44 ^ tmp22;
  int tmp46 = tmp45 ^ tmp21;
  float tmp47 = float(glob_cnst29[uint(tmp46) < uint(256) ? tmp46 : 0] ^ glob_cnst30[tmp43 ^ tmp15]);
  int tmp48 = tmp28 ^ tmp42;
  int tmp49 = tmp44 ^ tmp30;
  int tmp50 = tmp49 ^ tmp21;
  float tmp51 = float(glob_cnst29[uint(tmp50) < uint(256) ? tmp50 : 0] ^ glob_cnst30[tmp48 ^ tmp15]);
  int tmp52 = tmp35 ^ tmp42;
  int tmp53 = tmp44 ^ tmp37;
  int tmp54 = tmp53 ^ tmp21;
  float tmp55 = float(glob_cnst29[uint(tmp54) < uint(256) ? tmp54 : 0] ^ glob_cnst30[tmp52 ^ tmp15]);
  float tmp56 = tmp10 - tmp8 + 0.5;
  int tmp57 = tmp18 & 255;
  int tmp58 = tmp17 ^ tmp57;
  int tmp59 = glob_cnst25[tmp57];
  int tmp60 = tmp59 ^ tmp22;
  int tmp61 = tmp60 ^ tmp21;
  float tmp62 = float(glob_cnst29[uint(tmp61) < uint(256) ? tmp61 : 0] ^ glob_cnst30[tmp58 ^ tmp15]);
  int tmp63 = tmp28 ^ tmp57;
  int tmp64 = tmp59 ^ tmp30;
  int tmp65 = tmp64 ^ tmp21;
  float tmp66 = float(glob_cnst29[uint(tmp65) < uint(256) ? tmp65 : 0] ^ glob_cnst30[tmp63 ^ tmp15]);
  int tmp67 = tmp35 ^ tmp57;
  int tmp68 = tmp59 ^ tmp37;
  int tmp69 = tmp68 ^ tmp21;
  float tmp70 = float(glob_cnst29[uint(tmp69) < uint(256) ? tmp69 : 0] ^ glob_cnst30[tmp67 ^ tmp15]);
  float tmp71 = tmp3 + 0.5 - tmp4;
  int tmp72 = (tmp14 + 1) & 255;
  int tmp73 = glob_cnst21[tmp72];
  int tmp74 = tmp73 ^ tmp24;
  float tmp75 = float(glob_cnst29[uint(tmp74) < uint(256) ? tmp74 : 0] ^ glob_cnst30[tmp20 ^ tmp72]);
  int tmp76 = tmp73 ^ tmp31;
  float tmp77 = float(glob_cnst29[uint(tmp76) < uint(256) ? tmp76 : 0] ^ glob_cnst30[tmp29 ^ tmp72]);
  int tmp78 = tmp73 ^ tmp38;
  float tmp79 = float(glob_cnst29[uint(tmp78) < uint(256) ? tmp78 : 0] ^ glob_cnst30[tmp36 ^ tmp72]);
  int tmp80 = tmp73 ^ tmp45;
  float tmp81 = float(glob_cnst29[uint(tmp80) < uint(256) ? tmp80 : 0] ^ glob_cnst30[tmp43 ^ tmp72]);
  int tmp82 = tmp73 ^ tmp49;
  float tmp83 = float(glob_cnst29[uint(tmp82) < uint(256) ? tmp82 : 0] ^ glob_cnst30[tmp48 ^ tmp72]);
  int tmp84 = tmp73 ^ tmp53;
  float tmp85 = float(glob_cnst29[uint(tmp84) < uint(256) ? tmp84 : 0] ^ glob_cnst30[tmp52 ^ tmp72]);
  int tmp86 = tmp73 ^ tmp60;
  float tmp87 = float(glob_cnst29[uint(tmp86) < uint(256) ? tmp86 : 0] ^ glob_cnst30[tmp58 ^ tmp72]);
  int tmp88 = tmp73 ^ tmp64;
  float tmp89 = float(glob_cnst29[uint(tmp88) < uint(256) ? tmp88 : 0] ^ glob_cnst30[tmp63 ^ tmp72]);
  int tmp90 = tmp73 ^ tmp68;
  float tmp91 = float(glob_cnst29[uint(tmp90) < uint(256) ? tmp90 : 0] ^ glob_cnst30[tmp67 ^ tmp72]);
  int tmp92 = (tmp14 + 2) & 255;
  int tmp93 = glob_cnst21[tmp92];
  int tmp94 = tmp93 ^ tmp24;
  float tmp95 = float(glob_cnst29[uint(tmp94) < uint(256) ? tmp94 : 0] ^ glob_cnst30[tmp20 ^ tmp92]);
  int tmp96 = tmp93 ^ tmp31;
  float tmp97 = float(glob_cnst29[uint(tmp96) < uint(256) ? tmp96 : 0] ^ glob_cnst30[tmp29 ^ tmp92]);
  int tmp98 = tmp93 ^ tmp38;
  float tmp99 = float(glob_cnst29[uint(tmp98) < uint(256) ? tmp98 : 0] ^ glob_cnst30[tmp36 ^ tmp92]);
  int tmp100 = tmp93 ^ tmp45;
  float tmp101 = float(glob_cnst29[uint(tmp100) < uint(256) ? tmp100 : 0] ^ glob_cnst30[tmp43 ^ tmp92]);
  int tmp102 = tmp93 ^ tmp49;
  float tmp103 = float(glob_cnst29[uint(tmp102) < uint(256) ? tmp102 : 0] ^ glob_cnst30[tmp48 ^ tmp92]);
  int tmp104 = tmp93 ^ tmp53;
  float tmp105 = float(glob_cnst29[uint(tmp104) < uint(256) ? tmp104 : 0] ^ glob_cnst30[tmp52 ^ tmp92]);
  int tmp106 = tmp93 ^ tmp60;
  float tmp107 = float(glob_cnst29[uint(tmp106) < uint(256) ? tmp106 : 0] ^ glob_cnst30[tmp58 ^ tmp92]);
  int tmp108 = tmp93 ^ tmp64;
  float tmp109 = float(glob_cnst29[uint(tmp108) < uint(256) ? tmp108 : 0] ^ glob_cnst30[tmp63 ^ tmp92]);
  int tmp110 = tmp93 ^ tmp68;
  float tmp111 = float(glob_cnst29[uint(tmp110) < uint(256) ? tmp110 : 0] ^ glob_cnst30[tmp67 ^ tmp92]);
  float tmp112 = tmp13 * tmp12;
  float tmp113 = tmp112 * 0.5;
  float tmp114 = tmp13 + 0.001953125 - tmp112;
  float tmp115 = tmp113 - tmp13 + 0.001953125;
  float tmp116 = tmp114 * tmp33 + tmp113 * tmp26 + tmp115 * tmp40;
  float tmp117 = 1.0 - tmp8 - tmp8;
  float tmp118 = tmp114 * tmp51 + tmp113 * tmp47 + tmp115 * tmp55;
  float tmp119 = tmp8 + -1.0;
  float tmp120 = tmp114 * tmp66 + tmp113 * tmp62 + tmp115 * tmp70;
  float tmp121 = tmp114 * tmp77 + tmp113 * tmp75 + tmp115 * tmp79;
  float tmp122 = tmp114 * tmp83 + tmp113 * tmp81 + tmp115 * tmp85;
  float tmp123 = tmp114 * tmp89 + tmp113 * tmp87 + tmp115 * tmp91;
  float tmp124 = tmp114 * tmp97 + tmp113 * tmp95 + tmp115 * tmp99;
  float tmp125 = tmp114 * tmp103 + tmp113 * tmp101 + tmp115 * tmp105;
  float tmp126 = tmp114 * tmp109 + tmp113 * tmp107 + tmp115 * tmp111;
  float3 tmp127 = float3(((tmp27 * tmp83 + tmp13 * tmp81 + tmp34 * tmp85) * tmp41 + (tmp27 * tmp77 + tmp13 * tmp75 + tmp34 * tmp79) * tmp10 + (tmp27 * tmp89 + tmp13 * tmp87 + tmp34 * tmp91) * tmp56) * tmp71 + ((tmp27 * tmp51 + tmp13 * tmp47 + tmp34 * tmp55) * tmp41 + (tmp27 * tmp33 + tmp13 * tmp26 + tmp34 * tmp40) * tmp10 + (tmp27 * tmp66 + tmp13 * tmp62 + tmp34 * tmp70) * tmp56) * tmp6 + ((tmp27 * tmp103 + tmp13 * tmp101 + tmp34 * tmp105) * tmp41 + (tmp27 * tmp97 + tmp13 * tmp95 + tmp34 * tmp99) * tmp10 + (tmp27 * tmp109 + tmp13 * tmp107 + tmp34 * tmp111) * tmp56) * tmp5, (tmp122 * tmp117 + tmp121 * tmp8 + tmp123 * tmp119) * tmp71 + (tmp118 * tmp117 + tmp116 * tmp8 + tmp120 * tmp119) * tmp6 + (tmp125 * tmp117 + tmp124 * tmp8 + tmp126 * tmp119) * tmp5, (tmp122 * tmp41 + tmp121 * tmp10 + tmp123 * tmp56) * (tmp3 * -2.0 + 1.0) + (tmp118 * tmp41 + tmp116 * tmp10 + tmp120 * tmp56) * (tmp3 + -1.0) + (tmp125 * tmp41 + tmp124 * tmp10 + tmp126 * tmp56) * tmp3) * float3(2.0, 2.0, 2.0) + tmp1;
  int tmp128 = int(floor(tmp127.z));
  int tmp129 = tmp128 & 255;
  int tmp130 = int(floor(tmp127.x));
  int tmp131 = (tmp130 + 1) & 255;
  int tmp132 = int(floor(tmp127.y));
  int tmp133 = (tmp132 + 1) & 255;
  int tmp134 = tmp133 ^ tmp131;
  int tmp135 = glob_cnst23[tmp131];
  int tmp136 = glob_cnst25[tmp133];
  int tmp137 = tmp136 ^ tmp135;
  int tmp138 = glob_cnst21[tmp129];
  int tmp139 = tmp137 ^ tmp138;
  int tmp140 = glob_cnst29[uint(tmp139) < uint(256) ? tmp139 : 0] ^ glob_cnst30[tmp134 ^ tmp129];
  int tmp141 = (tmp130 + 255) & 255;
  int tmp142 = tmp133 ^ tmp141;
  int tmp143 = glob_cnst23[tmp141];
  int tmp144 = tmp143 ^ tmp136;
  int tmp145 = tmp144 ^ tmp138;
  int tmp146 = glob_cnst29[uint(tmp145) < uint(256) ? tmp145 : 0] ^ glob_cnst30[tmp142 ^ tmp129];
  int tmp147 = tmp130 & 255;
  int tmp148 = tmp133 ^ tmp147;
  int tmp149 = glob_cnst23[tmp147];
  int tmp150 = tmp149 ^ tmp136;
  int tmp151 = tmp150 ^ tmp138;
  int tmp152 = (tmp132 + 255) & 255;
  int tmp153 = tmp152 ^ tmp131;
  int tmp154 = glob_cnst25[tmp152];
  int tmp155 = tmp154 ^ tmp135;
  int tmp156 = tmp155 ^ tmp138;
  int tmp157 = glob_cnst29[uint(tmp156) < uint(256) ? tmp156 : 0] ^ glob_cnst30[tmp153 ^ tmp129];
  int tmp158 = tmp152 ^ tmp141;
  int tmp159 = tmp154 ^ tmp143;
  int tmp160 = tmp159 ^ tmp138;
  int tmp161 = glob_cnst29[uint(tmp160) < uint(256) ? tmp160 : 0] ^ glob_cnst30[tmp158 ^ tmp129];
  int tmp162 = tmp152 ^ tmp147;
  int tmp163 = tmp154 ^ tmp149;
  int tmp164 = tmp163 ^ tmp138;
  int tmp165 = (tmp128 + 1) & 255;
  int tmp166 = glob_cnst21[tmp165];
  int tmp167 = tmp166 ^ tmp137;
  int tmp168 = glob_cnst29[uint(tmp167) < uint(256) ? tmp167 : 0] ^ glob_cnst30[tmp134 ^ tmp165];
  int tmp169 = tmp166 ^ tmp144;
  int tmp170 = glob_cnst29[uint(tmp169) < uint(256) ? tmp169 : 0] ^ glob_cnst30[tmp142 ^ tmp165];
  int tmp171 = tmp166 ^ tmp150;
  int tmp172 = tmp170 + tmp168 + (glob_cnst29[uint(tmp171) < uint(256) ? tmp171 : 0] ^ glob_cnst30[tmp148 ^ tmp165]) * 6;
  int tmp173 = (tmp128 + 255) & 255;
  int tmp174 = glob_cnst21[tmp173];
  int tmp175 = tmp174 ^ tmp137;
  int tmp176 = glob_cnst29[uint(tmp175) < uint(256) ? tmp175 : 0] ^ glob_cnst30[tmp134 ^ tmp173];
  int tmp177 = tmp174 ^ tmp144;
  int tmp178 = glob_cnst29[uint(tmp177) < uint(256) ? tmp177 : 0] ^ glob_cnst30[tmp142 ^ tmp173];
  int tmp179 = tmp174 ^ tmp150;
  int tmp180 = tmp178 + tmp176 + (glob_cnst29[uint(tmp179) < uint(256) ? tmp179 : 0] ^ glob_cnst30[tmp148 ^ tmp173]) * 6;
  int tmp181 = tmp174 ^ tmp155;
  int tmp182 = glob_cnst29[uint(tmp181) < uint(256) ? tmp181 : 0] ^ glob_cnst30[tmp153 ^ tmp173];
  int tmp183 = tmp174 ^ tmp159;
  int tmp184 = glob_cnst29[uint(tmp183) < uint(256) ? tmp183 : 0] ^ glob_cnst30[tmp158 ^ tmp173];
  int tmp185 = tmp174 ^ tmp163;
  int tmp186 = tmp184 + tmp182 + (glob_cnst29[uint(tmp185) < uint(256) ? tmp185 : 0] ^ glob_cnst30[tmp162 ^ tmp173]) * 6;
  int tmp187 = tmp166 ^ tmp155;
  int tmp188 = glob_cnst29[uint(tmp187) < uint(256) ? tmp187 : 0] ^ glob_cnst30[tmp153 ^ tmp165];
  int tmp189 = tmp166 ^ tmp159;
  int tmp190 = glob_cnst29[uint(tmp189) < uint(256) ? tmp189 : 0] ^ glob_cnst30[tmp158 ^ tmp165];
  int tmp191 = tmp166 ^ tmp163;
  int tmp192 = tmp190 + tmp188 + (glob_cnst29[uint(tmp191) < uint(256) ? tmp191 : 0] ^ glob_cnst30[tmp162 ^ tmp165]) * 6;
  float3 tmp193 = float3(float(tmp180 + tmp172 - tmp186 - tmp192) * 3.051758e-05 + float(tmp146 + tmp140 - tmp157 - tmp161 + ((glob_cnst29[uint(tmp151) < uint(256) ? tmp151 : 0] ^ glob_cnst30[tmp148 ^ tmp129]) - (glob_cnst29[uint(tmp164) < uint(256) ? tmp164 : 0] ^ glob_cnst30[tmp162 ^ tmp129])) * 6) * 0.0001831055, 0.0, 0.0);
  int tmp194 = tmp132 & 255;
  int tmp195 = tmp131 ^ tmp194;
  int tmp196 = glob_cnst25[tmp194];
  int tmp197 = tmp196 ^ tmp135;
  int tmp198 = tmp197 ^ tmp138;
  int tmp199 = tmp141 ^ tmp194;
  int tmp200 = tmp196 ^ tmp143;
  int tmp201 = tmp200 ^ tmp138;
  int tmp202 = tmp197 ^ tmp166;
  int tmp203 = glob_cnst29[uint(tmp202) < uint(256) ? tmp202 : 0] ^ glob_cnst30[tmp195 ^ tmp165];
  int tmp204 = tmp197 ^ tmp174;
  int tmp205 = glob_cnst29[uint(tmp204) < uint(256) ? tmp204 : 0] ^ glob_cnst30[tmp195 ^ tmp173];
  int tmp206 = tmp200 ^ tmp174;
  int tmp207 = glob_cnst29[uint(tmp206) < uint(256) ? tmp206 : 0] ^ glob_cnst30[tmp199 ^ tmp173];
  int tmp208 = tmp200 ^ tmp166;
  int tmp209 = glob_cnst29[uint(tmp208) < uint(256) ? tmp208 : 0] ^ glob_cnst30[tmp199 ^ tmp165];
  float3 tmp210 = float3(float((glob_cnst29[uint(tmp198) < uint(256) ? tmp198 : 0] ^ glob_cnst30[tmp195 ^ tmp129]) - (glob_cnst29[uint(tmp201) < uint(256) ? tmp201 : 0] ^ glob_cnst30[tmp199 ^ tmp129])) * 0.001098633 + float(tmp168 - tmp170 + tmp176 - tmp178 + tmp182 - tmp184 + tmp188 - tmp190) * 3.051758e-05 + float(tmp140 - tmp146 + tmp157 - tmp161 + tmp203 + tmp205 - tmp207 - tmp209) * 0.0001831055, 0.0, 0.0);
  int tmp211 = tmp194 ^ tmp147;
  int tmp212 = tmp196 ^ tmp149;
  int tmp213 = tmp212 ^ tmp166;
  int tmp214 = tmp212 ^ tmp174;
  float3 tmp215 = float3(abs(((float(tmp209 + tmp203) - float(tmp207 + tmp205)) * 4.577637e-05 + (float(glob_cnst29[uint(tmp213) < uint(256) ? tmp213 : 0] ^ glob_cnst30[tmp211 ^ tmp165]) - float(glob_cnst29[uint(tmp214) < uint(256) ? tmp214 : 0] ^ glob_cnst30[tmp211 ^ tmp173])) * 0.0002746582 + (float(tmp192 + tmp172) - float(tmp186 + tmp180)) * 7.629395e-06) * 4.0) + 4.0, 0.0, 0.0);
  float3 tmp216 = tmp210.xxx * tmp0.m_1 + tmp193.xxx * tmp0.m_2 + tmp215.xxx * state.normal;
  float3 tmp217 = float3(sqrt(tmp216.x * tmp216.x + tmp216.y * tmp216.y + tmp216.z * tmp216.z), 0.0, 0.0);
  return tmp216 / tmp217.xxx;
  }

  float expr_lambda_3_(in Shading_state_material state)
  {
  float3 tmp0 = expr_lambda_2_(state).m_0 * float3(125000.0, 125000.0, 125000.0);
  float tmp1 = floor(tmp0.z);
  float tmp2 = tmp0.z - tmp1;
  float tmp3 = tmp2 * tmp2;
  float tmp4 = tmp3 * 0.5;
  float tmp5 = tmp4 - tmp2 + 0.5;
  float tmp6 = floor(tmp0.y);
  float tmp7 = tmp0.y - tmp6;
  float tmp8 = tmp7 * tmp7;
  float tmp9 = tmp8 * 0.5;
  float tmp10 = floor(tmp0.x);
  float tmp11 = tmp0.x - tmp10;
  float tmp12 = tmp11 * 0.00390625;
  int tmp13 = int(tmp1);
  int tmp14 = tmp13 & 255;
  int tmp15 = int(tmp10);
  int tmp16 = (tmp15 + 2) & 255;
  int tmp17 = int(tmp6);
  int tmp18 = (tmp17 + 2) & 255;
  int tmp19 = tmp16 ^ tmp18;
  int tmp20 = glob_cnst21[tmp14];
  int tmp22 = glob_cnst23[tmp16];
  int tmp24 = glob_cnst25[tmp18];
  int tmp26 = tmp24 ^ tmp22;
  int tmp27 = tmp26 ^ tmp20;
  float tmp28 = float(glob_cnst29[uint(tmp27) < uint(256) ? tmp27 : 0] ^ glob_cnst30[tmp19 ^ tmp14]);
  float tmp31 = 0.00390625 - tmp12 - tmp12;
  int tmp32 = (tmp15 + 1) & 255;
  int tmp33 = tmp32 ^ tmp18;
  int tmp34 = glob_cnst23[tmp32];
  int tmp35 = tmp34 ^ tmp24;
  int tmp36 = tmp35 ^ tmp20;
  float tmp37 = float(glob_cnst29[uint(tmp36) < uint(256) ? tmp36 : 0] ^ glob_cnst30[tmp33 ^ tmp14]);
  float tmp38 = tmp12 + -0.00390625;
  int tmp39 = tmp15 & 255;
  int tmp40 = tmp39 ^ tmp18;
  int tmp41 = glob_cnst23[tmp39];
  int tmp42 = tmp41 ^ tmp24;
  int tmp43 = tmp42 ^ tmp20;
  float tmp44 = float(glob_cnst29[uint(tmp43) < uint(256) ? tmp43 : 0] ^ glob_cnst30[tmp40 ^ tmp14]);
  float tmp45 = tmp7 + 0.5 - tmp8;
  int tmp46 = (tmp17 + 1) & 255;
  int tmp47 = tmp16 ^ tmp46;
  int tmp48 = glob_cnst25[tmp46];
  int tmp49 = tmp48 ^ tmp22;
  int tmp50 = tmp49 ^ tmp20;
  float tmp51 = float(glob_cnst29[uint(tmp50) < uint(256) ? tmp50 : 0] ^ glob_cnst30[tmp47 ^ tmp14]);
  int tmp52 = tmp32 ^ tmp46;
  int tmp53 = tmp48 ^ tmp34;
  int tmp54 = tmp53 ^ tmp20;
  float tmp55 = float(glob_cnst29[uint(tmp54) < uint(256) ? tmp54 : 0] ^ glob_cnst30[tmp52 ^ tmp14]);
  int tmp56 = tmp39 ^ tmp46;
  int tmp57 = tmp48 ^ tmp41;
  int tmp58 = tmp57 ^ tmp20;
  float tmp59 = float(glob_cnst29[uint(tmp58) < uint(256) ? tmp58 : 0] ^ glob_cnst30[tmp56 ^ tmp14]);
  float tmp60 = tmp9 - tmp7 + 0.5;
  int tmp61 = tmp17 & 255;
  int tmp62 = tmp16 ^ tmp61;
  int tmp63 = glob_cnst25[tmp61];
  int tmp64 = tmp63 ^ tmp22;
  int tmp65 = tmp64 ^ tmp20;
  float tmp66 = float(glob_cnst29[uint(tmp65) < uint(256) ? tmp65 : 0] ^ glob_cnst30[tmp62 ^ tmp14]);
  int tmp67 = tmp32 ^ tmp61;
  int tmp68 = tmp63 ^ tmp34;
  int tmp69 = tmp68 ^ tmp20;
  float tmp70 = float(glob_cnst29[uint(tmp69) < uint(256) ? tmp69 : 0] ^ glob_cnst30[tmp67 ^ tmp14]);
  int tmp71 = tmp39 ^ tmp61;
  int tmp72 = tmp63 ^ tmp41;
  int tmp73 = tmp72 ^ tmp20;
  float tmp74 = float(glob_cnst29[uint(tmp73) < uint(256) ? tmp73 : 0] ^ glob_cnst30[tmp71 ^ tmp14]);
  float tmp75 = tmp2 + 0.5 - tmp3;
  int tmp76 = (tmp13 + 1) & 255;
  int tmp77 = glob_cnst21[tmp76];
  int tmp78 = tmp77 ^ tmp26;
  float tmp79 = float(glob_cnst29[uint(tmp78) < uint(256) ? tmp78 : 0] ^ glob_cnst30[tmp19 ^ tmp76]);
  int tmp80 = tmp77 ^ tmp35;
  float tmp81 = float(glob_cnst29[uint(tmp80) < uint(256) ? tmp80 : 0] ^ glob_cnst30[tmp33 ^ tmp76]);
  int tmp82 = tmp77 ^ tmp42;
  float tmp83 = float(glob_cnst29[uint(tmp82) < uint(256) ? tmp82 : 0] ^ glob_cnst30[tmp40 ^ tmp76]);
  int tmp84 = tmp77 ^ tmp49;
  float tmp85 = float(glob_cnst29[uint(tmp84) < uint(256) ? tmp84 : 0] ^ glob_cnst30[tmp47 ^ tmp76]);
  int tmp86 = tmp77 ^ tmp53;
  float tmp87 = float(glob_cnst29[uint(tmp86) < uint(256) ? tmp86 : 0] ^ glob_cnst30[tmp52 ^ tmp76]);
  int tmp88 = tmp77 ^ tmp57;
  float tmp89 = float(glob_cnst29[uint(tmp88) < uint(256) ? tmp88 : 0] ^ glob_cnst30[tmp56 ^ tmp76]);
  int tmp90 = tmp77 ^ tmp64;
  float tmp91 = float(glob_cnst29[uint(tmp90) < uint(256) ? tmp90 : 0] ^ glob_cnst30[tmp62 ^ tmp76]);
  int tmp92 = tmp77 ^ tmp68;
  float tmp93 = float(glob_cnst29[uint(tmp92) < uint(256) ? tmp92 : 0] ^ glob_cnst30[tmp67 ^ tmp76]);
  int tmp94 = tmp77 ^ tmp72;
  float tmp95 = float(glob_cnst29[uint(tmp94) < uint(256) ? tmp94 : 0] ^ glob_cnst30[tmp71 ^ tmp76]);
  int tmp96 = (tmp13 + 2) & 255;
  int tmp97 = glob_cnst21[tmp96];
  int tmp98 = tmp97 ^ tmp26;
  float tmp99 = float(glob_cnst29[uint(tmp98) < uint(256) ? tmp98 : 0] ^ glob_cnst30[tmp19 ^ tmp96]);
  int tmp100 = tmp97 ^ tmp35;
  float tmp101 = float(glob_cnst29[uint(tmp100) < uint(256) ? tmp100 : 0] ^ glob_cnst30[tmp33 ^ tmp96]);
  int tmp102 = tmp97 ^ tmp42;
  float tmp103 = float(glob_cnst29[uint(tmp102) < uint(256) ? tmp102 : 0] ^ glob_cnst30[tmp40 ^ tmp96]);
  int tmp104 = tmp97 ^ tmp49;
  float tmp105 = float(glob_cnst29[uint(tmp104) < uint(256) ? tmp104 : 0] ^ glob_cnst30[tmp47 ^ tmp96]);
  int tmp106 = tmp97 ^ tmp53;
  float tmp107 = float(glob_cnst29[uint(tmp106) < uint(256) ? tmp106 : 0] ^ glob_cnst30[tmp52 ^ tmp96]);
  int tmp108 = tmp97 ^ tmp57;
  float tmp109 = float(glob_cnst29[uint(tmp108) < uint(256) ? tmp108 : 0] ^ glob_cnst30[tmp56 ^ tmp96]);
  int tmp110 = tmp97 ^ tmp64;
  float tmp111 = float(glob_cnst29[uint(tmp110) < uint(256) ? tmp110 : 0] ^ glob_cnst30[tmp62 ^ tmp96]);
  int tmp112 = tmp97 ^ tmp68;
  float tmp113 = float(glob_cnst29[uint(tmp112) < uint(256) ? tmp112 : 0] ^ glob_cnst30[tmp67 ^ tmp96]);
  int tmp114 = tmp97 ^ tmp72;
  float tmp115 = float(glob_cnst29[uint(tmp114) < uint(256) ? tmp114 : 0] ^ glob_cnst30[tmp71 ^ tmp96]);
  float tmp116 = tmp12 * tmp11;
  float tmp117 = tmp116 * 0.5;
  float tmp118 = tmp12 + 0.001953125 - tmp116;
  float tmp119 = tmp117 - tmp12 + 0.001953125;
  float tmp120 = tmp118 * tmp37 + tmp117 * tmp28 + tmp119 * tmp44;
  float tmp121 = 1.0 - tmp7 - tmp7;
  float tmp122 = tmp118 * tmp55 + tmp117 * tmp51 + tmp119 * tmp59;
  float tmp123 = tmp7 + -1.0;
  float tmp124 = tmp118 * tmp70 + tmp117 * tmp66 + tmp119 * tmp74;
  float tmp125 = tmp118 * tmp81 + tmp117 * tmp79 + tmp119 * tmp83;
  float tmp126 = tmp118 * tmp87 + tmp117 * tmp85 + tmp119 * tmp89;
  float tmp127 = tmp118 * tmp93 + tmp117 * tmp91 + tmp119 * tmp95;
  float tmp128 = tmp118 * tmp101 + tmp117 * tmp99 + tmp119 * tmp103;
  float tmp129 = tmp118 * tmp107 + tmp117 * tmp105 + tmp119 * tmp109;
  float tmp130 = tmp118 * tmp113 + tmp117 * tmp111 + tmp119 * tmp115;
  float3 tmp131 = float3(((tmp31 * tmp87 + tmp12 * tmp85 + tmp38 * tmp89) * tmp45 + (tmp31 * tmp81 + tmp12 * tmp79 + tmp38 * tmp83) * tmp9 + (tmp31 * tmp93 + tmp12 * tmp91 + tmp38 * tmp95) * tmp60) * tmp75 + ((tmp31 * tmp55 + tmp12 * tmp51 + tmp38 * tmp59) * tmp45 + (tmp31 * tmp37 + tmp12 * tmp28 + tmp38 * tmp44) * tmp9 + (tmp31 * tmp70 + tmp12 * tmp66 + tmp38 * tmp74) * tmp60) * tmp5 + ((tmp31 * tmp107 + tmp12 * tmp105 + tmp38 * tmp109) * tmp45 + (tmp31 * tmp101 + tmp12 * tmp99 + tmp38 * tmp103) * tmp9 + (tmp31 * tmp113 + tmp12 * tmp111 + tmp38 * tmp115) * tmp60) * tmp4, (tmp126 * tmp121 + tmp125 * tmp7 + tmp127 * tmp123) * tmp75 + (tmp122 * tmp121 + tmp120 * tmp7 + tmp124 * tmp123) * tmp5 + (tmp129 * tmp121 + tmp128 * tmp7 + tmp130 * tmp123) * tmp4, (tmp126 * tmp45 + tmp125 * tmp9 + tmp127 * tmp60) * (tmp2 * -2.0 + 1.0) + (tmp122 * tmp45 + tmp120 * tmp9 + tmp124 * tmp60) * (tmp2 + -1.0) + (tmp129 * tmp45 + tmp128 * tmp9 + tmp130 * tmp60) * tmp2) * float3(2.0, 2.0, 2.0) + tmp0;
  int tmp132 = int(floor(tmp131.z));
  int tmp133 = tmp132 & 255;
  int tmp134 = int(floor(tmp131.x));
  int tmp135 = tmp134 & 255;
  int tmp136 = int(floor(tmp131.y));
  int tmp137 = tmp136 & 255;
  int tmp138 = tmp137 ^ tmp135;
  int tmp139 = glob_cnst23[tmp135];
  int tmp140 = glob_cnst25[tmp137];
  int tmp141 = tmp140 ^ tmp139;
  int tmp142 = glob_cnst21[tmp133];
  int tmp143 = tmp141 ^ tmp142;
  int tmp144 = (tmp134 + 255) & 255;
  int tmp145 = tmp144 ^ tmp137;
  int tmp146 = glob_cnst23[tmp144];
  int tmp147 = tmp146 ^ tmp140;
  int tmp148 = tmp147 ^ tmp142;
  int tmp149 = (tmp134 + 1) & 255;
  int tmp150 = tmp149 ^ tmp137;
  int tmp151 = glob_cnst23[tmp149];
  int tmp152 = tmp151 ^ tmp140;
  int tmp153 = tmp152 ^ tmp142;
  int tmp154 = (tmp136 + 255) & 255;
  int tmp155 = tmp154 ^ tmp149;
  int tmp156 = glob_cnst25[tmp154];
  int tmp157 = tmp156 ^ tmp151;
  int tmp158 = tmp157 ^ tmp142;
  int tmp159 = tmp154 ^ tmp144;
  int tmp160 = tmp156 ^ tmp146;
  int tmp161 = tmp160 ^ tmp142;
  int tmp162 = tmp154 ^ tmp135;
  int tmp163 = tmp156 ^ tmp139;
  int tmp164 = tmp163 ^ tmp142;
  int tmp165 = (tmp136 + 1) & 255;
  int tmp166 = tmp165 ^ tmp149;
  int tmp167 = glob_cnst25[tmp165];
  int tmp168 = tmp167 ^ tmp151;
  int tmp169 = tmp168 ^ tmp142;
  int tmp170 = tmp165 ^ tmp144;
  int tmp171 = tmp167 ^ tmp146;
  int tmp172 = tmp171 ^ tmp142;
  int tmp173 = tmp165 ^ tmp135;
  int tmp174 = tmp167 ^ tmp139;
  int tmp175 = tmp174 ^ tmp142;
  int tmp176 = (tmp132 + 255) & 255;
  int tmp177 = glob_cnst21[tmp176];
  int tmp178 = tmp177 ^ tmp141;
  int tmp179 = tmp177 ^ tmp147;
  int tmp180 = tmp177 ^ tmp152;
  int tmp181 = tmp177 ^ tmp157;
  int tmp182 = tmp177 ^ tmp160;
  int tmp183 = tmp177 ^ tmp163;
  int tmp184 = tmp177 ^ tmp168;
  int tmp185 = tmp177 ^ tmp171;
  int tmp186 = tmp177 ^ tmp174;
  int tmp187 = (tmp132 + 1) & 255;
  int tmp188 = glob_cnst21[tmp187];
  int tmp189 = tmp188 ^ tmp141;
  int tmp190 = tmp188 ^ tmp147;
  int tmp191 = tmp188 ^ tmp152;
  int tmp192 = tmp188 ^ tmp157;
  int tmp193 = tmp188 ^ tmp160;
  int tmp194 = tmp188 ^ tmp163;
  int tmp195 = tmp188 ^ tmp168;
  int tmp196 = tmp188 ^ tmp171;
  int tmp197 = tmp188 ^ tmp174;
  return pow((float(glob_cnst29[uint(tmp178) < uint(256) ? tmp178 : 0] ^ glob_cnst30[tmp138 ^ tmp176]) + float((glob_cnst29[uint(tmp153) < uint(256) ? tmp153 : 0] ^ glob_cnst30[tmp150 ^ tmp133]) + (glob_cnst29[uint(tmp148) < uint(256) ? tmp148 : 0] ^ glob_cnst30[tmp145 ^ tmp133])) + float(glob_cnst29[uint(tmp189) < uint(256) ? tmp189 : 0] ^ glob_cnst30[tmp138 ^ tmp187])) * 0.0002746582 + float(glob_cnst29[uint(tmp143) < uint(256) ? tmp143 : 0] ^ glob_cnst30[tmp138 ^ tmp133]) * 0.001647949 + (float((glob_cnst29[uint(tmp180) < uint(256) ? tmp180 : 0] ^ glob_cnst30[tmp150 ^ tmp176]) + (glob_cnst29[uint(tmp179) < uint(256) ? tmp179 : 0] ^ glob_cnst30[tmp145 ^ tmp176])) + float((glob_cnst29[uint(tmp161) < uint(256) ? tmp161 : 0] ^ glob_cnst30[tmp159 ^ tmp133]) + (glob_cnst29[uint(tmp158) < uint(256) ? tmp158 : 0] ^ glob_cnst30[tmp155 ^ tmp133]) + (glob_cnst29[uint(tmp169) < uint(256) ? tmp169 : 0] ^ glob_cnst30[tmp166 ^ tmp133]) + (glob_cnst29[uint(tmp172) < uint(256) ? tmp172 : 0] ^ glob_cnst30[tmp170 ^ tmp133]) + ((glob_cnst29[uint(tmp175) < uint(256) ? tmp175 : 0] ^ glob_cnst30[tmp173 ^ tmp133]) + (glob_cnst29[uint(tmp164) < uint(256) ? tmp164 : 0] ^ glob_cnst30[tmp162 ^ tmp133])) * 6) + float((glob_cnst29[uint(tmp191) < uint(256) ? tmp191 : 0] ^ glob_cnst30[tmp150 ^ tmp187]) + (glob_cnst29[uint(tmp190) < uint(256) ? tmp190 : 0] ^ glob_cnst30[tmp145 ^ tmp187]))) * 4.577637e-05 + (float((glob_cnst29[uint(tmp193) < uint(256) ? tmp193 : 0] ^ glob_cnst30[tmp159 ^ tmp187]) + (glob_cnst29[uint(tmp192) < uint(256) ? tmp192 : 0] ^ glob_cnst30[tmp155 ^ tmp187]) + (glob_cnst29[uint(tmp195) < uint(256) ? tmp195 : 0] ^ glob_cnst30[tmp166 ^ tmp187]) + (glob_cnst29[uint(tmp196) < uint(256) ? tmp196 : 0] ^ glob_cnst30[tmp170 ^ tmp187]) + ((glob_cnst29[uint(tmp197) < uint(256) ? tmp197 : 0] ^ glob_cnst30[tmp173 ^ tmp187]) + (glob_cnst29[uint(tmp194) < uint(256) ? tmp194 : 0] ^ glob_cnst30[tmp162 ^ tmp187])) * 6) + float((glob_cnst29[uint(tmp182) < uint(256) ? tmp182 : 0] ^ glob_cnst30[tmp159 ^ tmp176]) + (glob_cnst29[uint(tmp181) < uint(256) ? tmp181 : 0] ^ glob_cnst30[tmp155 ^ tmp176]) + (glob_cnst29[uint(tmp184) < uint(256) ? tmp184 : 0] ^ glob_cnst30[tmp166 ^ tmp176]) + (glob_cnst29[uint(tmp185) < uint(256) ? tmp185 : 0] ^ glob_cnst30[tmp170 ^ tmp176]) + ((glob_cnst29[uint(tmp186) < uint(256) ? tmp186 : 0] ^ glob_cnst30[tmp173 ^ tmp176]) + (glob_cnst29[uint(tmp183) < uint(256) ? tmp183 : 0] ^ glob_cnst30[tmp162 ^ tmp176])) * 6)) * 7.629395e-06, 3.333333) * 0.8;
  }

  void gen_simple_glossy_bsdf_pdf(inout Bsdf_pdf_data p_00, in Shading_state_material p_11, in float3 p_22)
  {
  float phi_in;
  float phi_out;
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
  float tmp32 = abs(tmp31.x + tmp31.y + tmp31.z);
  float tmp33 = tmp26 + tmp5;
  float tmp34 = tmp27 + tmp6;
  float tmp35 = tmp28 + tmp7;
  float3 tmp36 = float3(sqrt(tmp34 * tmp34 + tmp33 * tmp33 + tmp35 * tmp35), 0.0, 0.0);
  float3 tmp37 = float3(tmp33, tmp34, tmp35) / tmp36.xxx;
  float3 tmp38 = tmp37 * tmp17;
  float tmp39 = tmp38.x + tmp38.y + tmp38.z;
  float3 tmp40 = tmp37 * tmp8;
  float tmp41 = tmp40.x + tmp40.y + tmp40.z;
  float3 tmp42 = tmp37 * tmp29;
  if (tmp42.x + tmp42.y + tmp42.z < 0.0 || (tmp39 < 0.0 || tmp41 < 0.0))
  p_00.pdf = 0.0;
  else {
  float3 tmp43 = tmp37 * tmp25;
  float tmp44 = tmp43.x + tmp43.y + tmp43.z;
  float3 tmp45 = tmp37 * tmp24;
  float tmp46 = tmp45.x + tmp45.y + tmp45.z;
  phi_in = 3.695931;
  if (!(tmp39 > 0.99999)) {
  float phitmp = pow(tmp39, (tmp46 * tmp46 + tmp44 * tmp44) * 22.22222 / (1.0 - tmp39 * tmp39)) * 3.695931;
  phi_in = phitmp;
  }
  phi_out = phi_in;
  float tmp47 = tmp39 * tmp32;
  float tmp48 = tmp47 * 2.0 / tmp41;
  p_00.pdf = (tmp48 > 1.0 ? 1.0 : tmp48) * (0.25 / tmp47) * phi_out;
  }
  }
  }
  return;
  }

  void mdl_bsdf_scattering_0_sample(inout Bsdf_sample_data sret_ptr, in Shading_state_material state)
  {
  float3 tmp0;
  Bsdf_pdf_data tmp1;
  int phi_in;
  int phi_out;
  float3 phi_in31;
  float3 phi_out32;
  float phi_in33;
  float phi_out34;
  float phi_in35;
  float phi_out36;
  float phi_in37;
  float phi_out38;
  float phi_in39;
  float phi_out40;
  float phi_in41;
  float phi_out42;
  float phi_in43;
  float phi_out44;
  float phi_in45;
  float phi_out46;
  float phi_in74;
  float phi_out75;
  float phi_in76;
  float phi_out77;
  float phi_in99;
  float phi_out100;
  float phi_in130;
  float phi_out131;
  float phi_in132;
  float phi_out133;
  float phi_in134;
  float phi_out135;
  float phi_in143;
  float phi_out144;
  float phi_in145;
  float phi_out146;
  float phi_in147;
  float phi_out148;
  int phi_in159;
  int phi_out160;
  int phi_in161;
  int phi_out162;
  float3 phi_in163;
  float3 phi_out164;
  float phi_in165;
  float phi_out166;
  float phi_in167;
  float phi_out168;
  float phi_in169;
  float phi_out170;
  float phi_in171;
  float phi_out172;
  float phi_in173;
  float phi_out174;
  float phi_in175;
  float phi_out176;
  float phi_in177;
  float phi_out178;
  float phi_in188;
  float phi_out189;
  float phi_in195;
  float phi_out196;
  float phi_in197;
  float phi_out198;
  float phi_in199;
  float phi_out200;
  float3 tmp2 = state.normal;
  float3 tmp3 = expr_lambda_5_(state);
  float tmp4 = expr_lambda_3_(state);
  tmp0.x = tmp3.x;
  tmp0.y = tmp3.y;
  tmp0.z = tmp3.z;
  float tmp5 = tmp4 > 0.0 ? tmp4 : 0.0;
  bool tmp6 = tmp5 < 1.0;
  float tmp7 = tmp6 ? tmp5 : 1.0;
  float tmp8 = sret_ptr.xi.z;
  bool tmp9 = tmp8 < tmp7;
  if (tmp9) {
  float tmp10 = tmp8 / tmp7;
  sret_ptr.xi.z = tmp10;
  float3 tmp11 = state.tangent_u[0];
  float3 tmp12 = state.geom_normal;
  float tmp13 = sret_ptr.k1.x;
  float tmp14 = sret_ptr.k1.y;
  float tmp15 = sret_ptr.k1.z;
  float3 tmp16 = float3(tmp13, tmp14, tmp15);
  float3 tmp17 = tmp16 * tmp12;
  float tmp18 = asfloat((asint(tmp17.x + tmp17.y + tmp17.z) & -2147483648) | 1065353216);
  float3 tmp19 = float3(tmp12.x * tmp18, tmp12.y * tmp18, tmp12.z * tmp18);
  float3 tmp20 = tmp19 * tmp3;
  float tmp21 = asfloat((asint(tmp20.x + tmp20.y + tmp20.z) & -2147483648) | 1065353216);
  float tmp22 = tmp3.x * tmp21;
  float tmp23 = tmp3.y * tmp21;
  float tmp24 = tmp3.z * tmp21;
  float3 tmp25 = float3(tmp22, tmp23, tmp24);
  float3 tmp26 = tmp25.zxy;
  float3 tmp27 = tmp25.yzx;
  float3 tmp28 = tmp26 * tmp11.yzx - tmp27 * tmp11.zxy;
  float3 tmp29 = tmp28 * tmp28;
  float tmp30 = tmp29.x + tmp29.y + tmp29.z;
  if (tmp30 < 1e-08) {
  sret_ptr.pdf = 0.0;
  sret_ptr.event_type = 0;
  phi_in = 0;
  phi_in31 = float3(0.0, 0.0, 0.0);
  phi_in33 = 0.0;
  phi_in35 = 0.0;
  phi_in37 = 0.0;
  phi_in39 = 0.0;
  phi_in41 = 0.0;
  phi_in43 = 0.0;
  phi_in45 = 0.0;
  } else {
  float tmp47 = 1.0 / sqrt(tmp30);
  float tmp48 = tmp47 * tmp28.x;
  float tmp49 = tmp47 * tmp28.y;
  float tmp50 = tmp47 * tmp28.z;
  float3 tmp51 = float3(tmp48, tmp49, tmp50);
  float3 tmp52 = tmp51.zxy * tmp27 - tmp51.yzx * tmp26;
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
  float3 tmp53 = tmp25 * tmp16;
  float tmp54 = abs(tmp53.x + tmp53.y + tmp53.z);
  float3 tmp55 = tmp52 * tmp16;
  float3 tmp56 = tmp51 * tmp16;
  float tmp57 = sret_ptr.xi.x;
  float tmp58 = cos(frac(tmp57 * 4.0) * 3.141593) * 23.22222;
  float tmp59 = pow(1.0 - sret_ptr.xi.y, -0.08612441) * 0.0215311 + -0.0215311;
  float tmp60 = sqrt((tmp58 + 23.22222) * tmp59);
  float tmp61 = sqrt((23.22222 - tmp58) * tmp59);
  float tmp62 = tmp57 >= 0.75 || tmp57 < 0.25 ? tmp60 : -tmp60;
  float tmp63 = tmp57 >= 0.5 ? -tmp61 : tmp61;
  float3 tmp64 = float3(sqrt(tmp62 * tmp62 + 1.0 + tmp63 * tmp63), 0.0, 0.0);
  float3 tmp65 = float3(tmp62, 1.0, tmp63) / tmp64.xxx;
  float tmp66 = tmp65.y * tmp54;
  float tmp67 = tmp65.z * (tmp56.x + tmp56.y + tmp56.z) + tmp65.x * (tmp55.x + tmp55.y + tmp55.z);
  float tmp68 = tmp67 + tmp66;
  float tmp69 = tmp66 - tmp67;
  float tmp70 = tmp69 > 0.0 ? tmp69 : 0.0;
  float tmp71 = tmp70 / (tmp70 + (tmp68 > 0.0 ? tmp68 : 0.0));
  if (tmp10 < tmp71) {
  sret_ptr.xi.z = tmp10 / tmp71;
  float tmp72 = -tmp65.x;
  float tmp73 = -tmp65.z;
  phi_in74 = tmp72;
  phi_in76 = tmp73;
  } else {
  sret_ptr.xi.z = (tmp10 - tmp71) / (1.0 - tmp71);
  phi_in74 = tmp65.x;
  phi_in76 = tmp65.z;
  }
  phi_out75 = phi_in74;
  phi_out77 = phi_in76;
  if (tmp65.y == 0.0) {
  sret_ptr.pdf = 0.0;
  sret_ptr.event_type = 0;
  phi_in = 0;
  phi_in31 = float3(0.0, 0.0, 0.0);
  phi_in33 = 0.0;
  phi_in35 = 0.0;
  phi_in37 = 0.0;
  phi_in39 = 0.0;
  phi_in41 = 0.0;
  phi_in43 = 0.0;
  phi_in45 = 0.0;
  } else {
  float tmp78 = phi_out75 * tmp52.x + tmp65.y * tmp22 + phi_out77 * tmp48;
  float tmp79 = phi_out75 * tmp52.y + tmp65.y * tmp23 + phi_out77 * tmp49;
  float tmp80 = phi_out75 * tmp52.z + tmp65.y * tmp24 + phi_out77 * tmp50;
  float3 tmp81 = float3(tmp78, tmp79, tmp80);
  float3 tmp82 = tmp81 * tmp16;
  float tmp83 = tmp82.x + tmp82.y + tmp82.z;
  if (tmp83 > 0.0) {
  float tmp84 = tmp83 * 2.0;
  float tmp85 = tmp84 * tmp78 - tmp13;
  float tmp86 = tmp84 * tmp79 - tmp14;
  float tmp87 = tmp84 * tmp80 - tmp15;
  sret_ptr.k2.x = tmp85;
  sret_ptr.k2.y = tmp86;
  sret_ptr.k2.z = tmp87;
  sret_ptr.event_type = 10;
  float3 tmp88 = float3(tmp85, tmp86, tmp87);
  float3 tmp89 = tmp88 * tmp19;
  if (tmp89.x + tmp89.y + tmp89.z > 0.0) {
  sret_ptr.bsdf_over_pdf.x = 1.0;
  sret_ptr.bsdf_over_pdf.y = 1.0;
  sret_ptr.bsdf_over_pdf.z = 1.0;
  float3 tmp90 = tmp88 * tmp25;
  float3 tmp91 = tmp88 * tmp81;
  float tmp92 = tmp65.y * 2.0;
  float tmp93 = tmp92 * tmp54 / tmp83;
  float tmp94 = tmp93 > 1.0 ? 1.0 : tmp93;
  float tmp95 = abs(tmp90.x + tmp90.y + tmp90.z) * tmp92 / abs(tmp91.x + tmp91.y + tmp91.z);
  float tmp96 = tmp95 > 1.0 ? 1.0 : tmp95;
  float tmp97 = tmp94 < tmp96 ? tmp94 : tmp96;
  if (tmp97 > 0.0) {
  float tmp98 = tmp97 / tmp94;
  sret_ptr.bsdf_over_pdf.x = tmp98;
  sret_ptr.bsdf_over_pdf.y = tmp98;
  sret_ptr.bsdf_over_pdf.z = tmp98;
  phi_in99 = 0.1591549;
  if (!(tmp65.y > 0.99999)) {
  float tmp101 = pow(tmp65.y, (phi_out77 * phi_out77 + phi_out75 * phi_out75) * 22.22222 / (1.0 - tmp65.y * tmp65.y)) * 0.1591549;
  phi_in99 = tmp101;
  }
  phi_out100 = phi_in99;
  float tmp102 = tmp94 * (5.805555 / tmp66) * phi_out100;
  sret_ptr.pdf = tmp102;
  sret_ptr.handle = 0;
  phi_in = 1;
  phi_in31 = tmp12;
  phi_in33 = tmp87;
  phi_in35 = tmp86;
  phi_in37 = tmp85;
  phi_in39 = tmp102;
  phi_in41 = tmp15;
  phi_in43 = tmp14;
  phi_in45 = tmp13;
  } else {
  sret_ptr.pdf = 0.0;
  sret_ptr.event_type = 0;
  phi_in = 0;
  phi_in31 = float3(0.0, 0.0, 0.0);
  phi_in33 = 0.0;
  phi_in35 = 0.0;
  phi_in37 = 0.0;
  phi_in39 = 0.0;
  phi_in41 = 0.0;
  phi_in43 = 0.0;
  phi_in45 = 0.0;
  }
  } else {
  sret_ptr.pdf = 0.0;
  sret_ptr.event_type = 0;
  phi_in = 0;
  phi_in31 = float3(0.0, 0.0, 0.0);
  phi_in33 = 0.0;
  phi_in35 = 0.0;
  phi_in37 = 0.0;
  phi_in39 = 0.0;
  phi_in41 = 0.0;
  phi_in43 = 0.0;
  phi_in45 = 0.0;
  }
  } else {
  sret_ptr.pdf = 0.0;
  sret_ptr.event_type = 0;
  phi_in = 0;
  phi_in31 = float3(0.0, 0.0, 0.0);
  phi_in33 = 0.0;
  phi_in35 = 0.0;
  phi_in37 = 0.0;
  phi_in39 = 0.0;
  phi_in41 = 0.0;
  phi_in43 = 0.0;
  phi_in45 = 0.0;
  }
  }
  }
  } else {
  sret_ptr.xi.z = (tmp8 - tmp7) / (1.0 - tmp7);
  float3 tmp103 = state.tangent_u[0];
  float3 tmp104 = state.geom_normal;
  float tmp105 = sret_ptr.k1.x;
  float tmp106 = sret_ptr.k1.y;
  float tmp107 = sret_ptr.k1.z;
  float3 tmp108 = float3(tmp105, tmp106, tmp107) * tmp104;
  float tmp109 = asfloat((asint(tmp108.x + tmp108.y + tmp108.z) & -2147483648) | 1065353216);
  float3 tmp110 = float3(tmp104.x * tmp109, tmp104.y * tmp109, tmp104.z * tmp109);
  float3 tmp111 = tmp110 * tmp2;
  float tmp112 = asfloat((asint(tmp111.x + tmp111.y + tmp111.z) & -2147483648) | 1065353216);
  float tmp113 = tmp2.x * tmp112;
  float tmp114 = tmp2.y * tmp112;
  float tmp115 = tmp2.z * tmp112;
  float3 tmp116 = float3(tmp113, tmp114, tmp115);
  float3 tmp117 = tmp116.zxy;
  float3 tmp118 = tmp116.yzx;
  float3 tmp119 = tmp117 * tmp103.yzx - tmp118 * tmp103.zxy;
  float3 tmp120 = tmp119 * tmp119;
  float tmp121 = tmp120.x + tmp120.y + tmp120.z;
  if (tmp121 < 1e-08) {
  sret_ptr.pdf = 0.0;
  sret_ptr.event_type = 0;
  phi_in = 0;
  phi_in31 = float3(0.0, 0.0, 0.0);
  phi_in33 = 0.0;
  phi_in35 = 0.0;
  phi_in37 = 0.0;
  phi_in39 = 0.0;
  phi_in41 = 0.0;
  phi_in43 = 0.0;
  phi_in45 = 0.0;
  } else {
  float tmp122 = 1.0 / sqrt(tmp121);
  float tmp123 = tmp122 * tmp119.x;
  float tmp124 = tmp122 * tmp119.y;
  float tmp125 = tmp122 * tmp119.z;
  float3 tmp126 = float3(tmp123, tmp124, tmp125);
  float3 tmp127 = tmp126.zxy * tmp118 - tmp126.yzx * tmp117;
  float tmp128 = sret_ptr.xi.x;
  float tmp129 = sret_ptr.xi.y;
  phi_in130 = 1.0;
  phi_in132 = 0.0;
  phi_in134 = 0.0;
  if (!(tmp128 == 0.0 && tmp129 == 0.0)) {
  float tmp136 = tmp128 * 2.0;
  float tmp137 = tmp129 * 2.0;
  float tmp138 = tmp136 < 1.0 ? tmp136 : tmp136 + -2.0;
  float tmp139 = tmp137 < 1.0 ? tmp137 : tmp137 + -2.0;
  float tmp140 = tmp138 * tmp138;
  float tmp141 = tmp139 * tmp139;
  if (tmp140 > tmp141) {
  float tmp142 = tmp139 / tmp138 * -0.7853982;
  phi_in143 = tmp138;
  phi_in145 = tmp142;
  phi_in147 = tmp140;
  } else {
  float tmp149 = tmp138 / tmp139 * 0.7853982 + -1.570796;
  phi_in143 = tmp139;
  phi_in145 = tmp149;
  phi_in147 = tmp141;
  }
  phi_out144 = phi_in143;
  phi_out146 = phi_in145;
  phi_out148 = phi_in147;
  float tmp150 = 1.0 - phi_out148;
  phi_in130 = 1.0;
  phi_in132 = 0.0;
  phi_in134 = 0.0;
  if (tmp150 > 0.0) {
  float tmp151 = sin(phi_out146) * phi_out144;
  float tmp152 = sqrt(tmp150);
  float tmp153 = cos(phi_out146) * phi_out144;
  phi_in130 = tmp152;
  phi_in132 = tmp151;
  phi_in134 = tmp153;
  }
  }
  phi_out131 = phi_in130;
  phi_out133 = phi_in132;
  phi_out135 = phi_in134;
  float tmp154 = phi_out133 * tmp127.x + phi_out131 * tmp113 + phi_out135 * tmp123;
  float tmp155 = phi_out133 * tmp127.y + phi_out131 * tmp114 + phi_out135 * tmp124;
  float tmp156 = phi_out133 * tmp127.z + phi_out131 * tmp115 + phi_out135 * tmp125;
  float3 tmp157 = float3(sqrt(tmp154 * tmp154 + tmp155 * tmp155 + tmp156 * tmp156), 0.0, 0.0);
  float3 tmp158 = float3(tmp154, tmp155, tmp156) / tmp157.xxx;
  sret_ptr.k2.x = tmp158.x;
  sret_ptr.k2.y = tmp158.y;
  sret_ptr.k2.z = tmp158.z;
  phi_in159 = 0;
  phi_in161 = 0;
  phi_in163 = float3(0.0, 0.0, 0.0);
  phi_in165 = 0.0;
  phi_in167 = 0.0;
  phi_in169 = 0.0;
  phi_in171 = 0.0;
  phi_in173 = 0.0;
  phi_in175 = 0.0;
  phi_in177 = 0.0;
  if (phi_out131 > 0.0) {
  float3 tmp179 = tmp158 * tmp110;
  phi_in159 = 0;
  phi_in161 = 0;
  phi_in163 = float3(0.0, 0.0, 0.0);
  phi_in165 = 0.0;
  phi_in167 = 0.0;
  phi_in169 = 0.0;
  phi_in171 = 0.0;
  phi_in173 = 0.0;
  phi_in175 = 0.0;
  phi_in177 = 0.0;
  if (tmp179.x + tmp179.y + tmp179.z > 0.0) {
  sret_ptr.bsdf_over_pdf.x = 0.016;
  sret_ptr.bsdf_over_pdf.y = 0.016;
  sret_ptr.bsdf_over_pdf.z = 0.016;
  float tmp180 = phi_out131 * 0.3183099;
  sret_ptr.pdf = tmp180;
  sret_ptr.event_type = 9;
  sret_ptr.handle = 0;
  phi_in159 = 1;
  phi_in161 = 1;
  phi_in163 = tmp104;
  phi_in165 = tmp158.z;
  phi_in167 = tmp158.y;
  phi_in169 = tmp158.x;
  phi_in171 = tmp180;
  phi_in173 = tmp107;
  phi_in175 = tmp106;
  phi_in177 = tmp105;
  }
  }
  phi_out160 = phi_in159;
  phi_out162 = phi_in161;
  phi_out164 = phi_in163;
  phi_out166 = phi_in165;
  phi_out168 = phi_in167;
  phi_out170 = phi_in169;
  phi_out172 = phi_in171;
  phi_out174 = phi_in173;
  phi_out176 = phi_in175;
  phi_out178 = phi_in177;
  phi_in = phi_out162;
  phi_in31 = phi_out164;
  phi_in33 = phi_out166;
  phi_in35 = phi_out168;
  phi_in37 = phi_out170;
  phi_in39 = phi_out172;
  phi_in41 = phi_out174;
  phi_in43 = phi_out176;
  phi_in45 = phi_out178;
  if (phi_out160 == 0) {
  sret_ptr.pdf = 0.0;
  sret_ptr.event_type = 0;
  phi_in = 0;
  phi_in31 = float3(0.0, 0.0, 0.0);
  phi_in33 = 0.0;
  phi_in35 = 0.0;
  phi_in37 = 0.0;
  phi_in39 = 0.0;
  phi_in41 = 0.0;
  phi_in43 = 0.0;
  phi_in45 = 0.0;
  }
  }
  }
  phi_out = phi_in;
  phi_out32 = phi_in31;
  phi_out34 = phi_in33;
  phi_out36 = phi_in35;
  phi_out38 = phi_in37;
  phi_out40 = phi_in39;
  phi_out42 = phi_in41;
  phi_out44 = phi_in43;
  phi_out46 = phi_in45;
  if (phi_out != 0) {
  if (tmp9 ? tmp6 : tmp7 > 0.0) {
  tmp1.ior1.x = asfloat(asint(sret_ptr.ior1.x));
  tmp1.ior1.y = asfloat(asint(sret_ptr.ior1.y));
  tmp1.ior1.z = asfloat(asint(sret_ptr.ior1.z));
  tmp1.ior2.x = asfloat(asint(sret_ptr.ior2.x));
  tmp1.ior2.y = asfloat(asint(sret_ptr.ior2.y));
  tmp1.ior2.z = asfloat(asint(sret_ptr.ior2.z));
  tmp1.k1.x = phi_out46;
  tmp1.k1.y = phi_out44;
  tmp1.k1.z = phi_out42;
  tmp1.k2.x = phi_out38;
  tmp1.k2.y = phi_out36;
  tmp1.k2.z = phi_out34;
  if (tmp9) {
  float3 tmp181 = phi_out32 * float3(phi_out46, phi_out44, phi_out42);
  float tmp182 = asfloat((asint(tmp181.x + tmp181.y + tmp181.z) & -2147483648) | 1065353216);
  float3 tmp183 = float3(phi_out32.x * tmp182, phi_out32.y * tmp182, phi_out32.z * tmp182);
  float3 tmp184 = tmp183 * tmp2;
  float tmp185 = asfloat((asint(tmp184.x + tmp184.y + tmp184.z) & -2147483648) | 1065353216);
  float3 tmp186 = float3(phi_out38, phi_out36, phi_out34);
  float3 tmp187 = tmp183 * tmp186;
  phi_in188 = 0.0;
  if (tmp187.x + tmp187.y + tmp187.z > 0.0) {
  float3 tmp190 = float3(tmp2.x * tmp185, tmp2.y * tmp185, tmp2.z * tmp185) * tmp186;
  float tmp191 = tmp190.x + tmp190.y + tmp190.z;
  float tmp192 = (tmp191 > 0.0 ? tmp191 : 0.0) * 0.3183099;
  phi_in188 = tmp192;
  }
  phi_out189 = phi_in188;
  tmp1.pdf = phi_out189;
  float tmp193 = 1.0 - tmp7;
  float tmp194 = phi_out40 * tmp7;
  phi_in195 = phi_out189;
  phi_in197 = tmp193;
  phi_in199 = tmp194;
  } else {
  gen_simple_glossy_bsdf_pdf(tmp1, state, tmp0);
  float tmp201 = tmp1.pdf;
  float tmp202 = sret_ptr.pdf * (1.0 - tmp7);
  phi_in195 = tmp201;
  phi_in197 = tmp7;
  phi_in199 = tmp202;
  }
  phi_out196 = phi_in195;
  phi_out198 = phi_in197;
  phi_out200 = phi_in199;
  sret_ptr.pdf = phi_out200 + phi_out198 * phi_out196;
  }
  }
  return;
  }

  void mdl_bsdf_scattering_0_evaluate(inout Bsdf_evaluate_data sret_ptr, in Shading_state_material state)
  {
  float phi_in;
  float phi_out;
  float phi_in23;
  float phi_out24;
  float phi_in25;
  float phi_out26;
  float phi_in54;
  float phi_out55;
  float phi_in76;
  float phi_out77;
  float phi_in84;
  float phi_out85;
  float3 tmp0 = state.normal;
  sret_ptr.bsdf_diffuse = (float3)0;
  sret_ptr.bsdf_glossy = (float3)0;
  float3 tmp1 = expr_lambda_5_(state);
  float tmp2 = expr_lambda_3_(state);
  float tmp3 = tmp2 > 0.0 ? tmp2 : 0.0;
  bool tmp4 = tmp3 < 1.0;
  float tmp5 = tmp4 ? tmp3 : 1.0;
  phi_in = 0.0;
  if (tmp5 > 0.0) {
  float3 tmp6 = state.tangent_u[0];
  float3 tmp7 = state.geom_normal;
  float tmp8 = sret_ptr.k1.x;
  float tmp9 = sret_ptr.k1.y;
  float tmp10 = sret_ptr.k1.z;
  float3 tmp11 = float3(tmp8, tmp9, tmp10);
  float3 tmp12 = tmp11 * tmp7;
  float tmp13 = asfloat((asint(tmp12.x + tmp12.y + tmp12.z) & -2147483648) | 1065353216);
  float3 tmp14 = float3(tmp7.x * tmp13, tmp7.y * tmp13, tmp7.z * tmp13);
  float3 tmp15 = tmp14 * tmp1;
  float tmp16 = asfloat((asint(tmp15.x + tmp15.y + tmp15.z) & -2147483648) | 1065353216);
  float3 tmp17 = float3(tmp1.x * tmp16, tmp1.y * tmp16, tmp1.z * tmp16);
  float3 tmp18 = tmp17.zxy;
  float3 tmp19 = tmp17.yzx;
  float3 tmp20 = tmp18 * tmp6.yzx - tmp19 * tmp6.zxy;
  float3 tmp21 = tmp20 * tmp20;
  float tmp22 = tmp21.x + tmp21.y + tmp21.z;
  if (tmp22 < 1e-08) {
  sret_ptr.pdf = 0.0;
  phi_in23 = 0.0;
  phi_in25 = 0.0;
  } else {
  float tmp27 = 1.0 / sqrt(tmp22);
  float3 tmp28 = float3(tmp27 * tmp20.x, tmp27 * tmp20.y, tmp27 * tmp20.z);
  float3 tmp29 = tmp28.zxy * tmp19 - tmp28.yzx * tmp18;
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
  float tmp30 = sret_ptr.k2.x;
  float tmp31 = sret_ptr.k2.y;
  float tmp32 = sret_ptr.k2.z;
  float3 tmp33 = float3(tmp30, tmp31, tmp32);
  float3 tmp34 = tmp33 * tmp14;
  if (tmp34.x + tmp34.y + tmp34.z < 0.0) {
  sret_ptr.pdf = 0.0;
  phi_in23 = 0.0;
  phi_in25 = 0.0;
  } else {
  float3 tmp35 = tmp17 * tmp11;
  float tmp36 = abs(tmp35.x + tmp35.y + tmp35.z);
  float3 tmp37 = tmp33 * tmp17;
  float tmp38 = abs(tmp37.x + tmp37.y + tmp37.z);
  float tmp39 = tmp30 + tmp8;
  float tmp40 = tmp31 + tmp9;
  float tmp41 = tmp32 + tmp10;
  float3 tmp42 = float3(sqrt(tmp40 * tmp40 + tmp39 * tmp39 + tmp41 * tmp41), 0.0, 0.0);
  float3 tmp43 = float3(tmp39, tmp40, tmp41) / tmp42.xxx;
  float3 tmp44 = tmp43 * tmp17;
  float tmp45 = tmp44.x + tmp44.y + tmp44.z;
  float3 tmp46 = tmp43 * tmp11;
  float tmp47 = tmp46.x + tmp46.y + tmp46.z;
  float3 tmp48 = tmp43 * tmp33;
  float tmp49 = tmp48.x + tmp48.y + tmp48.z;
  if (tmp49 < 0.0 || (tmp45 < 0.0 || tmp47 < 0.0)) {
  sret_ptr.pdf = 0.0;
  phi_in23 = 0.0;
  phi_in25 = 0.0;
  } else {
  float3 tmp50 = tmp43 * tmp29;
  float tmp51 = tmp50.x + tmp50.y + tmp50.z;
  float3 tmp52 = tmp43 * tmp28;
  float tmp53 = tmp52.x + tmp52.y + tmp52.z;
  phi_in54 = 3.695931;
  if (!(tmp45 > 0.99999)) {
  float tmp56 = pow(tmp45, (tmp53 * tmp53 + tmp51 * tmp51) * 22.22222 / (1.0 - tmp45 * tmp45)) * 3.695931;
  phi_in54 = tmp56;
  }
  phi_out55 = phi_in54;
  float tmp57 = tmp45 * 2.0;
  float tmp58 = tmp57 * tmp36 / tmp47;
  float tmp59 = tmp58 > 1.0 ? 1.0 : tmp58;
  float tmp60 = tmp57 * tmp38 / tmp49;
  float tmp61 = tmp60 > 1.0 ? 1.0 : tmp60;
  float tmp62 = phi_out55 * (0.25 / (tmp45 * tmp36));
  float tmp63 = tmp62 * (tmp59 < tmp61 ? tmp59 : tmp61);
  float tmp64 = tmp62 * tmp59;
  sret_ptr.pdf = tmp64;
  phi_in23 = tmp64;
  phi_in25 = tmp63;
  }
  }
  }
  phi_out24 = phi_in23;
  phi_out26 = phi_in25;
  float tmp65 = phi_out26 * tmp5;
  sret_ptr.bsdf_glossy.x = tmp65;
  sret_ptr.bsdf_glossy.y = tmp65;
  sret_ptr.bsdf_glossy.z = tmp65;
  float tmp66 = phi_out24 * tmp5;
  phi_in = tmp66;
  }
  phi_out = phi_in;
  if (tmp4) {
  float tmp67 = 1.0 - tmp5;
  float3 tmp68 = state.geom_normal;
  float3 tmp69 = float3(sret_ptr.k1.x, sret_ptr.k1.y, sret_ptr.k1.z) * tmp68;
  float tmp70 = asfloat((asint(tmp69.x + tmp69.y + tmp69.z) & -2147483648) | 1065353216);
  float3 tmp71 = float3(tmp68.x * tmp70, tmp68.y * tmp70, tmp68.z * tmp70);
  float3 tmp72 = tmp71 * tmp0;
  float tmp73 = asfloat((asint(tmp72.x + tmp72.y + tmp72.z) & -2147483648) | 1065353216);
  float3 tmp74 = float3(sret_ptr.k2.x, sret_ptr.k2.y, sret_ptr.k2.z);
  float3 tmp75 = tmp71 * tmp74;
  phi_in76 = 0.0;
  if (tmp75.x + tmp75.y + tmp75.z > 0.0) {
  float3 tmp78 = float3(tmp0.x * tmp73, tmp0.y * tmp73, tmp0.z * tmp73) * tmp74;
  float tmp79 = tmp78.x + tmp78.y + tmp78.z;
  float tmp80 = tmp79 > 0.0 ? tmp79 : 0.0;
  float tmp81 = tmp80 * 0.3183099;
  float tmp82 = tmp67 * 0.005092958 * tmp80;
  sret_ptr.bsdf_diffuse.x = tmp82;
  sret_ptr.bsdf_diffuse.y = tmp82;
  sret_ptr.bsdf_diffuse.z = tmp82;
  phi_in76 = tmp81;
  }
  phi_out77 = phi_in76;
  sret_ptr.pdf = phi_out77;
  float tmp83 = phi_out77 * tmp67 + phi_out;
  phi_in84 = tmp83;
  } else
  phi_in84 = phi_out;
  phi_out85 = phi_in84;
  sret_ptr.pdf = phi_out85;
  return;
  }

  void mdl_bsdf_scattering_0_pdf(inout Bsdf_pdf_data sret_ptr, in Shading_state_material state)
  {
  float3 tmp0;
  float phi_in;
  float phi_out;
  float phi_in17;
  float phi_out18;
  float phi_in23;
  float phi_out24;
  float3 tmp1 = state.normal;
  float3 tmp2 = expr_lambda_5_(state);
  float tmp3 = expr_lambda_3_(state);
  float tmp4 = tmp3 > 0.0 ? tmp3 : 0.0;
  bool tmp5 = tmp4 < 1.0;
  float tmp6 = tmp5 ? tmp4 : 1.0;
  phi_in = 0.0;
  if (tmp6 > 0.0) {
  tmp0.x = tmp2.x;
  tmp0.y = tmp2.y;
  tmp0.z = tmp2.z;
  gen_simple_glossy_bsdf_pdf(sret_ptr, state, tmp0);
  float tmp7 = sret_ptr.pdf;
  float tmp8 = tmp7 * tmp6;
  phi_in = tmp8;
  }
  phi_out = phi_in;
  if (tmp5) {
  float3 tmp9 = state.geom_normal;
  float3 tmp10 = float3(sret_ptr.k1.x, sret_ptr.k1.y, sret_ptr.k1.z) * tmp9;
  float tmp11 = asfloat((asint(tmp10.x + tmp10.y + tmp10.z) & -2147483648) | 1065353216);
  float3 tmp12 = float3(tmp9.x * tmp11, tmp9.y * tmp11, tmp9.z * tmp11);
  float3 tmp13 = tmp12 * tmp1;
  float tmp14 = asfloat((asint(tmp13.x + tmp13.y + tmp13.z) & -2147483648) | 1065353216);
  float3 tmp15 = float3(sret_ptr.k2.x, sret_ptr.k2.y, sret_ptr.k2.z);
  float3 tmp16 = tmp12 * tmp15;
  phi_in17 = 0.0;
  if (tmp16.x + tmp16.y + tmp16.z > 0.0) {
  float3 tmp19 = float3(tmp1.x * tmp14, tmp1.y * tmp14, tmp1.z * tmp14) * tmp15;
  float tmp20 = tmp19.x + tmp19.y + tmp19.z;
  float tmp21 = (tmp20 > 0.0 ? tmp20 : 0.0) * 0.3183099;
  phi_in17 = tmp21;
  }
  phi_out18 = phi_in17;
  sret_ptr.pdf = phi_out18;
  float tmp22 = phi_out18 * (1.0 - tmp6) + phi_out;
  phi_in23 = tmp22;
  } else
  phi_in23 = phi_out;
  phi_out24 = phi_in23;
  sret_ptr.pdf = phi_out24;
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
