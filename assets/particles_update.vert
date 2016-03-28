#version 400 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 velocity;
layout (location = 2) in int index;

uniform samplerBuffer tex_velocity;
uniform samplerBuffer tex_position;
uniform samplerBuffer tex_fluid;

uniform float friction;
uniform int fluid_field_size;
uniform float fluid_velocity_factor;

out vec3 tf_position;
out vec3 tf_velocity;

//http://amindforeverprogramming.blogspot.ca/2013/07/random-floats-in-glsl-330.html
uint hash( uint x )
{
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

float random( float f )
{
    const uint mantissaMask = 0x007FFFFFu;
    const uint one          = 0x3F800000u;
    
    uint h = hash( floatBitsToUint( f ) );
    h &= mantissaMask;
    h |= one;
    
    float  r2 = uintBitsToFloat( h );
    return r2 - 1.0;
}

int computeIndex(int x, int y, int z)
{
    return x + y * fluid_field_size + z * fluid_field_size * fluid_field_size;
}

void main(void)
{
    vec3 pos = position + vec3(.5);
    
    int ix = int(pos.x * (fluid_field_size - 1.0));
    int iy = int(pos.y * (fluid_field_size - 1.0));
    int iz = int(pos.z * (fluid_field_size - 1.0));
    
    float nx = mod(pos.x * float(fluid_field_size - 1.0), 1.0);
    float ny = mod(pos.y * float(fluid_field_size - 1.0), 1.0);
    float nz = mod(pos.z * float(fluid_field_size - 1.0), 1.0);
    
    // http://paulbourke.net/miscellaneous/interpolation/
    // trilinear interpolation
    vec3 trilinearInterpolatedFluidVelocity =
        //V000 (1 - x) (1 - y) (1 - z) +
        texelFetch(tex_fluid, computeIndex(ix    , iy    , iz    )).xyz * (1.0 - nx) * (1.0 - ny) * (1.0 - nz) +
        //V100  x (1 - y) (1 - z) +
        texelFetch(tex_fluid, computeIndex(ix + 1, iy    , iz    )).xyz * nx * (1.0 - ny) * (1.0 - nz) +
        //V010 (1 - x) y (1 - z) +
        texelFetch(tex_fluid, computeIndex(ix    , iy + 1, iz    )).xyz * (1.0 - nx) * ny * (1.0 - nz) +
        //V001 (1 - x) (1 - y) z +
        texelFetch(tex_fluid, computeIndex(ix    , iy    , iz + 1)).xyz * (1.0 - nx) * (1.0 - ny) * nz +
        //V101  x (1 - y) z +
        texelFetch(tex_fluid, computeIndex(ix + 1, iy    , iz + 1)).xyz * nx * (1.0 - ny) * nz +
        //V011 (1 - x) y z +
        texelFetch(tex_fluid, computeIndex(ix    , iy + 1, iz + 1)).xyz * (1.0 - nx) * ny * nz +
        //V110  x y (1 - z) +
        texelFetch(tex_fluid, computeIndex(ix + 1, iy + 1, iz    )).xyz * nx * ny * (1.0 - nz) +
        //V111  x y z
        texelFetch(tex_fluid, computeIndex(ix + 1, iy + 1, iz + 1)).xyz * nx * ny * nz;
    
    vec3 vel = velocity + vec3(.0, .0, .0) + trilinearInterpolatedFluidVelocity * fluid_velocity_factor;
    pos += vel;
    
    //pos.x = mod(mod(pos.x, 1.0) + 1.0, 1.0);
    //pos.y = mod(mod(pos.y, 1.0) + 1.0, 1.0);
    //pos.z = mod(mod(pos.z, 1.0) + 1.0, 1.0);
    
    if (pos.x < .0 || pos.x > 1.0 || pos.y < .0 || pos.y > 1.0 || pos.z < .0 || pos.z > 1.0)
    {
        pos.x = random(pos.x);
        pos.y = random(pos.y);
        pos.z = random(pos.z);
        tf_velocity = vec3(.0);
    }
    else
    {
        tf_velocity = vel * (1.0 - friction);
    }
    
    tf_position = pos - vec3(.5);
}
