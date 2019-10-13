#pragma once

#include "SampleBase.h"
#include "BasicMath.h"

#include "MapHelper.h"

#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "CommonlyUsedStates.h"
#include "ShaderMacroHelper.h"
#include "FileSystem.h"
#include "imgui.h"
#include "imGuIZMO.h"
#include "InputController.h"

#include <locale>
#include <codecvt>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <filesystem>

#include <vector>
#include <map>
#include <ctime>
#include <algorithm>
#include <random>
#include <atomic>
#include <mutex>
#include <thread>

#include <memory>

#pragma warning(disable : 4996)

// Report an error to the Debug output in Visual Studio, display a message box with the error message and throw an exception.
void ReportErrorAndThrow(const std::string& file, int line, const std::string& function, const std::string& message);

// Report an error message and throw an std::exception.
#define ReportError( msg ) ReportErrorAndThrow( __FILE__, __LINE__, __FUNCTION__, (msg) )

template<typename T>
inline void SafeDelete(T& ptr)
{
	if (ptr != NULL)
	{
		delete ptr;
		ptr = NULL;
	}
}

template<typename T>
inline void SafeDeleteArray(T& ptr)
{
	if (ptr != NULL)
	{
		delete[] ptr;
		ptr = NULL;
	}
}

// Convert a multi-byte character string (UTF-8) to a wide (UTF-16) encoded string.
inline std::wstring ConvertString(const std::string& string)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(string);
}

// Converts a wide (UTF-16) encoded string into a multi-byte (UTF-8) character string.
inline std::string ConvertString(const std::wstring& wstring)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(wstring);
}


class Object
{
public:
	virtual ~Object() {
		//
	}
};

class Camera : public Object
{
	Diligent::MouseState m_LastMouseState;

	Diligent::float4x4   m_cameraTransform;
	Diligent::float3 pos;
	Diligent::float3 look;

	const Diligent::float3 up = { 0.0f, 1.0f, 0.0f };

public:
	Camera();

	virtual ~Camera();

	void reset();

	void update(Diligent::InputController* pInputController, float ElapsedTime);

	const Diligent::float3& getPos() const {
		return pos;
	}

	const Diligent::float3& getLook() const {
		return look;
	}

	const Diligent::float4x4& getTransform() const {
		return m_cameraTransform;
	}

};

class RenderEventArgs
{
public:
	void* Caller;
	float CurrTime;
	float ElapsedTime;

	Camera* pCamera;
public:
	RenderEventArgs()	{
	}

	void set(void* caller, float currentTime, float elapsedTime, Camera* camera) {
		Caller = caller;
		pCamera = camera;
		CurrTime = currentTime;
		ElapsedTime = elapsedTime;
	}

};

class Texture : public Object
{
public:
	// Get the width of the textures in texels.
	virtual uint16_t GetWidth() const = 0;
	// Get the height of the texture in texles.
	virtual uint16_t GetHeight() const = 0;
	// Get the depth of the texture in texture slices for 3D textures, or 
	// cube faces for cubemap textures.
	virtual uint16_t GetDepth() const = 0;

	// Get the bits-per-pixel of the texture.
	virtual uint8_t GetBPP() const = 0;

	// Check to see if this texture has an alpha channel.
	virtual bool IsTransparent() const = 0;
};


class Buffer : public Object
{
public:
};

// A material class is used to wrap the shaders and to 
// manage the shader parameters.
class Material : public Object
{
public:
	typedef Object base;

	// These are the texture slots that will be used to bind the material's textures
	// to the shader. Make sure you use the same texture slots in your own shaders.
	enum class TextureType
	{
		Ambient = 0,
		Emissive = 1,
		Diffuse = 2,
		Specular = 3,
		SpecularPower = 4,
		Normal = 5,
		Bump = 6,
		Opacity = 7,
	};

	Material(Diligent::IRenderDevice* device);

	virtual ~Material();

	virtual void Bind(Diligent::IShader* pShader) const;

	const Diligent::float4& GetDiffuseColor() const;
	void SetDiffuseColor(const Diligent::float4& diffuse);

