#include "Model.h"
#include <iostream>

void Model::Draw(Shader shader){
    for(unsigned int i = 0; i < meshes.size(); i++){
        meshes[i].Draw(shader);
    }
}

void Model::loadModel(std::string path){
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path,aiProcess_Triangulate|aiProcess_FlipUVs);
    if(!scene||scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE||!scene->mRootNode){
        std::cout << "ERROR::ASSIMP::"<<importer.GetErrorString()<<std::endl;
        return;
    }
    directory = path.substr(0,path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
}

