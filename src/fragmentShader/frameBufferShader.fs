#version 330 

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D hdrColorTexture;
uniform sampler2D bloomColorTexture;
uniform float exposure;

void main()
{   
    
    vec3 result = texture(hdrColorTexture,TexCoords).rgb;
    result += texture(bloomColorTexture,TexCoords).rgb;
    result = vec3(1.0f)-exp(-result*exposure);
    result = pow(result,vec3(1.0f/2.2f));
    FragColor = vec4(result,1.0f);
}
