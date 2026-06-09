#version 330 core
out vec4 FragColor;

in vec3 vNormal;
in vec3 fragPos;

uniform vec3 cameraPos;
uniform samplerCube screenTexture;

void main()
{             
    vec3 I = normalize(fragPos - cameraPos);
    vec3 R = reflect(I, normalize(vNormal));
    FragColor = vec4(texture(screenTexture, R).rgb, 1.0);
}