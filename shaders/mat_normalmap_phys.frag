#include <common>

#include <vertexoutput.glsl>

layout(location = 0) out vec4 out_color;
#if MOTIONBLUR_ENABLED
layout(location = 1) out vec4 out_motion;
#endif

#param (color, 0, 0, 4, diffuse)
#param (diffuse, 1, 0, 1, diffuse)
#param (specular, 1, 1, 1, specular)
#param (reflection, 1, 2, 1, one)
#param (glossiness, 1, 3, 1, glossiness)

#texture (albedo, 0, sRGB_A)
uniform sampler2D albedo;

#texture (normalmap, 1, RG)
uniform sampler2D normalmap;

#texture (params, 2, RGB)
uniform sampler2D params;

#texture (brdf, 3, RG)
uniform sampler2D brdf;

#texture (irradiance, 4, RGB)
uniform samplerCube irradiance;

#texture (radiance, 5, RGB)
uniform samplerCube radiance;


#define NORMALMAP
#include <light_phys.glsl>
#include <apply_fog.glsl>
#include <tonemapping.glsl>
#include <tbn.glsl>

void main()
{
	vec4 tex_albedo = texture(albedo, f_coord);
	vec4 tex_params = texture(params, f_coord);

	bool alphatestfail = ( opacity >= 0.0 ? (tex_albedo.a < opacity) : (tex_albedo.a >= -opacity) );
	if(alphatestfail)
		discard;
//	if (tex_albedo.a < opacity)
//		discard;

	vec3 normal;
	normal.xy = (texture(normalmap, f_coord).rg * 2.0 - 1.0);
	normal.z = sqrt(1.0 - clamp((dot(normal.xy, normal.xy)), 0.0, 1.0));
	vec3 fragnormal = normalize(getTbn() * normal);
	
	vec4 fragcolor = calcColor(vec4(tex_albedo.xyz, tex_albedo.a * alpha_mult), fragnormal, tex_params, tex_albedo);
	
	fragcolor.xyz = fragcolor.xyz + (fragcolor.xyz * emission);
  
  //fragcolor = mat3(1.,0.,0.,0.,0.,1.,0.,1.,0.) * transpose(modelviewnormal) * fragnormal * .5 + .5;
	// float reflectivity = param[1].z * texture(normalmap, f_coord).a;
	// float specularity = texture(specgloss, f_coord).r;
	// glossiness = texture(specgloss, f_coord).g * abs(param[1].w);
	// float metalic = texture(specgloss, f_coord).b;

	// fragcolor = apply_lights(fragcolor, fragnormal, tex_albedo.rgb, reflectivity, specularity, shadow_tone);

	vec4 color = vec4(apply_fog(fragcolor.xyz), fragcolor.a);
#if POSTFX_ENABLED
    out_color = color;
#else
    out_color = tonemap(color);
#endif
	//out_color = vec4(specularity,specularity,specularity,1.0);
#if MOTIONBLUR_ENABLED
	{
        vec2 a = (f_clip_future_pos.xy / f_clip_future_pos.w) * 0.5 + 0.5;;
        vec2 b = (f_clip_pos.xy / f_clip_pos.w) * 0.5 + 0.5;;
        
        out_motion = vec4(a - b, 0.0f, 0.0f);
	}
#endif
}
