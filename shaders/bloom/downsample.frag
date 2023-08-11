#include <bloom/bloom.glsl>

void main()
{
	out_color = vec4(Sample13Tap(f_coords, input_size, size), 1.);
}  
