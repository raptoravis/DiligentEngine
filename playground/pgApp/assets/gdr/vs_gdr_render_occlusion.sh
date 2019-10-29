
#include "gdr_common.sh"

float4 main(AppData IN) : SV_POSITION
{
	float4x4 model;
	model[0] = IN.i_data0;
	model[1] = IN.i_data1;
	model[2] = IN.i_data2;
	model[3] = IN.i_data3;

	//float4 worldPos = mul(model, float4(IN.position, 1.0) );
	float4 worldPos = mul(float4(IN.position, 1.0), model );
	float4 pos = mul(u_viewProj, worldPos);
	
	return pos;
}
