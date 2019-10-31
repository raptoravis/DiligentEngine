#pragma once

#include "../engine.h"

struct aiMaterial;
struct aiMesh;
struct aiNode;

namespace ade
{

class Material;
class Buffer;
class Mesh;
class Texture;
class Camera;
class SceneNode;

namespace fs = std::filesystem;

// A model base class.
// Implements a basic model loader using Assimp.
class SceneAss : public Scene
{
  public:
    typedef Scene base;

    virtual std::shared_ptr<SceneNode> GetRootNode() const;
    virtual bool LoadFromFile(const std::wstring& fileName);
    virtual bool LoadFromString(const std::string& scene, const std::string& format);

    SceneAss();
    virtual ~SceneAss();

    static std::shared_ptr<SceneAss> CreateScene();
    static std::shared_ptr<Scene>
        CreatePlane(float size, const Diligent::float3& N = Diligent::float3(0, 1, 0));
    static std::shared_ptr<Scene> CreateScreenQuad(float left = -1.0f, float right = 1.0f,
                                                   float bottom = -1.0f, float top = 1.0f,
                                                   float z = 0.0f);
    static std::shared_ptr<Scene> CreateSphere(float radius, float tesselation = 4);
    static std::shared_ptr<Scene> CreateCube(float size);
    static std::shared_ptr<Scene>
        CreateCylinder(float baseRadius, float apexRadius, float height,
                       const Diligent::float3& axis = Diligent::float3(0, 1, 0));
    static std::shared_ptr<Scene> CreateCone(float baseRadius, float height);
    static std::shared_ptr<Scene>
        CreateArrow(const Diligent::float3& tail = Diligent::float3(0, 0, 0),
                    const Diligent::float3& head = Diligent::float3(0, 0, 1), float radius = 0.05f);
    static std::shared_ptr<Scene> CreateAxis(float radius = 0.05f, float length = 0.5f);
    static void DestroyScene(std::shared_ptr<Scene> scene);

  protected:
    friend class ProgressHandler;

    std::shared_ptr<Buffer> CreateVertexBufferFloat(const float* data, uint32_t count,
                                                    uint32_t stride);
    std::shared_ptr<Buffer> CreateIndexBufferUInt(const uint32_t* data, uint32_t count);

    std::shared_ptr<Mesh> CreateMesh();
    std::shared_ptr<Material> CreateMaterial();
    std::shared_ptr<Texture> CreateTexture(const std::wstring& fileName);
    std::shared_ptr<Texture> CreateTexture2D(uint16_t width, uint16_t height);

    // virtual std::shared_ptr<Texture> GetDefaultTexture() = 0;

  protected:
    typedef std::map<std::string, std::shared_ptr<Material>> MaterialMap;
    typedef std::vector<std::shared_ptr<Material>> MaterialList;
    typedef std::vector<std::shared_ptr<Mesh>> MeshList;

    MaterialMap m_MaterialMap;
    MaterialList m_Materials;
    MeshList m_Meshes;

    void ImportMaterial(const aiMaterial& material, fs::path parentPath);
    void ImportMesh(const aiMesh& mesh);
    std::shared_ptr<SceneNode> ImportSceneNode(std::shared_ptr<SceneNode> parent, aiNode* aiNode);

    std::wstring m_SceneFile;
};

}    // namespace ade