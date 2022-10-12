#version 430

#define NUMBER_POINT_LIGHTS 6
#define NUMBER_SPOT_LIGHTS 2

uniform sampler2D texmap;
uniform sampler2D texmap1;
uniform sampler2D texmap2;

uniform int texMode;

out vec4 colorOut;

struct DirectionalLight {
    vec4 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {    
    vec4 position;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
	vec4 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform bool sun_mode;
uniform bool point_lights_mode;
uniform bool spotlight_mode;

uniform vec4 coneDir;	
uniform float spotCosCutOff;

uniform PointLight pointLights[NUMBER_POINT_LIGHTS];
uniform SpotLight spotLights[NUMBER_SPOT_LIGHTS];
uniform DirectionalLight dirLight;

uniform Materials mat;
vec4 auxColorOut = { 0.0, 0.0, 0.0, 0.0};
// in float dist;

in Data {
	vec4 pos;
	vec3 normal;
	vec3 eye;
} DataIn;

void main() {

	if(sun_mode){
		
		vec4 spec = vec4(0.0);
		vec3 lightDir = vec3(dirLight.position - DataIn.pos);
		vec3 n = normalize(DataIn.normal);
		vec3 l = normalize(lightDir);
		vec3 e = normalize(DataIn.eye);

		float intensity = max(dot(n,l), 0.0);
	
		if (intensity > 0.0) {
			vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = mat.specular * pow(intSpec, mat.shininess);
		}
		auxColorOut += max(intensity * mat.diffuse, mat.ambient);
	}
	if(point_lights_mode){
		for(int i = 0; i < NUMBER_POINT_LIGHTS; i++){
			vec4 spec = vec4(0.0);
			vec3 lightDir = vec3(pointLights[i].position - DataIn.pos);
			vec3 n = normalize(DataIn.normal);
			vec3 l = normalize(lightDir);
			vec3 e = normalize(DataIn.eye);

			float intensity = max(dot(n,l), 0.0);
	
			if (intensity > 0.0) {
				vec3 h = normalize(l + e);
				float intSpec = max(dot(h,n), 0.0);
				spec = mat.specular * pow(intSpec, mat.shininess);
			}
			auxColorOut += max(intensity * mat.diffuse + spec, mat.ambient);

		}
	}
	if (spotlight_mode) {
		float att = 0.0;
		float spotExp = 80.0;

		for (int i = 0; i < NUMBER_SPOT_LIGHTS; i++) {
			vec4 spec = vec4(0.0);
			vec3 lightDir = vec3(spotLights[i].position - DataIn.pos);
			vec3 n = normalize(DataIn.normal);
			vec3 l = normalize(lightDir);
			vec3 e = normalize(DataIn.eye);
			vec3 sd = normalize(vec3(-coneDir));
			float spotCos = dot(l, sd);

			if(spotCos > spotCosCutOff)  {	//inside cone?
				att = pow(spotCos, spotExp);
				float intensity = max(dot(n,l), 0.0) * att;
				if (intensity > 0.0) {
					vec3 h = normalize(l + e);
					float intSpec = max(dot(h,n), 0.0);
					spec = mat.specular * pow(intSpec, mat.shininess) * att;
					auxColorOut +=  max(intensity * mat.diffuse + spec, mat.ambient);
				}
			}
		}
	}
	/*
	//float fogAmount = exp( -dist*0.05 );
	vec4 fogColor = vec4(0.5, 0.6, 0.7, 1);
	//não sei se da para fazer mix com vec4 ou se tem q ser com vec3
	colorOut = mix(fogColor, auxColorOut, fogAmount);*/
	colorOut = auxColorOut;
}