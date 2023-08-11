in vec2 f_coords;

layout(location = 0) out vec4 out_color;

#texture (source, 0, RGB)
uniform sampler2D source;
#texture (bloom, 1, RGB)
uniform sampler2D bloom;

layout (std140) uniform mix_ubo
{
	float mix_factor;
};

void main()
{
	out_color = vec4(texture(source, f_coords).rgb + texture(bloom, f_coords).rgb * mix_factor, 1.);
}  
