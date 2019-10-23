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

// Report an error to the Debug output in Visual Studio, display a message box with the error
// message and throw an exception.
void ReportErrorAndThrow(const std::string& file, int line, const std::string& function,
                         const std::string& message);

// Report an error message and throw an std::exception.
#define ReportError(msg) ReportErrorAndThrow(__FILE__, __LINE__, __FUNCTION__, (msg))

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

class pgObject
{
    static uint32_t s_uuid;

  protected:
    uint32_t m_uuid;

  public:
    pgObject();
    virtual ~pgObject()
    {
        //
    }
};

class pgScene;
class pgSceneNode;
class pgMesh;

class Visitor : public pgObject
{
  public:
    virtual void Visit(pgScene& scene) = 0;
    virtual void Visit(pgSceneNode& node) = 0;
    virtual void Visit(pgMesh& mesh) = 0;
};


class pgCamera : public pgObject
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
    pgCamera(const Diligent::float3 pos, const Diligent::float3 dir);

    virtual ~pgCamera();

    void reset();
    void reset(const Diligent::float3& p, const Diligent::float3& dir);
    void setProjectionMatrix(float NearPlane, float FarPlane, bool bRightHanded = false);

    void update(Diligent::InputController* pInputController, float ElapsedTime);

    void setPos(const Diligent::float3& p) { pos = p; }

    const Diligent::float3& getPos() const { return pos; }

    void setLook(const Diligent::float3& dir) { look = dir; }

    const Diligent::float3& getLook() const { return look; }

    const Diligent::float4x4& getViewMatrix() const { return m_viewMatrix; }

    const Diligent::float4x4& getProjectionMatrix() const { return m_projectionMatrix; }
};

class pgApp;
class pgTechnique;
class pgPass;
class pgPipeline;
class pgScene;
class pgSceneNode;
class pgMaterial;
class pgMesh;

class pgScene;
class pgSceneNode;
class pgMesh;

// CPU Access. Used for textures and Buffers
enum CPUAccess {
    None = 0,            // No CPU access to this texture is necessary.
    Read = (1 << 0),     // CPU reads permitted.
    Write = (1 << 1),    // CPU writes permitted.
    ReadWrite = Read | Write
};


class pgRenderEventArgs
{
  public:
    float CurrTime;
    float ElapsedTime;

    pgApp* pApp;
    pgCamera* pCamera;

    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> pDeviceContext;

    //
    pgTechnique* pTechnique;
    pgPass* pPass;
    pgPipeline* pPipeline;
    pgScene* pScene;
    pgSceneNode* pSceneNode;
    pgMaterial* pMaterial;
    pgMesh* pMesh;

  public:
    pgRenderEventArgs();

    void set(float currentTime, float elapsedTime, pgApp* caller, pgCamera* camera,
             Diligent::RefCntAutoPtr<Diligent::IDeviceContext> ctx);
};

// Defines either a semantic (HLSL) or an input index (GLSL/HLSL)
// to bind an input buffer.
struct pgBufferBinding {
    pgBufferBinding() : Index(0) {}

    pgBufferBinding(const std::string& name, unsigned int index) : Name(name), Index(index) {}

    // Provide the < operator for STL containers.
    bool operator<(const pgBufferBinding& rhs) const
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


class pgBuffer;
class pgTexture;
class ConstantBuffer;
class StructuredBuffer;
class SamplerState;

class ShaderParameter : public pgObject
{
  public:
    typedef pgObject base;

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

    std::weak_ptr<pgObject> Get();
    void Set(std::shared_ptr<pgObject> resource);

  private:
    std::string m_Name;

    uint32_t m_uiSlotID;
    const std::string m_ShaderType;
    Type m_ParameterType;

    // Shader parameter does not take ownership of these types.
    std::weak_ptr<pgObject> m_pResource;
};


class Shader : public pgObject
{
  public:
    typedef pgObject base;

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
    bool LoadShaderFromFile(
        ShaderType type, const std::string& fileName, const std::string& entryPoint = "main",
        const std::string& searchPaths = "", bool UseCombinedTextureSamplers = false,
        const Diligent::ShaderMacro* shaderMacros = nullptr);

    Diligent::RefCntAutoPtr<Diligent::IShader> GetShader() { return m_pShader; }

    // virtual uint32_t GetConstantBufferIndex( const std::string& name );
    ShaderParameter& GetShaderParameterByName(const std::string& name) const;

