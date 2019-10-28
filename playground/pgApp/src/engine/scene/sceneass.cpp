#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>
#include <assimp/ProgressHandler.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "sceneass.h"

#define EXPORT_FORMAT "assbin"
#define EXPORT_EXTENSION "assbin"

namespace ade
{

// A private class that is registered with Assimp's importer
// Provides feedback on the loading progress of the scene files.
//

class ProgressHandler : public Assimp::ProgressHandler
{
  public:
    ProgressHandler(SceneAss& scene, const std::wstring& fileName)
        : m_Scene(scene), m_FileName(fileName)
    {
    }

    virtual bool Update(float percentage) { return false; }

  private:
    SceneAss& m_Scene;
    std::wstring m_FileName;
};

SceneAss::SceneAss() : base()
{
    //
}

SceneAss::~SceneAss() {}

bool SceneAss::LoadFromString(const std::string& sceneStr, const std::string& format)
{
    Assimp::Importer importer;
    const aiScene* scene = nullptr;

    importer.SetProgressHandler(new ProgressHandler(*this, L"String"));

    importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
                                aiPrimitiveType_POINT | aiPrimitiveType_LINE);

    uint32_t preprocessFlags = aiProcessPreset_TargetRealtime_MaxQuality;

    scene = importer.ReadFileFromMemory(sceneStr.data(), sceneStr.size(), preprocessFlags,
                                        format.c_str());

    if (!scene) {
        ReportError(importer.GetErrorString());
        return false;
    } else {
        // If we have a previously loaded scene, delete it.
        if (m_pRootNode) {
            m_pRootNode.reset();
        }

        // Import scene materials.
        for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
            ImportMaterial(*scene->mMaterials[i], fs::current_path());
        }
        // Import meshes
        for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
            ImportMesh(*scene->mMeshes[i]);
        }

        m_pRootNode = ImportSceneNode(m_pRootNode, scene->mRootNode);
    }

    return true;
}

bool SceneAss::LoadFromFile(const std::wstring& fileName)
{
    fs::path filePath(fileName);
    fs::path parentPath;

    m_SceneFile = fileName;

    if (filePath.has_parent_path()) {
        parentPath = filePath.parent_path();
    } else {
        parentPath = fs::current_path();
    }

    const aiScene* scene;
    Assimp::Importer importer;

    importer.SetProgressHandler(new ProgressHandler(*this, fileName));

    fs::path exportPath = filePath;
    exportPath.replace_extension(EXPORT_EXTENSION);

    if (fs::exists(exportPath) && fs::is_regular_file(exportPath)) {
        // If a previously exported file exists, load that instead (scene has already been
        // preprocessed).
        scene = importer.ReadFile(exportPath.string(), 0);
    } else {
        // If no serialized version of the model file exists (or the original model is newer than
        // the serialized version), reimport the original scene and export it as binary.
        importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);
        importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
                                    aiPrimitiveType_POINT | aiPrimitiveType_LINE);

        uint32_t preprocessFlags =
            aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_OptimizeGraph;
        scene = importer.ReadFile(filePath.string(), preprocessFlags);

        if (scene) {
            // Now export the preprocessed scene so we can load it faster next time.
            Assimp::Exporter exporter;
            exporter.Export(scene, EXPORT_FORMAT, exportPath.string(), preprocessFlags);
        }
    }

    if (!scene) {
        ReportError(importer.GetErrorString());
        return false;
    } else {
        // If we have a previously loaded scene, delete it.
        Diligent::float4x4 localTransform = Diligent::float4x4::Identity();
        if (m_pRootNode) {
            // Save the root nodes local transform
            // so it can be restored on reload.
            localTransform = m_pRootNode->GetLocalTransform();
            m_pRootNode.reset();
        }
        // Delete the previously loaded assets.
        m_MaterialMap.clear();
        m_Materials.clear();
        m_Meshes.clear();

        // Import scene materials.
        for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
            ImportMaterial(*scene->mMaterials[i], parentPath);
        }
        // Import meshes
        for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
            ImportMesh(*scene->mMeshes[i]);
        }

        m_pRootNode = ImportSceneNode(m_pRootNode, scene->mRootNode);
        m_pRootNode->SetLocalTransform(localTransform);
    }

    return true;
}

