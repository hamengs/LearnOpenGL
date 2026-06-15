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
uniform vec3 lightPos;
uniform samplerCube depthMap;
uniform float far_plane;

in vec3 fragPos;
in vec3 vNormal;
in vec2 TexCoords;

out vec4 FragColor;

float near = 0.1f;
float far = 100.0f;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 baseColor,float shadow){
    vec3 lightDir = normalize(light.Position - fragPos);
    vec3 halfVector = normalize(viewDir + lightDir);

    float distanceToLight = distance(light.Position, fragPos);
    float attenuation = 1.0 / (light.Constant + light.Linear * distanceToLight + light.Quadratic * distanceToLight * distanceToLight);
    float diff = max(dot(normal, lightDir), 0.0);
    float spec = diff > 0.0 ? pow(max(dot(normal, halfVector), 0.0), material.Shininess) : 0.0;

    vec3 ambient = light.Ambient * baseColor;
    vec3 diffuse = light.Diffuse * diff * baseColor;
    vec3 specular = light.Specular * spec * 0.2;
    return (ambient +(1-shadow)* (diffuse + specular)) * attenuation;
}

float LinearizeDepth(float depth){
    float z = depth * 2.0f -1.0f;
    return (2.0f*near*far)/(far+near-z*(far-near));
}

float ShadowCalculation(vec3 fragPos){
    vec3 sampleOffsetDirections[20] = vec3[]
    (
       vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
       vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
       vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
       vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
       vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );
    float shadow = 0;
    float diskRadius = 0.04+0.04 *length(fragPos.xyz-viewPos)/far_plane;
    int sample = 20;
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(pointLight.Position - fragPos);
    vec3 fragToLight = fragPos - pointLight.Position;
    for(int i = 0; i<20;i++){
        float closestDepth = texture(depthMap,fragToLight+sampleOffsetDirections[i]*diskRadius).r;
        closestDepth *= far_plane;
        float currentDepth = length(fragToLight);
        float bias = max(0.05*(1-dot(normal,lightDir)),0.005);
        if(currentDepth-bias>closestDepth){
            shadow += 1;
        }
    }
    shadow /= float(sample);

    return shadow;
}

void main(){
    vec3 normal = normalize(vNormal);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 baseColor = texture(material.Diffuse, TexCoords).rgb;
    float shadow = ShadowCalculation(fragPos);

    vec3 result = CalcPointLight(pointLight, normal, viewDir, baseColor,shadow);

    float depth = LinearizeDepth(gl_FragCoord.z)/25;
    FragColor = vec4(result, 1.0f);
}
