#version 150 compatibility

smooth out vec3 p_normalVec;
smooth out vec3 p_viewVec;

uniform mat3 eyetoworld; //we need world coordinates (the environment map should be fixed to our world), but ignore view translation for direction vectors (use only 3x3 matrix)

void main() {
	p_viewVec = eyetoworld * (gl_ModelViewMatrix * gl_Vertex).xyz; //calculate view direction in world space
	p_normalVec = eyetoworld * (gl_NormalMatrix * gl_Normal).xyz; //calculate normal vector direction in world space (we are allowed to use the same matrix for the normal vector when only translation and rotation is done in the eyetoworld matrix)

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

