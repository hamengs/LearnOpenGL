#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 offset;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightMatrices;

out vec3 fragPos;
out vec3 vNormal;
out vec2 TexCoords;
out vec4 lightSpacePosition;

out VS_OUT {
    vec2 texCoords;
    vec3 fragPos;
    vec3 vNormal;
} vs_out;

void main(){

    mat4 finalModel = model * offset;

    vs_out.fragPos = vec3(model * vec4(aPos,1.0f));
    fragPos = vec3(model * vec4(aPos,1.0f));

    mat3 normalMatrix = mat3(transpose(inverse(model)));
    vs_out.vNormal = normalize(normalMatrix * aNormal);
    vNormal = normalize(normalMatrix * aNormal);

    vs_out.texCoords = aTexCoords;
    TexCoords = aTexCoords;

    lightSpacePosition = lightMatrices * model * vec4(aPos,1.0f);

    gl_PointSize = gl_Position.z;
    gl_Position = projection * view * model * vec4(aPos,1.0f);
}