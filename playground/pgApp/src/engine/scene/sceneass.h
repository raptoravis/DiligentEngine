#pragma once

#include "../engine.h"

struct aiMaterial;
struct aiMesh;
struct aiNode;

class pgMaterial;
class pgBuffer;
class pgMesh;
class pgTexture;
class pgCamera;
class pgSceneNode;

namespace fs = std::filesystem;

// A model base class.
// Implements a basic model loader using Assimp.
class pgSceneAss : public pgScene
{
public:
    typedef pgScene base;

	virtual std::shared_ptr<pgSceneNode> getRootNode() const;
    virtual bool LoadFromFile( const std::wstring& fileName );
    virtual bool LoadFromString( const std::string& scene, const std::string& format );

	pgSceneAss(const pgSceneCreateInfo& ci);
	virtual ~pgSceneAss();

	static std::shared_ptr<pgBuffer> createFloatVertexBuffer(Diligent::IRenderDevice* device, 
		const float* data, uint32_t count, uint32_t stride);
	static std::shared_ptr<pgBuffer> createUIntIndexBuffer(Diligent::IRenderDevice* device, 
		const uint32_t* data, uint32_t sizeInBytes);

protected:
    friend class ProgressHandler;

    std::shared_ptr<pgBuffer> CreateFloatVertexBuffer( const float* data, uint32_t count, uint32_t stride );
    std::shared_ptr<pgBuffer> CreateUIntIndexBuffer( const uint32_t* data, uint32_t count);

    std::shared_ptr<pgMesh> CreateMesh();
    std::shared_ptr<pgMaterial> CreateMaterial();
    std::shared_ptr<pgTexture> CreateTexture( const std::wstring& fileName );
    std::shared_ptr<pgTexture> CreateTexture2D( uint16_t width, uint16_t height );

    //virtual std::shared_ptr<pgTexture> GetDefaultTexture() = 0;

protected:
    typedef std::map<std::string, std::shared_ptr<pgMaterial> > MaterialMap;
    typedef std::vector< std::shared_ptr<pgMaterial> > MaterialList;
    typedef std::vector< std::shared_ptr<pgMesh> > MeshList;

    MaterialMap m_MaterialMap;
    MaterialList m_Materials;
    MeshList m_Meshes;

    void ImportMaterial( const aiMaterial& material, fs::path parentPath );
    void ImportMesh( const aiMesh& mesh );
    std::shared_ptr<pgSceneNode> ImportSceneNode( std::shared_ptr<pgSceneNode> parent, aiNode* aiNode );

    std::wstring m_SceneFile;
};
