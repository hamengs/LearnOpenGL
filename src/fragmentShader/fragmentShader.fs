#version 330 core

struct Material{
    sampler2D Diffuse;
    sampler2D Specular;
    float Shininess; 
};

uniform Material material;

struct Light{
    vec3 Position;
    vec3 Direction;
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
    float CutOff;
};

uniform Light light;

in vec3 vPos;
in vec3 vNormal;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 viewPos;


void main(){
    vec3 norm = normalize(vNormal);
    vec3 lightDirection = normalize(-light.Direction);
    vec3 viewDirection = normalize(viewPos-vPos);

    //距离衰减(平行光不需要)
    float r = distance(light.Position,vPos);
    float attenuation = 1.0f / (1.0f + 0.09f * r + 0.032f * r * r);

    //环境光
    vec3 ambient = light.Ambient * vec3(texture(material.Diffuse,TexCoords));

    //漫反射
    float diff = max(dot(lightDirection,norm),0);
    vec3 diffuse = light.Diffuse * diff * vec3(texture(material.Diffuse,TexCoords)) ;

    //镜面反射
    vec3 halfVector = normalize(viewDirection + lightDirection);
    vec3 specular = vec3(0.0f);
    if(diff>0.0f){
        specular = light.Specular * vec3(texture(material.Specular,TexCoords))*pow(max(dot(halfVector,norm),0),material.Shininess);
    }

    vec3 result =ambient +diffuse + specular;
    FragColor = vec4(result,1.0f);
}
