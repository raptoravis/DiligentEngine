struct AppData
{
    float3 position : ATTRIB0;
};


struct VertexShaderOutput
{
	uint materialID		: TEXCOORD0;
    float4 position     : SV_POSITION;  // Clip space position.
};


cbuffer PerObject : register( b0 )
{
    float4x4 u_model;
	float4x4 u_viewProj;
}


cbuffer Colors : register( b1 )
{
	float4 u_color[32];
}

cbuffer MaterialId : register( b2 )
{
    uint u_materialID;
}
