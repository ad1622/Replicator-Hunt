#version 150 compatibility

const float PI = 3.14;


uniform vec4 mycolor;

uniform sampler2D mytexture;

in vec2 mytextureCoordinates;

uniform float wobbletime;

void main (void) { 	
	
	vec2 wobblecoords = mytextureCoordinates;
	wobblecoords.s = wobblecoords.s + sin(wobblecoords.t*PI+wobbletime)*0.1;
	gl_FragColor = texture(mytexture,wobblecoords);
}
