#version 330 core

struct Material{
    sampler2D Diffuse;
    float Shininess;
};

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedo;

uniform Material material;

in vec3 fragPos;
in vec3 vNormal;
in vec2 TexCoords;

void main(){
    gPosition = fragPos;
    gNormal = vNormal;
    gAlbedo.rgb = texture(material.Diffuse,TexCoords).rgb;
    gAlbedo.a = 0.3;
}
