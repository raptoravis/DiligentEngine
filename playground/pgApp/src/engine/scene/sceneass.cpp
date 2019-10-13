#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/ProgressHandler.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "sceneass.h"

#define EXPORT_FORMAT "assbin"
#define EXPORT_EXTENSION "assbin"

// A private class that is registered with Assimp's importer
// Provides feedback on the loading progress of the scene files.
// 

class ProgressHandler : public Assimp::ProgressHandler
{
public:
	ProgressHandler(pgSceneAss& scene, const std::wstring& fileName)
		: m_Scene(scene)
		, m_FileName(fileName)
	{}

	virtual bool Update(float percentage)
	{
		return false;
	}

private:
	pgSceneAss& m_Scene;
	std::wstring m_FileName;
};

pgSceneAss::pgSceneAss(const pgSceneCreateInfo& ci)
    : base(ci)
    , m_pRootNode( nullptr )
{
	//
}

pgSceneAss::~pgSceneAss()
{
}

bool pgSceneAss::LoadFromString( const std::string& sceneStr, const std::string& format )
{
    Assimp::Importer importer;
    const aiScene* scene = nullptr;

    importer.SetProgressHandler( new ProgressHandler( *this, L"String" ) );

    importer.SetPropertyFloat( AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f );
    importer.SetPropertyInteger( AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE );

    uint32_t preprocessFlags = aiProcessPreset_TargetRealtime_MaxQuality;

    scene = importer.ReadFileFromMemory( sceneStr.data(), sceneStr.size(), preprocessFlags, format.c_str() );

    if ( !scene )
    {
        ReportError( importer.GetErrorString() );
        return false;
    }
    else
    {
        // If we have a previously loaded scene, delete it.
        if ( m_pRootNode )
        {
            m_pRootNode.reset();
        }

        // Import scene materials.
        for ( uint32_t i = 0; i < scene->mNumMaterials; ++i )
        {
            ImportMaterial( *scene->mMaterials[i], fs::current_path() );
        }
        // Import meshes
        for ( uint32_t i = 0; i < scene->mNumMeshes; ++i )
        {
            ImportMesh( *scene->mMeshes[i] );
        }

        m_pRootNode = ImportSceneNode( m_pRootNode, scene->mRootNode );
    }

    return true;
}

