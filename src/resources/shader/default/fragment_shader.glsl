#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D mer;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform PointLight pointLights[20];
uniform int pointLightsAmount;

vec3 CalcPointLight(int index, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor, float metallic, float roughness);

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec4 baseColor1 = texture(material.diffuse, TexCoords);
    if(baseColor1.a < 0.1) discard;
    vec3 baseColor = baseColor1.rgb;

    vec3 result = vec3(0.0);

    vec3 mer = texture(material.mer, TexCoords).rgb;
    float metallic = clamp(mer.r, 0.0, 1.0);
    float emissive = clamp(mer.g, 0.0, 1.0);
    float roughness = clamp(mer.b, 0.05, 1.0);

    for (int i = 0; i < pointLightsAmount; i++) {
        result += CalcPointLight(i, norm, FragPos, viewDir, baseColor, metallic, roughness);
    }

    result += baseColor * emissive;

    FragColor = vec4(result, 1.0);
}


vec3 CalcPointLight(int index, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor, float metallic, float roughness) {
    PointLight light = pointLights[index];
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(normal, halfwayDir), 0.0);
    float spec = pow(specAngle, mix(1.0, 128.0, 1.0 - roughness));

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * baseColor * (1.0 - metallic);
    vec3 specular = light.specular * spec * mix(0.04, 1.0, metallic);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}
