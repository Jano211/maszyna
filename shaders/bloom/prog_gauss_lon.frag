#include <bloom/bloom.glsl>

const int offsets[3] = int[](
  -1,
   0,
   1
);

const float weights[3] = float[](
  .25,
  .5,
  .25
);

void main()
{
	vec3 b_output = vec3(0.);
	b_output += weights[0] * textureOffset(source, f_coords, ivec2(offsets[0], 0)).rgb;
	b_output += weights[1] * textureOffset(source, f_coords, ivec2(offsets[1], 0)).rgb;
	b_output += weights[2] * textureOffset(source, f_coords, ivec2(offsets[2], 0)).rgb;
	out_color = vec4(output, 1.0);
}  
