#version 330 core

struct PointLight{
    vec3 Position;
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
    float Constant;
    float Linear;
    float Quadratic;
    float radius;
};

in vec2 TexCoords;

out vec4 FragColor;

uniform PointLight pointLight;
uniform vec3 viewPos;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpecular;
uniform sampler2D ssaoInput;
uniform bool useSSAO;
uniform vec2 screenSize;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 baseColor, float specularStrength, vec3 fragPos, vec2 screenTexCoords){
    float ambientOcclusion = useSSAO ? max(texture(ssaoInput,screenTexCoords).r, 0.35) : 1.0;
    vec3 lightDir = normalize(light.Position - fragPos);
    vec3 halfVector = normalize(viewDir + lightDir);

    float distanceToLight = distance(light.Position, fragPos);
    float attenuation = 1.0 / (light.Constant + light.Linear * distanceToLight + light.Quadratic * distanceToLight * distanceToLight);
    float diff = max(dot(normal, lightDir), 0.0);
    float spec = diff > 0.0 ? pow(max(dot(normal, halfVector), 0.0), 32.0) : 0.0;

    vec3 ambient = light.Ambient * ambientOcclusion * baseColor;
    vec3 diffuse = light.Diffuse * diff * baseColor;
    vec3 specular = light.Specular * spec * specularStrength;;
    return (ambient + diffuse + specular) * attenuation;
}

void main(){
    vec2 screenTexCoords = gl_FragCoord.xy / screenSize;
    vec3 result = vec3(0.0f);
    vec3 fragPos = texture(gPosition,screenTexCoords).rgb;
    vec3 normal = normalize(texture(gNormal,screenTexCoords).rgb);
    vec3 baseColor = texture(gAlbedoSpecular,screenTexCoords).rgb;
    float specularStrength = texture(gAlbedoSpecular, screenTexCoords).a;
    vec3 viewDir = normalize(viewPos-fragPos);
    float lightPointDistance = length(pointLight.Position-fragPos);
    if(pointLight.radius>lightPointDistance){
        result += CalcPointLight(pointLight,normal,viewDir,baseColor,specularStrength,fragPos,screenTexCoords);
    }else{
        discard;
    }

    FragColor = vec4(result,1.0f);
}
