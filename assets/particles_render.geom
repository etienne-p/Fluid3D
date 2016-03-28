#version 330 core

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

in float[] velocity_factor;

out float alpha_factor;
out vec2 tex_coord;
out vec3 var_color;

uniform float particle_size;
uniform float velocity_to_size_factor;
uniform float velocity_to_alpha_factor;
uniform mat4 ciProjection; // explicitely set from our code despite its name
uniform mat4 ciModelView;
uniform float fog_start;
uniform float fog_end;

const float border = .05;

void main()
{
    vec4 center = ciModelView * gl_in[0].gl_Position;
    vec4 projCenter = ciProjection * center;
    
    float fog = clamp((fog_end - projCenter.z / projCenter.w) / (fog_end - fog_start), .0, 1.0);
    
    alpha_factor = velocity_factor[0] * velocity_to_alpha_factor * fog;
    /* Happy Accident
    float borderFactor = min(1.0, .5 - abs(gl_in[0].gl_Position.x) / border) *
                         min(1.0, .5 - abs(gl_in[0].gl_Position.y) / border) *
                         min(1.0, .5 - abs(gl_in[0].gl_Position.z) / border);
    */
    var_color = gl_in[0].gl_Position.xyz + vec3(.5);
    
    //float d = particle_size * borderFactor * (velocity_factor[0] * velocity_to_size_factor) * .5; //Happy Accident
    float d = particle_size * fog * .5;
    
    //if (d < .001 || alpha_factor < .01) return;
    
    tex_coord = vec2( .0, .0 );
    gl_Position = ciProjection * (center + vec4(-d, -d, 0, 0));
    EmitVertex();
    
    tex_coord = vec2( .0, 1.0 );
    gl_Position = ciProjection * (center + vec4(-d, d, 0, 0));
    EmitVertex();
    
    tex_coord = vec2( 1.0, .0 );
    gl_Position = ciProjection * (center + vec4(d, -d, 0, 0));
    EmitVertex();
    
    tex_coord = vec2( 1.0, 1.0 );
    gl_Position = ciProjection * (center + vec4(d, d, 0, 0));
    EmitVertex();

    EndPrimitive();
}