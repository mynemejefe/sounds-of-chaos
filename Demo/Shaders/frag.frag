#version 410

uniform vec2 offset;
uniform float zoom_value;
uniform vec3 fractal_inside_col;
uniform vec3 fractal_outside_col;
uniform float background_brightness;
uniform int max_iter;
uniform int fractal_type;

layout(location = 0) in vec2 fs_in_tex;
out vec4 fs_out_col;

vec2 mul(vec2 u, vec2 v){
	return vec2(u.x*v.x - u.y*v.y, u.x*v.y + u.y*v.x);
}

void main()
{	
	vec2 z = fs_in_tex.xy / zoom_value + offset;
	//z.y = -z.y;
	vec2 c = z;
	int iter = 0;

	switch(fractal_type){
		case 0:
		{
			//mandelbrot
			for(; length(z) <= 2 && iter < max_iter; ++iter){
				z = mul(z, z) + c;
			}
			break;
		}
		case 1:
		{
			//multibrot
			for(; length(z) <= 2 && iter < max_iter; ++iter){
				z = mul(mul(z, z),z) + c;
			}
			break;
		}
		case 2:
		{
			//burning ship
			for(; length(z) <= 2 && iter < max_iter; ++iter){
				vec2 z_abs = abs(z);
				z = mul(z_abs,z_abs) + c;
			}
			break;
		}
	}

	float outside_dim =  background_brightness * iter / 32.0f / sqrt(zoom_value);

	if (iter == max_iter) { 
		fs_out_col = vec4(fractal_inside_col, 1);
	} else if (iter < max_iter) {
		fs_out_col = vec4(fractal_outside_col * outside_dim, 1);
	}
}
