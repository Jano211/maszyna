layout(location = 0) in vec3 v_vert;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_coord;
layout(location = 3) in vec4 v_tangent;

#include <common>

#define IN_VERTEX
#include <vertexoutput.glsl>
#undef IN_VERTEX

#include <tbn.glsl>


out vec4 f_light_pos[MAX_CASCADES];

void main()
{
	f_normal = modelviewnormal * v_normal;
	f_tangent.xyz = modelviewnormal * v_tangent.xyz;
    f_tangent.w = v_tangent.w;
	f_normal_raw = v_normal;
	f_coord = v_coord;
	f_pos = modelview * vec4(v_vert, 1.0);
	for (uint idx = 0U ; idx < MAX_CASCADES ; ++idx) {
		f_light_pos[idx] = lightview[idx] * f_pos;
	}	
	f_clip_pos = (projection * modelview) * vec4(v_vert, 1.0);
	f_clip_future_pos = (projection * future * modelview) * vec4(v_vert, 1.0);
	
	gl_Position = f_clip_pos;
	gl_PointSize = param[1].x;

	f_tbn = getTbn();
	
	mat3 inverseTbn = transpose(f_tbn);
//	TangentLightPos = inverseTbn * f_light_pos.xyz;
//	TangentViewPos = inverseTbn * vec3(0.0, 0.0, 0.0);
	TangentFragPos = inverseTbn * f_pos.xyz;
}
