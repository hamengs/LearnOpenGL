#version 330 core
layout(triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT{
    vec2 texCoords;
    vec3 fragPos;
    vec3 vNormal;
}gs_in[];

uniform float time;
uniform mat4 view;
uniform mat4 projection;

vec3 GetNormal()
{    
    vec3 a = vec3(gl_in[0].gl_Position-gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position-gl_in[1].gl_Position);
    return normalize(cross(a,b));

}

vec4 explode(vec4 position, vec3 normal){
    float magnitude = 2.0;
    vec3 direction = normal *((sin(time)+1.0)/10)*magnitude;
    return position + vec4(direction,0.0);
}

void GenerateLine(int index){
    float mag = 0.2;
    vec3 start = gs_in[index].fragPos;
    vec3 end = start + normalize(gs_in[index].vNormal) * mag;

    gl_Position = projection * view * vec4(start, 1.0);
    EmitVertex();
    gl_Position = projection * view * vec4(end, 1.0);
    EmitVertex();
    EndPrimitive();
}

void main() {    
    GenerateLine(0);
    GenerateLine(1);
    GenerateLine(2);
}
