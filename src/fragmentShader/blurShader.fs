#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform bool horizontal;

void main(){
    vec2 texelSize = 1.0 / vec2(textureSize(image, 0));
    float weight[5] = float[](0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f);

    vec3 result = texture(image, TexCoords).rgb * weight[0];
    for(int i = 1; i < 5; i++){
        vec2 offset = horizontal
            ? vec2(texelSize.x * i, 0.0f)
            : vec2(0.0f, texelSize.y * i);
        result += texture(image, TexCoords + offset).rgb * weight[i];
        result += texture(image, TexCoords - offset).rgb * weight[i];
    }

    FragColor = vec4(result, 1.0f);
}
