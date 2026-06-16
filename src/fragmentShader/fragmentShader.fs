#version 330 core

struct Material{
    sampler2D Diffuse;
    float Shininess;
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

uniform Material material;
uniform PointLight pointLight;
uniform vec3 viewPos;
uniform sampler2D brickNormalMap;

in vec3 fragPos;
in vec3 vNormal;
in vec2 TexCoords;

out vec4 FragColor;

float near = 0.1f;
float far = 100.0f;



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

void main(){
    vec3 normal = texture(brickNormalMap,TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 baseColor = texture(material.Diffuse, TexCoords).rgb;

    vec3 result = CalcPointLight(pointLight, normal, viewDir, baseColor);

    FragColor = vec4(result, 1.0f);
}
