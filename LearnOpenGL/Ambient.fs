#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct LightActivation { 
    bool ambient; 
    bool diffuse; 
    bool specular; 
};

struct Light {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 position;
    vec3 direction;
    float cutoff;
    float outerCutoff;
    float constant;
    float linear;
    float quadratic;
};

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    vec3 color;
    float shininess;
};

uniform Light light;
uniform Material material;
uniform LightActivation lightActivation;
uniform vec3 viewPos;
uniform int lightType; // 0: directional, 1: point, 2: spotlight

void main()
{
    vec3 norm     = normalize(Normal);
    vec3 viewDir  = normalize(viewPos - FragPos);
    vec3 lightDir = (lightType == 0) ? normalize(-light.direction) : normalize(light.position - FragPos);

    float attenuation = 1.0;
    if (lightType != 0) {
        float d = length(light.position - FragPos);
        attenuation = 1.0 / (light.constant + light.linear * d + light.quadratic * d * d);
    }

    float intensity = 1.0;
    if (lightType == 2) {
        float theta   = dot(lightDir, normalize(-light.direction));
        float epsilon = max(light.cutoff - light.outerCutoff, 0.0001);
        intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
    }

    vec3 diffTex = texture(material.diffuse,  TexCoords).rgb;
    vec3 specTex = texture(material.specular, TexCoords).rgb;

    vec3 ambient = vec3(0.0), diffuse = vec3(0.0), specular = vec3(0.0);

    if (lightActivation.ambient)
        ambient = light.ambient * diffTex * attenuation;

    if (lightActivation.diffuse) {
        float diff = max(dot(norm, lightDir), 0.0);
        diffuse = light.diffuse * diff * diffTex * attenuation * intensity;
    }

    if (lightActivation.specular) {
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        specular = light.specular * spec * specTex * attenuation * intensity;
    }

    FragColor = vec4((ambient + diffuse + specular) * material.color, 1.0);
}