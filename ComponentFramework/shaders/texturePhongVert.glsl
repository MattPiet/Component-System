#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vVertex;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 uvCoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 lightPos[5];

layout(location = 0) out vec3 vertNormal;
layout(location = 1) out vec3 lightDir[5];
layout(location = 6) out vec3 eyeDir;
layout(location = 7) out vec2 textureCoords; 


void main() {
    textureCoords = uvCoord;
    textureCoords.y *= -1.0; // Flip the Y coordinate for texture mapping

    mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
    vertNormal = normalize(normalMatrix * vNormal); /// Rotate the normal to the correct orientation 
    vec3 vertPos = vec3(viewMatrix * modelMatrix * vVertex);
    vec3 vertDir = normalize(vertPos);
    eyeDir = -vertDir;
   for(int i = 0; i < 5; i++){
    lightDir[i] = normalize(vec3(lightPos[i]) - vertPos); 
    }
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vVertex;
    
}