void SceneAss::ImportMaterial(const aiMaterial& material, fs::path parentPath)
{
    aiString materialName;
    aiString aiTexturePath;
    aiTextureOp aiBlendOperation;
    float blendFactor;
    aiColor4D diffuseColor;
    aiColor4D specularColor;
    aiColor4D ambientColor;
    aiColor4D emissiveColor;
    float opacity;
    float indexOfRefraction;
    float reflectivity;
    float shininess;
    float bumpIntensity;

    // if ( material.Get( AI_MATKEY_NAME, materialName ) == aiReturn_SUCCESS )
    //{
    //    MaterialMap::iterator itr = m_MaterialMap.find( materialName.C_Str() );
    //    if ( itr != m_MaterialMap.end() )
    //    {
    //        // A material with this name already exists in our materials array.
    //        // Skip it.
    //        return;
    //    }
    //}

    std::shared_ptr<Material> pMaterial = CreateMaterial();

    if (material.Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == aiReturn_SUCCESS) {
        pMaterial->SetAmbientColor(
            Diligent::float4(ambientColor.r, ambientColor.g, ambientColor.b, ambientColor.a));
    }
    if (material.Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) == aiReturn_SUCCESS) {
        pMaterial->SetEmissiveColor(
            Diligent::float4(emissiveColor.r, emissiveColor.g, emissiveColor.b, emissiveColor.a));
    }
    if (material.Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == aiReturn_SUCCESS) {
        pMaterial->SetDiffuseColor(
            Diligent::float4(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a));
    }
    if (material.Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == aiReturn_SUCCESS) {
        pMaterial->SetSpecularColor(
            Diligent::float4(specularColor.r, specularColor.g, specularColor.b, specularColor.a));
    }
    if (material.Get(AI_MATKEY_SHININESS, shininess) == aiReturn_SUCCESS) {
        pMaterial->SetSpecularPower(shininess);
    }
    if (material.Get(AI_MATKEY_OPACITY, opacity) == aiReturn_SUCCESS) {
        pMaterial->SetOpacity(opacity);
    }
    if (material.Get(AI_MATKEY_REFRACTI, indexOfRefraction)) {
        pMaterial->SetIndexOfRefraction(indexOfRefraction);
    }
    if (material.Get(AI_MATKEY_REFLECTIVITY, reflectivity) == aiReturn_SUCCESS) {
        pMaterial->SetReflectance(
            Diligent::float4(reflectivity, reflectivity, reflectivity, reflectivity));
    }
    if (material.Get(AI_MATKEY_BUMPSCALING, bumpIntensity) == aiReturn_SUCCESS) {
        pMaterial->SetBumpIntensity(bumpIntensity);
    }

    // Load ambient textures.
    if (material.GetTextureCount(aiTextureType_AMBIENT) > 0 &&
        material.GetTexture(aiTextureType_AMBIENT, 0, &aiTexturePath, nullptr, nullptr,
                            &blendFactor, &aiBlendOperation) == aiReturn_SUCCESS) {
        fs::path texturePath(aiTexturePath.C_Str());
        std::shared_ptr<Texture> pTexture = CreateTexture((parentPath / texturePath).wstring());
        pMaterial->SetTexture(Material::TextureType::Ambient, pTexture);
    }

    // Load emissive textures.
    if (material.GetTextureCount(aiTextureType_EMISSIVE) > 0 &&
        material.GetTexture(aiTextureType_EMISSIVE, 0, &aiTexturePath, nullptr, nullptr,
                            &blendFactor, &aiBlendOperation) == aiReturn_SUCCESS) {
        fs::path texturePath(aiTexturePath.C_Str());
        std::shared_ptr<Texture> pTexture = CreateTexture((parentPath / texturePath).wstring());
        pMaterial->SetTexture(Material::TextureType::Emissive, pTexture);
    }

    // Load diffuse textures.
    if (material.GetTextureCount(aiTextureType_DIFFUSE) > 0 &&
        material.GetTexture(aiTextureType_DIFFUSE, 0, &aiTexturePath, nullptr, nullptr,
                            &blendFactor, &aiBlendOperation) == aiReturn_SUCCESS) {
        fs::path texturePath(aiTexturePath.C_Str());
        std::shared_ptr<Texture> pTexture = CreateTexture((parentPath / texturePath).wstring());
        pMaterial->SetTexture(Material::TextureType::Diffuse, pTexture);
    }

    // Load specular texture.
    if (material.GetTextureCount(aiTextureType_SPECULAR) > 0 &&
        material.GetTexture(aiTextureType_SPECULAR, 0, &aiTexturePath, nullptr, nullptr,
                            &blendFactor, &aiBlendOperation) == aiReturn_SUCCESS) {
        fs::path texturePath(aiTexturePath.C_Str());
        std::shared_ptr<Texture> pTexture = CreateTexture((parentPath / texturePath).wstring());
        pMaterial->SetTexture(Material::TextureType::Specular, pTexture);
    }


    // Load specular power texture.
    if (material.GetTextureCount(aiTextureType_SHININESS) > 0 &&
        material.GetTexture(aiTextureType_SHININESS, 0, &aiTexturePath, nullptr, nullptr,
                            &blendFactor, &aiBlendOperation) == aiReturn_SUCCESS) {
        fs::path texturePath(aiTexturePath.C_Str());
        std::shared_ptr<Texture> pTexture = CreateTexture((parentPath / texturePath).wstring());
        pMaterial->SetTexture(Material::TextureType::SpecularPower, pTexture);
    }

    if (material.GetTextureCount(aiTextureType_OPACITY) > 0 &&
        material.GetTexture(aiTextureType_OPACITY, 0, &aiTexturePath, nullptr, nullptr,
                            &blendFactor, &aiBlendOperation) == aiReturn_SUCCESS) {
        fs::path texturePath(aiTexturePath.C_Str());
        std::shared_ptr<Texture> pTexture = CreateTexture((parentPath / texturePath).wstring());
        pMaterial->SetTexture(Material::TextureType::Opacity, pTexture);
    }

    // Load normal map texture.
    if (material.GetTextureCount(aiTextureType_NORMALS) > 0 &&
        material.GetTexture(aiTextureType_NORMALS, 0, &aiTexturePath) == aiReturn_SUCCESS) {
        fs::path texturePath(aiTexturePath.C_Str());
        std::shared_ptr<Texture> pTexture = CreateTexture((parentPath / texturePath).wstring());
        pMaterial->SetTexture(Material::TextureType::Normal, pTexture);
    }
    // Load bump map (only if there is no normal map).
    else if (material.GetTextureCount(aiTextureType_HEIGHT) > 0 &&
             material.GetTexture(aiTextureType_HEIGHT, 0, &aiTexturePath, nullptr, nullptr,
                                 &blendFactor) == aiReturn_SUCCESS) {
        fs::path texturePath(aiTexturePath.C_Str());
        std::shared_ptr<Texture> pTexture = CreateTexture((parentPath / texturePath).wstring());

        // Some materials actually store normal maps in the bump map slot. Assimp can't tell the
        // difference between these two texture types, so we try to make an assumption about whether
        // the texture is a normal map or a bump map based on its pixel depth. Bump maps are usually
        // 8 BPP (grayscale) and normal maps are usually 24 BPP or higher.
        Material::TextureType textureType = (pTexture->GetBPP() >= 24)
                                                ? Material::TextureType::Normal
                                                : Material::TextureType::Bump;

        pMaterial->SetTexture(textureType, pTexture);
    }

    // m_MaterialMap.insert( MaterialMap::value_type( materialName.C_Str(), pMaterial ) );
    m_Materials.push_back(pMaterial);
}

