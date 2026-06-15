#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

struct Light {
    vec3 Position;
    vec3 Color;
};

uniform Light lights[4];
uniform sampler2D diffuseTexture;
uniform vec3 viewPos;
uniform bool inverseNormals;
uniform bool useTexture;
uniform float textureScale;
uniform vec3 objectColor;
uniform vec3 emissiveColor;

void main()
{
    vec3 color = objectColor;
    if (useTexture)
    {
        color *= texture(diffuseTexture, TexCoords * textureScale).rgb;
    }

    vec3 normal = normalize(Normal);
    if (inverseNormals)
    {
        normal = -normal;
    }

    vec3 lighting = color * 0.05;
    vec3 viewDir = normalize(viewPos - WorldPos);
    for (int i = 0; i < 4; ++i)
    {
        vec3 lightDir = normalize(lights[i].Position - WorldPos);
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = diff * lights[i].Color;

        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = spec * lights[i].Color * 0.15;

        float distance = length(WorldPos - lights[i].Position);
        float attenuation = 1.0 / (distance * distance);
        lighting += (diffuse + specular) * color * attenuation;
    }

    FragColor = vec4(lighting + emissiveColor, 1.0);
}
