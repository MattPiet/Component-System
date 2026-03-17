#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
uniform vec4 colour;
void main() {
if(colour!=vec4(0.0,0.0,0.0,0.0)){
fragColor = colour;
}
else{
    fragColor = vec4(1.0,0.4,0.4,0.0);
}
}