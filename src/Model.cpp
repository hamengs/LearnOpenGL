#include "Model.h"
#include <iostream>
#include <algorithm>


void Model::Draw(Shader shader){
    for(unsigned int i = 0; i < meshes.size(); i++){
        meshes[i].Draw(shader);
    }
}

void Model::DrawInstances(Shader shader,int amount){
    for(unsigned int i = 0; i < meshes.size(); i++){
        meshes[i].DrawInstances(shader,amount);
    }
}

void Model::setupInstances(const std::vector<glm::mat4> &data){
     for(unsigned int i = 0; i < meshes.size(); i++){
        meshes[i].setupInstances(data);
    }
}

void Model::loadModel(std::string path){
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path,aiProcess_Triangulate|aiProcess_GenSmoothNormals);
    if(!scene||scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE||!scene->mRootNode){
        std::cout << "ERROR::ASSIMP::"<<importer.GetErrorString()<<std::endl;
        return;
    }
    directory = path.substr(0,path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene){
    for(unsigned int i = 0; i < node->mNumMeshes;i++){
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh,scene));
    }

    for(unsigned int i = 0; i< node->mNumChildren; i++){
        processNode(node->mChildren[i],scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene){
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    glm::vec3 diffuseColor(1.0f);
    bool hasDiffuseTexture = false;
    for(unsigned int i = 0; i < mesh->mNumVertices; i++){
        Vertex vertex;
        //处理顶点
        //Process position
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        //Process normal
        if(mesh->HasNormals()){
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }else{
            vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        //Process
        vertex.TexCoords = glm::vec2(0.0f);
        if(mesh->mTextureCoords[0]){
            glm::vec2 texCoord;
            texCoord.x = mesh->mTextureCoords[0][i].x;
            texCoord.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = texCoord;
        }
        vertices.push_back(vertex);
    } 
    //处理索引
    for(unsigned int i = 0; i < mesh->mNumFaces; i++){
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++){
            indices.push_back(face.mIndices[j]);
        }
    }

    //process materials
    if(mesh->mMaterialIndex>=0){
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material,aiTextureType_DIFFUSE, "Diffuse");
        if(diffuseMaps.empty()){
            diffuseMaps = loadMaterialTextures(material,aiTextureType_BASE_COLOR, "Diffuse");
        }
        hasDiffuseTexture = !diffuseMaps.empty();
        textures.insert(textures.end(),diffuseMaps.begin(),diffuseMaps.end());

        aiColor3D color(1.0f, 1.0f, 1.0f);
        if(material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS){
            diffuseColor = glm::vec3(color.r, color.g, color.b);
        }

        std::vector<Texture> specularMaps = loadMaterialTextures(material,aiTextureType_SPECULAR, "Specular");
        textures.insert(textures.end(),specularMaps.begin(),specularMaps.end());
    }



    return Mesh(vertices,indices,textures,diffuseColor,hasDiffuseTexture);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat,aiTextureType type, std::string typeName){
    std::vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type);i++){
        aiString str;
        mat->GetTexture(type,i, &str);
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded.size(); j++){
            if(textures_loaded[j].path == str.C_Str()){
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if(!skip){
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);            
        }

    }
    return textures;
}

unsigned int Model::TextureFromFile(const char *path, const std::string &directory){
    std::string filename = std::string(path);
    filename = directory + "/" + filename;
    std::replace(filename.begin(), filename.end(), '\\', '/');

    unsigned int textureId;
    glGenTextures(1,&textureId);
    int width,height,nrChannels;
    unsigned char* data = stbi_load(filename.c_str(),&width,&height,&nrChannels,0);
    if(data==NULL){
        std::cout<<"Texture failed to load: "<<filename<<std::endl;
        return 0;
    }
    glBindTexture(GL_TEXTURE_2D,textureId);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    GLenum format = GL_RGB;
    if(nrChannels == 1) format = GL_RED;
    else if(nrChannels == 2) format = GL_RG;
    else if(nrChannels == 3) format = GL_RGB;
    else if (nrChannels == 4) format = GL_RGBA;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D,0,format,width,height,0,format,GL_UNSIGNED_BYTE,data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return textureId;
}
