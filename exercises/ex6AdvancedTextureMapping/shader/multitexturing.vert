#version 150 compatibility


uniform sampler2D texDisplacement;

smooth out vec3 p_lightVec;
smooth out  vec3 p_eyeVec;

in vec3 tangentVec;

vec4 computeDisplacedVertex() {
	float displace = texture(texDisplacement, gl_MultiTexCoord0.st).r;

	vec3 normalVec = gl_Normal.xyz;

	vec3 displacement = normalVec * displace * 0.5;
	return vec4(displacement,0);
}

void main() {
	vec4 positionVec = gl_Vertex + computeDisplacedVertex();

	// compute normal, tangent and bitangent vectors
	vec3 n = normalize(gl_NormalMatrix * gl_Normal).xyz;
	vec3 t = normalize((gl_ModelViewMatrix * vec4(tangentVec,0)).xyz);
	vec3 b = normalize(cross(t, n));

	vec3 eyePosition = (gl_ModelViewMatrix * positionVec).xyz;

	// compute vector from light to each vertex
	vec3 tmpVec = gl_LightSource[0].position.xyz - eyePosition;

	// compute light vector
	p_lightVec.x = dot(tmpVec, t);
	p_lightVec.y = dot(tmpVec, b);
	p_lightVec.z = dot(tmpVec, n);

	// compute eye vector
	tmpVec = -eyePosition;
	p_eyeVec.x = dot(tmpVec, t);
	p_eyeVec.y = dot(tmpVec, b);
	p_eyeVec.z = dot(tmpVec, n);

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * positionVec;
}