    // virtual ConstantBuffer* GetConstantBufferByName( const std::string& name );

    // Check to see if this shader supports a given semantic.
    bool HasSemantic(const pgBufferBinding& binding) const;
    uint32_t GetSlotIDBySemantic(const pgBufferBinding& binding) const;

    typedef std::vector<std::shared_ptr<ShaderParameter>> ParametersList;
    ParametersList GetConstantBuffers();
    ParametersList GetNonConstantBuffers();

    void Bind();
    void UnBind();

  protected:
    // Destroy the contents of this shader (in case we are loading a new shader).
    virtual void Destroy();

  private:
    ShaderType m_ShaderType;
    Diligent::RefCntAutoPtr<Diligent::IShader> m_pShader;

    typedef std::map<std::string, std::shared_ptr<ShaderParameter>> ParameterMap;
    ParameterMap m_ShaderParameters;

    // A map to convert a vertex attribute semantic to a slot.
    typedef std::map<pgBufferBinding, uint32_t> SemanticMap;
    SemanticMap m_InputSemantics;

    // Parameters necessary to reload the shader at runtime if it is modified on disc.
    ShaderMacros m_ShaderMacros;
    std::string m_EntryPoint;
    std::string m_Profile;
    std::wstring m_ShaderFileName;
};

class pgResource : public pgObject
{
    //
};

class pgBuffer : public pgResource
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

    pgBuffer(uint32_t stride, uint32_t count, Diligent::IBuffer* buffer);

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
    virtual void Copy(std::shared_ptr<pgBuffer> other);

    // Is this an index buffer or an attribute/vertex buffer?
    BufferType GetType() const;
    // How many elements does this buffer contain?
    unsigned int GetElementCount() const { return m_uiCount; }
};

class ConstantBuffer : public pgBuffer
{
    typedef pgBuffer base;

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


class StructuredBuffer : public pgBuffer
{
    typedef pgBuffer base;

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

class SamplerState : public pgResource
{
  public:
    typedef pgResource base;

    enum MinFilter {
        MinNearest,    // The nearest texel to the sampled texel.
        MinLinear,     // Linear average of the 4 closest texels.
    };

    enum MagFilter {
        MagNearest,    // The nearest texel to the sampled texel.
        MagLinear,     // Weighted average of the closest texels.
    };

    enum MipFilter {
        MipNearest,    // Choose the nearest mip level.
        MipLinear,     // Linear interpolate between the 2 nearest mip map levels.
    };

    enum WrapMode {
        Repeat,    // Texture is repeated when texture coordinates are out of range.
        Mirror,    // Texture is mirrored when texture coordinates are out of range.
        Clamp,     // Texture coordinate is clamped to [0, 1]
        Border,    // Texture border color is used when texture coordinates are out of range.
    };


    enum CompareMode {
        None,                   // Don't perform any comparison
        CompareRefToTexture,    // Compare the reference value (usually the currently bound depth
                                // buffer) to the value in the texture.
    };

    enum CompareFunc {
        Never,           // Never pass the comparison function.
        Less,            // Pass if the source data is less than the destination data.
        Equal,           // Pass if the source data is equal to the destination data.
        LessEqual,       // Pass if the source data is less than or equal to the destination data.
        Greater,         // Pass if the source data is greater than the destination data.
        NotEqual,        // Pass if the source data is not equal to the destination data.
        GreaterEqual,    // Pass if the source data is greater than or equal to the destination
                         // data.
        Always,          // Always pass the comparison function.
    };

    virtual void SetFilter(MinFilter minFilter, MagFilter magFilter, MipFilter mipFilter) = 0;
    virtual void GetFilter(MinFilter& minFilter, MagFilter& magFilter,
                           MipFilter& mipFilter) const = 0;

    virtual void SetWrapMode(WrapMode u = WrapMode::Repeat, WrapMode v = WrapMode::Repeat,
                             WrapMode w = WrapMode::Repeat) = 0;
    virtual void GetWrapMode(WrapMode& u, WrapMode& v, WrapMode& w) const = 0;

    virtual void SetCompareFunction(CompareFunc compareFunc) = 0;
    virtual CompareFunc GetCompareFunc() const = 0;

