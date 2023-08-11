#ifdef IN_VERTEX
  #define MAP out
#else
  #define MAP in
#endif

MAP vec4 f_tangent;
MAP vec3 f_normal;
flat MAP vec3 f_normal_raw;
MAP vec2 f_coord;
MAP vec4 f_pos;
MAP mat3 f_tbn;
//MAP vec4 f_light_pos[MAX_CASCADES];

MAP vec4 f_clip_pos;
MAP vec4 f_clip_future_pos;

//out vec3 TangentLightPos;
//out vec3 TangentViewPos;
MAP vec3 TangentFragPos;

#undef MAP