#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 positionWorld;
layout(location = 2) in vec3 normalWorldSpace;
layout(location = 3) in vec2 fragTexCoord; // texture coordinates

layout(location = 0) out vec4 outColor;

struct PointLight {
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

layout (set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

layout (set = 0, binding = 1) uniform sampler2D textSampler;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix; //[3][3] is the Texture Binding
} push;

void main()
{
    vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 specularLight = vec3(0.0);
    vec3 surfaceNormal = normalize(normalWorldSpace);

    vec3 cameraPosWorld = ubo.invView[3].xyz;
    vec3 viewDir = normalize(cameraPosWorld - positionWorld);

    for (int i=0;i<ubo.numLights;i++) {
        PointLight light = ubo.pointLights[i];
        vec3 directionToLight = light.position.xyz - positionWorld;
        float attenuation = 1.0/dot(directionToLight,directionToLight);
        directionToLight = normalize(directionToLight);

        float cosAngIncidence = max(dot(surfaceNormal,directionToLight),0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosAngIncidence;

        // Specular Lighting
        vec3 halfAngle = normalize(directionToLight + viewDir);
        float blinnTerm = dot(surfaceNormal,halfAngle);
        blinnTerm = clamp(blinnTerm,0,1);
        blinnTerm = pow(blinnTerm,100.0); // shininess - higher is more shiny
        specularLight += blinnTerm * intensity;
    }

    int fragTextureId = int(round(push.normalMatrix[3][3]));
    vec4 tFragColor = vec4(fragColor,1.0);
    if (fragTextureId == 1)
        tFragColor = texture(textSampler,fragTexCoord);  //fragTexCoord is 2D

    outColor = vec4(diffuseLight * tFragColor.xyz + specularLight * tFragColor.xyz,1.0);
}