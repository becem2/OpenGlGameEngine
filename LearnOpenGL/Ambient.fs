#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

struct Light{
    vec3 color;
    vec3 ambient;
    vec3 position;
    
};

struct Material{
    vec3 color;
    vec3 ambient;
    float specularStrength;
    
};

uniform Light light;
uniform Material material;
uniform vec3 viewPos;


void main(){
// ambient
    vec3 ambient = light.ambient * material.ambient;

// diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position-FragPos);
    float diffAngle = max(dot(norm,lightDir),0.0);
    vec3 diffuse = diffAngle * light.color;


// specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir,norm);
    float specAngle = max(dot(viewDir,reflectDir),0.0);
    float spec = pow(specAngle,32.0);
    vec3 specular = material.specularStrength * spec * light.color;


// result
    vec3 result = (ambient + diffuse + specular) * material.color;;
    FragColor = vec4(result, 1.0);

}