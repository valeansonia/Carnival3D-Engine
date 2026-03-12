#version 410 core

in vec3 fragmentPos;
in vec3 fragmentNormal;
in vec2 fragmentTexCoords;
in vec3 fragmentColor;
in vec4 fragPosLightSpace;

uniform sampler2D textureUnit;
uniform vec3 viewPos;

uniform int numLights;
uniform vec3 lightPositions[8];
uniform vec3 lightColors[8];
uniform float lightIntensities[8];

vec3 dirLightDir = normalize(vec3(0, 5, 20));
vec3 dirLightColor = vec3(0.0, 0.0, 1.0);

out vec4 FragColor;

uniform sampler2D shadowMap;

vec3 calculateLight(vec3 lightPos, vec3 lightColor, float intensity) {
    vec3 norm = normalize(fragmentNormal);
    vec3 lightDir = normalize(lightPos - fragmentPos);
    vec3 viewDir = normalize(viewPos - fragmentPos);
    
    float ambientStrength = 0.05;
    vec3 ambient = ambientStrength * lightColor * intensity;
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * intensity;
    
    float specularStrength = 0.5;
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;
    
    float distance = length(lightPos - fragmentPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    
    return (ambient + diffuse + specular) * attenuation;
}

vec3 calculateDirLight(vec3 norm, vec3 viewDir, float shadow) {
    vec3 lightDir = normalize(dirLightDir);
    
    vec3 ambient = 0.1 * dirLightColor;
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * dirLightColor;
    
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), 32.0);
    vec3 specular = 0.5 * spec * dirLightColor;
    
    return (ambient + (1.0 - shadow) * (diffuse + specular));
}

float calculateFog(float fogDensity) {
    float distance = length(viewPos - fragmentPos);
    float fogFactor = exp(-pow(distance * fogDensity, 2.0));

    return clamp(fogFactor, 0.0, 1.0);
}

float ShadowCalculation()
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{
    vec4 texColorRGBA = texture(textureUnit, fragmentTexCoords);
    
    if(texColorRGBA.a < 0.1) discard; //elimina pixelii invizibili

    vec3 lighting = vec3(0.0);
    
    for(int i = 0; i < numLights; i++) {
        lighting += calculateLight(lightPositions[i], lightColors[i], lightIntensities[i]);
    }
    float shadow = ShadowCalculation();
    vec3 dirLighting = calculateDirLight(normalize(fragmentNormal), normalize(viewPos - fragmentPos), shadow);
    lighting += dirLighting;

    vec3 finalColor = texColorRGBA.rgb * lighting;
    
    finalColor = finalColor / (finalColor + vec3(1.0));
    finalColor = pow(finalColor, vec3(1.0/2.2));
    float fog = calculateFog(0.05);
    finalColor = mix(vec3(1.0,0.0, 0.0), finalColor, fog);
    FragColor = vec4(finalColor, 1.0);
}