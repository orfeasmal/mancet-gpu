#version 460 core

layout(location = 0) out vec4 color;

uniform vec2 u_window_dimensions;
uniform vec2 u_offset;
uniform float u_scale;
uniform float u_escape;
uniform uint u_iterations;

vec3 hsv_to_rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

dvec2 complex_mul(dvec2 z1, dvec2 z2)
{
	return vec2(z1.x * z2.x - z1.y * z2.y, z1.x * z2.y + z1.y * z2.x);
}

#define ESCAPE 4.0f //??

void main()
{
	dvec2 window_half = u_window_dimensions / 2.0f;
	dvec2 c;
	c.x = gl_FragCoord.x - u_offset.x - window_half.x;
	c.y = gl_FragCoord.y - u_offset.y - window_half.y;
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
		color.xyz = hsv_to_rgb(vec3(360.0f * i / u_iterations, 1.0f, 1.0f));
}
