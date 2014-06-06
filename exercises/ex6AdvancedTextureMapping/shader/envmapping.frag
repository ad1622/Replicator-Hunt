#version 150 compatibility


/*----------2.1----------*/

uniform bool useReflection;

smooth in vec3 p_normalVec;
smooth in vec3 p_viewVec;

vec4 envMapping(in vec3 viewVec, in vec3 normalVec) {
	normalVec = normalize(normalVec);
	viewVec = normalize(viewVec);

	vec3 texCoords;

	if(useReflection)
	{
/*----------2.2----------*/
		texCoords = vec3(0);
	}
	else
	{
		texCoords = viewVec;
	}

/*----------2.3----------*/
	return vec4(0);
}

void main() {
	gl_FragColor = envMapping(p_viewVec, p_normalVec);
}
