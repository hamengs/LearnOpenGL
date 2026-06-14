#version 330 core

struct Material{
    sampler2D Diffuse;
    float Shininess;
};

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
    float Quadratic;
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

uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[4];
uniform SpotLight spotLight;
uniform vec3 viewPos;
uniform sampler2D shadowMapTexture;

in vec3 fragPos;
in vec3 vNormal;
in vec2 TexCoords;
in vec4 lightSpacePosition;

out vec4 FragColor;

float near = 0.1f;
float far = 100.0f;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor){
    vec3 lightDir = normalize(-light.Direction);
    vec3 halfVector = normalize(viewDir + lightDir);

    float diff = max(dot(normal, lightDir), 0.0);
    float spec = diff > 0.0 ? pow(max(dot(normal, halfVector), 0.0), material.Shininess) : 0.0;

    vec3 ambient = light.Ambient * baseColor;
    vec3 diffuse = light.Diffuse * diff * baseColor;
    vec3 specular = light.Specular * spec * 0.2;
    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 baseColor){
    vec3 lightDir = normalize(light.Position - fragPos);
    vec3 halfVector = normalize(viewDir + lightDir);

    float distanceToLight = distance(light.Position, fragPos);
    float attenuation = 1.0 / (light.Constant + light.Linear * distanceToLight + light.Quadratic * distanceToLight * distanceToLight);
    float diff = max(dot(normal, lightDir), 0.0);
    float spec = diff > 0.0 ? pow(max(dot(normal, halfVector), 0.0), material.Shininess) : 0.0;

    vec3 ambient = light.Ambient * baseColor;
    vec3 diffuse = light.Diffuse * diff * baseColor;
    vec3 specular = light.Specular * spec * 0.2;
    return (ambient + diffuse + specular) * attenuation;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 baseColor){
    vec3 lightDir = normalize(light.Position - fragPos);
    vec3 halfVector = normalize(viewDir + lightDir);

    float theta = dot(lightDir, normalize(-light.Direction));
    float epsilon = light.CutOff - light.OuterCutOff;
    float intensity = clamp((theta - light.OuterCutOff) / epsilon, 0.0, 1.0);

    float distanceToLight = distance(light.Position, fragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distanceToLight + 0.032 * distanceToLight * distanceToLight);
    float diff = max(dot(normal, lightDir), 0.0);
    float spec = diff > 0.0 ? pow(max(dot(normal, halfVector), 0.0), material.Shininess) : 0.0;

    vec3 ambient = light.Ambient * baseColor;
    vec3 diffuse = light.Diffuse * diff * baseColor;
    vec3 specular = light.Specular * spec * 0.2;
    return (ambient + diffuse + specular) * attenuation * intensity;
}

float LinearizeDepth(float depth){
    float z = depth * 2.0f -1.0f;
    return (2.0f*near*far)/(far+near-z*(far-near));
}

float CalcShadowMap(){
    vec4 lightSpacePositionNDC = lightSpacePosition/lightSpacePosition.w;
    vec4 lightSpacePositionViewPort = (lightSpacePositionNDC + 1.0f) / 2;
    float closestDepth = texture(shadowMapTexture,lightSpacePositionViewPort.xy).r;
    float shadow = 0;;
    vec2 texelSize = 1.0/textureSize(shadowMapTexture,0);
    float pcfShadowDepth;
    for(int i = -1; i < 2; i++){
        for(int j = -1; j <2; j++){
          pcfShadowDepth = texture(shadowMapTexture,lightSpacePositionViewPort.xy+vec2(i,j)*texelSize).r;
          if(pcfShadowDepth<lightSpacePositionViewPort.z&&lightSpacePositionViewPort.z<1.0f){
            shadow += 1;
          }else{
            shadow += 0;
          }
        }
    }
    
    shadow = shadow/9;

    return shadow;
}

void main(){

    vec3 normal = normalize(vNormal);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 baseColor = texture(material.Diffuse, TexCoords).rgb;
    
    vec3 lightDir = normalize(-dirLight.Direction);
    vec3 halfVector = normalize(viewDir + lightDir);

    float bias = max(0.005*(1-dot(normal,lightDir)),0.0005);

    float diff = max(dot(normal, lightDir), 0.0);
    float spec = diff > 0.0 ? pow(max(dot(normal, halfVector), 0.0), material.Shininess) : 0.0;
    vec3 ambient = dirLight.Ambient * baseColor;
    vec3 diffuse = dirLight.Diffuse * diff * baseColor;
    vec3 specular = dirLight.Specular * spec * 0.2;

    vec3 result;

    result = ambient + (1-CalcShadowMap()) * (diffuse + specular);

    FragColor = vec4(result, 1.0);

}
