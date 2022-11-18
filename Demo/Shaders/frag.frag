#version 410

uniform float x_offset;
uniform float y_offset;

layout(location = 0) in vec2 fs_in_tex;
out vec4 fs_out_col;

vec2 mul(vec2 u, vec2 v){
	return vec2(u.x*v.x - u.y*v.y, u.x*v.y + u.y*v.x);
}

void main()
{
	int max_iter = 5000;
	vec2 z = fs_in_tex.xy + vec2(x_offset,y_offset);
	vec2 c = z;
	int iter = 0;

	for(; length(z) <= 2 && iter < max_iter; ++iter){
		z = mul(z,z) + c;
	}
	float background_col =  iter / 32.0f;

	if (iter == max_iter) { 
		fs_out_col = vec4(vec3(0.9,0.5,0.3), 1);
	} else if (iter < max_iter) {
		fs_out_col = vec4(vec3(background_col),1);
	}
}
