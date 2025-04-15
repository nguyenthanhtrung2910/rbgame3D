#include <string>
#include <iostream>
#include <vector>
#include <cmath>
#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "stb_image.h"
#include "shader.hpp"
#include "model.hpp"

std::ostream& operator<<(std::ostream& os, Orientation o) 
{
    switch (o) {
        case Orientation::UP:   return os << "UP";
        case Orientation::DOWN: return os << "DOWN";
        case Orientation::LEFT:  return os << "LEFT";
        case Orientation::RIGHT:  return os << "RIGHT";
    }
    return os << "Unknown";
}

// copy from arguments
Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, Material* material)
: vertices{vertices}
, indices{indices}
, _material{material}
{
    // create buffers
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->EBO);
    // load data into vertex buffers
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);  
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), &this->indices[0], GL_STATIC_DRAW);
    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneIDs));
    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, boneWeights));
    glBindVertexArray(0);
}

// move from arguments
Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, Material* material)
: vertices{std::move(vertices)}
, indices{std::move(indices)}
, _material{material}
{
    // create buffers
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->EBO);
    // load data into vertex buffers
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);  
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), &this->indices[0], GL_STATIC_DRAW);
    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneIDs));
    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, boneWeights));
    glBindVertexArray(0);
}

Material* Mesh::material() const
{
    return this->_material;
};

void Mesh::draw() const
{           
    // draw mesh
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(this->indices.size()), GL_UNSIGNED_INT, 0);
    // always good practice to set everything back to defaults once configured.
    glBindVertexArray(0);
};

Model::Model(
    std::vector<Material>& materials, 
    std::vector<Mesh>& meshes, 
    std::string& directory,
    const glm::mat4& projectionMatrix,
    const glm::mat4& viewMatrix,
    const glm::mat4& modelMatrix
)
: _materials{materials}
, _meshes{meshes}
, _directory{directory}
, _projectionMatrix{projectionMatrix}
, _viewMatrix{viewMatrix}
, _modelMatrix{modelMatrix}
{
};

Model::Model(
    const std::string& path,
    const glm::mat4& projectionMatrix,
    const glm::mat4& viewMatrix,
    const glm::mat4& modelMatrix
)
: _projectionMatrix{projectionMatrix}
, _viewMatrix{viewMatrix}
, _modelMatrix{modelMatrix}
{
    _loadModel(path);
};

const glm::mat4& Model::projectionMatrix() const
{
    return this->_projectionMatrix;
};

void Model::setProjectionMatrix(const glm::mat4& projectionTransform)
{
    this->_projectionMatrix = projectionTransform;
};

const glm::mat4& Model::viewMatrix() const
{
    return this->_viewMatrix;
};

void Model::setViewMatrix(const glm::mat4& viewTransform)
{
    this->_viewMatrix = viewTransform;
};

const glm::mat4& Model::modelMatrix() const
{
    return this->_modelMatrix;
};

void Model::setModelMatrix(const glm::mat4& modelTransform)
{
    this->_modelMatrix = modelTransform;
};

void Model::translate(const glm::vec3& vector)
{
    this->_modelMatrix = glm::translate(this->_modelMatrix, vector);
}
void Model::rotate(float angle, const glm::vec3& axis)
{
    this->_modelMatrix = glm::rotate(this->_modelMatrix, angle, axis);
}

void Model::_loadModel(const std::string& path)
{
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    };
    // retrieve the directory path of the filepath
    this->_directory = path.substr(0, path.find_last_of('/'));
    this->_materials = this->_setupMaterials(scene);
    for(unsigned int i = 0; i < scene->mNumMeshes; i++)
    {   
        aiMesh* mesh{scene->mMeshes[i]};
        _meshes.emplace_back(setupVertices(mesh), setupIndices(mesh), &this->_materials[mesh->mMaterialIndex]);
    };
};

std::vector<Material> Model::_setupMaterials(const aiScene* aiscene)
{   
    // data to fill
    std::vector<Material> materials(aiscene->mNumMaterials);
    for (unsigned int i=0; i<aiscene->mNumMaterials; ++i)
    {
        Material* material{&materials[i]};
        aiMaterial* aiMaterial{aiscene->mMaterials[i]};
        aiColor3D ambient;
        aiColor3D diffuse;
        aiColor3D specular;
        float shininess;
        material->name = aiMaterial->GetName().C_Str();
        // Ambient color
        if (aiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambient) == AI_SUCCESS) {
            material->Ka.x=ambient.r;
            material->Ka.y=ambient.b;
            material->Ka.z=ambient.g;
        };
        // Diffuse color
        if (aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS) {
            material->Kd.x=diffuse.r;
            material->Kd.y=diffuse.b;
            material->Kd.z=diffuse.g;
        };
        // Specular color
        if (aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specular) == AI_SUCCESS) {
            material->Ks.x=specular.r;
            material->Ks.y=specular.b;
            material->Ks.z=specular.g;
        };
        // Shininess (Specular exponent)
        if (aiMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
            material->Ns=shininess;
        };
        // Diffuse textures
        material->diffuseTextures=this->_setupTextures(aiMaterial, aiTextureType_DIFFUSE);
    };
    return materials;
};

