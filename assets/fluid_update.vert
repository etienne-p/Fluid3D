#version 400 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 velocity_pressure;
layout (location = 2) in ivec4 connectionA;
layout (location = 3) in ivec3 connectionB;

uniform samplerBuffer tex_field;

uniform float friction;
uniform float inputRadius;
uniform vec3 inputPosition;
uniform vec3 inputVelocity;

out vec4 tf_position;
out vec4 tf_velocity_pressure; // vec3 velocity, float pressure

subroutine void compute_func(vec4 right, vec4 left , vec4 top, vec4 bottom, vec4 back, vec4 front);

subroutine (compute_func)
void compute_pressure(vec4 right, vec4 left , vec4 top, vec4 bottom, vec4 back, vec4 front)
{
    float px = (left.x   - right.x);
    float py = (bottom.y - top.y  );
    float pz = (front.z  - back.z );
    
    tf_position = position;
    tf_velocity_pressure = vec4(velocity_pressure.xyz, (px + py + pz) / 3.0);
}

subroutine (compute_func)
void compute_velocity(vec4 right, vec4 left , vec4 top, vec4 bottom, vec4 back, vec4 front)
{
    const float m = 1.0 / 3.0;
    
    float vx = velocity_pressure.x * (1.0 - friction) + (left.w   - right.w) * m;
    float vy = velocity_pressure.y * (1.0 - friction) + (bottom.w - top.w  ) * m;
    float vz = velocity_pressure.z * (1.0 - friction) + (front.w  - back.w ) * m;
    
    vec3 n = inputPosition - position.xyz;
    vec3 force = inputVelocity * clamp(1.0 - (length(n) / inputRadius), .0, 1.0);
    tf_position = position + vec4(velocity_pressure.xyz, .0f) * .1;
    tf_velocity_pressure = vec4(vec3(vx, vy, vz) + force, velocity_pressure.w);
}

subroutine uniform compute_func compute_func_selected;

void main(void)
{

    vec4 right  = texelFetch(tex_field, connectionA[0]);
    vec4 left   = texelFetch(tex_field, connectionB[0]);
    vec4 top    = texelFetch(tex_field, connectionA[1]);
    vec4 bottom = texelFetch(tex_field, connectionB[1]);
    vec4 back   = texelFetch(tex_field, connectionA[2]);
    vec4 front  = texelFetch(tex_field, connectionB[2]);
    
    compute_func_selected(right, left, top, bottom, back, front);
}
