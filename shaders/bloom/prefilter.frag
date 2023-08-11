#include <bloom/bloom.glsl>

vec3 QuadraticThreshold(in vec3 color, in vec4 curve) {
  float br = max(color.r, max(color.g, color.b));
  float rq = clamp(br - curve.y, 0., curve.z);
  rq = curve.w * rq * rq;
  color *= max(rq, br - curve.x) / max(br, 1e-5);
  return color;
}

void main()
{
	out_color = vec4(QuadraticThreshold(Sample13Tap(f_coords, input_size, size), bloom_curve), 1.0);
}  
