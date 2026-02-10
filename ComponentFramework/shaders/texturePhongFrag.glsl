#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 vertNormal;
layout(location = 1) in vec3 lightDir[5];
layout(location = 6) in vec3 eyeDir; 
layout(location = 7) in vec2 textureCoords; 

uniform vec4 Specular[5];
uniform vec4 Diffuse[5];
//uniform vec4 Ambient[5]; could use later for what idk thought id do it

uniform sampler2D myTexture; 

void main() {
	vec4 ka = 0.6f * Diffuse[0];
	vec4 colour = vec4(0.0f);
	vec4 kt = texture(myTexture,textureCoords); 
	
		for(int i = 0; i < 5; i++){
	float diff = max(dot(lightDir[i], vertNormal), 0.0);
	vec3 reflection = normalize(reflect(-lightDir[i], vertNormal));

	float spec = max(dot(eyeDir, reflection), 0.0);
	spec = pow(spec,35.0);
	colour += (diff * Diffuse[i]) + (spec * Specular[i]);
		}
	fragColor =  (-ka + colour) * kt;	
	}