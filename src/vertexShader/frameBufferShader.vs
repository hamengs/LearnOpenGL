#version 330

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 lightMatrices;
layout (std140) uniform Matrices{
    mat4 view;
    mat4 projection;
};

out vec2 TexCoords;

void main(){
    gl_Position = lightMatrices * model*vec4(aPos,1.0f);
    TexCoords = aTexCoords;
}