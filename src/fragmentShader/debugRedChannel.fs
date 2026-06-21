#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D redChannelTexture;

void main(){
    float value = texture(redChannelTexture, TexCoords).r;
    FragColor = vec4(vec3(value), 1.0);
}
