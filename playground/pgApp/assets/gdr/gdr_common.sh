struct AppData
{
    float3 position : ATTRIB0;
	float4 i_data0 : ATTRIB1;
	float4 i_data1 : ATTRIB2;
	float4 i_data2 : ATTRIB3;
	float4 i_data3 : ATTRIB4;
};


struct VertexShaderOutput
{
	uint materialID		: TEXCOORD0;
    float4 position     : SV_POSITION;  // Clip space position.
};


cbuffer CBMatrix : register( b0 )
{
	float4x4 u_viewProj;
}


cbuffer InputRTSize : register( b1 )
{
	float4 u_inputRTSize;
}

struct ComputeShaderInput
{
    uint3 groupID           : SV_GroupID;           // 3D index of the thread group in the dispatch.
    uint3 groupThreadID     : SV_GroupThreadID;     // 3D index of local thread ID in a thread group.
    uint3 dispatchThreadID  : SV_DispatchThreadID;  // 3D index of global thread ID in the dispatch.
    uint  groupIndex        : SV_GroupIndex;        // Flattened local index of the thread within a thread group.
};
