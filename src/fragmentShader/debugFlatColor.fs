#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

struct Material{
    sampler2D Diffuse;
    vec3 DiffuseColor;
    bool HasDiffuseTexture;
};

uniform Material material;

void main(){
    vec3 albedo = material.HasDiffuseTexture ? texture(material.Diffuse, TexCoords).rgb : material.DiffuseColor;
    FragColor = vec4(albedo, 1.0);
}