bool pgSceneAss::LoadFromFile( const std::wstring& fileName )
{
    fs::path filePath( fileName );
    fs::path parentPath;

    m_SceneFile = fileName;

    if ( filePath.has_parent_path() )
    {
        parentPath = filePath.parent_path();
    }
    else
    {
        parentPath = fs::current_path();
    }

    const aiScene* scene;
    Assimp::Importer importer;

    importer.SetProgressHandler( new ProgressHandler( *this, fileName ) );

    fs::path exportPath = filePath;
    exportPath.replace_extension( EXPORT_EXTENSION );

    if ( fs::exists( exportPath ) && fs::is_regular_file( exportPath ) )
    {
        // If a previously exported file exists, load that instead (scene has already been preprocessed).
        scene = importer.ReadFile( exportPath.string(), 0 );
    }
    else
    {
        // If no serialized version of the model file exists (or the original model is newer than the serialized version),
        // reimport the original scene and export it as binary.
        importer.SetPropertyFloat( AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f );
        importer.SetPropertyInteger( AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE );

        uint32_t preprocessFlags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_OptimizeGraph;
        scene = importer.ReadFile( filePath.string(), preprocessFlags );

        if ( scene )
        {
            // Now export the preprocessed scene so we can load it faster next time.
            Assimp::Exporter exporter;
            exporter.Export( scene, EXPORT_FORMAT, exportPath.string(), preprocessFlags );
        }
    }

    if ( !scene )
    {
        ReportError( importer.GetErrorString() );
        return false;
    }
    else
    {
        // If we have a previously loaded scene, delete it.
        Diligent::float4x4 localTransform( 1 );
        if ( m_pRootNode )
        {
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
        for ( uint32_t i = 0; i < scene->mNumMaterials; ++i )
        {
            ImportMaterial( *scene->mMaterials[i], parentPath );
        }
        // Import meshes
        for ( uint32_t i = 0; i < scene->mNumMeshes; ++i )
        {
            ImportMesh( *scene->mMeshes[i] );
        }

        m_pRootNode = ImportSceneNode( m_pRootNode, scene->mRootNode );
        m_pRootNode->SetLocalTransform( localTransform );
    }

    return true;
}

void pgSceneAss::ImportMaterial( const aiMaterial& material, fs::path parentPath )
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

    //if ( material.Get( AI_MATKEY_NAME, materialName ) == aiReturn_SUCCESS )
    //{
    //    MaterialMap::iterator itr = m_MaterialMap.find( materialName.C_Str() );
    //    if ( itr != m_MaterialMap.end() )
    //    {
    //        // A material with this name already exists in our materials array.
    //        // Skip it.
    //        return;
    //    }
    //}

    std::shared_ptr<pgMaterial> pMaterial = CreateMaterial();

    if ( material.Get( AI_MATKEY_COLOR_AMBIENT, ambientColor ) == aiReturn_SUCCESS )
    {
        pMaterial->SetAmbientColor( Diligent::float4( ambientColor.r, ambientColor.g, ambientColor.b, ambientColor.a ) );
    }
    if ( material.Get( AI_MATKEY_COLOR_EMISSIVE, emissiveColor ) == aiReturn_SUCCESS )
    {
        pMaterial->SetEmissiveColor( Diligent::float4( emissiveColor.r, emissiveColor.g, emissiveColor.b, emissiveColor.a ) );
    }
    if ( material.Get( AI_MATKEY_COLOR_DIFFUSE, diffuseColor ) == aiReturn_SUCCESS )
    {
        pMaterial->SetDiffuseColor( Diligent::float4( diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a ) );
    }
    if ( material.Get( AI_MATKEY_COLOR_SPECULAR, specularColor ) == aiReturn_SUCCESS )
    {
        pMaterial->SetSpecularColor( Diligent::float4( specularColor.r, specularColor.g, specularColor.b, specularColor.a ) );
    }
    if ( material.Get( AI_MATKEY_SHININESS, shininess ) == aiReturn_SUCCESS )
    {
        pMaterial->SetSpecularPower( shininess );
    }
    if ( material.Get( AI_MATKEY_OPACITY, opacity ) == aiReturn_SUCCESS )
    {
        pMaterial->SetOpacity( opacity );
    }
    if ( material.Get( AI_MATKEY_REFRACTI, indexOfRefraction ) )
    {
        pMaterial->SetIndexOfRefraction( indexOfRefraction );
    }
    if ( material.Get( AI_MATKEY_REFLECTIVITY, reflectivity ) == aiReturn_SUCCESS )
    {
        pMaterial->SetReflectance( Diligent::float4( reflectivity, reflectivity, reflectivity, reflectivity) );
    }
    if ( material.Get( AI_MATKEY_BUMPSCALING, bumpIntensity ) == aiReturn_SUCCESS )
    {
        pMaterial->SetBumpIntensity( bumpIntensity );
    }

    // Load ambient textures.
    if ( material.GetTextureCount( aiTextureType_AMBIENT ) > 0 &&
         material.GetTexture( aiTextureType_AMBIENT, 0, &aiTexturePath, nullptr, nullptr, &blendFactor, &aiBlendOperation ) == aiReturn_SUCCESS )
    {
        fs::path texturePath( aiTexturePath.C_Str() );
        std::shared_ptr<pgTexture> pTexture = CreateTexture( ( parentPath / texturePath ).wstring() );
        pMaterial->SetTexture( pgMaterial::TextureType::Ambient, pTexture );
    }

    // Load emissive textures.
    if ( material.GetTextureCount( aiTextureType_EMISSIVE ) > 0 &&
         material.GetTexture( aiTextureType_EMISSIVE, 0, &aiTexturePath, nullptr, nullptr, &blendFactor, &aiBlendOperation ) == aiReturn_SUCCESS )
    {
        fs::path texturePath( aiTexturePath.C_Str() );
        std::shared_ptr<pgTexture> pTexture = CreateTexture( ( parentPath / texturePath ).wstring() );
        pMaterial->SetTexture( pgMaterial::TextureType::Emissive, pTexture );
    }

    // Load diffuse textures.
    if ( material.GetTextureCount( aiTextureType_DIFFUSE ) > 0 &&
         material.GetTexture( aiTextureType_DIFFUSE, 0, &aiTexturePath, nullptr, nullptr, &blendFactor, &aiBlendOperation ) == aiReturn_SUCCESS )
    {
        fs::path texturePath( aiTexturePath.C_Str() );
        std::shared_ptr<pgTexture> pTexture = CreateTexture( ( parentPath / texturePath ).wstring() );
        pMaterial->SetTexture( pgMaterial::TextureType::Diffuse, pTexture );
    }

    // Load specular texture.
    if ( material.GetTextureCount( aiTextureType_SPECULAR ) > 0 &&
         material.GetTexture( aiTextureType_SPECULAR, 0, &aiTexturePath, nullptr, nullptr, &blendFactor, &aiBlendOperation ) == aiReturn_SUCCESS )
    {
        fs::path texturePath( aiTexturePath.C_Str() );
        std::shared_ptr<pgTexture> pTexture = CreateTexture( ( parentPath / texturePath ).wstring() );
        pMaterial->SetTexture( pgMaterial::TextureType::Specular, pTexture );
    }


    // Load specular power texture.
    if ( material.GetTextureCount( aiTextureType_SHININESS ) > 0 &&
         material.GetTexture( aiTextureType_SHININESS, 0, &aiTexturePath, nullptr, nullptr, &blendFactor, &aiBlendOperation ) == aiReturn_SUCCESS )
    {
        fs::path texturePath( aiTexturePath.C_Str() );
        std::shared_ptr<pgTexture> pTexture = CreateTexture( ( parentPath / texturePath ).wstring() );
        pMaterial->SetTexture( pgMaterial::TextureType::SpecularPower, pTexture );
    }

    if ( material.GetTextureCount( aiTextureType_OPACITY ) > 0 &&
         material.GetTexture( aiTextureType_OPACITY, 0, &aiTexturePath, nullptr, nullptr, &blendFactor, &aiBlendOperation ) == aiReturn_SUCCESS )
    {
        fs::path texturePath( aiTexturePath.C_Str() );
        std::shared_ptr<pgTexture> pTexture = CreateTexture( ( parentPath / texturePath ).wstring() );
        pMaterial->SetTexture( pgMaterial::TextureType::Opacity, pTexture );
    }

    // Load normal map texture.
    if ( material.GetTextureCount( aiTextureType_NORMALS ) > 0 &&
         material.GetTexture( aiTextureType_NORMALS, 0, &aiTexturePath ) == aiReturn_SUCCESS )
    {
        fs::path texturePath( aiTexturePath.C_Str() );
        std::shared_ptr<pgTexture> pTexture = CreateTexture( ( parentPath / texturePath ).wstring() );
        pMaterial->SetTexture( pgMaterial::TextureType::Normal, pTexture );
    }
    // Load bump map (only if there is no normal map).
    else if ( material.GetTextureCount( aiTextureType_HEIGHT ) > 0 &&
         material.GetTexture( aiTextureType_HEIGHT, 0, &aiTexturePath, nullptr, nullptr, &blendFactor ) == aiReturn_SUCCESS )
    {
        fs::path texturePath( aiTexturePath.C_Str() );
        std::shared_ptr<pgTexture> pTexture = CreateTexture( ( parentPath / texturePath ).wstring() );
        
        // Some materials actually store normal maps in the bump map slot. Assimp can't tell the difference between 
        // these two texture types, so we try to make an assumption about whether the texture is a normal map or a bump
        // map based on its pixel depth. Bump maps are usually 8 BPP (grayscale) and normal maps are usually 24 BPP or higher.
        pgMaterial::TextureType textureType = ( pTexture->GetBPP() >= 24 ) ? pgMaterial::TextureType::Normal : pgMaterial::TextureType::Bump;

        pMaterial->SetTexture( textureType, pTexture );
    }

    //m_MaterialMap.insert( MaterialMap::value_type( materialName.C_Str(), pMaterial ) );
    m_Materials.push_back( pMaterial );
}

