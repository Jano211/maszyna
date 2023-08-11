#include <bloom/bloom.glsl>

#texture (source2, 1, RGB)
uniform sampler2D source2;

const vec2 OffsetsTent[9] = vec2[](
  vec2( -1., -1. ),
  vec2(  0., -1. ),
  vec2(  1., -1. ),
  vec2( -1.,  0. ),
  vec2(  0.,  0. ),
  vec2(  1.,  0. ),
  vec2( -1.,  1. ),
  vec2(  0.,  1. ),
  vec2(  1.,  1. )
);

const float WeightsTent[9] = float[](
  1. / 16., 2. / 16., 1 / 16.,
  2. / 16., 4. / 16., 2 / 16.,
  1. / 16., 2. / 16., 1 / 16.
);

vec3 SampleTent(in vec2 uv, in vec2 inputDimensions, in vec2 outputDimensions) {
  //float2 uv = (outputCoords + .5) / outputDimensions;
  vec3 b_output = vec3( 0., 0., 0. );
  for (int i = 0; i < 9; ++i) {
    b_output += WeightsTent[i] * texture(source2, uv + (OffsetsTent[i] / inputDimensions)).rgb;
  }
  return b_output;
}


void main()
{
	out_color = vec4(texture(source, f_coords).rgb + SampleTent(f_coords, input_size, size), 1.);
}  
