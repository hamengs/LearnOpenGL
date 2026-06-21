#version 330 

struct DirLight{
    vec3 Direction;
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
};



in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpecular;
uniform sampler2D ssaoInput;
uniform bool useSSAO;
uniform vec3 viewPos;
uniform DirLight dirLight;



vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor,float specularStrength){
    float ambientOcclusion = useSSAO ? texture(ssaoInput,TexCoords).r : 1.0;
    vec3 lightDir = normalize(-light.Direction);
    vec3 halfVector = normalize(viewDir + lightDir);

    float diff = max(dot(normal, lightDir), 0.0);
    float spec = diff > 0.0 ? pow(max(dot(normal, halfVector), 0.0), 32) : 0.0;

    vec3 ambient = light.Ambient * ambientOcclusion * baseColor;
    vec3 diffuse = light.Diffuse * diff * baseColor;
    vec3 specular = light.Specular * spec * specularStrength;
    return ambient + diffuse + specular;
}



void main()
{   

    vec3 result = vec3(0.0f);
    vec3 fragPos = texture(gPosition,TexCoords).rgb;
    vec3 normal = normalize(texture(gNormal,TexCoords).rgb);
    vec3 baseColor = texture(gAlbedoSpecular,TexCoords).rgb;
    float specularStrength = texture(gAlbedoSpecular, TexCoords).a;
    vec3 viewDir = viewPos - fragPos;
    viewDir = normalize(viewDir);
    result += CalcDirLight(dirLight,normal,viewDir,baseColor,specularStrength);
    FragColor = vec4(result,1.0f);
}
