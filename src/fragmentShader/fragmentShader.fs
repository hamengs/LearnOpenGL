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

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 bloomColor;

uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[4];
uniform SpotLight spotLight;
uniform vec3 viewPos;
uniform bool isLight;
uniform vec3 lightColor;

in vec3 fragPos;
in vec3 vNormal;
in vec2 TexCoords;

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

void main(){
    vec3 result;
    if(isLight){
        result = lightColor;
    }else{
        vec3 normal = normalize(vNormal);
        vec3 viewDir = normalize(viewPos - fragPos);
        vec3 baseColor = texture(material.Diffuse, TexCoords).rgb;

        result = CalcDirLight(dirLight, normal, viewDir, baseColor);
        for(int i = 0; i < 2; i++){
            result += CalcPointLight(pointLights[i], normal, viewDir, baseColor);
        }
        //result += CalcSpotLight(spotLight,normal,viewDir,baseColor);
    }

    FragColor = vec4(result, 1.0f);
    float brightness = dot(result, vec3(0.2126f, 0.7152f, 0.0722f));
    if(brightness > 2.0f){
        bloomColor = vec4(result,1.0f);
    }else{
        bloomColor = vec4(0.0f,0.0f,0.0f,1.0f);
    }
}