	const Diligent::float4& GetGlobalAmbientColor() const;
	void SetGlobalAmbientColor(const Diligent::float4& globalAmbient);

	const Diligent::float4& GetAmbientColor() const;
	void SetAmbientColor(const Diligent::float4& ambient);

	const Diligent::float4& GetEmissiveColor() const;
	void SetEmissiveColor(const Diligent::float4& emissive);

	const Diligent::float4& GetSpecularColor() const;
	void SetSpecularColor(const Diligent::float4& phong);

	float GetSpecularPower() const;
	void SetSpecularPower(float phongPower);

	const Diligent::float4& GetReflectance() const;
	void SetReflectance(const Diligent::float4& reflectance);

	const float GetOpacity() const;
	void SetOpacity(float Opacity);

	float GetIndexOfRefraction() const;
	void SetIndexOfRefraction(float indexOfRefraction);

	// When using bump maps, we can adjust the "intensity" of the normals generated
	// from the bump maps. We can even inverse the normals by using a negative intensity.
	// Default bump intensity is 1.0 and a value of 0 will remove the bump effect altogether.
	float GetBumpIntensity() const;
	void SetBumpIntensity(float bumpIntensity);

	std::shared_ptr<Texture> GetTexture(TextureType ID) const;
	void SetTexture(TextureType type, std::shared_ptr<Texture> texture);

	// This material defines a transparent material 
	// if the opacity value is < 1, or there is an opacity map, or the diffuse texture has an alpha channel.
	bool IsTransparent() const;

private:
	// If the material properties have changed, update the contents of the constant buffer.
	void UpdateConstantBuffer();

	__declspec(align(16)) struct MaterialProperties
	{
		MaterialProperties()
			: m_GlobalAmbient(0.1f, 0.1f, 0.15f, 1)
			, m_AmbientColor(0, 0, 0, 1)
			, m_EmissiveColor(0, 0, 0, 1)
			, m_DiffuseColor(1, 1, 1, 1)
			, m_SpecularColor(0, 0, 0, 1)
			, m_Reflectance(0, 0, 0, 0)
			, m_Opacity(1.0f)
			, m_SpecularPower(-1.0f)
			, m_IndexOfRefraction(-1.0f)
			, m_HasAmbientTexture(false)
			, m_HasEmissiveTexture(false)
			, m_HasDiffuseTexture(false)
			, m_HasSpecularTexture(false)
			, m_HasSpecularPowerTexture(false)
			, m_HasNormalTexture(false)
			, m_HasBumpTexture(false)
			, m_HasOpacityTexture(false)
			, m_BumpIntensity(5.0f)
			, m_SpecularScale(128.0f)
			, m_AlphaThreshold(0.1f)
		{}

		Diligent::float4   m_GlobalAmbient;
		//-------------------------- ( 16 bytes )
		Diligent::float4   m_AmbientColor;
		//-------------------------- ( 16 bytes )
		Diligent::float4   m_EmissiveColor;
		//-------------------------- ( 16 bytes )
		Diligent::float4   m_DiffuseColor;
		//-------------------------- ( 16 bytes )
		Diligent::float4   m_SpecularColor;
		//-------------------------- ( 16 bytes )
		Diligent::float4   m_Reflectance;
		//-------------------------- ( 16 bytes )
		// If Opacity < 1, then the material is transparent.
		float       m_Opacity;
		float       m_SpecularPower;
		// For transparent materials, IOR > 0.
		float       m_IndexOfRefraction;
		uint32_t    m_HasAmbientTexture;
		//-------------------------- ( 16 bytes )
		uint32_t    m_HasEmissiveTexture;
		uint32_t    m_HasDiffuseTexture;
		uint32_t    m_HasSpecularTexture;
		uint32_t    m_HasSpecularPowerTexture;
		//-------------------------- ( 16 bytes )
		uint32_t    m_HasNormalTexture;
		uint32_t    m_HasBumpTexture;
		uint32_t    m_HasOpacityTexture;
		float       m_BumpIntensity;    // When using bump textures (heightmaps) we need 
										// to scale the height values so the normals are visible.
		//-------------------------- ( 16 bytes )
		float       m_SpecularScale;    // When reading specular power from a texture, 
										// we need to scale it into the correct range.
		float       m_AlphaThreshold;   // Pixels with alpha < m_AlphaThreshold will be discarded.
		Diligent::float2   m_Padding;          // Pad to 16 byte boundary.
		//-------------------------- ( 16 bytes )
	};  //--------------------------- ( 16 * 10 = 160 bytes )

