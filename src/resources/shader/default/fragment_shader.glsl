#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D mer;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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
uniform DirLight dirLight;
uniform PointLight pointLights[100];
uniform int pointLightsAmount;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor, float metallic, float roughness);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor, float metallic, float roughness);

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 baseColor = texture(material.diffuse, TexCoords).rgb;

    float metallic = 0.0;
    float emissive = 0.0;
    float roughness = 1.0;

    vec3 mer = texture(material.mer, TexCoords).rgb;
    metallic = clamp(mer.r, 0.0, 1.0);
    emissive = clamp(mer.g, 0.0, 1.0);
    roughness = clamp(mer.b, 0.05, 1.0);

    vec3 result = CalcDirLight(dirLight, norm, viewDir, baseColor, metallic, roughness);
    for (int i = 0; i < pointLightsAmount; i++)
    result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, baseColor, metallic, roughness);

    result += baseColor * emissive;
    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor, float metallic, float roughness) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(normal, halfwayDir), 0.0);
    float spec = pow(specAngle, mix(1.0, 128.0, 1.0 - roughness));

    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * baseColor * (1.0 - metallic);
    vec3 specular = light.specular * spec * mix(0.04, 1.0, metallic); // Non-metals reflect ~4%

    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor, float metallic, float roughness) {
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
