#version 430

#define NUMBER_POINT_LIGHTS 6
#define NUMBER_SPOT_LIGHTS 2

uniform sampler2D texmap0;
uniform sampler2D texmap1;

uniform int texMode;


uniform	sampler2D texUnitDiff;
uniform	sampler2D texUnitDiff1;
uniform	sampler2D texUnitSpec;
uniform	sampler2D texUnitNormalMap;

uniform bool normalMap;  //for normal mapping
uniform bool specularMap;
uniform uint diffMapCount;
uniform bool shadowMode;

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
uniform vec4 coneDir;	
uniform float spotCosCutOff;
uniform bool particles;
uniform bool billboard;
uniform bool rover;
uniform bool flare;
uniform bool bumpmap;

uniform PointLight pointLights[NUMBER_POINT_LIGHTS];
uniform SpotLight spotLights[NUMBER_SPOT_LIGHTS];
uniform DirectionalLight dirLight;

uniform Materials mat;
vec4 auxColorOut = { 0.0, 0.0, 0.0, 1.0};
float dist;
vec4 diff, auxSpec;


in Data {
	vec4 pos;
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
	vec2 tex_coord;
} DataIn;

void main() {

	if(shadowMode)
		colorOut = vec4(0.5, 0.5, 0.5, 1.0);
	else { 

		//range based
		dist = length(DataIn.pos);

		vec4 texel, texel1, texel2; 
		float intensity;
		vec3 specSum = vec3(0.0);
		vec3 n;

		if(normalMap)
			n = normalize(2.0 * texture(texmap1, DataIn.tex_coord).rgb - 1.0);  //normal in tangent space
		else
			n = normalize(DataIn.normal);
	
		vec3 l = normalize(DataIn.lightDir);
		vec3 e = normalize(DataIn.eye);

		if(mat.texCount == 0) {
			diff = mat.diffuse;
			auxSpec = mat.specular;
		} else{
			if(diffMapCount == 0)
				diff = mat.diffuse;
			else if(diffMapCount == 1)
				diff = mat.diffuse * texture(texUnitDiff, DataIn.tex_coord);
			else
				diff = mat.diffuse * texture(texUnitDiff, DataIn.tex_coord) * texture(texUnitDiff1, DataIn.tex_coord);
	
			if(specularMap) 
				auxSpec = mat.specular * texture(texUnitSpec, DataIn.tex_coord);
			else
				auxSpec = mat.specular;
		}
	

		if(sun_mode){
		
			vec3 spec = vec3(0.0);
			vec3 lightDir = vec3(dirLight.position - DataIn.pos);
			//vec3 n = normalize(DataIn.normal);
			vec3 l = normalize(lightDir);
			vec3 e = normalize(DataIn.eye);

			intensity = max(dot(n,l), 0.0);
	
			if (intensity > 0.0) {
				vec3 h = normalize(l + e);
				float intSpec = max(dot(h,n), 0.0);
				spec = auxSpec.rgb * pow(intSpec, mat.shininess);
				specSum += spec;
			}
			auxColorOut += vec4(max(intensity * diff.rgb + 0.3 * spec, mat.ambient.rgb)*0.8, diff.a);
		}
		if(point_lights_mode){
			for(int i = 0; i < NUMBER_POINT_LIGHTS; i++){
				vec3 spec = vec3(0.0);
				vec3 lightDir = vec3(pointLights[i].position - DataIn.pos);
				//vec3 n = normalize(DataIn.normal);
				vec3 l = normalize(lightDir);
				vec3 e = normalize(DataIn.eye);

				intensity = max(dot(n,l), 0.0);
	
				if (intensity > 0.0) {
					vec3 h = normalize(l + e);
					float intSpec = max(dot(h,n), 0.0);
					spec = auxSpec.rgb * pow(intSpec, mat.shininess);
					specSum += spec;
				}
				auxColorOut += vec4(max(intensity * diff.rgb + spec, mat.ambient.rgb)/6, diff.a);

			}
		}
		if (spotlight_mode) {
			float att = 0.1;
			float spotExp = 60.0;

			for (int i = 0; i < NUMBER_SPOT_LIGHTS; i++) {
				vec3 spec = vec3(0.0);
				vec3 lightDir = vec3(spotLights[i].position - DataIn.pos);
				//vec3 n = normalize(DataIn.normal);
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
						spec = auxSpec.rgb * pow(intSpec, mat.shininess) * att;
						specSum += spec;
						auxColorOut +=  vec4(0.2*max(intensity * diff.rgb + spec, mat.ambient.rgb), diff.a);
					}
				}
			}
		}

		if (texMode == 2){ // multitexturing for ground
			texel = texture(texmap0, DataIn.tex_coord * 40);  // texel from mars_texture.tga
			texel1 = texture(texmap1, DataIn.tex_coord * 40);  // texel from rock_texture.tga
			auxColorOut += vec4(max(intensity*texel.rgb*texel1.rgb + specSum, 0.07*texel.rgb*texel1.rgb), texel.a*texel1.a);
		}
		else if(texMode == 1){
			if(rover){
				texel = texture(texmap0, DataIn.tex_coord * 2);  // texel from steel.tga
				auxColorOut += vec4(max(intensity*texel.rgb*0.7 + specSum, 0.07*texel.rgb), texel.a);
			}
			else if(particles){
				texel = texture(texmap0, DataIn.tex_coord); // texel from particle.tga
		
				if((texel.a == 0.0)  || (mat.diffuse.a == 0.0) ) discard;
				else
					auxColorOut += mat.diffuse * texel;
			}
			else if(billboard){
				texel = texture(texmap0, DataIn.tex_coord);
	
				if(texel.a == 0.0) discard;
				else
					auxColorOut += vec4(max(intensity*texel.rgb + specSum, 0.1*texel.rgb), texel.a);
			}
			else if(flare){
				texel = texture(texmap0, DataIn.tex_coord);  //texel from element flare texture
				if((texel.a == 0.0)  || (mat.diffuse.a == 0.0) ) discard;
				else
					auxColorOut = mat.diffuse * texel;
			}
			else if(bumpmap){
				texel = texture(texmap0, DataIn.tex_coord);  // texel from stone.tga
				auxColorOut = vec4((max(intensity*texel + auxSpec, 0.2*texel)).rgb, 1.0);
			}
			else{
				texel = texture(texmap0, DataIn.tex_coord);
				auxColorOut += vec4(max(intensity*texel.rgb + specSum, texel.rgb), texel.a);
			}
		}
		if(!flare)
			auxColorOut[3] = mat.diffuse.a;
		if (fog_mode) {
		
			float fogAmount = exp( -dist*0.05 );
			vec4 fogColor = vec4(0.5, 0.5, 0.5, 1);
			colorOut = mix(fogColor, auxColorOut, fogAmount);
		}
		else
			colorOut = auxColorOut;
	}
}