#include <glad/glad.h>
#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"
#include "Mesh.h"
#include "stb_image.h"


class Model{
    public:
        Model(const std::string &path){
            loadModel(path);
        }
        void Draw(Shader shader);
        unsigned int TextureFromFile(
            const char *path,
            const std::string &directory
        );
    private:
        std::vector<Mesh> meshes;
        std::vector<Texture> textures_loaded;
        std::string directory;
        void loadModel(std::string path);
        void processNode(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);

};