void SceneAss::ImportMesh(const aiMesh& mesh)
{
    std::shared_ptr<Mesh> pMesh = CreateMesh();

    assert(mesh.mMaterialIndex < m_Materials.size());
    pMesh->SetMaterial(m_Materials[mesh.mMaterialIndex]);

    if (mesh.HasPositions()) {
        std::shared_ptr<Buffer> positions =
            CreateFloatVertexBuffer(&(mesh.mVertices[0].x), mesh.mNumVertices, sizeof(aiVector3D));
        pMesh->AddVertexBuffer(BufferBinding("POSITION", 0), positions);
    }

    if (mesh.HasNormals()) {
        std::shared_ptr<Buffer> normals =
            CreateFloatVertexBuffer(&(mesh.mNormals[0].x), mesh.mNumVertices, sizeof(aiVector3D));
        pMesh->AddVertexBuffer(BufferBinding("NORMAL", 0), normals);
    }

    if (mesh.HasTangentsAndBitangents()) {
        std::shared_ptr<Buffer> tangents =
            CreateFloatVertexBuffer(&(mesh.mTangents[0].x), mesh.mNumVertices, sizeof(aiVector3D));
        pMesh->AddVertexBuffer(BufferBinding("TANGENT", 0), tangents);

        std::shared_ptr<Buffer> bitangents = CreateFloatVertexBuffer(
            &(mesh.mBitangents[0].x), mesh.mNumVertices, sizeof(aiVector3D));
        pMesh->AddVertexBuffer(BufferBinding("BINORMAL", 0), bitangents);
    }

    for (uint32_t i = 0; mesh.HasVertexColors(i); ++i) {
        std::shared_ptr<Buffer> colors =
            CreateFloatVertexBuffer(&(mesh.mColors[i][0].r), mesh.mNumVertices, sizeof(aiColor4D));
        pMesh->AddVertexBuffer(BufferBinding("COLOR", i), colors);
    }

    for (uint32_t i = 0; mesh.HasTextureCoords(i); ++i) {
        switch (mesh.mNumUVComponents[i]) {
        case 1:    // 1-component texture coordinates (U)
        {
            std::vector<float> texcoods1D(mesh.mNumVertices);
            for (uint32_t j = 0; j < mesh.mNumVertices; ++j) {
                texcoods1D[j] = mesh.mTextureCoords[i][j].x;
            }
            std::shared_ptr<Buffer> texcoords = CreateFloatVertexBuffer(
                texcoods1D.data(), (uint32_t)texcoods1D.size(), sizeof(float));
            pMesh->AddVertexBuffer(BufferBinding("TEXCOORD", i), texcoords);
        } break;
        case 2:    // 2-component texture coordinates (U,V)
        {
            std::vector<aiVector2D> texcoods2D(mesh.mNumVertices);
            for (uint32_t j = 0; j < mesh.mNumVertices; ++j) {
                texcoods2D[j] =
                    aiVector2D(mesh.mTextureCoords[i][j].x, mesh.mTextureCoords[i][j].y);
            }
            std::shared_ptr<Buffer> texcoords = CreateFloatVertexBuffer(
                &(texcoods2D[0].x), (uint32_t)texcoods2D.size(), sizeof(aiVector2D));
            pMesh->AddVertexBuffer(BufferBinding("TEXCOORD", i), texcoords);
        } break;
        case 3:    // 3-component texture coordinates (U,V,W)
        {
            std::vector<aiVector3D> texcoods3D(mesh.mNumVertices);
            for (uint32_t j = 0; j < mesh.mNumVertices; ++j) {
                texcoods3D[j] = mesh.mTextureCoords[i][j];
            }
            std::shared_ptr<Buffer> texcoords = CreateFloatVertexBuffer(
                &(texcoods3D[0].x), (uint32_t)texcoods3D.size(), sizeof(aiVector3D));
            pMesh->AddVertexBuffer(BufferBinding("TEXCOORD", i), texcoords);
        } break;
        }
    }

    // Extract the index buffer.
    if (mesh.HasFaces()) {
        std::vector<uint32_t> indices;
        for (uint32_t i = 0; i < mesh.mNumFaces; ++i) {
            const aiFace& face = mesh.mFaces[i];
            // Only extract triangular faces
            if (face.mNumIndices == 3) {
                indices.push_back(face.mIndices[0]);
                indices.push_back(face.mIndices[1]);
                indices.push_back(face.mIndices[2]);
            }
        }
        if (indices.size() > 0) {
            std::shared_ptr<Buffer> indexBuffer =
                CreateUIntIndexBuffer(indices.data(), (uint32_t)indices.size());
            pMesh->SetIndexBuffer(indexBuffer);
        }
    }


    m_Meshes.push_back(pMesh);
}