void pgSceneAss::ImportMesh( const aiMesh& mesh )
{
    std::shared_ptr<pgMesh> pMesh = CreateMesh();

    assert( mesh.mMaterialIndex < m_Materials.size() );
    pMesh->SetMaterial( m_Materials[mesh.mMaterialIndex] );

    if ( mesh.HasPositions() )
    {
        std::shared_ptr<pgBuffer> positions = CreateFloatVertexBuffer( &( mesh.mVertices[0].x ), mesh.mNumVertices, sizeof( aiVector3D ) );
        pMesh->AddVertexBuffer( pgBufferBinding( "POSITION", 0 ), positions );
    }

    if ( mesh.HasNormals() )
    {
        std::shared_ptr<pgBuffer> normals = CreateFloatVertexBuffer( &( mesh.mNormals[0].x ), mesh.mNumVertices, sizeof( aiVector3D ) );
        pMesh->AddVertexBuffer( pgBufferBinding( "NORMAL", 0 ), normals );
    }

    if ( mesh.HasTangentsAndBitangents() )
    {
        std::shared_ptr<pgBuffer> tangents = CreateFloatVertexBuffer( &( mesh.mTangents[0].x ), mesh.mNumVertices, sizeof( aiVector3D ) );
        pMesh->AddVertexBuffer( pgBufferBinding( "TANGENT", 0 ), tangents );

        std::shared_ptr<pgBuffer> bitangents = CreateFloatVertexBuffer( &( mesh.mBitangents[0].x ), mesh.mNumVertices, sizeof( aiVector3D ) );
        pMesh->AddVertexBuffer( pgBufferBinding( "BINORMAL", 0 ), bitangents );
    }

    for ( uint32_t i = 0; mesh.HasVertexColors( i ); ++i )
    {
        std::shared_ptr<pgBuffer> colors = CreateFloatVertexBuffer( &( mesh.mColors[i][0].r ), mesh.mNumVertices, sizeof( aiColor4D ) );
        pMesh->AddVertexBuffer( pgBufferBinding( "COLOR", i ), colors );
    }

    for ( uint32_t i = 0; mesh.HasTextureCoords( i ); ++i )
    {
        switch ( mesh.mNumUVComponents[i] )
        {
        case 1: // 1-component texture coordinates (U)
        {
            std::vector<float> texcoods1D( mesh.mNumVertices );
            for ( uint32_t j = 0; j < mesh.mNumVertices; ++j )
            {
                texcoods1D[j] = mesh.mTextureCoords[i][j].x;
            }
            std::shared_ptr<pgBuffer> texcoords = CreateFloatVertexBuffer( texcoods1D.data(), (uint32_t)texcoods1D.size(), sizeof( float ) );
            pMesh->AddVertexBuffer( pgBufferBinding( "TEXCOORD", i ), texcoords );
        }
        break;
        case 2: // 2-component texture coordinates (U,V)
        {
            std::vector<aiVector2D> texcoods2D( mesh.mNumVertices );
            for ( uint32_t j = 0; j < mesh.mNumVertices; ++j )
            {
                texcoods2D[j] = aiVector2D( mesh.mTextureCoords[i][j].x, mesh.mTextureCoords[i][j].y );
            }
            std::shared_ptr<pgBuffer> texcoords = CreateFloatVertexBuffer( &( texcoods2D[0].x ), (uint32_t)texcoods2D.size(), sizeof( aiVector2D ) );
            pMesh->AddVertexBuffer( pgBufferBinding( "TEXCOORD", i ), texcoords );
        }
        break;
        case 3: // 3-component texture coordinates (U,V,W)
        {
            std::vector<aiVector3D> texcoods3D( mesh.mNumVertices );
            for ( uint32_t j = 0; j < mesh.mNumVertices; ++j )
            {
                texcoods3D[j] = mesh.mTextureCoords[i][j];
            }
            std::shared_ptr<pgBuffer> texcoords = CreateFloatVertexBuffer( &( texcoods3D[0].x ), (uint32_t)texcoods3D.size(), sizeof( aiVector3D ) );
            pMesh->AddVertexBuffer( pgBufferBinding( "TEXCOORD", i ), texcoords );
        }
        break;
        }
    }

    // Extract the index buffer.
    if ( mesh.HasFaces() )
    {
        std::vector<uint32_t> indices;
        for ( uint32_t i = 0; i < mesh.mNumFaces; ++i )
        {
            const aiFace& face = mesh.mFaces[i];
            // Only extract triangular faces
            if ( face.mNumIndices == 3 )
            {
                indices.push_back( face.mIndices[0] );
                indices.push_back( face.mIndices[1] );
                indices.push_back( face.mIndices[2] );
            }
        }
        if ( indices.size() > 0 )
        {
            std::shared_ptr<pgBuffer> indexBuffer = CreateUIntIndexBuffer( indices.data(), (uint32_t)indices.size() );
            pMesh->SetIndexBuffer( indexBuffer );
        }
    }


    m_Meshes.push_back( pMesh );
}

