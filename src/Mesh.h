#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"

struct Vertex{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture{
    unsigned int id;
    std::string type;
};

class Mesh{
    public:
        //网格数据
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;
        /*构造函数and函数*/
        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
        void Draw(Shader &shader);
    private:
        unsigned int VAO, VBO, EBO;
        void setupMesh();
};

