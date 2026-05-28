#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main(){
    vec3 viewDirection = viewPos-aPos;
    vec3 halfVector = normalize(viewDirection + aNormal);
    vec3 lightDirection = lightPos - aPos;
    float diffusion = max(dot(lightDirection,aNormal),0);
    gl_Position = projection * view * model * vec4(aPos,1.0f);
}