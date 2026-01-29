#version 460 core

layout(location = 0) in vec4 vertex_pos;

void main()
{
	gl_Position = vertex_pos;
}
