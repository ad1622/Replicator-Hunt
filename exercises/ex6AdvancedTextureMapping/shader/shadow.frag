// Phong Fragment Shader
#version 150 compatibility


smooth in vec3 p_normalVec;
smooth in vec3 p_lightVec;
smooth in vec3 p_eyeVec;

smooth in vec2 texCoordinates;

smooth in vec4 shadowProjTexCoords;

uniform sampler2DShadow depthMap;

uniform sampler2D objectTexture;
uniform bool enableObjectTexture;

vec4 calculateSimplePointLight(in vec3 lightVec, in vec3 normalVec, in vec3 eyeVec) {

/*----------4.2----------*/

	lightVec = normalize(lightVec);
	normalVec = normalize(normalVec);
	eyeVec = normalize(eyeVec);

	vec4 texColor = vec4(1,1,1,1);

	if(enableObjectTexture)
		texColor = texture(objectTexture,texCoordinates);

	float diffuse = max(dot(normalVec,lightVec),0.0);
	vec3 reflectVec = reflect(-lightVec,normalVec); 
	float spec = pow( max(dot(reflectVec, eyeVec),0.0) ,gl_FrontMaterial.shininess);
	
	vec4 Iamb = texColor*clamp(gl_LightSource[0].ambient * gl_FrontMaterial.ambient, 0.0, 1.0);
	vec4 Idiff = texColor*clamp(diffuse * gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse, 0.0, 1.0);
	vec4 Ispec = clamp(spec * gl_LightSource[0].specular * gl_FrontMaterial.specular, 0.0, 1.0);

/*----------4.3----------*/
	return Iamb + Idiff + Ispec;
}

void main (void) { 	

	gl_FragColor = calculateSimplePointLight(p_lightVec, p_normalVec, p_eyeVec);

}