	// Material properties have to be 16 byte aligned.
	// To guarantee alignment, we'll use _aligned_malloc to allocate memory
	// for the material properties.
	MaterialProperties* m_pProperties;

	Diligent::RefCntAutoPtr<Diligent::IRenderDevice>		  m_RenderDevice;

	// Constant buffer that stores material properties.
	// This material owns this constant buffer and will delete it 
	// when the material is destroyed.
	Diligent::RefCntAutoPtr<Diligent::IBuffer>		  m_pConstantBuffer;

	// Textures are stored by which texture unit (or texture register)
	// they are bound to.
	typedef std::map<TextureType, std::shared_ptr<Texture> > TextureMap;
	TextureMap m_Textures;

	// Set to true if the contents of the constant buffer needs to be updated.
	bool    m_Dirty;
};

// Defines either a semantic (HLSL) or an input index (GLSL/HLSL)
// to bind an input buffer.
struct BufferBinding
{
	BufferBinding()
		: Index(0)
	{}

	BufferBinding(const std::string& name, unsigned int index)
		: Name(name)
		, Index(index)
	{}

	// Provide the < operator for STL containers.
	bool operator<(const BufferBinding& rhs) const
	{
		if (Name < rhs.Name) return true;
		if (Name > rhs.Name) return false;
		// Names are equal...
		if (Index < rhs.Index) return true;
		if (Index > rhs.Index) return false;
		// Indexes are equal...

		return false;
	}

	std::string Name;
	unsigned int Index;
};

// A mesh contains the geometry and materials required to render this mesh.
class Mesh : public Object
{
protected:
	Diligent::RefCntAutoPtr<Diligent::IRenderDevice>		  m_pDevice;
	Diligent::RefCntAutoPtr<Diligent::IDeviceContext>		  m_pImmediateContext;
public:
	Mesh(Diligent::IRenderDevice* device, Diligent::IDeviceContext* ctx)
		: m_pDevice(device)
		, m_pImmediateContext(ctx)
	{
		//
	}

	virtual ~Mesh() {
		//
	}

	// Adds a buffer to this mesh with a particular semantic (HLSL) or register ID (GLSL).
	virtual void AddVertexBuffer(const BufferBinding& binding, std::shared_ptr<Buffer> buffer) {
		//
	}

	virtual void SetIndexBuffer(std::shared_ptr<Buffer> buffer) {
		//
	}

	virtual void SetMaterial(std::shared_ptr<Material> material) {
		//
	}

	virtual std::shared_ptr<Material> GetMaterial() const {
		return nullptr;
	}

	virtual void Render(RenderEventArgs& renderEventArgs) = 0;
};

class SceneNode : public Object, public std::enable_shared_from_this<SceneNode>
{
public:
	typedef Object base;

	explicit SceneNode(const Diligent::float4x4& localTransform = Diligent::float4x4::Identity());
	virtual ~SceneNode();

	/**
	 * Assign a name to this scene node so that it can be searched for later.
	 */
	const std::string& GetName() const;
	void SetName(const std::string& name);

	Diligent::float4x4 GetLocalTransform() const;
	void SetLocalTransform(const Diligent::float4x4& localTransform);
	Diligent::float4x4 GetInverseLocalTransform() const;
	Diligent::float4x4 GetWorldTransfom() const;
	void SetWorldTransform(const Diligent::float4x4& worldTransform);

	Diligent::float4x4 GetInverseWorldTransform() const;

	void AddChild(std::shared_ptr<SceneNode> pNode);
	void RemoveChild(std::shared_ptr<SceneNode> pNode);
	void SetParent(std::weak_ptr<SceneNode> pNode);

