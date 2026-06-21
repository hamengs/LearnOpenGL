#version 330 core
out float FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 projection;
uniform mat4 view;
uniform vec2 noiseScale;

const int kernelSize = 64;
const float radius = 0.5;
const float bias = 0.025;

void main(){
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float fragLinearDepth = -(view * vec4(fragPos, 1.0)).z;
    float occlusion = 0.0;

    for(int i = 0; i < kernelSize; i++){
        vec3 sampleDir = TBN * samples[i];
        vec3 sampleWorld = fragPos + sampleDir * radius;
        vec3 sampleView = vec3(view * vec4(sampleWorld, 1.0));
        float sampleLinearDepth = -sampleView.z;

        vec4 offset = projection * vec4(sampleView, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        if(offset.x < 0.0 || offset.x > 1.0 || offset.y < 0.0 || offset.y > 1.0){
            continue;
        }

        vec3 sampledFragPos = texture(gPosition, offset.xy).xyz;
        float sampledLinearDepth = -(view * vec4(sampledFragPos, 1.0)).z;
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragLinearDepth - sampledLinearDepth));
        occlusion += (sampledLinearDepth <= sampleLinearDepth - bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / kernelSize);
    FragColor = occlusion;
}
