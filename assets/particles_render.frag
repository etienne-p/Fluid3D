#version 330 core

layout (location = 0) out vec4 color;

in vec2 tex_coord;
in vec3 var_color;
in float alpha_factor;

uniform sampler2D tex_particle;


void main(void)
{
    color = texture(tex_particle, tex_coord) * vec4(var_color, alpha_factor);
}