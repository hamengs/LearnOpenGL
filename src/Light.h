#include <glm/glm.hpp>

class Light{
    public:
        glm::vec3 Position;
        glm::vec3 Ambient;
        glm::vec3 Diffusion;
        glm::vec3 Specular;

    Light(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffusion, glm::vec3 specular){
        Position = position;
        Ambient = ambient;
        Diffusion = diffusion;
        Specular = specular;
    }
};