std::shared_ptr<SceneNode> SceneAss::ImportSceneNode(std::shared_ptr<SceneNode> parent,
                                                     aiNode* aiNode)
{
    if (!aiNode) {
        return nullptr;
    }

    // Assimp stores its matrices in row-major but GLM uses column-major.
    // We have to transpose the matrix before using it to construct a glm matrix.
    aiMatrix4x4 mat = aiNode->mTransformation;
    Diligent::float4x4 localTransform(mat.a1, mat.b1, mat.c1, mat.d1, mat.a2, mat.b2, mat.c2,
                                      mat.d2, mat.a3, mat.b3, mat.c3, mat.d3, mat.a4, mat.b4,
                                      mat.c4, mat.d4);

    std::shared_ptr<SceneNode> pNode = std::make_shared<SceneNode>(localTransform);
    pNode->SetParent(parent);

    std::string nodeName(aiNode->mName.C_Str());
    if (!nodeName.empty()) {
        pNode->SetName(nodeName);
    }

    // Add meshes to scene node
    for (uint32_t i = 0; i < aiNode->mNumMeshes; ++i) {
        assert(aiNode->mMeshes[i] < m_Meshes.size());

        std::shared_ptr<Mesh> pMesh = m_Meshes[aiNode->mMeshes[i]];
        pNode->AddMesh(pMesh);
    }

    // Recursively Import children
    for (uint32_t i = 0; i < aiNode->mNumChildren; ++i) {
        std::shared_ptr<SceneNode> pChild = ImportSceneNode(pNode, aiNode->mChildren[i]);
        pNode->AddChild(pChild);
    }

    return pNode;
}

