#version 330 core

out vec4 FragColor;

uniform vec3 lightColor;
uniform float exposure;

void main(){
    vec3 result = 1.0f-exp(-lightColor*exposure);
    FragColor = vec4(result,1.0f);
}