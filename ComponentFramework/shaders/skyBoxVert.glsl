#version 450
#extension GL_ARB_separate_shader_objects : enable


layout (location = 0) in vec3 inUvcoord;
layout(location = 1) out vec3 textureCoords;  

layout(location = 0) uniform mat4 projectionMatrix;
layout(location = 1) uniform mat4 viewMatrix;

void main() {
    textureCoords = inUvcoord;  // Pass 3D texture coordinates
    textureCoords.x *= -1.0;
    textureCoords.y *= -1.0; // Flip the X and Y coordinates for correct orientation
    gl_Position = projectionMatrix * viewMatrix * vec4(inUvcoord, 1.0);
}