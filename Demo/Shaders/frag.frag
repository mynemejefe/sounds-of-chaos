#version 410

uniform vec4 col;
uniform float x_offset;
uniform float y_offset;
uniform float zoom_value;
uniform vec3 fractal_inside_col;
uniform vec3 fractal_outside_col;
uniform float background_dim;
uniform int max_iter;
uniform float fractal_complexity;

layout(location = 0) in vec2 fs_in_tex;
out vec4 fs_out_col;

vec2 mul(vec2 u, vec2 v){
	return vec2(u.x*v.x - u.y*v.y, u.x*v.y + u.y*v.x);
}

void main()
{	
	float zoom = 1/zoom_value/zoom_value;
//	float zoom = log(1+zoom_value);
//	float zoom = exp(zoom_value) - 1;

	vec2 z = fs_in_tex.xy * zoom + vec2(x_offset,y_offset);
	vec2 c = z;
	int iter = 0;

	for(; length(z) <= 2 * fractal_complexity && iter < max_iter; ++iter){
		z = mul(z,z) + c;
	}
	float outside_dim =  background_dim * iter / 32.0f / zoom_value;

	if (iter == max_iter) { 
		fs_out_col = vec4(fractal_inside_col, 1);
	} else if (iter < max_iter) {
		fs_out_col = vec4(fractal_outside_col * outside_dim, 1);
	}
}
