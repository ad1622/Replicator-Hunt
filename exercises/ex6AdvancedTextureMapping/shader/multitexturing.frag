#version 150 compatibility


uniform sampler2D texColor;
uniform sampler2D texNormal;
uniform sampler2D texSpecular;

smooth in vec3 p_lightVec;
smooth in vec3 p_eyeVec;

vec4 compute(in vec3 lightVec, in vec3 eyeVec) {
	vec2 texCoord = gl_TexCoord[0].st;

	lightVec = normalize(lightVec);
	vec3 normalVec = normalize(texture(texNormal, texCoord).xyz * 2.0 - 1.0);
	eyeVec = normalize(eyeVec);
	float specFactor = texture(texSpecular, texCoord).x;

	vec4 base = texture(texColor, texCoord);

	float diffuse = max(dot(normalVec,lightVec),0.0);
	vec3 reflectVec = reflect(-lightVec,normalVec);
	float spec = pow( max(dot(reflectVec, eyeVec),0.0) ,gl_FrontMaterial.shininess);

	vec4 Iamb  = base * clamp(gl_FrontMaterial.ambient * gl_LightSource[0].ambient, 0.0, 1.0);
	vec4 Idiff = base * clamp(diffuse * gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse, 0.0, 1.0);
	vec4 Ispec = specFactor * clamp(spec * gl_LightSource[0].specular * gl_FrontMaterial.specular, 0.0, 1.0);

	return Iamb + Idiff + Ispec;
}

void main() {
	gl_FragColor = compute(p_lightVec, p_eyeVec);
}
