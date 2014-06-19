#version 150 compatibility


out vec2 mytextureCoordinates;

void main() {
	mytextureCoordinates = gl_MultiTexCoord0.st;
	gl_Position =  gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex; 
	//alternative (also depricated): gl_Position = ftransform();
}
