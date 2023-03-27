#version 410

uniform vec2 offset;
uniform vec2 last_click;
uniform vec3 last_click_col;
uniform float zoom_value;
uniform vec3 fractal_inside_col;
uniform vec3 fractal_outside_col;
uniform float background_brightness;
uniform int max_iter;
uniform int fractal_type;
uniform int power;

layout(location = 0) in vec2 fs_in_tex;
out vec4 fs_out_col;

vec2 mul(vec2 u, vec2 v){
	return vec2(u.x*v.x - u.y*v.y, u.x*v.y + u.y*v.x);
}

vec2 vec_pow(vec2 u, int power){
	int i = 1;
	vec2 v = u;
	while (i < power) {
		v = mul(v,u);
		i++;
	}
	return v;
}

void main()
{	
	vec2 z = fs_in_tex.xy / zoom_value + offset;
	z.y = -z.y;
	vec2 c = z;
	int iter = 0;

	switch(fractal_type){
		case 0:
		{
			//mandelbrot
			for(; length(z) <= 2 && iter < max_iter; ++iter){
				z = vec_pow(z,power) + c;
			}
			break;
		}
		case 1:
		{
			//burning ship
			for(; length(z) <= 2 && iter < max_iter; ++iter){
				vec2 z_abs = abs(z);
				z = vec_pow(z_abs,power) + c;
			}
			break;
		}
	}

	float outside_dim = background_brightness * iter / 32.0f / log2(zoom_value+1);

	if (iter == max_iter) { 
		fs_out_col = vec4(fractal_inside_col, 1);
	} else if (iter < max_iter) {
		fs_out_col = vec4(fractal_outside_col * outside_dim, 1);
	}

	//Click visualization
	float circleX = (last_click.x - c.x);
	float circleY = (last_click.y - c.y);
	float radius = 0.02 / zoom_value;

	if(circleX * circleX + circleY * circleY < radius * radius){
		fs_out_col = fs_out_col*0.1 + vec4(last_click_col, 1);
	}
}
