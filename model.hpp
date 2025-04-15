#ifndef MODEL_H
#define MODEL_H
#define MAX_BONE_INFLUENCE 4
#include <string>
#include <vector>
#include <memory>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glad/glad.h> 
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader.hpp"

enum class Action
{   
    STAND,
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

enum class Orientation
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

std::ostream& operator<<(std::ostream& os, Orientation o);

struct Vertex {
    // position
    glm::vec3 position;
    // normal
    glm::vec3 normal;
    // texCoords
    glm::vec2 texCoords;
    // tangent
    glm::vec3 tangent;
    // bitangent
    glm::vec3 bitangent;
	//bone indexes which will influence this vertex
	int boneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float boneWeights[MAX_BONE_INFLUENCE];
};

struct Material
{   
    std::string name;
    // Specular exponent
    float Ns;
    // Ambient reflectivity
    glm::vec3 Ka;
    // Diffuse reflectivity
    glm::vec3 Kd;
    // Specular reflectivity
    glm::vec3 Ks;
    // IDs of diffuse textures
    std::vector<unsigned int> diffuseTextures;
};

class Mesh 
{
public:
    // mesh data
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    // constructors
    Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, Material* material);
    Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, Material* material);
    Material* material() const;
    // render the mesh
    void draw() const;
private:
    // render data 
    unsigned int VAO, VBO, EBO;
    Material* _material;
};

class Model 
{
public:
    Model() = default;
    Model(
        std::vector<Material>& materials, 
        std::vector<Mesh>& meshes, 
        std::string& directory,
        const glm::mat4& projectionMatrix,
        const glm::mat4& viewMatrix,
        const glm::mat4& modelMatrix
    );
    // constructor, expects a filepath to a 3D model.
    Model(
        const std::string& path,
        const glm::mat4& projectionMatrix,
        const glm::mat4& viewMatrix,
        const glm::mat4& modelMatrix
    );
    virtual ~Model() = default;
    const glm::mat4& projectionMatrix() const;
    void setProjectionMatrix(const glm::mat4& projectionTransform);
    const glm::mat4& viewMatrix() const;
    void setViewMatrix(const glm::mat4& viewTransform);
    const glm::mat4& modelMatrix() const;
    void setModelMatrix(const glm::mat4& modelTransform);
    virtual void translate(const glm::vec3& vector);
    virtual void rotate(float angle, const glm::vec3& axis);

protected:
    std::vector<Material> _materials;
    std::vector<Mesh> _meshes;
    std::string _directory;
    glm::mat4 _projectionMatrix;
    glm::mat4 _viewMatrix;
    glm::mat4 _modelMatrix;

    void _loadModel(const std::string& path);
    std::vector<Material> _setupMaterials(const aiScene* aiscene);
    std::vector<unsigned int> _setupTextures(aiMaterial *mat, aiTextureType type);
};

unsigned int textureFromFile(const char *path, const std::string &directory);
std::vector<Vertex> setupVertices(aiMesh* mesh);
std::vector<unsigned int> setupIndices(aiMesh* mesh);

class Box;

class Forklift: public Model
{
public:
    Forklift() = default;
    Forklift(
        std::vector<Material>& materials, 
        std::vector<Mesh>& meshes, 
        std::string& directory,
        const glm::mat4& projectionMatrix,
        const glm::mat4& viewMatrix,
        const glm::mat4& modelMatrix,
        unsigned int x,
        unsigned int y
    );
    // constructor, expects a filepath to a 3D model.
    Forklift(
        const std::string& path,
        const glm::mat4& projectionMatrix,
        const glm::mat4& viewMatrix,
        const glm::mat4& modelMatrix,
        unsigned int x,
        unsigned int y
    );
    // copy constructor
    Forklift(const Forklift& forklift);
    // copy assignment
    Forklift& operator=(const Forklift& forklift);
    void draw(const Shader& shader, const Shader& boxShader) const;
    void translate(const glm::vec3& vector) override;
    void rotate(float angle, const glm::vec3& axis) override;
    unsigned int x() const;
    void setX(unsigned int x);
    unsigned int y() const;
    void setY(unsigned int y);
    Orientation orientation() const;
    void setOrientation(Orientation orient);
    const std::unique_ptr<Box>& box() const;
    void setBox(std::unique_ptr<Box>& box); 
    void setBox(std::unique_ptr<Box>&& box);
private:
    unsigned int _x;
    unsigned int _y;
    Orientation _orient{Orientation::DOWN};
    std::unique_ptr<Box> _box{nullptr};
};

class Board: public Model
{
public:
    Board() = default;
    Board(        
        std::vector<Material>& materials, 
        std::vector<Mesh>& meshes, 
        std::string& directory,
        const glm::mat4& projectionMatrix,
        const glm::mat4& viewMatrix,
        const glm::mat4& modelMatrix
    );
    // constructors, expects a filepath to a 3D model.
    Board(
        const std::string& path,
        const glm::mat4& projectionMatrix,
        const glm::mat4& viewMatrix,
        const glm::mat4& modelMatrix
    );
    void draw(const Shader& shader) const;
};

class Box: public Model
{
public:
    Box() = default;
    Box(
        std::vector<Material>& materials, 
        std::vector<Mesh>& meshes, 
        std::string& directory,
        const glm::mat4& projectionMatrix,
        const glm::mat4& viewMatrix,
        const glm::mat4& modelMatrix
    );
    // constructor, expects a filepath to a 3D model.
    Box(
        const std::string& path,
        const glm::mat4& projectionMatrix,
        const glm::mat4& viewMatrix,
        const glm::mat4& modelMatrix
    );
    void draw(const Shader& shader) const;
};

// class Cell 
// {
// public:
//     Cell(
//         unsigned int x,
//         unsigned int y,
//         char color,
//         unsigned int target
//     );
//     unsigned int x() const;
//     unsigned int y() const;
//     char color() const;
//     unsigned int target() const;
//     Cell* front() const;
//     Cell* back() const;
//     Cell* left() const;
//     Cell* right() const;
//     Forklift* forklift() const;
//     void setForklift(Forklift* forklift); 
// private:
//     const unsigned int _x;
//     const unsigned int _y;
//     const char _color;
//     const unsigned int _target;
//     Cell* _front{nullptr};
//     Cell* _back{nullptr};
//     Cell* _left{nullptr};
//     Cell* _right{nullptr};
//     Forklift* _forklift{nullptr};
// };

#endif