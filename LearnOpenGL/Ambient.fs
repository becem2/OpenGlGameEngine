#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

struct LightActivation{
    bool ambient;
    bool diffuse;
    bool specular;
};

struct Light{
    vec3 color;
    vec3 ambient;
    vec3 position;
    vec3 direction;
    float cutoff;
    float outerCutoff;
    float constant;
    float linear;
    float quadratic;
};

struct Material{
    vec3 color;
    vec3 ambient;
    float specularStrength;
    int shininess;
};

uniform Light light;
uniform Material material;
uniform LightActivation lightActivation;
uniform vec3 viewPos;
uniform int lightType; // 0: directional, 1: point, 2: spotlight

void main(){
    vec3 ambient  = vec3(0.0);
    vec3 diffuse  = vec3(0.0);
    vec3 specular = vec3(0.0);
    vec3 lightDir;
    vec3 viewDir  = normalize(viewPos - FragPos);

    if (lightType==0)       lightDir = normalize(-light.direction);
    else if (lightType==1)  lightDir = normalize(light.position - FragPos);
    else if (lightType==2)  lightDir = normalize(light.position - FragPos);

    vec3 norm = normalize(Normal);

    float diffAngle = max(dot(norm, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, norm);
    float specAngle = max(dot(viewDir, reflectDir), 0.0);
    float spec      = pow(specAngle, material.shininess);

    float distance = length(light.position - FragPos);
    float attenuation = 1.0/(light.constant + (light.linear*distance) + (light.quadratic*distance*distance));

    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutoff - light.outerCutoff;
    float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

    // ambient
    if (lightActivation.ambient){
        if(lightType == 0){
            ambient = light.ambient * material.ambient;
        }
        else if(lightType == 1){
            ambient = light.ambient * material.ambient * attenuation;
        }
        else if(lightType == 2){
            // ambient stays independent of the cone so the object is never pitch black outside the beam
            ambient = light.ambient * material.ambient * attenuation;
        }
    }

    // diffuse
    if (lightActivation.diffuse){
        if(lightType == 0){
            diffuse = diffAngle * light.color;
        }
        else if(lightType == 1){
            diffuse = diffAngle * light.color * attenuation;
        }
        else if(lightType == 2){
            diffuse = diffAngle * light.color * attenuation * intensity;
        }
    }

    // specular
    if (lightActivation.specular){
        if(lightType == 0){
            specular = material.specularStrength * spec * light.color;
        }
        else if(lightType == 1){
            specular = material.specularStrength * spec * light.color * attenuation;
        }
        else if(lightType == 2){
            specular = material.specularStrength * spec * light.color * attenuation * intensity;
        }
    }

    // result
    vec3 result = (ambient + diffuse + specular) * material.color;
    FragColor = vec4(result, 1.0);
}