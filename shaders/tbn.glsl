
mat3 getTbn() {
  vec3 bitangent = f_tangent.w * cross(f_normal, f_tangent.xyz);
  return mat3(f_tangent.xyz, bitangent, f_normal);
}
