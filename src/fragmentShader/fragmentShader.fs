#version 330 core

struct Material{
    sampler2D Diffuse;
    sampler2D Specular;
    float Shininess; 
};

uniform Material material;

struct DirLight{
    vec3 Direction;
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
};

struct PointLight{
    vec3 Position;
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
    float Constant;
    float Linear;
    float Qudratic;
};

struct SpotLight{
    vec3 Position;
    vec3 Direction;
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
    float CutOff;
    float OuterCutOff;
};

uniform DirLight dirLight;
uniform PointLight pointLights[4];
uniform SpotLight spotLight;

in vec3 fragPos;
in vec3 vNormal;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 viewPos;

vec3 CalcDirLight(DirLight dirLight, vec3 normal, vec3 viewDir){
    //环境光
    vec3 ambient = dirLight.Ambient * vec3(texture(material.Diffuse,TexCoords));

    //漫反射
    float diff = max(dot(dirLight.Direction,normal),0);
    vec3 diffuse = dirLight.Diffuse * diff * vec3(texture(material.Diffuse,TexCoords));

    //镜面反射
    vec3 halfVector = normalize(viewDir+ dirLight.Direction);
    vec3 specular = vec3(0.0f);
    if(diff > 0.0f){
        specular = dirLight.Specular * vec3(texture(material.Specular,TexCoords))*pow(max(dot(halfVector,normal),0),material.Shininess);
    }

    vec3 result = vec3(0.0f);
    result = ambient + diffuse + specular;

    return result;
}

vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 viewDir, vec3 fragPos){
    //距离衰减(平行光不需要)
    vec3 lightDir = normalize(pointLight.Position-fragPos);
    float r = distance(pointLight.Position,fragPos);
    float attenuation = 1.0f / (pointLight.Constant + pointLight.Linear * r + pointLight.Qudratic * r * r);

    //环境光
    vec3 ambient = pointLight.Ambient * vec3(texture(material.Diffuse,TexCoords));

    //漫反射
    float diff = max(dot(lightDir,normal),0);
    vec3 diffuse = pointLight.Diffuse * diff * vec3(texture(material.Diffuse,TexCoords))*attenuation;

    //镜面反射
    vec3 halfVector = normalize(viewDir+ lightDir);
    vec3 specular = vec3(0.0f);
    if(diff > 0.0f){
        specular = pointLight.Specular * vec3(texture(material.Specular,TexCoords))*pow(max(dot(halfVector,normal),0),material.Shininess)*attenuation;
    }

    vec3 result = vec3(0.0f);
    result = ambient + diffuse + specular;

    return result;
}

vec3 CalcSpotLight(SpotLight spotLight, vec3 normal, vec3 viewDir, vec3 fragPos){
    vec3 lightDirection = normalize(spotLight.Position-fragPos);
    //光源朝向方向和光源连线方向的dot
    float theta = dot(normalize(spotLight.Direction),normalize(-lightDirection));
    float epsilon = spotLight.CutOff-spotLight.OuterCutOff;
    float Intensity = clamp((theta - spotLight.OuterCutOff)/epsilon,0.0f,1.0f);

    //距离衰减(平行光不需要)
    float r = distance(spotLight.Position,fragPos);
    float attenuation = 1.0f / (1.0f + 0.09f * r + 0.032f * r * r);

    //环境光
    vec3 ambient = spotLight.Ambient * vec3(texture(material.Diffuse,TexCoords));

    //漫反射
    float diff = max(dot(lightDirection,normal),0);
    vec3 diffuse = spotLight.Diffuse * diff * vec3(texture(material.Diffuse,TexCoords))*attenuation*Intensity;

    //镜面反射
    vec3 halfVector = normalize(viewDir+ lightDirection);
    vec3 specular = vec3(0.0f);
    if(diff > 0.0f){
        specular = spotLight.Specular * vec3(texture(material.Specular,TexCoords))*pow(max(dot(halfVector,normal),0),material.Shininess)*attenuation*Intensity;
    }

    vec3 result = vec3(0.0f);
    result = ambient + diffuse + specular;

    return result;
}

void main(){
    vec3 viewDirection = normalize(viewPos-fragPos);
    vec3 result = vec3(0.0f);
    result += CalcDirLight(dirLight,vNormal,viewDirection);
    for(int i = 0; i<4; i++){
        result += CalcPointLight(pointLights[i],vNormal,viewDirection,fragPos);
    }
    result += CalcSpotLight(spotLight,vNormal,viewDirection,fragPos);

    FragColor = vec4(result,1.0f);
}
