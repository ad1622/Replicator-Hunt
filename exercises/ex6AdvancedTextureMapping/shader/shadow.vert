// Phong Vertex Shader
#version 150 compatibility


//output of this shader
smooth out vec3 p_normalVec;
smooth out vec3 p_lightVec;
smooth out vec3 p_eyeVec;

smooth out vec2 texCoordinates;

smooth out vec4 shadowProjTexCoords;

uniform mat4 eyeToLightMatrix;

void main() {
	vec4 eyePosition = gl_ModelViewMatrix * gl_Vertex;
    
    p_normalVec = (gl_NormalMatrix * gl_Normal).xyz;
    p_eyeVec = -eyePosition.xyz;
	p_lightVec = (gl_LightSource[0].position.xyz - eyePosition.xyz);

	//calculate vertex position from light perspective
/*----------4.1----------*/
	shadowProjTexCoords = vec4(0);

	texCoordinates = gl_MultiTexCoord0.st;
	
	gl_Position = gl_ProjectionMatrix * eyePosition;
}