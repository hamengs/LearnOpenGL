#version 330 core
out vec4 FragColor;

in VS_OUT{
    vec3 vNormal;
    vec3 fragPos;
}fs_in;

uniform vec3 cameraPos;
uniform samplerCube screenTexture;

void main()
{             
    vec3 I = normalize(fs_in.fragPos - cameraPos);
    vec3 R = reflect(I, normalize(fs_in.vNormal));
    if(gl_FrontFacing){
        FragColor = vec4(texture(screenTexture, R).rgb, 1.0);
    }else{
        FragColor = vec4(0.0,1.0,0.0,1.0);
    }
}