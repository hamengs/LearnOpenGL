#include <glm/glm.hpp>

class Material{
    public:
        glm::vec3 Ambient;
        glm::vec3 Diffusion;
        glm::vec3 Specular;
        float Shiness; 

        Material(glm::vec3 ambient, glm::vec3 diffusion, glm::vec3 specular, float shiness){
        Ambient = ambient;
        Diffusion = diffusion;
        Specular = specular;
        Shiness = shiness;
    }
};