std::shared_ptr<SceneNode> SceneAss::GetRootNode() const
{
    return m_pRootNode;
}

std::shared_ptr<Mesh> SceneAss::CreateMesh()
{
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

    return mesh;
}

std::shared_ptr<Material> SceneAss::CreateMaterial()
{
    std::shared_ptr<Material> mat = std::make_shared<Material>();

    return mat;
}

std::shared_ptr<Texture> SceneAss::CreateTexture(const std::wstring& fileName)
{
    assert(0);
    Diligent::ITexture* texture = 0;
    std::shared_ptr<Texture> tex = std::make_shared<Texture>(texture);

    return tex;
}

std::shared_ptr<Texture> SceneAss::CreateTexture2D(uint16_t width, uint16_t height)
{
    assert(0);
    Diligent::ITexture* texture = 0;

    std::shared_ptr<Texture> tex = std::make_shared<Texture>(texture);

    return tex;
}

std::shared_ptr<Buffer> SceneAss::CreateFloatVertexBuffer(const float* data, uint32_t count,
                                                          uint32_t stride)
{
    return Scene::CreateFloatVertexBuffer(App::s_device, data, count, stride);
}

std::shared_ptr<Buffer> SceneAss::CreateUIntIndexBuffer(const uint32_t* data, uint32_t count)
{
    return Scene::CreateUIntIndexBuffer(App::s_device, data, count);
}

std::shared_ptr<SceneAss> SceneAss::CreateScene()
{
    std::shared_ptr<SceneAss> pScene = std::make_shared<SceneAss>();

    return pScene;
}

void SceneAss::DestroyScene(std::shared_ptr<Scene> scene)
{
    // SceneList::iterator iter = std::find(m_Scenes.begin(), m_Scenes.end(), scene);
    // if (iter != m_Scenes.end())
    //{
    //	m_Scenes.erase(iter);
    //}
}

Diligent::Quaternion RotationFromTwoVectors(const Diligent::float3& u, const Diligent::float3& v)
{
    float normUV = sqrt(Diligent::dot(u, u) * Diligent::dot(v, v));
    float real = normUV + Diligent::dot(u, v);

    Diligent::float3 vec;

    if (real < 1.e-6f * normUV) {
        /* If u and v are exactly opposite, rotate 180 degrees
         * around an arbitrary orthogonal axis. Axis normalisation
         * can happen later, when we normalise the quaternion.
         */
        real = 0.0f;
        vec = (abs(u.x) > abs(u.z)) ? Diligent::float3(-u.y, u.x, 0.0f)
                                    : Diligent::float3(0.0f, -u.z, u.y);
    } else {
        /* Otherwise, build quaternion the standard way. */
        vec = Diligent::cross(u, v);
    }

    return Diligent::normalize(Diligent::Quaternion(vec.x, vec.y, vec.z, real));
}


