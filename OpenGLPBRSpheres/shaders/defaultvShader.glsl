

#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;


uniform mat4 cameraMatrix;
uniform mat4 worldMatrix;


out vec3 fragPosition;
out vec2 uv;
out vec3 normal;

void main()
{
    fragPosition= vec3(worldMatrix*vec4(position,1.0));
    uv=in_uv;

    
    normal = normalize( mat3(worldMatrix) * in_normal);

    gl_Position = cameraMatrix * worldMatrix * vec4(position, 1.0);

    


}  