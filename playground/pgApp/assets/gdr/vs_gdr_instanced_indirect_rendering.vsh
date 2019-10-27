
#include "CommonInclude.sh"


VertexShaderOutput main(AppData IN, uint instanceId: SV_InstanceID)
{
	VertexShaderOutput OUT;
	
	OUT.materialID = u_materialID;

    float4 worldPos = mul(u_models[instanceId], float4(IN.position, 1.0));
	
	OUT.position = mul(u_viewProj, worldPos);
	
	return OUT;
}
