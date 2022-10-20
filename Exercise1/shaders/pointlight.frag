#version 430

#define NUMBER_POINT_LIGHTS 6
#define NUMBER_SPOT_LIGHTS 2

uniform sampler2D texmap;
uniform sampler2D texmap1;
uniform sampler2D texmap2;
uniform sampler2D texmap3;

uniform int texMode;


uniform	sampler2D texUnitDiff;
uniform	sampler2D texUnitDiff1;
uniform	sampler2D texUnitSpec;
uniform	sampler2D texUnitNormalMap;

uniform bool normalMap;  //for normal mapping
uniform bool specularMap;
uniform uint diffMapCount;

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
uniform bool fog_mode;
uniform bool multitexture_mode;
uniform vec4 coneDir;	
uniform float spotCosCutOff;
uniform bool ground;

uniform PointLight pointLights[NUMBER_POINT_LIGHTS];
uniform SpotLight spotLights[NUMBER_SPOT_LIGHTS];
uniform DirectionalLight dirLight;

uniform Materials mat;
vec4 auxColorOut = { 0.0, 0.0, 0.0, 1.0};
float dist;

in Data {
	vec4 pos;
	vec3 normal;
	vec3 eye;
	vec2 tex_coord;
} DataIn;

void main() {

	//range based
	dist = length(DataIn.pos);   //use it;

	vec4 texel, texel1; 
	float intensity;
	vec3 specSum = vec3(0.0);
	if(sun_mode){
		
		vec3 spec = vec3(0.0);
		vec3 lightDir = vec3(dirLight.position - DataIn.pos);
		vec3 n = normalize(DataIn.normal);
		vec3 l = normalize(lightDir);
		vec3 e = normalize(DataIn.eye);

		intensity = max(dot(n,l), 0.0);
	
		if (intensity > 0.0) {
			vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = mat.specular.rgb * pow(intSpec, mat.shininess);
			specSum += spec;
		}
		auxColorOut += vec4(max(intensity * mat.diffuse.rgb + 0.3 * spec, mat.ambient.rgb)*0.8, mat.diffuse.a);
	}
	if(point_lights_mode){
		for(int i = 0; i < NUMBER_POINT_LIGHTS; i++){
			vec3 spec = vec3(0.0);
			vec3 lightDir = vec3(pointLights[i].position - DataIn.pos);
			vec3 n = normalize(DataIn.normal);
			vec3 l = normalize(lightDir);
			vec3 e = normalize(DataIn.eye);

			intensity = max(dot(n,l), 0.0);
	
			if (intensity > 0.0) {
				vec3 h = normalize(l + e);
				float intSpec = max(dot(h,n), 0.0);
				spec = mat.specular.rgb * pow(intSpec, mat.shininess);
				specSum += spec;
			}
			auxColorOut += vec4(max(intensity * mat.diffuse.rgb + spec, mat.ambient.rgb)/6, mat.diffuse.a);

		}
	}
	if (spotlight_mode) {
		float att = 0.1;
		float spotExp = 60.0;

		for (int i = 0; i < NUMBER_SPOT_LIGHTS; i++) {
			vec3 spec = vec3(0.0);
			vec3 lightDir = vec3(spotLights[i].position - DataIn.pos);
			vec3 n = normalize(DataIn.normal);
			vec3 l = normalize(lightDir);
			vec3 e = normalize(DataIn.eye);
			vec3 sd = normalize(vec3(-coneDir));
			float spotCos = dot(sd, l);

			if(spotCos > spotCosCutOff)  {	//inside cone?
				att = pow(spotCos, spotExp);
				intensity = max(dot(n,l), 0.0) * att;
				if (intensity > 0.0) {
					vec3 h = normalize(l + e);
					float intSpec = max(dot(h,n), 0.0);
					spec = mat.specular.rgb * pow(intSpec, mat.shininess) * att;
					specSum += spec;
					auxColorOut +=  vec4(max(intensity * mat.diffuse.rgb + spec, mat.ambient.rgb), mat.diffuse.a);
				}
			}
		}
	}
	if (multitexture_mode && ground){
		texel = texture(texmap, DataIn.tex_coord * 40);  // texel from mars_texture.tga
		texel1 = texture(texmap2, DataIn.tex_coord * 40);  // texel from rock_texture.tga
		auxColorOut += vec4(max(intensity*texel.rgb*texel1.rgb + specSum, 0.07*texel.rgb*texel1.rgb), texel.a*texel1.a);
	}
	
	if (texMode == 4){ //particles
		texel = texture(texmap3, DataIn.tex_coord); // texel from particle.tga
		if((texel.a == 0.0)  || (mat.diffuse.a == 0.0) ) discard;
		else
			auxColorOut += mat.diffuse * texel;
	}

	auxColorOut[3] = mat.diffuse.a;
	if (fog_mode) {
		
		float fogAmount = exp( -dist*0.05 );
		vec4 fogColor = vec4(0.5, 0.5, 0.5, 1);
		colorOut = mix(fogColor, auxColorOut, fogAmount);
	}
	else
		colorOut = auxColorOut;
}