#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures){
    this->vertices = vertices;
    this->indices = indices;
    this->textures =  textures;
    this->diffuseColor = glm::vec3(1.0f);
    this->hasDiffuseTexture = !textures.empty();

    setupMesh();
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, glm::vec3 diffuseColor, bool hasDiffuseTexture){
    this->vertices = vertices;
    this->indices = indices;
    this->textures =  textures;
    this->diffuseColor = diffuseColor;
    this->hasDiffuseTexture = hasDiffuseTexture;

    setupMesh();
}


void Mesh::DrawInstances(Shader &shader,int amount){

    for(int i = 0; i<textures.size();i++){
        glActiveTexture(GL_TEXTURE0+i);
        glBindTexture(GL_TEXTURE_2D,textures[i].id);
        shader.setInt(("material."+textures[i].type).c_str(),i);
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(VAO);
   
    glDrawElementsInstanced(GL_TRIANGLES,indices.size(),GL_UNSIGNED_INT,0,amount);
    glBindVertexArray(0);
}

void Mesh::Draw(Shader &shader){
    bool hasSpecular = false;
    for(int i = 0; i<textures.size();i++){
        if(textures[i].type == "Specular"){
            hasSpecular = true;
        }
        glActiveTexture(GL_TEXTURE0+i);
        glBindTexture(GL_TEXTURE_2D,textures[i].id);
        shader.setInt(("material."+textures[i].type).c_str(),i);
    }
    glActiveTexture(GL_TEXTURE0);
    shader.setBool("material.HasDiffuseTexture", hasDiffuseTexture);
    shader.setVec3("material.DiffuseColor", diffuseColor);
    shader.setBool("material.HasSpecular", hasSpecular);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES,indices.size(),GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
}

void Mesh::setupMesh(){
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(Vertex),&vertices[0],GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(unsigned int),&indices[0],GL_STATIC_DRAW);

    //顶点位置
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
    glEnableVertexAttribArray(0);
    //顶点法线
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(offsetof(Vertex,Normal)));
    glEnableVertexAttribArray(1);
    //材质坐标
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(offsetof(Vertex,TexCoords)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

}

void Mesh::setupInstances(const std::vector<glm::mat4> &data){
    glBindVertexArray(VAO);

    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER,instanceVBO);
    glBufferData(GL_ARRAY_BUFFER,data.size()*sizeof(glm::mat4),data.data(),GL_STATIC_DRAW);

    glVertexAttribPointer(3,4,GL_FLOAT,GL_FALSE,sizeof(glm::mat4),(void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4,4,GL_FLOAT,GL_FALSE,sizeof(glm::mat4),(void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(5,4,GL_FLOAT,GL_FALSE,sizeof(glm::mat4),(void*)(sizeof(glm::vec4)*2));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(6,4,GL_FLOAT,GL_FALSE,sizeof(glm::mat4),(void*)(sizeof(glm::vec4)*3));
    glEnableVertexAttribArray(6);

    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
}
