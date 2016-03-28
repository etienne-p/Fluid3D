#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 velocity;

//uniform mat4 ciModelViewProjection;

out float velocity_factor;

void main(void)
{
    gl_Position = vec4(position, 1.0);
    velocity_factor = length(velocity);
}
