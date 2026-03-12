#version 410 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexColor;
layout (location = 3) in vec2 vertexTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform mat4 lightSpace;

out vec3 fragmentPos;
out vec3 fragmentNormal;
out vec2 fragmentTexCoords;
out vec3 fragmentColor;
out vec4 fragPosLightSpace;

void main(){
    fragPosLightSpace = lightSpace * vec4(vertexPosition, 1.0);
    fragmentPos = vec3(model * vec4(vertexPosition, 1.0));
    fragmentNormal = normalize(normalMatrix * vertexNormal);
    fragmentTexCoords = vertexTexCoords;
    fragmentColor = vertexColor;    
    gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
}