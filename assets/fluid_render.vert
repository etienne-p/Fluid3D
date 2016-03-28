#version 330 core

layout (location = 0) in vec4 position;	// POSITION_INDEX
layout (location = 1) in vec4 color;	// VELOCITY_PRESSURE_INDEX

uniform mat4 ciModelViewProjection;

out vec4 oColor;

void main(void)
{
	gl_Position = ciModelViewProjection * position;
	oColor = color;
}
