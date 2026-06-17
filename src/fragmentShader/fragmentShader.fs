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
uniform sampler2D brickNormalMap;
uniform sampler2D brickDepth;

uniform float height_scale;


in vec3 TBNfragPos;
in vec3 vNormal;
in vec2 TexCoords;
in vec3 TBNLightPos;
in vec3 TBNViewPos;

out vec4 FragColor;

float near = 0.1f;
float far = 100.0f;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir){
    const float numLayers = 10;
    float layerDepth = 1.0/numLayers;
    float currentLayerDepth;
    vec2 p = viewDir.xy * height_scale;
    vec2 deltaTexCoords = p/numLayers;
    vec2 currentTexCoords = texCoords;

    
    float currentDepthMapValue = texture(brickDepth,currentTexCoords).r;
    while(currentLayerDepth<currentDepthMapValue){
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(brickDepth,currentTexCoords).r;
        currentLayerDepth +=layerDepth;
    }
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth = currentDepthMapValue -currentLayerDepth;
    float beforeDepth = texture(brickDepth,prevTexCoords).r-currentLayerDepth+layerDepth;
    float weight = afterDepth/(afterDepth-beforeDepth);
    vec2 finalTexCoords = prevTexCoords*weight + currentTexCoords*(1.0-weight);

    return finalTexCoords;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 baseColor){
    vec3 lightDir = normalize(TBNLightPos - TBNfragPos);
    vec3 halfVector = normalize(viewDir + lightDir);

    float distanceToLight = distance(TBNLightPos, TBNfragPos);
    float attenuation = 1.0 / (light.Constant + light.Linear * distanceToLight + light.Quadratic * distanceToLight * distanceToLight);
    float diff = max(dot(normal, lightDir), 0.0);
    float spec = diff > 0.0 ? pow(max(dot(normal, halfVector), 0.0), material.Shininess) : 0.0;

    vec3 ambient = light.Ambient * baseColor;
    vec3 diffuse = light.Diffuse * diff * baseColor;
    vec3 specular = light.Specular * spec * 0.2;
    return (ambient + diffuse + specular) * attenuation;
}

void main(){
    vec3 TBNviewDir = normalize(TBNViewPos-TBNfragPos);
    vec2 texCoords = ParallaxMapping(TexCoords,TBNviewDir);
    if(texCoords.x>1.0||texCoords.x<0.0||texCoords.y>1.0||texCoords.y<0.0){
        discard;
    }
    vec3 normal = texture(brickNormalMap,texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    vec3 baseColor = texture(material.Diffuse, texCoords).rgb;
    vec3 result = CalcPointLight(pointLight, normal, TBNviewDir, baseColor);

    FragColor = vec4(result, 1.0f);
}
