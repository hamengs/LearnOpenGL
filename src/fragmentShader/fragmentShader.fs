#version 330 core

struct Material{
    sampler2D Diffuse;
    sampler2D Specular;
    vec3 DiffuseColor;
    bool HasDiffuseTexture;
    bool HasSpecular;
    float SpecularStrength;
    float Shininess;
};

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedo;

uniform Material material;
const float NEAR = 0.1f;
const float FAR = 100.0f;

in vec3 fragPos;
in vec3 vNormal;
in vec2 TexCoords;

float LinearizeDepth(float depth){
    float z = depth *2.0-1.0;
    return (2.0*NEAR*FAR)/(FAR+NEAR-z*(FAR-NEAR));
}

void main(){
    gPosition.xyz = fragPos;
    gPosition.a = LinearizeDepth(gl_FragCoord.z);
    gNormal = vNormal;
    gAlbedo.rgb = material.HasDiffuseTexture ? texture(material.Diffuse,TexCoords).rgb : material.DiffuseColor;
    if(material.HasSpecular){
        gAlbedo.a = texture(material.Specular, TexCoords).r;
    }else{
        gAlbedo.a = material.SpecularStrength;
    }
}
