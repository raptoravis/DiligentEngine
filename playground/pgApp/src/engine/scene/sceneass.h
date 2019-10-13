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
class SceneAss : public pgScene
{
public:
    typedef pgScene base;

	virtual std::shared_ptr<pgSceneNode> getRootNode() const;
    virtual bool LoadFromFile( const std::wstring& fileName );
    virtual bool LoadFromString( const std::string& scene, const std::string& format );
    virtual void Render( pgRenderEventArgs& renderArgs );

	SceneAss(const pgSceneCreateInfo& ci);
	virtual ~SceneAss();

protected:
    friend class ProgressHandler;

    virtual std::shared_ptr<pgBuffer> CreateFloatVertexBuffer( const float* data, unsigned int count, unsigned int stride );
    virtual std::shared_ptr<pgBuffer> CreateUIntIndexBuffer( const unsigned int* data, unsigned int sizeInBytes );

    virtual std::shared_ptr<pgMesh> CreateMesh();
    virtual std::shared_ptr<pgMaterial> CreateMaterial();
    virtual std::shared_ptr<pgTexture> CreateTexture( const std::wstring& fileName );
    virtual std::shared_ptr<pgTexture> CreateTexture2D( uint16_t width, uint16_t height );

    //virtual std::shared_ptr<pgTexture> GetDefaultTexture() = 0;

private:
    typedef std::map<std::string, std::shared_ptr<pgMaterial> > MaterialMap;
    typedef std::vector< std::shared_ptr<pgMaterial> > MaterialList;
    typedef std::vector< std::shared_ptr<pgMesh> > MeshList;

    MaterialMap m_MaterialMap;
    MaterialList m_Materials;
    MeshList m_Meshes;

    std::shared_ptr<pgSceneNode> m_pRootNode;

    void ImportMaterial( const aiMaterial& material, fs::path parentPath );
    void ImportMesh( const aiMesh& mesh );
    std::shared_ptr<pgSceneNode> ImportSceneNode( std::shared_ptr<pgSceneNode> parent, aiNode* aiNode );

    std::wstring m_SceneFile;
};
