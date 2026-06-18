#version 330 

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

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpecular;
uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[2];
uniform float exposure;



vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor,float shiness){
    vec3 lightDir = normalize(-light.Direction);
    vec3 halfVector = normalize(viewDir + lightDir);

    float diff = max(dot(normal, lightDir), 0.0);
    float spec = diff > 0.0 ? pow(max(dot(normal, halfVector), 0.0), shiness) : 0.0;

    vec3 ambient = light.Ambient * baseColor;
    vec3 diffuse = light.Diffuse * diff * baseColor;
    vec3 specular = light.Specular * spec * 0.2;
    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 baseColor, float shiness, vec3 fragPos){
    vec3 lightDir = normalize(light.Position - fragPos);
    vec3 halfVector = normalize(viewDir + lightDir);

    float distanceToLight = distance(light.Position, fragPos);
    float attenuation = 1.0 / (light.Constant + light.Linear * distanceToLight + light.Quadratic * distanceToLight * distanceToLight);
    float diff = max(dot(normal, lightDir), 0.0);
    float spec = diff > 0.0 ? pow(max(dot(normal, halfVector), 0.0), shiness) : 0.0;

    vec3 ambient = light.Ambient * baseColor;
    vec3 diffuse = light.Diffuse * diff * baseColor;
    vec3 specular = light.Specular * spec * 0.2;
    return (ambient + diffuse + specular) * attenuation;
}

void main()
{   
    vec3 result = vec3(0.0f);
    vec3 fragPos = texture(gPosition,TexCoords).rgb;
    vec3 normal = texture(gNormal,TexCoords).rgb;
    vec3 baseColor = texture(gAlbedoSpecular,TexCoords).rgb;
    float specular = texture(gAlbedoSpecular,TexCoords).a;
    vec3 viewDir = viewPos - fragPos;
    result += CalcDirLight(dirLight,normal,viewDir,baseColor,specular);
    for(int i = 0; i<2; i++){
        result += CalcPointLight(pointLights[i],normal,viewDir,baseColor,specular,fragPos);
    }

    result = 1.0f-exp(-result*exposure);
    FragColor = vec4(result,1.0f);
}
