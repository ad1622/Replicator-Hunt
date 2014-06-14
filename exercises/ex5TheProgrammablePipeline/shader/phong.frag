// Phong Fragment Shader
#version 150 compatibility


smooth in vec3 p_normalVec;
smooth in vec3 p_lightVec;
smooth in vec3 p_eyeVec;

uniform vec4 mycolor;

uniform sampler2D mytexture;

in vec2 mytextureCoordinates;

smooth in vec3 p_lightVec2;
smooth in vec3 p_lightVec3;

vec4 calculateSimpleSpotLight(in vec3 lightVec, in vec3 normalVec, in vec3 eyeVec, in int lightSourceIndex) {

	lightVec = normalize(lightVec);
	normalVec = normalize(normalVec);
	eyeVec = normalize(eyeVec);
	vec3 spot = normalize(gl_LightSource[lightSourceIndex].spotDirection);

	float diffuse = max(dot(normalVec,lightVec),0.0);
	vec3 reflectVec = reflect(-lightVec,normalVec); 
	float spec = pow( max(dot(reflectVec, eyeVec),0.0) ,gl_FrontMaterial.shininess);
	vec4 result = vec4(0.0,0.0,0.0,1.0);

	if(dot(-lightVec, spot) > gl_LightSource[lightSourceIndex].spotCosCutoff){
		vec4 c_amb = clamp(gl_LightSource[lightSourceIndex].ambient * gl_FrontMaterial.ambient, 0.0, 1.0);
		vec4 c_diff = clamp(diffuse * gl_LightSource[lightSourceIndex].diffuse * gl_FrontMaterial.diffuse, 0.0, 1.0);
		vec4 c_spec = clamp(spec * gl_LightSource[lightSourceIndex].specular * gl_FrontMaterial.specular, 0.0, 1.0);
	
		result = c_amb*texture(mytexture,mytextureCoordinates) + c_diff* texture(mytexture,mytextureCoordinates) + c_spec;
	} 
	return result;
}


vec4 calculateSimplePointLight(in vec3 lightVec, in vec3 normalVec, in vec3 eyeVec, in int lightSourceIndex) {
	lightVec = normalize(lightVec);
	normalVec = normalize(normalVec);
	eyeVec = normalize(eyeVec);

	float diffuse = max(dot(normalVec,lightVec),0.0);
	vec3 reflectVec = reflect(-lightVec,normalVec); 
	float spec = pow( max(dot(reflectVec, eyeVec),0.0) ,gl_FrontMaterial.shininess);
	
	vec4 c_amb = clamp(gl_LightSource[lightSourceIndex].ambient * gl_FrontMaterial.ambient, 0.0, 1.0);
	vec4 c_diff = clamp(diffuse * gl_LightSource[lightSourceIndex].diffuse * gl_FrontMaterial.diffuse, 0.0, 1.0);
	vec4 c_spec = clamp(spec * gl_LightSource[lightSourceIndex].specular * gl_FrontMaterial.specular, 0.0, 1.0);
	
	return c_amb*texture(mytexture,mytextureCoordinates) + c_diff* texture(mytexture,mytextureCoordinates) + c_spec;
}


void main() { 	
	gl_FragColor = (calculateSimpleSpotLight(p_lightVec, p_normalVec, p_eyeVec, 0) + calculateSimplePointLight(p_lightVec2, p_normalVec, p_eyeVec, 1)+ calculateSimplePointLight(p_lightVec3, p_normalVec, p_eyeVec, 2));
}