std::shared_ptr<Scene> SceneAss::CreatePlane(float size, const Diligent::float3& N)
{
    float halfSize = size * 0.5f;
    Diligent::float3 p[4];
    // Crate the 4 points of the plane aligned to the X,Z plane.
    // Vertex winding is assuming a right-handed coordinate system
    // (counter-clockwise winding order for front-facing polygons)
    p[0] = Diligent::float3(halfSize, 0, halfSize);
    p[1] = Diligent::float3(-halfSize, 0, halfSize);
    p[2] = Diligent::float3(-halfSize, 0, -halfSize);
    p[3] = Diligent::float3(halfSize, 0, -halfSize);

    // Rotate the plane vertices in the direction of the surface normal.
    Diligent::Quaternion rot = RotationFromTwoVectors(Diligent::float3(0, 1, 0), N);
    Diligent::float4x4 mat = rot.ToMatrix();

    for (int i = 0; i < 4; i++) {
        p[i] = p[i] * mat;
    }

    // Now create the plane polygon from the transformed vertices.
    std::shared_ptr<SceneAss> scene = CreateScene();

    std::stringstream ss;

    // Create a white diffuse material for the plane.
    // f red green blue Kd Ks Shine transmittance indexOfRefraction
    ss << "f 1 1 1 1 0 0 0 0" << std::endl;

    // Create a 4-point polygon
    ss << "p 4" << std::endl;
    for (int i = 0; i < 4; i++) {
        ss << p[i].x << " " << p[i].y << " " << p[i].z << std::endl;
    }

    if (scene->LoadFromString(ss.str(), "nff")) {
        return scene;
    }

    // An error occurred while loading the scene.
    DestroyScene(scene);
    return nullptr;
}

std::shared_ptr<Scene> SceneAss::CreateScreenQuad(float left, float right, float bottom, float top,
                                                  float z)
{
    Diligent::float3 p[4];    // Vertex position
    Diligent::float3 n[4];    // Vertex normal (required for texture patch polygons)
    Diligent::float2 t[4];    // Texture coordinates
    // Winding order is assumed to be right-handed. Front-facing polygons have
    // a counter-clockwise winding order.
    // Assimp flips the winding order of vertices.. Don't ask me why. To account for this,
    // the vertices are loaded in reverse order :)
    p[0] = Diligent::float3(right, bottom, z);
    n[0] = Diligent::float3(0, 0, 1);
    t[0] = Diligent::float2(1, 0);
    p[1] = Diligent::float3(left, bottom, z);
    n[1] = Diligent::float3(0, 0, 1);
    t[1] = Diligent::float2(0, 0);
    p[2] = Diligent::float3(left, top, z);
    n[2] = Diligent::float3(0, 0, 1);
    t[2] = Diligent::float2(0, 1);
    p[3] = Diligent::float3(right, top, z);
    n[3] = Diligent::float3(0, 0, 1);
    t[3] = Diligent::float2(1, 1);

    // Now create the quad.
    std::shared_ptr<SceneAss> scene = CreateScene();

    std::stringstream ss;

    // Create a white diffuse material for the quad.
    // f red green blue Kd Ks Shine transmittance indexOfRefraction
    ss << "f 1 1 1 1 0 0 0 0" << std::endl;

    // Create a 4-point textured polygon patch
    ss << "tpp 4" << std::endl;
    for (int i = 0; i < 4; i++) {
        // px py pz nx ny nz tu tv
        ss << p[i].x << " " << p[i].y << " " << p[i].z << " " << n[i].x << " " << n[i].y << " "
           << n[i].z << " " << t[i].x << " " << t[i].y << std::endl;
    }

    if (scene->LoadFromString(ss.str(), "nff")) {
        return scene;
    }

    // An error occurred while loading the scene.
    DestroyScene(scene);
    return nullptr;
}

std::shared_ptr<Scene> SceneAss::CreateSphere(float radius, float tesselation)
{
    std::shared_ptr<SceneAss> scene = CreateScene();
    std::stringstream ss;
    // Create a white diffuse material for the sphere.
    // f red green blue Kd Ks Shine transmittance indexOfRefraction
    ss << "f 1 1 1 1 0 0 0 0" << std::endl;

    // tess tesselation
    ss << "tess " << tesselation << std::endl;
    // s x y z radius
    ss << "s 0 0 0 " << radius << std::endl;

    if (scene->LoadFromString(ss.str(), "nff")) {
        return scene;
    }

    // An error occurred while loading the scene.
    DestroyScene(scene);
    return nullptr;
}

