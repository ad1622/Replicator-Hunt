#version 150 compatibility

out vec2 texcoords;

void main() {
	texcoords = gl_MultiTexCoord0.st;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
