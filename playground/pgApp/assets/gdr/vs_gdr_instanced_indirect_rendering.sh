
#include "gdr_common.sh"

VertexShaderOutput main(AppData IN)
{
	VertexShaderOutput OUT;
	
	float4x4 model;
	model[0] = float4(IN.i_data0.xyz, 0.0);
	model[1] = IN.i_data1;
	model[2] = IN.i_data2;
	model[3] = IN.i_data3;

	OUT.materialID = IN.i_data0.w;

	float4 worldPos = mul(float4(IN.position, 1.0), model );
	OUT.position = mul(u_viewProj, worldPos);
	
	return OUT;
}
