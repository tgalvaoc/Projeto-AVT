#version 430

#define NUMBER_POINT_LIGHTS 6
#define NUMBER_SPOT_LIGHTS 2

uniform sampler2D texmap;
uniform sampler2D texmap1;
uniform sampler2D texmap2;

uniform int texMode;

out vec4 colorOut;

struct DirectionalLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
	vec3 position;
	vec4 coneDir;
	float spotCosCutOff;

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


uniform PointLight pointLights[NUMBER_POINT_LIGHTS];
uniform SpotLight spotLights[NUMBER_SPOT_LIGHTS];
uniform DirectionalLight dirLight;

uniform Materials mat;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
	vec2 tex_coord;
} DataIn;

void main() {
	
	vec4 texel, texel1; 

	vec4 spec = vec4(0.0);

	vec3 n = normalize(DataIn.normal);
	vec3 l = normalize(DataIn.lightDir);
	vec3 e = normalize(DataIn.eye);

	float intensity = max(dot(n,l), 0.0);

	
	if (intensity > 0.0) {

		vec3 h = normalize(l + e);
		float intSpec = max(dot(h,n), 0.0);
		spec = mat.specular * pow(intSpec, mat.shininess);
	}
	
	// colorOut = max(intensity * mat.diffuse + spec, mat.ambient);

	if(texMode == 0) // mars texture
	{
		texel = texture(texmap, DataIn.tex_coord);  // texel from lighwood.tga
		colorOut = max(intensity * mat.diffuse * texel + spec,0.07 * texel);
	}
	else if (texMode == 1) // steel texture
	{
		texel = texture(texmap1, DataIn.tex_coord);  // texel from stone.tga
		colorOut = max(intensity * mat.diffuse * texel + spec,0.07 * texel);
	}
	else  if (texMode == 2) // rock texture
	{
		texel = texture(texmap2, DataIn.tex_coord);  // texel from checker.tga
		colorOut = max(intensity * mat.diffuse * texel + spec,0.07 * texel);
	}

}