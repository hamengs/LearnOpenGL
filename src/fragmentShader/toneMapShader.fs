#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D hdrBuffer;
uniform float exposure;

void main(){
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    FragColor = vec4(mapped, 1.0);
}
