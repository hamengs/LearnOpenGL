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
    std::string path;
};

class Mesh{
    public:
        //网格数据
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;
        int amount;
        std::vector<glm::mat4> data;

        /*构造函数and函数*/
        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
        void Draw(Shader &shader);
        void DrawInstances(Shader &shader,int amount);
        void setupInstances(const std::vector<glm::mat4> &data);
    private:
        unsigned int VAO, VBO, EBO;
        void setupMesh();
        
};

