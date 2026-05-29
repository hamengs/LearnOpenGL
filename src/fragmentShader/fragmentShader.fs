#version 330 core

struct Material{
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
    float Shiness; 
};

uniform Material material;

struct Light{
    vec3 Position;
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
};

uniform Light light;

in vec3 vPos;
in vec3 vNormal;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 ambientLight;

void main(){
    float ka = 1.5f;
    float kd = 0.8f;
    float ks = 0.5f;

    //环境光
    vec3 ambient = ambientLight * objectColor;

    //漫反射
    vec3 norm = normalize(vNormal);
    vec3 lightDirection = normalize(lightPos - vPos);
    float r = distance(lightPos,vPos);
    float diff = max(dot(lightDirection,norm),0);
    vec3 diffuse = lightColor/(r*r) * diff * material.Diffuse;

    //镜面反射
    vec3 viewDirection = normalize(viewPos-vPos);
    vec3 halfVector = normalize(viewDirection + lightDirection);
    vec3 specular = vec3(0.0f);
    if(diff>0.0f){
        specular = material.Specular*pow(max(dot(halfVector,norm),0),material.Shiness);
    }
    vec3 result =light.Ambient * material.Ambient + light.Diffuse * diffuse + light.Specular*specular;
    FragColor = vec4(result,1.0f);
}
