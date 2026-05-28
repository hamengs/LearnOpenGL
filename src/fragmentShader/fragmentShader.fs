#version 330 core

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 ambientLight;

void main(){
    FragColor = vec4(objectColor*ambientLight,1.0f);
}
