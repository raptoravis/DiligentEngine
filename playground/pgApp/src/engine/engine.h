#pragma once

#include "BasicMath.h"
#include "SampleBase.h"

#include "MapHelper.h"

#include "CommonlyUsedStates.h"
#include "FileSystem.h"
#include "GraphicsUtilities.h"
#include "InputController.h"
#include "ShaderMacroHelper.h"
#include "TextureUtilities.h"
#include "imGuIZMO.h"
#include "imgui.h"

#include <codecvt>
#include <locale>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>

#include <algorithm>
#include <atomic>
#include <ctime>
#include <map>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

#include <memory>

#pragma warning(disable : 4996)

// Report an error message and throw an std::exception.
#define ReportError(msg) ade::ReportErrorAndThrow(__FILE__, __LINE__, __FUNCTION__, (msg))

namespace ade
{
// Report an error to the Debug output in Visual Studio, display a message box with the error
// message and throw an exception.
void ReportErrorAndThrow(const std::string& file, int line, const std::string& function,
                         const std::string& message);

template <typename T>
inline void SafeDelete(T& ptr)
{
    if (ptr != NULL) {
        delete ptr;
        ptr = NULL;
    }
}

template <typename T>
inline void SafeDeleteArray(T& ptr)
{
    if (ptr != NULL) {
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


class App;
class Technique;
class Pass;
class Pipeline;
class Scene;
class SceneNode;
class Material;
class Mesh;


class Object
{
    static uint32_t s_uuid;

  protected:
    uint32_t m_uuid;

  public:
    Object();
    virtual ~Object()
    {
        //
    }
};


class Visitor : public Object
{
  public:
    virtual void Visit(Scene& scene, Pipeline* pipeline) = 0;
    virtual void Visit(SceneNode& node, Pipeline* pipeline) = 0;
    virtual void Visit(Mesh& mesh, Pipeline* pipeline) = 0;
};


class Camera : public Object
{
    Diligent::MouseState m_LastMouseState;

    Diligent::float4x4 m_viewMatrix;
    Diligent::float4x4 m_projectionMatrix;

    Diligent::float3 pos;
    Diligent::float3 look;

    // original
    const Diligent::float3 _pos;
    const Diligent::float3 _look;

    const Diligent::float3 up = { 0.0f, 1.0f, 0.0f };

  public:
    Camera(const Diligent::float3 pos, const Diligent::float3 dir);

    virtual ~Camera();

    void reset();
    void reset(const Diligent::float3& p, const Diligent::float3& dir);
    void setProjectionMatrix(float NearPlane, float FarPlane, bool bIsGL = false);

    void update(Diligent::InputController* pInputController, float ElapsedTime);

    void setPos(const Diligent::float3& p);
    const Diligent::float3& getPos() const { return pos; }

    void setLook(const Diligent::float3& dir);
    Diligent::float3 getLook() const;

    const Diligent::float4x4& getViewMatrix() const { return m_viewMatrix; }

    const Diligent::float4x4& getProjectionMatrix() const { return m_projectionMatrix; }
};

// CPU Access. Used for textures and Buffers
enum CPUAccess {
    None = 0,            // No CPU access to this texture is necessary.
    Read = (1 << 0),     // CPU reads permitted.
    Write = (1 << 1),    // CPU writes permitted.
    ReadWrite = Read | Write
};


class RenderEventArgs
{
  public:
    float CurrTime;
    float ElapsedTime;

    App* pApp;
    Camera* pCamera;

    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> pDeviceContext;

  public:
    RenderEventArgs();

    void set(float currentTime, float elapsedTime, App* caller, Camera* camera,
             Diligent::RefCntAutoPtr<Diligent::IDeviceContext> ctx);
};

// Defines either a semantic (HLSL) or an input index (GLSL/HLSL)
// to bind an input buffer.
struct BufferBinding {
    BufferBinding() : Index(0) {}

    BufferBinding(const std::string& name, unsigned int index) : Name(name), Index(index) {}

    // Provide the < operator for STL containers.
    bool operator<(const BufferBinding& rhs) const
    {
        if (Name < rhs.Name)
            return true;
        if (Name > rhs.Name)
            return false;
        // Names are equal...
        if (Index < rhs.Index)
            return true;
        if (Index > rhs.Index)
            return false;
        // Indexes are equal...

        return false;
    }

    std::string Name;
    unsigned int Index;
};


class Buffer;
class Texture;
class ConstantBuffer;
class StructuredBuffer;
class SamplerState;

class ShaderParameter : public Object
{
  public:
    typedef Object base;

    enum class Type {
        Invalid,     // Invalid parameter. Doesn't store a type.
        Sampler,     // Texture sampler.
        CBuffer,     // ConstantBuffers
        Buffer,      // Buffers, StructuredBuffers.
        RWBuffer,    // Read/write structured buffers.
        Texture,     // Texture.
        RWTexture    // Texture that can be written to in a shader (using Store operations).
    };

    // Shader resource parameter.
    ShaderParameter(const std::string& name, const std::string& shaderType, Type parameterType);

    bool IsValid() const;

    // Get the type of the stored parameter.
    Type GetType() const;
    const std::string& GetName() const;

    // Bind the shader parameter to a specific slot for the given shader type.
    virtual void Bind();
    virtual void UnBind();

    std::weak_ptr<Object> Get();
    void Set(std::shared_ptr<Object> resource);

  private:
    std::string m_Name;

    uint32_t m_uiSlotID;
    const std::string m_ShaderType;
    Type m_ParameterType;

    // Shader parameter does not take ownership of these types.
    std::weak_ptr<Object> m_pResource;
};


class Shader : public Object
{
  public:
    typedef Object base;

    enum ShaderType {
        UnknownShaderType = 0,
        VertexShader,
        TessellationControlShader,       // Hull Shader in DirectX
        TessellationEvaluationShader,    // Domain Shader in DirectX
        GeometryShader,
        PixelShader,
        ComputeShader,
    };

    typedef std::map<std::string, std::string> ShaderMacros;

    Shader();
    virtual ~Shader();

    ShaderType GetType() const;

    // Shader loading
    bool LoadShaderFromFile(ShaderType type, const std::string& fileName,
                            const std::string& entryPoint = "main",
                            const std::string& searchPaths = "",
                            bool UseCombinedTextureSamplers = false,
                            const Diligent::ShaderMacro* shaderMacros = nullptr);

    Diligent::RefCntAutoPtr<Diligent::IShader> GetShader() { return m_pShader; }

    // virtual uint32_t GetConstantBufferIndex( const std::string& name );
    ShaderParameter& GetShaderParameterByName(const std::string& name) const;

    // virtual ConstantBuffer* GetConstantBufferByName( const std::string& name );

    // Check to see if this shader supports a given semantic.
    bool HasSemantic(const BufferBinding& binding) const;
    uint32_t GetSlotIDBySemantic(const BufferBinding& binding) const;

    typedef std::vector<std::shared_ptr<ShaderParameter>> ParametersList;
    ParametersList GetStaticVariables();
    ParametersList GetDynamicVariables();
    ParametersList GetStaticSamplers();

    // void Bind();
    // void UnBind();

  protected:
    // Destroy the contents of this shader (in case we are loading a new shader).
    void Destroy();

  private:
    ShaderType m_ShaderType;
    Diligent::RefCntAutoPtr<Diligent::IShader> m_pShader;

    typedef std::map<std::string, std::shared_ptr<ShaderParameter>> ParameterMap;
    ParameterMap m_ShaderParameters;

    // A map to convert a vertex attribute semantic to a slot.
    typedef std::map<BufferBinding, uint32_t> SemanticMap;
    SemanticMap m_InputSemantics;

    // Parameters necessary to reload the shader at runtime if it is modified on disc.
    ShaderMacros m_ShaderMacros;
    std::string m_EntryPoint;
    std::string m_Profile;
    std::string m_ShaderFileName;
};

class Resource : public Object
{
    //
};

class Buffer : public Resource
{
  protected:
    // The stride of the vertex buffer in bytes.
    uint32_t m_uiStride;
    // How this buffer should be bound.
    uint32_t m_BindFlags;
    // The number of elements in this buffer.
    uint32_t m_uiCount;
    bool m_bIsBound;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_pBuffer;

  public:
    enum BufferType { Unknown = 0, VertexBuffer, IndexBuffer, StructuredBuffer, ConstantBuffer };

    Buffer(uint32_t stride, uint32_t count, Diligent::IBuffer* buffer);
    virtual ~Buffer();

    Diligent::IBuffer* GetBuffer();

    uint32_t GetCount() const;
    uint32_t GetSize() const;

    Diligent::IBufferView* GetUnorderedAccessView();
    Diligent::IBufferView* GetShaderResourceView();

    // Bind the buffer for rendering.
    virtual bool Bind(unsigned int id, Shader::ShaderType shaderType,
                      ShaderParameter::Type parameterType);
    // Unbind the buffer for rendering.
    virtual void UnBind(unsigned int id, Shader::ShaderType shaderType,
                        ShaderParameter::Type parameterType);

    // Copy the contents of another buffer to this one.
    // Buffers must be the same size in bytes.
    virtual void Copy(std::shared_ptr<Buffer> other);

    // Is this an index buffer or an attribute/vertex buffer?
    BufferType GetType() const;
    // How many elements does this buffer contain?
    unsigned int GetElementCount() const { return m_uiCount; }
};

class ConstantBuffer : public Buffer
{
    typedef Buffer base;

  public:
    ConstantBuffer(uint32_t size, void* data = 0);
    virtual ~ConstantBuffer();

    // The contents of a constant buffer can also be updated.
    template <typename T>
    void Set(const T& data)
    {
        Set(&data, sizeof(T));
    }

    // Always returns BufferType::ConstantBuffer
    virtual BufferType GetType() const;
    // Constant buffers only have 1 element.
    virtual unsigned int GetElementCount() const;

    virtual bool Bind(unsigned int id, Shader::ShaderType shaderType,
                      ShaderParameter::Type parameterType);
    virtual void UnBind(unsigned int id, Shader::ShaderType shaderType,
                        ShaderParameter::Type parameterType);

    // Copy the contents of a buffer to this one.
    // Buffers must be the same size.
    virtual void Copy(std::shared_ptr<ConstantBuffer> other);

    // Implementations must provide this method.
    virtual void Set(const void* data, size_t size);

  protected:
};


class StructuredBuffer : public Buffer
{
    typedef Buffer base;

  public:
    StructuredBuffer(const void* data, uint32_t count, uint32_t stride,
                     CPUAccess cpuAccess = CPUAccess::None, bool bUAV = false);
    virtual ~StructuredBuffer();

    // Bind the buffer for rendering.
    virtual bool Bind(unsigned int id, Shader::ShaderType shaderType,
                      ShaderParameter::Type parameterType);
    // Unbind the buffer for rendering.
    virtual void UnBind(unsigned int id, Shader::ShaderType shaderType,
                        ShaderParameter::Type parameterType);

    // Is this an index buffer or an attribute/vertex buffer?
    virtual BufferType GetType() const;
    // How many elements does this buffer contain?
    virtual unsigned int GetElementCount() const;

    // Copy the contents of another buffer to this one.
    // Buffers must be the same size.
    virtual void Copy(std::shared_ptr<StructuredBuffer> other);

    // Set the buffer data.
    template <typename T>
    void Set(const std::vector<T>& values)
    {
        SetData((void*)values.data(), sizeof(T), 0, values.size());
    }

    // Clear the contents of the buffer.
    virtual void Clear();

  protected:
    virtual void SetData(void* data, size_t elementSize, size_t offset, size_t numElements);
};

class SamplerState : public Resource
{
    typedef Resource base;

    Diligent::StaticSamplerDesc m_desc;

  public:
    SamplerState(const Diligent::StaticSamplerDesc& desc);
    virtual ~SamplerState();

    void Set(const Diligent::StaticSamplerDesc& desc);
    const Diligent::StaticSamplerDesc& Get() const;

    // Bind this sampler state to the ID for a specific shader type.
    virtual void Bind(uint32_t ID, Shader::ShaderType shaderType,
                      ShaderParameter::Type parameterType);
    // Unbind the sampler state.
    virtual void UnBind(uint32_t ID, Shader::ShaderType shaderType,
                        ShaderParameter::Type parameterType);
};


/**
 * Flags to specify which value should be cleared.
 */
enum class ClearFlags : uint8_t {
    Color = 1 << 0,
    Depth = 1 << 1,
    Stencil = 1 << 2,
    DepthStencil = Depth | Stencil,
    All = Color | Depth | Stencil,
};


class Texture : public Resource
{
  protected:
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_pTexture;

  public:
    Texture(Diligent::ITexture* texture);
    virtual ~Texture();

    Diligent::ITexture* GetTexture();

    // Get the width of the textures in texels.
    uint16_t GetWidth() const;
    // Get the height of the texture in texles.
    uint16_t GetHeight() const;
    // Get the depth of the texture in texture slices for 3D textures, or
    // cube faces for cubemap textures.
    uint16_t GetDepth() const;

    // Get the bits-per-pixel of the texture.
    uint8_t GetBPP() const;

    // Check to see if this texture has an alpha channel.
    bool IsTransparent() const;

    Diligent::ITextureView* GetShaderResourceView();
    Diligent::ITextureView* GetDepthStencilView();
    Diligent::ITextureView* GetRenderTargetView();
    Diligent::ITextureView* GetUnorderedAccessView();

    void Clear(ClearFlags clearFlags, const Diligent::float4& color, float depth,
               uint8_t stencil);
    void Copy(Texture* dstTexture);

    void Bind(uint32_t ID, Shader::ShaderType shaderType, ShaderParameter::Type parameterType);
    void UnBind(uint32_t ID, Shader::ShaderType shaderType, ShaderParameter::Type parameterType);
};

class RenderTarget : public Object
{
    typedef std::vector<std::shared_ptr<Texture>> TextureList;
    typedef std::vector<std::shared_ptr<Buffer>> StructuredBufferList;

    TextureList m_Textures;

    StructuredBufferList m_StructuredBuffers;

    // The width in pixels of textures associated to this render target.
    uint16_t m_Width;
    // The height in pixels of textures associated to this render target.
    uint16_t m_Height;

    // Check to see if the render target is valid.
    bool m_bCheckValidity;

  public:
    enum class AttachmentPoint : uint8_t {
        Color0,          // Must be a uncompressed color format.
        Color1,          // Must be a uncompressed color format.
        Color2,          // Must be a uncompressed color format.
        Color3,          // Must be a uncompressed color format.
        Color4,          // Must be a uncompressed color format.
        Color5,          // Must be a uncompressed color format.
        Color6,          // Must be a uncompressed color format.
        Color7,          // Must be a uncompressed color format.
        Depth,           // Must be a texture with a depth format.
        DepthStencil,    // Must be a texture with a depth/stencil format.
        NumAttachmentPoints
    };

    RenderTarget();
    virtual ~RenderTarget();
    /**
     * Attach a texture to the render target.
     * The dimension of all textures attached to a render target
     * must match.
     *
     * To remove a texture from an attachment point, just attach a NULL texture.
     */
    void AttachTexture(AttachmentPoint attachment, std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> GetTexture(AttachmentPoint attachment);

    uint32_t GetNumRTVs() const;

    /**
     * Clear the contents of a texture attached to a specific attachment point.
     * @param attachemnt The attachment point of which to clear the contents of the texture.
     * @param clearFlags Which values should be cleared.
     * @param color The clear color to use for color attachment points.
     * @param depth The depth value to use for depth attachment points.
     * @param stencil The stencil value to use for stencil attachment points.
     */
    void Clear(AttachmentPoint attachemnt, ClearFlags clearFlags = ClearFlags::All,
               const Diligent::float4& color = Diligent::float4(0, 0, 0, 0), float depth = 1.0f,
               uint8_t stencil = 0);

    /**
     * Clear the contents of all of the textures attached to the render target.
     * @param clearFlags Which values should be cleared.
     * @param color The clear color to use for color attachment points.
     * @param depth The depth value to use for depth attachment points.
     * @param stencil The stencil value to use for stencil attachment points.
     */
    void Clear(ClearFlags clearFlags = ClearFlags::All,
               const Diligent::float4& color = Diligent::float4(0, 0, 0, 0), float depth = 1.0f,
               uint8_t stencil = 0);

    /**
     * Generate mipmaps for all of the textures that are attached to the render target.
     */
    void GenerateMipMaps();

    /**
     * StructuredBuffers can be written to in a shader. StructuredBuffers must be bound to the
     * rendering pipeline at the same time as render target textures and depth stencil buffers.
     * The maximum number of StructuredBuffers that can be attached to a render target
     * are 8 - num color textures. So there can only be a total of 8 color textures
     * and RWbuffers attached to the render target at any time.
     */
    void AttachStructuredBuffer(uint8_t slot, std::shared_ptr<Buffer> rwBuffer);
    std::shared_ptr<Buffer> GetStructuredBuffer(uint8_t slot);

    /**
     * Resize the color and depth/stencil textures that are associated to this render target view.
     * Resizing a texture will clear it's contents.
     */
    void Resize(uint16_t width, uint16_t height);

    /**
     * Bind this render target to the rendering pipeline.
     * It will remain the active render target until another RenderTarget is bound
     * using this same method.
     */
    void Bind();

    /**
     * Unbind this render target from the rendering pipeline.
     */
    void UnBind();

    /**
     * After attaching color, depth, stencil, and StructuredBuffers to the render target,
     * you can check if the render target is valid using this method.
     * The render target will also be checked for validity before it is bound
     * to rendering pipeline (using the RenderTarget::Bind method).
     */
    bool IsValid() const;
};

// A material class is used to wrap the shaders and to
// manage the shader parameters.
class Material : public Object
{
  public:
    typedef Object base;

    // These are the texture slots that will be used to bind the material's textures
    // to the shader. Make sure you use the same texture slots in your own shaders.
    enum class TextureType {
        Ambient = 0,
        Emissive = 1,
        Diffuse = 2,
        Specular = 3,
        SpecularPower = 4,
        Normal = 5,
        Bump = 6,
        Opacity = 7,
    };

    Material();

    virtual ~Material();

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

    void Bind(std::weak_ptr<Shader> wpShader);

    // This material defines a transparent material
    // if the opacity value is < 1, or there is an opacity map, or the diffuse texture has an alpha
    // channel.
    bool IsTransparent() const;

    __declspec(align(16)) struct MaterialProperties {
        MaterialProperties()
            : m_GlobalAmbient(0.1f, 0.1f, 0.15f, 1), m_AmbientColor(0, 0, 0, 1),
              m_EmissiveColor(0, 0, 0, 1), m_DiffuseColor(1, 1, 1, 1), m_SpecularColor(0, 0, 0, 1),
              m_Reflectance(0, 0, 0, 0), m_Opacity(1.0f), m_SpecularPower(-1.0f),
              m_IndexOfRefraction(-1.0f), m_HasAmbientTexture(false), m_HasEmissiveTexture(false),
              m_HasDiffuseTexture(false), m_HasSpecularTexture(false),
              m_HasSpecularPowerTexture(false), m_HasNormalTexture(false), m_HasBumpTexture(false),
              m_HasOpacityTexture(false), m_BumpIntensity(5.0f), m_SpecularScale(128.0f),
              m_AlphaThreshold(0.1f)
        {
        }

        Diligent::float4 m_GlobalAmbient;
        //-------------------------- ( 16 bytes )
        Diligent::float4 m_AmbientColor;
        //-------------------------- ( 16 bytes )
        Diligent::float4 m_EmissiveColor;
        //-------------------------- ( 16 bytes )
        Diligent::float4 m_DiffuseColor;
        //-------------------------- ( 16 bytes )
        Diligent::float4 m_SpecularColor;
        //-------------------------- ( 16 bytes )
        Diligent::float4 m_Reflectance;
        //-------------------------- ( 16 bytes )
        // If Opacity < 1, then the material is transparent.
        float m_Opacity;
        float m_SpecularPower;
        // For transparent materials, IOR > 0.
        float m_IndexOfRefraction;
        uint32_t m_HasAmbientTexture;
        //-------------------------- ( 16 bytes )
        uint32_t m_HasEmissiveTexture;
        uint32_t m_HasDiffuseTexture;
        uint32_t m_HasSpecularTexture;
        uint32_t m_HasSpecularPowerTexture;
        //-------------------------- ( 16 bytes )
        uint32_t m_HasNormalTexture;
        uint32_t m_HasBumpTexture;
        uint32_t m_HasOpacityTexture;
        float m_BumpIntensity;    // When using bump textures (heightmaps) we need
                                  // to scale the height values so the normals are visible.
        //-------------------------- ( 16 bytes )
        float m_SpecularScale;         // When reading specular power from a texture,
                                       // we need to scale it into the correct range.
        float m_AlphaThreshold;        // Pixels with alpha < m_AlphaThreshold will be discarded.
        Diligent::float2 m_Padding;    // Pad to 16 byte boundary.
                                       //-------------------------- ( 16 bytes )
    };                                 //--------------------------- ( 16 * 10 = 160 bytes )

    static uint32_t getConstantBufferSize() { return sizeof(MaterialProperties); }

    Material::MaterialProperties* GetMaterialProperties() const { return m_pProperties; }

  private:
    // Material properties have to be 16 byte aligned.
    // To guarantee alignment, we'll use _aligned_malloc to allocate memory
    // for the material properties.
    MaterialProperties* m_pProperties;

    // Textures are stored by which texture unit (or texture register)
    // they are bound to.
    typedef std::map<TextureType, std::shared_ptr<Texture>> TextureMap;
    TextureMap m_Textures;

    // Set to true if the contents of the constant buffer needs to be updated.
    bool m_Dirty;
};


__declspec(align(16)) struct Light {
    enum class LightType : uint32_t { Point = 0, Spot = 1, Directional = 2 };

    /**
     * Position for point and spot lights (World space).
     */
    Diligent::float4 m_PositionWS;
    //--------------------------------------------------------------( 16 bytes )
    /**
     * Direction for spot and directional lights (World space).
     */
    Diligent::float4 m_DirectionWS;
    //--------------------------------------------------------------( 16 bytes )
    /**
     * Position for point and spot lights (View space).
     */
    Diligent::float4 m_PositionVS;
    //--------------------------------------------------------------( 16 bytes )
    /**
     * Direction for spot and directional lights (View space).
     */
    Diligent::float4 m_DirectionVS;
    //--------------------------------------------------------------( 16 bytes )
    /**
     * Color of the light. Diffuse and specular colors are not separated.
     */
    Diligent::float4 m_Color;
    //--------------------------------------------------------------( 16 bytes )
    /**
     * The half angle of the spotlight cone.
     */
    float m_SpotlightAngle;
    /**
     * The range of the light.
     */
    float m_Range;

    /**
     * The intensity of the light.
     */
    float m_Intensity;

    /**
     * Disable or enable the light.
     */
    uint32_t m_Enabled;
    //--------------------------------------------------------------(16 bytes )

    /**
     * True if the light is selected in the editor.
     */
    uint32_t m_Selected;
    /**
     * The type of the light.
     */
    LightType m_Type;

    Diligent::float2 m_Padding;
    //--------------------------------------------------------------(16 bytes )
    //--------------------------------------------------------------( 16 * 7 = 112 bytes )
    Light::Light()
        : m_PositionWS(0, 0, 0, 1), m_DirectionWS(0, 0, -1, 0), m_PositionVS(0, 0, 0, 1),
          m_DirectionVS(0, 0, 1, 0), m_Color(1, 1, 1, 1), m_SpotlightAngle(45.0f), m_Range(100.0f),
          m_Intensity(1.0f), m_Enabled(true), m_Selected(false), m_Type(LightType::Point)
    {
    }
};

// A mesh contains the geometry and materials required to render this mesh.
class Mesh : public Object
{
  protected:
    typedef std::map<BufferBinding, std::shared_ptr<Buffer>> BufferMap;
    BufferMap m_VertexBuffers;

    std::shared_ptr<Buffer> m_pIndexBuffer;
    std::shared_ptr<Material> m_pMaterial;

  public:
    Mesh();
    virtual ~Mesh();

    // Adds a buffer to this mesh with a particular semantic (HLSL) or register ID (GLSL).
    virtual void addVertexBuffer(const BufferBinding& binding, std::shared_ptr<Buffer> buffer);
    virtual void setIndexBuffer(std::shared_ptr<Buffer> buffer);

    virtual void setMaterial(std::shared_ptr<Material> material);
    virtual std::shared_ptr<Material> getMaterial() const;

    virtual void Render(Pipeline* pipeline);
    virtual void Accept(Visitor& visitor, Pipeline* pipeline);
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
    const std::string& getName() const;
    void setName(const std::string& name);

    Diligent::float4x4 getLocalTransform() const;
    void setLocalTransform(const Diligent::float4x4& localTransform);
    Diligent::float4x4 getInverseLocalTransform() const;
    Diligent::float4x4 getWorldTransfom() const;
    void setWorldTransform(const Diligent::float4x4& worldTransform);

    Diligent::float4x4 getInverseWorldTransform() const;

    void addChild(std::shared_ptr<SceneNode> pNode);
    void removeChild(std::shared_ptr<SceneNode> pNode);
    void setParent(std::weak_ptr<SceneNode> pNode);

    void addMesh(std::shared_ptr<Mesh> mesh);
    void RemoveMesh(std::shared_ptr<Mesh> mesh);

    virtual void Accept(Visitor& visitor, Pipeline* pipeline);

  protected:
    Diligent::float4x4 GetParentWorldTransform() const;

  private:
    typedef std::vector<std::shared_ptr<SceneNode>> NodeList;
    typedef std::multimap<std::string, std::shared_ptr<SceneNode>> NodeNameMap;
    typedef std::vector<std::shared_ptr<Mesh>> MeshList;

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

class Scene : public Object
{
    typedef Object base;

  protected:
    std::shared_ptr<SceneNode> m_pRootNode;

  public:
    Scene();
    virtual ~Scene();

    std::shared_ptr<SceneNode> getRootNode() const { return m_pRootNode; }
    void setRootNode(std::shared_ptr<SceneNode> root) { m_pRootNode = root; }

    virtual void Accept(Visitor& visitor, Pipeline* pipeline);

    static std::shared_ptr<Texture> CreateTexture2D(uint16_t width, uint16_t height,
                                                      uint16_t slices,
                                                      Diligent::TEXTURE_FORMAT format,
                                                      CPUAccess cpuAccess, bool gpuWrite,
                                                      bool bGenerateMipmaps = false);
};

class Pipeline : public Object
{
    typedef std::map<Shader::ShaderType, std::shared_ptr<Shader>> ShaderMap;

    bool m_bInited = false;
    uint32_t m_stencilRef = 0;

  protected:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pPSO;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_pSRB;

    Diligent::PipelineStateDesc m_PSODesc;
    ShaderMap m_Shaders;

    std::shared_ptr<RenderTarget> m_pRenderTarget;
    bool m_bDirty;

    virtual void InitPSODesc();

  private:
    void SetStaticVariables();
    void SetDynamicVariables();

    std::vector<Diligent::ShaderResourceVariableDesc> GetDynamicVariables() const;
    std::vector<Diligent::StaticSamplerDesc> GetStaticSamplers() const;

  public:
    Pipeline(std::shared_ptr<RenderTarget> rt);
    virtual ~Pipeline();

    std::shared_ptr<RenderTarget> getRenderTarget() { return m_pRenderTarget; }

    //
    void SetShader(Shader::ShaderType type, std::shared_ptr<Shader> pShader);
    std::shared_ptr<Shader> GetShader(Shader::ShaderType type) const;
    const ShaderMap& GetShaders() const;

    void SetBlendState(const Diligent::BlendStateDesc& blendState);
    Diligent::BlendStateDesc& GetBlendState();

    void SetRasterizerState(const Diligent::RasterizerStateDesc& rasterizerState);
    Diligent::RasterizerStateDesc& GetRasterizerState();

    void SetDepthStencilState(const Diligent::DepthStencilStateDesc& depthStencilState);
    Diligent::DepthStencilStateDesc& GetDepthStencilState();

    void SetStencilRef(uint32_t ref);

    // void SetRenderTarget(std::shared_ptr<RenderTarget> renderTarget);
    std::shared_ptr<RenderTarget> GetRenderTarget() const;

    virtual void Bind();
    virtual void UnBind();
};

class Pass : public Visitor
{
    bool m_bEnabled;

    friend class Mesh;
    friend class Material;

  protected:
    Technique* m_parentTechnique;

  public:
    Pass(Technique* parentTechnique);
    virtual ~Pass();

    // Enable or disable the pass. If a pass is disabled, the technique will skip it.
    virtual void SetEnabled(bool enabled) { m_bEnabled = enabled; }

    virtual bool IsEnabled() const { return m_bEnabled; }

    virtual void PreRender();
    virtual void Render(Pipeline* pipeline);
    virtual void PostRender();

    virtual void Visit(Scene& scene, Pipeline* pipeline);
    virtual void Visit(SceneNode& node, Pipeline* pipeline);
    virtual void Visit(Mesh& mesh, Pipeline* pipeline);
};


class PassPilpeline : public Pass
{
    typedef Pass base;

  protected:
    std::shared_ptr<Scene> m_pScene;
    std::shared_ptr<Pipeline> m_pPipeline;

  public:
    PassPilpeline(Technique* parentTechnique, std::shared_ptr<Scene> scene,
                    std::shared_ptr<Pipeline> pipeline);
    virtual ~PassPilpeline();

    virtual void PreRender();
    virtual void Render(Pipeline* pipeline);
};

class RenderPass;

class Technique : public Object
{
  private:
    typedef std::vector<std::shared_ptr<Pass>> RenderPassList;
    RenderPassList m_Passes;

    using ResourceMap = std::map<std::string, std::shared_ptr<Object>>;
    ResourceMap m_resourceMap;

  protected:
    std::shared_ptr<RenderTarget> m_pRenderTarget;
    std::shared_ptr<Texture> m_pBackBuffer;

  public:
    Technique(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture> backBuffer);
    virtual ~Technique();

    // Add a pass to the technique. The ID of the added pass is returned
    // and can be used to retrieve the pass later.
    unsigned int AddPass(std::shared_ptr<Pass> pass);
    std::shared_ptr<Pass> GetPass(unsigned int ID) const;

    void Set(const std::string& name, std::shared_ptr<Object> res);
    std::shared_ptr<Object> Get(const std::string& name);

    virtual void Render();
    virtual void Update();
};

class App : public Diligent::SampleBase
{
  public:
    static Diligent::RefCntAutoPtr<Diligent::IRenderDevice> s_device;
    static Diligent::RefCntAutoPtr<Diligent::IDeviceContext> s_ctx;
    static Diligent::RefCntAutoPtr<Diligent::ISwapChain> s_swapChain;
    static Diligent::RefCntAutoPtr<Diligent::IEngineFactory> s_engineFactory;
    static std::shared_ptr<RenderTarget> s_rt;
    static std::shared_ptr<Texture> s_backBuffer;

    static Diligent::SwapChainDesc s_desc;
    static RenderEventArgs s_eventArgs;

  protected:
    std::shared_ptr<Camera> m_pCamera;

  public:
    App();
    virtual ~App();

    virtual void Initialize(Diligent::IEngineFactory* pEngineFactory,
                            Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext** ppContexts,
                            Diligent::Uint32 NumDeferredCtx,
                            Diligent::ISwapChain* pSwapChain) override;

    virtual void Render() override;
    virtual void Update(double CurrTime, double ElapsedTime) override;
    virtual const Diligent::Char* GetSampleName() const override { return "App"; }
};
}    // namespace ade