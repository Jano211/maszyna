// Next Generation Post Processing in Call of Duty: Advanced Warfare (Jorge Jimenez, SIGGRAPH 2014)

in vec2 f_coords;

layout(location = 0) out vec4 out_color;

#texture (source, 0, RGB)
uniform sampler2D source;

layout (std140) uniform bloom_ubo
{
	vec2 size;
	vec2 input_size;
	vec4 bloom_curve;
};

const vec2 Offsets13Tap[13] = vec2[](
  vec2(-1.,  1.),
  vec2( 0.,  1.),
  vec2( 1.,  1.),
  vec2(-1.,  0.),
  vec2( 1.,  0.),
  vec2(-1., -1.),
  vec2( 0., -1.),
  vec2( 1., -1.),
  vec2(-.5, -.5),
  vec2( .5, -.5),
  vec2(-.5,  .5),
  vec2( .5,  .5),
  vec2( 0.,  0.)
);

vec3 Sample13Tap(in vec2 uv, in vec2 inputDimensions, in vec2 outputDimensions) {
  //float2 uv = (outputCoords + .5) / outputDimensions;
  vec3 samples[13];
  for (int i = 0; i < 13; ++i) {
    samples[i] = texture(source, uv + (Offsets13Tap[i] / inputDimensions)).rgb;
  }
  vec3 b_output = vec3(0., 0., 0.);

  b_output += .25 *   .5 * (samples[ 8] + samples[ 9] + samples[10] + samples[11]);
  b_output += .25 * .125 * (samples[ 0] + samples[ 1] + samples[ 3] + samples[12]);
  b_output += .25 * .125 * (samples[ 1] + samples[ 2] + samples[ 4] + samples[12]);
  b_output += .25 * .125 * (samples[ 5] + samples[ 6] + samples[ 3] + samples[12]);
  b_output += .25 * .125 * (samples[ 6] + samples[ 7] + samples[ 4] + samples[12]);

  return b_output;
}
