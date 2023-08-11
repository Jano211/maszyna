#include <bloom/bloom.glsl>

const int b_offsets[3] = int[](
  -1,
   0,
   1
);

const float weights[3] = float[](
  0.25,
  0.5,
  0.25
);

void main()
{
	vec3 b_output = vec3(0.0);
	b_output += weights[0] * textureOffset(source, f_coords, ivec2(0, b_offsets[0])).rgb;
	b_output += weights[1] * textureOffset(source, f_coords, ivec2(0, b_offsets[1])).rgb;
	b_output += weights[2] * textureOffset(source, f_coords, ivec2(0, b_offsets[2])).rgb;
	out_color = vec4(b_output, 1.0);
}  
