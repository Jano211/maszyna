#if SHADOWMAP_ENABLED
in vec4 f_light_pos[MAX_CASCADES];
uniform sampler2DArrayShadow shadowmap;
#endif
uniform sampler2D headlightmap;
#define PI 3.14159265359
#define AMBIENT_SCALE .6
#define HORIZON_FADE .9

float D(float ndoth, float rough) {
  float alpha = rough * rough;
  float alphaSqr = alpha * alpha;
  float denom = max(ndoth * ndoth * (alphaSqr - 1.) + 1., alphaSqr);
  return alphaSqr / (PI * denom * denom);
}

float G1(float ndotv, float k) {
  return ndotv / (ndotv * (1. - k) + k);
}

float G(float ndotv, float ndotl, float rough) {
  float k = (rough + 1.) * (rough + 1.) * .125f;
  return G1(ndotl, k) * G1(ndotv, k);
}

vec3 F(float ndotv, vec3 f0) {
  return f0 + (1. - f0) * pow(2., (-5.55473 * ndotv - 6.98316) * ndotv);
}

vec3 F(float ndotv, vec3 f0, float roughness) {
  return f0 + (max(f0, 1. - roughness) - f0) * pow(2., (-5.55473 * ndotv - 6.98316) * ndotv);
}

float calc_horizon_fade(vec3 r) {
  float horiz = clamp(1. + HORIZON_FADE * dot(r, f_normal), 0., 1.);
  horiz *= horiz;
  return horiz;
}

float calc_shadow()
{
#if SHADOWMAP_ENABLED
	float distance = dot(f_pos.xyz, f_pos.xyz);
	uint cascade;
	for (cascade = 0U; cascade < MAX_CASCADES; cascade++)
		if (distance <= cascade_end[cascade])
			break;

	vec3 coords = f_light_pos[cascade].xyz / f_light_pos[cascade].w;
	if (coords.z < 0.)
		return 0.;

	float shadow = 0.;
	//basic
//	shadow = texture(shadowmap, coords.xyz + vec3(0., 0., bias));
	//PCF
	float bias = .00005 * float(cascade + 1U);
	vec2 texel = vec2(1.) / vec2(textureSize(shadowmap, 0));
	float radius = 1.;
	for (float y = -1.5; y <= 1.5; y += 1.)
		for (float x = -1.5; x <= 1.5; x += 1.)
			shadow += texture( shadowmap, vec4(coords.xy + vec2(x, y) * radius * texel, cascade, coords.z + bias) );
	shadow /= 16.;

	return shadow;
#else
	return 0.;
#endif
}


vec2 calc_headlights(light_s light, vec3 n)
{
	vec4 headlightpos = light.headlight_projection * f_pos;
	vec3 coords = headlightpos.xyz / headlightpos.w;

	if (coords.z > 1.0)
		return vec2(0.0);
	if (coords.z < 0.0)
		return vec2(0.0);

	vec3 light_dir = normalize(light.pos - f_pos.xyz);
	vec2 part = vec2(1.0) * clamp(dot(n, light_dir) + 0.25, 0.0, 1.0);
	float distance = length(light.pos - f_pos.xyz);
	float atten = 1.0f / (1.0f + light.linear * distance + light.quadratic * (distance * distance));
	atten *= mix(1.0, 0.0, clamp((coords.z - 0.998) * 500.0, 0.0, 1.0));
	vec3 lights = textureProj(headlightmap, headlightpos).rgb * light.headlight_weights.rgb;
	float lightintensity = max(max(lights.r, lights.g), lights.b);
	return part * atten * lightintensity;
}
vec3 calcEnv(vec3 albedo, vec3 f0, vec3 n, vec3 r, float ndotv, vec4 params) {
  float maxLod = 6.;
  
  float perceptualRoughness = 1. - params.g;
  float lod = perceptualRoughness * maxLod;
  
  vec2 brdf = texture(brdf, vec2(max(0., ndotv), params.g)).rg;
  
  mat3 view_to_world = mat3(inv_view);
  
  vec3 dr = texture(irradiance, view_to_world * n).rgb;
  vec3 sr = textureLod(radiance, view_to_world * r, lod).rgb;
  
  vec3 ks = F(ndotv, f0, 1. - params.g);
  vec3 kd = 1. - ks;
  
  vec3 specular = sr * (ks * brdf.x + brdf.y);
  vec3 diffuse = kd * albedo * (1. - params.r) * dr;
  
  return diffuse + calc_horizon_fade(r) * specular;
}

vec4 calcColor(vec4 albedo, vec3 n, vec4 params, vec4 emissioncolor) {
  vec3 fragcolor;
  vec3 v = normalize(-f_pos.xyz);
  vec3 l = normalize(-lights[0].dir);
  vec3 h = normalize(v + l);
  vec3 r = -normalize(reflect(v, n));
  
  float ndotl = clamp(dot(n, l), .001, 1.);
  float ndotv = clamp(dot(n, v), .001, 1.);
  float ndoth = clamp(dot(n, h), 0., 1.);
  float ldoth = clamp(dot(l, h), 0., 1.);
  float vdoth = clamp(dot(v, h), 0., 1.);
  
  vec3 f0 = mix(vec3(.04, .04, .04), albedo.xyz, params.r);
  vec3 f = F(ndotv, f0);
  
  float roughness = max(1. - params.g, 1e-2);
  float g = G(ndotl, ndotv, roughness);
  float d = D(ndoth, roughness);
  
  
  	for (uint i = 0U; i < lights_count; i++)
	{
		light_s light = lights[i];
		vec2 part = vec2(0.0);

			part = calc_headlights(light, n);

		fragcolor += light.color * (part.x * param[1].x + part.y * param[1].y) * light.intensity;
	}
	
  
  vec3 specular = f * g * d / (4. * ndotl * ndotv);
  vec3 diffuse = (1. - f) * albedo.xyz * (1. - params.r);
  
  if(emissioncolor == albedo)
  {
  emissioncolor = emissioncolor * emission;
  
  }
  fragcolor += emissioncolor.xyz;
  float ao = params.b;
  

  vec3 color = lights[0].color * ndotl;

  specular *= color;
  diffuse *= color;
  
    //specular = 
  
  vec3 ldiff = vec3(0., 0., 0.);
  vec3 lspec = vec3(0., 0., 0.);
  
  vec3 occlusion = vec3(1., 1., 1.); // pow(calcOcclusion(input, n), 2.);
  
  //applyLights(input, ldiff, lspec, occlusion, albedo.xyz, f0, n, r, ndotv, params);  
  
  vec4 result;

  result.xyz = ao * ((diffuse + calc_horizon_fade(r) * specular) * clamp(1. - calc_shadow(), 0., 1.) * clamp(1. - shadow_tone, 0., 1.) + ldiff + lspec + AMBIENT_SCALE * ambient * calcEnv(albedo.xyz, f0, n, r, ndotv, params));
  result.xyz += mix(specular,fragcolor,albedo.xyz);
  result.xyz += emissioncolor.xyz;
  result.a = F(ndotv, albedo.aaa).x;
  
  	
	
  return result;
}
