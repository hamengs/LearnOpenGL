#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec3 TBNfragPos;
out vec3 vNormal;
out vec2 TexCoords;
out vec3 TBNViewPos;
out vec3 TBNLightPos;

void main(){

    vec3 T = normalize(vec3(model*vec4(tangent,0.0)));
    vec3 B = normalize(vec3(model*vec4(bitangent,0.0)));
    vec3 N = normalize(vec3(model*vec4(aNormal,0.0)));

    mat3 TBN =transpose(mat3(T,B,N));

    TBNLightPos = TBN * lightPos;
    TBNViewPos = TBN * viewPos;
    TBNfragPos = TBN * vec3(model * vec4(aPos,1.0f));

    mat3 normalMatrix = mat3(transpose(inverse(model)));
    vNormal = normalize(normalMatrix * aNormal);
    TexCoords = aTexCoords;

    gl_Position = projection * view * model * vec4(aPos,1.0f);
}