std::shared_ptr<pgSceneNode> pgSceneAss::ImportSceneNode( std::shared_ptr<pgSceneNode> parent, aiNode* aiNode )
{
    if ( !aiNode )
    {
        return nullptr;
    }

    // Assimp stores its matrices in row-major but GLM uses column-major.
    // We have to transpose the matrix before using it to construct a glm matrix.
    aiMatrix4x4 mat = aiNode->mTransformation;
    Diligent::float4x4 localTransform( mat.a1, mat.b1, mat.c1, mat.d1,
                              mat.a2, mat.b2, mat.c2, mat.d2,
                              mat.a3, mat.b3, mat.c3, mat.d3,
                              mat.a4, mat.b4, mat.c4, mat.d4 );

    std::shared_ptr<pgSceneNode> pNode = std::make_shared<pgSceneNode>( localTransform );
    pNode->SetParent( parent );

    std::string nodeName( aiNode->mName.C_Str() );
    if ( !nodeName.empty() )
    {
        pNode->SetName( nodeName );
    }

    // Add meshes to scene node
    for ( uint32_t i = 0; i < aiNode->mNumMeshes; ++i )
    {
        assert( aiNode->mMeshes[i] < m_Meshes.size() );

        std::shared_ptr<pgMesh> pMesh = m_Meshes[aiNode->mMeshes[i]];
        pNode->addMesh( pMesh );
    }

    // Recursively Import children
    for ( uint32_t i = 0; i < aiNode->mNumChildren; ++i )
    {
        std::shared_ptr<pgSceneNode> pChild = ImportSceneNode( pNode, aiNode->mChildren[i] );
        pNode->AddChild( pChild );
    }

    return pNode;
}