std::vector<unsigned int> Model::_setupTextures(aiMaterial *mat, aiTextureType type)
{
    std::vector<unsigned int> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        textures.push_back(textureFromFile(str.C_Str(), this->_directory));
    };
    return textures;
};

std::vector<Vertex> setupVertices(aiMesh* mesh)
{
    // data to fill
    std::vector<Vertex> vertices(mesh->mNumVertices);
    // walk through each of the mesh's vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex* vertex{&vertices[i]};
        // positions
        aiVector3D* aiVertices{mesh->mVertices};
        vertex->position.x = aiVertices[i].x;
        vertex->position.y = aiVertices[i].y;
        vertex->position.z = aiVertices[i].z;
        // normals
        if (mesh->HasNormals())
        {
            aiVector3D* aiNormals{mesh->mNormals};
            vertex->normal.x = aiNormals[i].x;
            vertex->normal.y = aiNormals[i].y;
            vertex->normal.z = aiNormals[i].z;
        };
        // texture coordinates
        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            aiVector3D* aiTextureCoords_0{mesh->mTextureCoords[0]};
            vertex->texCoords.x = aiTextureCoords_0[i].x;
            vertex->texCoords.y = aiTextureCoords_0[i].y;
            // tangent
            aiVector3D* aiTangent{mesh->mTangents};
            vertex->tangent.x = aiTangent[i].x;
            vertex->tangent.y = aiTangent[i].y;
            vertex->tangent.z = aiTangent[i].z;
            // bitangent
            aiVector3D* aiBitangent{mesh->mBitangents};
            vertex->bitangent.x = aiBitangent[i].x;
            vertex->bitangent.y = aiBitangent[i].y;
            vertex->bitangent.z = aiBitangent[i].z;
        };
    };
    return vertices;
};

std::vector<unsigned int> setupIndices(aiMesh* mesh)
{
    std::vector<unsigned int> indices;
    for(unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace* face = mesh->mFaces;
        for(unsigned int j = 0; j < face[i].mNumIndices; ++j)
            indices.push_back(face[i].mIndices[j]);        
    };
    return indices;
};

unsigned int textureFromFile(const char *path, const std::string& directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    };

    return textureID;
};

Forklift::Forklift(
    std::vector<Material>& materials, 
    std::vector<Mesh>& meshes, 
    std::string& directory,
    const glm::mat4& projectionMatrix,
    const glm::mat4& viewMatrix,
    const glm::mat4& modelMatrix,
    unsigned int x,
    unsigned int y,
    glm::vec3 color
)
: Model(materials, meshes, directory, projectionMatrix, viewMatrix, modelMatrix)
, _x{x}
, _y{y}
, _color{color}
{
}

Forklift::Forklift(
    const std::string& path,
    const glm::mat4& projectionMatrix,
    const glm::mat4& viewMatrix,
    const glm::mat4& modelMatrix,
    unsigned int x,
    unsigned int y,
    glm::vec3 color
)
: Model(path, projectionMatrix, viewMatrix, modelMatrix)
, _x{x}
, _y{y}
, _color{color}
{
    
}

Forklift::Forklift(const Forklift& forklift)
: Model{forklift}
, _x{forklift._x}
, _y{forklift._y}
, _orient{forklift._orient}
, _color{forklift._color}
{
    if (forklift._box) {
        this->_box = std::make_unique<Box>(*forklift._box);
    } else {
        this->_box = nullptr;  
    }
}
Forklift& Forklift::operator=(const Forklift& forklift)
{
    if (this != &forklift) {
        Model::operator=(forklift);
        this->_x = forklift._x;
        this->_y = forklift._y;
        this->_orient = forklift._orient;
        this->_color = forklift._color;
        if (forklift._box) {
            this->_box = std::make_unique<Box>(*forklift._box);
        } else {
            this->_box = nullptr;  
        }
    }
    return *this;
}

void Forklift::setX(unsigned int x)
{
    this->_x = x;
}

void Forklift::setY(unsigned int y)
{
    this->_y = y;
}

unsigned int Forklift::x() const
{
    return this->_x;
}

unsigned int Forklift::y() const
{
    return this->_y;
}

Orientation Forklift::orientation() const
{
    return this->_orient;
}

const glm::vec3& Forklift::color() const
{
    return this->_color;
}

void Forklift::setOrientation(Orientation orient)
{
    this->_orient = orient;
}

const std::unique_ptr<Box>& Forklift::box() const
{
    return this->_box;
}