std::shared_ptr<Scene> SceneAss::CreateCube(float size)
{
    std::shared_ptr<SceneAss> scene = CreateScene();
    std::stringstream ss;

    // Create a white diffuse material for the cube.
    // f red green blue Kd Ks Shine transmittance indexOfRefraction
    ss << "f 1 1 1 1 0 0 0 0" << std::endl;

    // hex x y z size
    ss << "hex 0 0 0 " << size;

    if (scene->LoadFromString(ss.str(), "nff")) {
        return scene;
    }

    // An error occurred while loading the scene.
    DestroyScene(scene);
    return nullptr;
}

std::shared_ptr<Scene> SceneAss::CreateCylinder(float baseRadius, float apexRadius, float height,
                                                const Diligent::float3& axis)
{
    std::shared_ptr<SceneAss> scene = CreateScene();
    std::stringstream ss;

    // Create a white diffuse material for the cylinder.
    // f red green blue Kd Ks Shine transmittance indexOfRefraction
    ss << "f 1 1 1 1 0 0 0 0" << std::endl;

    ss << "c" << std::endl;
    // base.x base.y base.z baseRadius
    ss << "0 0 0 " << baseRadius << std::endl;

    Diligent::float3 apex = axis * height;
    // apex.x apex.y apex.z apexRadius
    ss << apex.x << " " << apex.y << " " << apex.z << " " << apexRadius << std::endl;

    if (scene->LoadFromString(ss.str(), "nff")) {
        return scene;
    }

    // An error occurred while loading the scene.
    DestroyScene(scene);
    return nullptr;
}

std::shared_ptr<Scene> SceneAss::CreateCone(float baseRadius, float height)
{
    // A cone is just a cylinder with a 0 size apex.
    return CreateCylinder(baseRadius, 0, height);
}

std::shared_ptr<Scene> SceneAss::CreateArrow(const Diligent::float3& tail,
                                             const Diligent::float3& head, float radius)
{
    std::shared_ptr<SceneAss> scene = CreateScene();
    std::stringstream ss;

    Diligent::float3 dir = head - tail;
    Diligent::float3 apex = head + (dir * 0.5f);

    // Create a white diffuse material for the arrow.
    // f red green blue Kd Ks Shine transmittance indexOfRefraction
    ss << "f 1 1 1 1 0 0 0 0" << std::endl;

    // Create a cylinder for the arrow body.
    ss << "c" << std::endl;
    // base.x base.y base.z baseRadius
    ss << tail.x << " " << tail.y << " " << tail.z << " " << radius << std::endl;
    // apex.x apex.y apex.z apexRadius
    ss << head.x << " " << head.y << " " << head.z << " " << radius << std::endl;

    // Create a cone for the arrow head.
    ss << "c" << std::endl;
    // base.x base.y base.z baseRadius
    ss << head.x << " " << head.y << " " << head.z << " " << radius * 2.0f << std::endl;

    // apex.x apex.y apex.z apexRadius
    ss << apex.x << " " << apex.y << " " << apex.z << " 0" << std::endl;

    if (scene->LoadFromString(ss.str(), "nff")) {
        return scene;
    }

    // An error occurred while loading the scene.
    DestroyScene(scene);
    return nullptr;
}

