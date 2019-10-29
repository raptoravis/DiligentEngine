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
