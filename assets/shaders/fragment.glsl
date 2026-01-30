#version 460 core

#define M_PI 3.1415926535897932384626433832795

layout(location = 0) out vec4 color;

uniform vec2 u_window_size_half;
uniform vec2 u_offset;
uniform float u_scale;
uniform float u_escape;
uniform uint u_iterations;

vec3 hsv_to_rgb(vec3 hsv)
{
	const float hue_norm = hsv.x / (M_PI / 3.0f);
	const float saturation_shift = 1.0f - hsv.y;

	const float gradient_inc = mod(hue_norm, 1.0f);
	const float gradient_dec = 1.0f - gradient_inc;

	vec3 rgb = vec3(0.0f);
	switch (uint(hue_norm)) {
		case 0:
			rgb.x = 1.0f;
			rgb.y = gradient_inc * hsv.y + saturation_shift;
			rgb.z = saturation_shift;
			break;
		case 1:
			rgb.x = gradient_dec * hsv.y + saturation_shift;
			rgb.y = 1.0f;
			rgb.z = saturation_shift;
			break;
		case 2:
			rgb.x = saturation_shift;
			rgb.y = 1.0f;
			rgb.z = gradient_inc * hsv.y + saturation_shift;
			break;
		case 3:
			rgb.x = saturation_shift;
			rgb.y = gradient_dec * hsv.y + saturation_shift;
			rgb.z = 1.0f;
			break;
		case 4:
			rgb.x = gradient_inc * hsv.y + saturation_shift;
			rgb.y = saturation_shift;
			rgb.z = 1.0f;
			break;
		case 5:
			rgb.x = 1;
			rgb.y = saturation_shift;
			rgb.z = gradient_dec * hsv.y + saturation_shift;
			break;
		default:
			break;
	}

	return rgb * hsv.z;
}

vec2 complex_mul(dvec2 z1, dvec2 z2)
{
	return vec2(z1.x * z2.x - z1.y * z2.y, z1.x * z2.y + z1.y * z2.x);
}

#define ESCAPE 4.0f //??

void main()
{
	dvec2 c;
	c.x = gl_FragCoord.x - u_offset.x - u_window_size_half.x;
	c.y = gl_FragCoord.y + u_offset.y - u_window_size_half.y;
	c /= u_scale;

	dvec2 z = c;

	uint i;
	for (i = 0; i < u_iterations; ++i) {
		z = complex_mul(z, z) + c;

		if (z.x*z.x + z.y*z.y > ESCAPE)
			break;
	}

	color = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	if (i < u_iterations - 1)
		color.xyz = hsv_to_rgb(vec3(2 * M_PI * i / u_iterations, 1.0f, 1.0f));
}