	void AddMesh(std::shared_ptr<Mesh> mesh);
	void RemoveMesh(std::shared_ptr<Mesh> mesh);

	/**
	 * Render meshes associated with this scene node.
	 * This method will traverse it's children.
	 */
	void Render(RenderEventArgs& renderEventArgs);

protected:

	Diligent::float4x4 GetParentWorldTransform() const;

private:
	typedef std::vector< std::shared_ptr<SceneNode> > NodeList;
	typedef std::multimap< std::string, std::shared_ptr<SceneNode> > NodeNameMap;
	typedef std::vector< std::shared_ptr<Mesh> > MeshList;

	std::string m_Name;

	// Transforms node from parent's space to world space for rendering.
	Diligent::float4x4 m_LocalTransform;
	// This is the inverse of the local -> world transform.
	Diligent::float4x4 m_InverseTransform;

	std::weak_ptr<SceneNode> m_pParentNode;
	NodeList m_Children;
	NodeNameMap m_ChildrenByName;
	MeshList m_Meshes;
};

class Scene : public Object {
protected:
	std::shared_ptr<SceneNode> m_pRootNode; 
public:
	Scene() {
		//
	}

	Scene(std::shared_ptr<SceneNode> root)
		: m_pRootNode(root)
	{
	}

	virtual std::shared_ptr<SceneNode> GetRootNode() const {
		return m_pRootNode;
	}

	virtual void Render(RenderEventArgs& renderEventArgs);
};


struct pgPassCreateInfo {
	Diligent::IRenderDevice*		device;
	Diligent::IDeviceContext*		ctx;
	Diligent::IEngineFactory*		factory;

	Diligent::SwapChainDesc			desc;

	std::shared_ptr<Scene>			scene;

	pgPassCreateInfo() : scene(0)
	{}
};

class pgPass : public Object
{
	bool m_bEnabled;
protected:
	Diligent::RefCntAutoPtr<Diligent::IRenderDevice>	m_pDevice;
	Diligent::RefCntAutoPtr<Diligent::IDeviceContext>	m_pImmediateContext;
	Diligent::RefCntAutoPtr<Diligent::IEngineFactory>   m_pEngineFactory;

	Diligent::SwapChainDesc								m_desc;
	std::shared_ptr<Scene>								m_scene;

public:
	pgPass(const pgPassCreateInfo& ci) 
		: m_bEnabled(true)
		, m_pDevice(ci.device)
		, m_pImmediateContext(ci.ctx)
		, m_pEngineFactory(ci.factory)
		, m_desc(ci.desc)
		, m_scene(ci.scene)
	{
	}

	// Enable or disable the pass. If a pass is disabled, the technique will skip it.
	void SetEnabled(bool enabled) {
		m_bEnabled = enabled;
	}

	bool IsEnabled() const {
		return m_bEnabled;
	}

	// Render the pass. This should only be called by the pgTechnique.
	virtual void Update(RenderEventArgs& e) = 0;
	virtual void Render(RenderEventArgs& e) = 0;
};

class pgTechnique : public Object
{
public:
	pgTechnique();
	virtual ~pgTechnique();

	// Add a pass to the technique. The ID of the added pass is returned
	// and can be used to retrieve the pass later.
	unsigned int addPass(std::shared_ptr<pgPass> pass);
	std::shared_ptr<pgPass> getPass(unsigned int ID) const;

	// Render the scene using the passes that have been configured.
	virtual void Update(RenderEventArgs& e);
	virtual void Render(RenderEventArgs& e);

private:
	typedef std::vector<std::shared_ptr<pgPass>> RenderPassList;
	RenderPassList m_Passes;

};

// Base pass provides implementations for functions used by most passes.
class pgBasePass : public pgPass
{
public:
	typedef pgPass base;

	pgBasePass(const pgPassCreateInfo& ci);
	virtual ~pgBasePass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void Update(RenderEventArgs& e);
	virtual void Render(RenderEventArgs& e);
};