    /**
     * Set the offset from the calculated mipmap level.  For example, if mipmap texture 1 should be
     * sampled and LOD bias is set to 2, then the texture will be sampled at mipmap level 3.
     */
    virtual void SetLODBias(float lodBias) = 0;
    virtual float GetLODBias() const = 0;

    /**
     * Set the minimum LOD level that will be sampled.  The highest resolution mip map is level 0.
     */
    virtual void SetMinLOD(float minLOD) = 0;
    virtual float GetMinLOD() const = 0;

    /**
     * Set the maximum LOD level that will be sampled. The LOD level increases as the resolution of
     * the mip-map decreases.
     */
    virtual void SetMaxLOD(float maxLOD) = 0;
    virtual float GetMaxLOD() const = 0;

    /**
     * Sets the border color to use if the wrap mode is set to Border.
     */
    virtual void SetBorderColor(const Diligent::float4& borderColor) = 0;
    virtual Diligent::float4 GetBorderColor() const = 0;

    /**
     * Enable Anisotropic filtering (where supported).
     */
    virtual void EnableAnisotropicFiltering(bool enabled) = 0;
    virtual bool IsAnisotropicFilteringEnabled() const = 0;

    /**
     * When Anisotropic filtering is enabled, use this value to determine the maximum level
     * of anisotropic filtering to apply.  Valid values are in the range [1, 16].
     * May not be supported on all platforms.
     */
    virtual void SetMaxAnisotropy(uint8_t maxAnisotropy) = 0;
    virtual uint8_t GetMaxAnisotropy() const = 0;

    // Bind this sampler state to the ID for a specific shader type.
    virtual void Bind(uint32_t ID, Shader::ShaderType shaderType,
                      ShaderParameter::Type parameterType) = 0;
    // Unbind the sampler state.
    virtual void UnBind(uint32_t ID, Shader::ShaderType shaderType,
                        ShaderParameter::Type parameterType) = 0;
};


/**
 * Flags to specify which value should be cleared.
 */
enum class pgClearFlags : uint8_t {
    Color = 1 << 0,
    Depth = 1 << 1,
    Stencil = 1 << 2,
    DepthStencil = Depth | Stencil,
    All = Color | Depth | Stencil,
};


class pgTexture : public pgResource
{
  protected:
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_pTexture;

  public:
    pgTexture(Diligent::ITexture* texture);
    virtual ~pgTexture();

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

    void Clear(pgClearFlags clearFlags, const Diligent::float4& color, float depth,
               uint8_t stencil);
    void Copy(pgTexture* dstTexture);

    void Bind(uint32_t ID, Shader::ShaderType shaderType, ShaderParameter::Type parameterType);
    void UnBind(uint32_t ID, Shader::ShaderType shaderType, ShaderParameter::Type parameterType);
};

class pgRenderTarget : public pgObject
{
    typedef std::vector<std::shared_ptr<pgTexture>> TextureList;
    typedef std::vector<std::shared_ptr<pgBuffer>> StructuredBufferList;

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

    pgRenderTarget();
    virtual ~pgRenderTarget();
    /**
     * Attach a texture to the render target.
     * The dimension of all textures attached to a render target
     * must match.
     *
     * To remove a texture from an attachment point, just attach a NULL texture.
     */
    void AttachTexture(AttachmentPoint attachment, std::shared_ptr<pgTexture> texture);
    std::shared_ptr<pgTexture> GetTexture(AttachmentPoint attachment);

    uint32_t GetNumRTVs() const;

    /**
     * Clear the contents of a texture attached to a specific attachment point.
     * @param attachemnt The attachment point of which to clear the contents of the texture.
     * @param clearFlags Which values should be cleared.
     * @param color The clear color to use for color attachment points.
     * @param depth The depth value to use for depth attachment points.
     * @param stencil The stencil value to use for stencil attachment points.
     */
    void Clear(AttachmentPoint attachemnt, pgClearFlags clearFlags = pgClearFlags::All,
               const Diligent::float4& color = Diligent::float4(0, 0, 0, 0), float depth = 1.0f,
               uint8_t stencil = 0);