std::shared_ptr<pgSceneNode> pgSceneAss::getRootNode() const
{
    return m_pRootNode;
}

std::shared_ptr<pgBuffer> pgSceneAss::createFloatVertexBuffer(Diligent::IRenderDevice* device, 
	const float* data, uint32_t count, uint32_t stride)
{
	// Create a vertex buffer that stores cube vertices
	Diligent::BufferDesc VertBuffDesc;
	VertBuffDesc.Name = "Float vertex buffer";
	VertBuffDesc.Usage = Diligent::USAGE_STATIC;
	VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
	VertBuffDesc.uiSizeInBytes = stride * count;

	Diligent::BufferData VBData;
	VBData.pData = data;
	VBData.DataSize = stride * count;

	std::shared_ptr<pgBuffer> buffer = std::make_shared<pgBuffer>(count);

	device->CreateBuffer(VertBuffDesc, &VBData, &buffer->m_pBuffer);

	return buffer;
}

std::shared_ptr<pgBuffer> pgSceneAss::createUIntIndexBuffer(Diligent::IRenderDevice* device, 
	const uint32_t* data, uint32_t count)
{
	Diligent::BufferDesc IndBuffDesc;
	IndBuffDesc.Name = "UInt index buffer";
	IndBuffDesc.Usage = Diligent::USAGE_STATIC;
	IndBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
	IndBuffDesc.uiSizeInBytes = sizeof(uint32_t) * count;

	Diligent::BufferData IBData;
	IBData.pData = data;
	IBData.DataSize = sizeof(uint32_t) * count;

	std::shared_ptr<pgBuffer> buffer = std::make_shared<pgBuffer>(count);

	device->CreateBuffer(IndBuffDesc, &IBData, &buffer->m_pBuffer);

	return buffer;
}

std::shared_ptr<pgMesh> pgSceneAss::CreateMesh()
{
	std::shared_ptr<pgMesh> mesh = std::make_shared<pgMesh>(m_pDevice, m_pImmediateContext);

	return mesh;
}

std::shared_ptr<pgMaterial> pgSceneAss::CreateMaterial()
{
	std::shared_ptr<pgMaterial> mat = std::make_shared<pgMaterial>(m_pDevice);

	return mat;
}

std::shared_ptr<pgTexture> pgSceneAss::CreateTexture(const std::wstring& fileName) 
{
	std::shared_ptr<pgTexture> tex = std::make_shared<pgTexture>();

	return tex;
}

std::shared_ptr<pgTexture> pgSceneAss::CreateTexture2D(uint16_t width, uint16_t height)
{
	std::shared_ptr<pgTexture> tex = std::make_shared<pgTexture>();

	return tex;
}

std::shared_ptr<pgBuffer> pgSceneAss::CreateFloatVertexBuffer(const float* data, uint32_t count, uint32_t stride) {
	return createFloatVertexBuffer(m_pDevice, data, count, stride);
}

std::shared_ptr<pgBuffer> pgSceneAss::CreateUIntIndexBuffer(const uint32_t* data, uint32_t count) {
	return createUIntIndexBuffer(m_pDevice, data, count);
}


void pgSceneAss::Render( pgRenderEventArgs& args )
{
    if ( m_pRootNode )
    {
        m_pRootNode->Render( args );
    }
}