void Forklift::setBox(std::unique_ptr<Box>& box)
{
    if (box) {
        this->_box = std::make_unique<Box>(*box);
    } else {
        this->_box = nullptr;  
    }
}

void Forklift::setBox(std::unique_ptr<Box>&& box)
{
    this->_box = std::move(box);
}

void Forklift::translate(const glm::vec3& vector)
{
    this->_modelMatrix = glm::translate(this->_modelMatrix, vector);
    if (this->_box)
        this->_box->translate(vector);
}

void Forklift::rotate(float angle, const glm::vec3& axis)
{
    this->_modelMatrix = glm::rotate(this->_modelMatrix, angle, axis);
    if (this->_box)
        this->_box->rotate(angle, axis);
}

void Forklift::draw(const Shader& shader, const Shader& boxShader) const
{
    shader.use();
    shader.setMat4("projection", this->_projectionMatrix);
    shader.setMat4("view", this->_viewMatrix);
    shader.setMat4("model", this->_modelMatrix);
    for(const Mesh& mesh: this->_meshes)
    {
        shader.setVec3("material.ambient", mesh.material()->Kd.x, mesh.material()->Kd.y, mesh.material()->Kd.z);
        if (mesh.material()->name == "geel1")
            shader.setVec3("material.diffuse", this->_color.x, this->_color.y, this->_color.z);
        else
            shader.setVec3("material.diffuse", mesh.material()->Kd.x, mesh.material()->Kd.y, mesh.material()->Kd.z);
        shader.setVec3("material.specular", mesh.material()->Kd.x, mesh.material()->Kd.y, mesh.material()->Kd.z);
        shader.setFloat("material.shininess", mesh.material()->Ns);
        shader.setVec3("light.ambient", 1.0, 1.0, 1.0);
        shader.setVec3("light.diffuse", 1.0, 1.0, 1.0);
        shader.setVec3("light.specular", 1.0, 1.0, 1.0);
        mesh.draw();
    }
    if (this->_box)
        this->_box->draw(boxShader);
}

Board::Board(
    std::vector<Material>& materials, 
    std::vector<Mesh>& meshes, 
    std::string& directory,
    const glm::mat4& projectionMatrix,
    const glm::mat4& viewMatrix,
    const glm::mat4& modelMatrix
)
: Model(materials, meshes, directory, projectionMatrix, viewMatrix, modelMatrix)
{
}

Board::Board(
    const std::string& path,
    const glm::mat4& projectionMatrix,
    const glm::mat4& viewMatrix,
    const glm::mat4& modelMatrix
)
: Model(path, projectionMatrix, viewMatrix, modelMatrix)
{
}

void Board::draw(const Shader& shader) const
{
    shader.use();
    shader.setMat4("projection", this->_projectionMatrix);
    shader.setMat4("view", this->_viewMatrix);
    shader.setMat4("model", this->_modelMatrix);
    for(const Mesh& mesh: this->_meshes)
    {   
        for(unsigned int i = 0; i < mesh.material()->diffuseTextures.size(); ++i)
        {
            // active proper texture unit before binding
            glActiveTexture(GL_TEXTURE0 + i); 
            // now set the sampler to the correct texture unit
            shader.setInt("texture_diffuse"+std::to_string(i+1), i);
            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, mesh.material()->diffuseTextures[i]);
        };
        mesh.draw();
        glActiveTexture(GL_TEXTURE0);
    }
}

Box::Box(
    std::vector<Material>& materials, 
    std::vector<Mesh>& meshes, 
    std::string& directory,
    const glm::mat4& projectionMatrix,
    const glm::mat4& viewMatrix,
    const glm::mat4& modelMatrix
)
: Model(materials, meshes, directory, projectionMatrix, viewMatrix, modelMatrix)
{
}

Box::Box(
    const std::string& path,
    const glm::mat4& projectionMatrix,
    const glm::mat4& viewMatrix,
    const glm::mat4& modelMatrix
)
: Model(path, projectionMatrix, viewMatrix, modelMatrix)
{
}

void Box::draw(const Shader& shader) const
{
    shader.use();
    shader.setMat4("projection", this->_projectionMatrix);
    shader.setMat4("view", this->_viewMatrix);
    shader.setMat4("model", this->_modelMatrix);
    for(const Mesh& mesh: this->_meshes)
    {   
        for(unsigned int i = 0; i < mesh.material()->diffuseTextures.size(); ++i)
        {
            // active proper texture unit before binding
            glActiveTexture(GL_TEXTURE0 + i); 
            // now set the sampler to the correct texture unit
            shader.setInt("texture_diffuse"+std::to_string(i+1), i);
            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, mesh.material()->diffuseTextures[i]);
        };
        mesh.draw();
        glActiveTexture(GL_TEXTURE0);
    }
}