    /**
     * Clear the contents of all of the textures attached to the render target.
     * @param clearFlags Which values should be cleared.
     * @param color The clear color to use for color attachment points.
     * @param depth The depth value to use for depth attachment points.
     * @param stencil The stencil value to use for stencil attachment points.
     */
    void Clear(pgClearFlags clearFlags = pgClearFlags::All,
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
    void AttachStructuredBuffer(uint8_t slot, std::shared_ptr<pgBuffer> rwBuffer);
    std::shared_ptr<pgBuffer> GetStructuredBuffer(uint8_t slot);

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
class pgMaterial : public pgObject
{
  public:
    typedef pgObject base;

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

    pgMaterial();

    virtual ~pgMaterial();

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

    std::shared_ptr<pgTexture> GetTexture(TextureType ID) const;
    void SetTexture(TextureType type, std::shared_ptr<pgTexture> texture);

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

    pgMaterial::MaterialProperties* GetMaterialProperties() const { return m_pProperties; }

  private:
    // Material properties have to be 16 byte aligned.
    // To guarantee alignment, we'll use _aligned_malloc to allocate memory
    // for the material properties.
    MaterialProperties* m_pProperties;

    // Textures are stored by which texture unit (or texture register)
    // they are bound to.
    typedef std::map<TextureType, std::shared_ptr<pgTexture>> TextureMap;
    TextureMap m_Textures;

    // Set to true if the contents of the constant buffer needs to be updated.
    bool m_Dirty;
};

class pgSceneNode;

// A mesh contains the geometry and materials required to render this mesh.
class pgMesh : public pgObject
{
  protected:
    typedef std::map<pgBufferBinding, std::shared_ptr<pgBuffer>> BufferMap;
    BufferMap m_VertexBuffers;

    std::shared_ptr<pgBuffer> m_pIndexBuffer;
    std::shared_ptr<pgMaterial> m_pMaterial;

  public:
    pgMesh();
    virtual ~pgMesh();

    // Adds a buffer to this mesh with a particular semantic (HLSL) or register ID (GLSL).
    virtual void addVertexBuffer(const pgBufferBinding& binding, std::shared_ptr<pgBuffer> buffer);
    virtual void setIndexBuffer(std::shared_ptr<pgBuffer> buffer);

    virtual void setMaterial(std::shared_ptr<pgMaterial> material);
    virtual std::shared_ptr<pgMaterial> getMaterial() const;

    virtual void Render();
    virtual void Accept(Visitor& visitor);
};

class pgSceneNode : public pgObject, public std::enable_shared_from_this<pgSceneNode>
{
  public:
    typedef pgObject base;

    explicit pgSceneNode(const Diligent::float4x4& localTransform = Diligent::float4x4::Identity());
    virtual ~pgSceneNode();

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

    void addChild(std::shared_ptr<pgSceneNode> pNode);
    void removeChild(std::shared_ptr<pgSceneNode> pNode);
    void setParent(std::weak_ptr<pgSceneNode> pNode);

    void addMesh(std::shared_ptr<pgMesh> mesh);
    void RemoveMesh(std::shared_ptr<pgMesh> mesh);

    virtual void Accept(Visitor& visitor);

  protected:
    Diligent::float4x4 GetParentWorldTransform() const;

  private:
    typedef std::vector<std::shared_ptr<pgSceneNode>> NodeList;
    typedef std::multimap<std::string, std::shared_ptr<pgSceneNode>> NodeNameMap;
    typedef std::vector<std::shared_ptr<pgMesh>> MeshList;

    std::string m_Name;

    // Transforms node from parent's space to world space for rendering.
    Diligent::float4x4 m_LocalTransform;
    // This is the inverse of the local -> world transform.
    Diligent::float4x4 m_InverseTransform;

    std::weak_ptr<pgSceneNode> m_pParentNode;
    NodeList m_Children;
    NodeNameMap m_ChildrenByName;
    MeshList m_Meshes;
};

class pgScene : public pgObject
{
    typedef pgObject base;

  protected:
    std::shared_ptr<pgSceneNode> m_pRootNode;

  public:
    pgScene();
    virtual ~pgScene();

    std::shared_ptr<pgSceneNode> getRootNode() const { return m_pRootNode; }
    void setRootNode(std::shared_ptr<pgSceneNode> root) { m_pRootNode = root; }

    virtual void Accept(Visitor& visitor);

    static std::shared_ptr<pgTexture> CreateTexture2D(uint16_t width, uint16_t height,
                                                      uint16_t slices,
                                                      Diligent::TEXTURE_FORMAT format,
                                                      CPUAccess cpuAccess, bool gpuWrite,
                                                      bool bGenerateMipmaps = false);
};

class pgPipeline : public pgObject
{
    typedef std::map<Shader::ShaderType, std::shared_ptr<Shader>> ShaderMap;

    bool m_bInited = false;
    uint32_t m_stencilRef = 0;

  protected:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pPSO;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_pSRB;

    Diligent::PipelineStateDesc m_PSODesc;
    ShaderMap m_Shaders;

    std::shared_ptr<pgRenderTarget> m_pRenderTarget;
    bool m_bDirty;

    virtual void InitPSODesc();

  private:
    void SetStaticVariables();
    void SetVariables();
    std::vector<Diligent::ShaderResourceVariableDesc> GetVariableDecalarations() const;

  public:
    pgPipeline(std::shared_ptr<pgRenderTarget> rt);
    virtual ~pgPipeline();

    std::shared_ptr<pgRenderTarget> getRenderTarget() { return m_pRenderTarget; }

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

    void SetRenderTarget(std::shared_ptr<pgRenderTarget> renderTarget);
    std::shared_ptr<pgRenderTarget> GetRenderTarget() const;

    virtual void Bind();
    virtual void UnBind();
};

class pgPass : public Visitor
{
    bool m_bEnabled;

    friend class pgMesh;
    friend class pgMaterial;

  protected:
    pgTechnique* m_parentTechnique;

  public:
    pgPass(pgTechnique* parentTechnique);
    virtual ~pgPass();

    // Enable or disable the pass. If a pass is disabled, the technique will skip it.
    virtual void SetEnabled(bool enabled) { m_bEnabled = enabled; }

    virtual bool IsEnabled() const { return m_bEnabled; }

    virtual void PreRender();
    virtual void Render();
    virtual void PostRender();

    virtual void Visit(pgScene& scene);
    virtual void Visit(pgSceneNode& node);
    virtual void Visit(pgMesh& mesh);
};


class pgPassPilpeline : public pgPass
{
    typedef pgPass base;

  protected:
    std::shared_ptr<pgScene> m_pScene;
    std::shared_ptr<pgPipeline> m_pPipeline;

  public:
    pgPassPilpeline(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
                    std::shared_ptr<pgPipeline> pipeline);
    virtual ~pgPassPilpeline();

    virtual void PreRender();
    virtual void Render();
};

class RenderPass;

class pgTechnique : public pgObject
{
  private:
    typedef std::vector<std::shared_ptr<pgPass>> RenderPassList;
    RenderPassList m_Passes;

    using ResourceMap = std::map<std::string, std::shared_ptr<pgObject>>;
    ResourceMap m_resourceMap;

  protected:
    std::shared_ptr<pgRenderTarget> m_pRenderTarget;
    std::shared_ptr<pgTexture> m_pBackBuffer;

  public:
    pgTechnique(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer);
    virtual ~pgTechnique();

    // Add a pass to the technique. The ID of the added pass is returned
    // and can be used to retrieve the pass later.
    unsigned int AddPass(std::shared_ptr<pgPass> pass);
    std::shared_ptr<pgPass> GetPass(unsigned int ID) const;

    void Set(const std::string& name, std::shared_ptr<pgObject> res);
    std::shared_ptr<pgObject> Get(const std::string& name);

    virtual void Render();
};

class pgApp : public Diligent::SampleBase
{
  public:
    static Diligent::RefCntAutoPtr<Diligent::IRenderDevice> s_device;
    static Diligent::RefCntAutoPtr<Diligent::IDeviceContext> s_ctx;
    static Diligent::RefCntAutoPtr<Diligent::ISwapChain> s_swapChain;
    static Diligent::RefCntAutoPtr<Diligent::IEngineFactory> s_engineFactory;
    static std::shared_ptr<pgRenderTarget> s_rt;
    static std::shared_ptr<pgTexture> s_backBuffer;

    static Diligent::SwapChainDesc s_desc;
    static pgRenderEventArgs s_eventArgs;

  protected:
    std::shared_ptr<pgCamera> m_pCamera;

  public:
    pgApp();
    virtual ~pgApp();

    virtual void Initialize(Diligent::IEngineFactory* pEngineFactory,
                            Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext** ppContexts,
                            Diligent::Uint32 NumDeferredCtx,
                            Diligent::ISwapChain* pSwapChain) override;

    virtual void Render() override;
    virtual void Update(double CurrTime, double ElapsedTime) override;
    virtual const Diligent::Char* GetSampleName() const override { return "pgApp"; }
};