std::shared_ptr<Scene> SceneAss::CreateAxis(float radius, float length)
{
    std::shared_ptr<SceneAss> scene = CreateScene();
    std::stringstream ss;

    // Create a red material for the +X axis.
    // f red green blue Kd Ks Shine transmittance indexOfRefraction
    ss << "f 1 0 0 1 0 0 0 0" << std::endl;

    // Create a cylinder aligned to the +X axis.
    ss << "c" << std::endl;
    // base.x base.y base.z baseRadius
    ss << "0 0 0 " << radius << std::endl;
    // apex.x apex.y apex.z apexRadius
    ss << length << " 0 0 " << radius << std::endl;

    // Create a cone for the +X axis.
    ss << "c" << std::endl;
    // base.x base.y base.z baseRadius
    ss << length << " 0 0 " << radius * 2.0f << std::endl;
    // apex.x apex.y apex.z apexRadius
    ss << length * 1.5f << " 0 0 0" << std::endl;

    // Create a green material for the +Y axis.
    // f red green blue Kd Ks Shine transmittance indexOfRefraction
    ss << "f 0 1 0 1 0 0 0 0" << std::endl;

    // Create a cylinder aligned to the +Y axis.
    ss << "c" << std::endl;
    // base.x base.y base.z baseRadius
    ss << "0 0 0 " << radius << std::endl;
    // apex.x apex.y apex.z apexRadius
    ss << "0 " << length << " 0 " << radius << std::endl;

    // Create a cone for the +Y axis.
    ss << "c" << std::endl;
    // base.x base.y base.z baseRadius
    ss << "0 " << length << " 0 " << radius * 2.0f << std::endl;
    // apex.x apex.y apex.z apexRadius
    ss << "0 " << length * 1.5f << " 0 0" << std::endl;

    // Create a blue material for the +Z axis.
    // f red green blue Kd Ks Shine transmittance indexOfRefraction
    ss << "f 0 0 1 1 0 0 0 0" << std::endl;

    // Create a cylinder aligned to the +Z axis.
    ss << "c" << std::endl;
    // base.x base.y base.z baseRadius
    ss << "0 0 0 " << radius << std::endl;
    // apex.x apex.y apex.z apexRadius
    ss << "0 0 " << length << " " << radius << std::endl;

    // Create a cone for the +Z axis.
    ss << "c" << std::endl;
    // base.x base.y base.z baseRadius
    ss << "0 0 " << length << " " << radius * 2.0f << std::endl;
    // apex.x apex.y apex.z apexRadius
    ss << "0 0 " << length * 1.5f << " 0" << std::endl;

    // Create a cyan material for the -X axis.
    // f red green blue Kd Ks Shine transmittance indexOfRefraction
    ss << "f 0 1 1 1 0 0 0 0" << std::endl;

    // Create a cylinder aligned to the -X axis.
    ss << "c" << std::endl;
    // base.x base.y base.z baseRadius
    ss << "0 0 0 " << radius << std::endl;
    // apex.x apex.y apex.z apexRadius
    ss << -length << " 0 0 " << radius << std::endl;

    // Create a cone for the -X axis.
    ss << "c" << std::endl;
    // base.x base.y base.z baseRadius
    ss << -length << " 0 0 " << radius * 2.0f << std::endl;
    // apex.x apex.y apex.z apexRadius
    ss << -length * 1.5f << " 0 0 0" << std::endl;

    // Create a yellow material for the -Y axis.
    // f red green blue Kd Ks Shine transmittance indexOfRefraction
    ss << "f 1 0 1 1 0 0 0 0" << std::endl;

    // Create a cylinder aligned to the -Y axis.
    ss << "c" << std::endl;
    // base.x base.y base.z baseRadius
    ss << "0 0 0 " << radius << std::endl;
    // apex.x apex.y apex.z apexRadius
    ss << "0 " << -length << " 0 " << radius << std::endl;

    // Create a cone for the -Y axis.
    ss << "c" << std::endl;
    // base.x base.y base.z baseRadius
    ss << "0 " << -length << " 0 0" << std::endl;
    // apex.x apex.y apex.z apexRadius
    ss << "0 " << -length * 1.5f << " 0 " << radius * 2.0f << std::endl;

    // Create a magenta material for the -Z axis.
    // f red green blue Kd Ks Shine transmittance indexOfRefraction
    ss << "f 1 1 0 1 0 0 0 0" << std::endl;

    // Create a cylinder aligned to the -Z axis.
    ss << "c" << std::endl;
    // base.x base.y base.z baseRadius
    ss << "0 0 0 " << radius << std::endl;
    // apex.x apex.y apex.z apexRadius
    ss << "0 0 " << -length << " " << radius << std::endl;

    // Create a cone for the -Z axis.
    ss << "c" << std::endl;
    // base.x base.y base.z baseRadius
    ss << "0 0 " << -length << " " << radius * 2.0f << std::endl;
    // apex.x apex.y apex.z apexRadius
    ss << "0 0 " << -length * 1.5f << " 0" << std::endl;

    if (scene->LoadFromString(ss.str(), "nff")) {
        return scene;
    }

    // An error occurred while loading the scene.
    DestroyScene(scene);
    return nullptr;
}

}    // namespace ade