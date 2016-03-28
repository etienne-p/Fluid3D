#version 330 core

layout (location = 0) out vec4 color;

in vec4 oColor;

void main(void)
{
	//vec3 colorSpectrum = clamp( normalize( abs( oColor ) ), vec3( 0.1 ), vec3( 1.0 ) );
	//color = vec4(oColor.xyz + vec3(.5), 1.0);
    float ratio = clamp(length(oColor.xyz), .0, 1.0);
    color = vec4(ratio, 1.0 - ratio, 1.0 - ratio, max(.2, ratio));
}
