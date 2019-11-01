
#include "gdr_common.sh"

//the per drawcall data that is constant (noof indices and offsets to vertex/index buffers)
Buffer<uint>  drawcallConstData : register( t0 );

//instance data for all instances (pre culling)
Buffer<float4>  instanceDataIn : register( t1 );

//per instance visibility (output of culling pass)
Buffer<uint>  instancePredicates : register( t2 );


//how many instances per drawcall
RWBuffer<uint> drawcallInstanceCount : register(u0);

//drawcall data that will drive drawIndirect
RWBuffer<uint4> drawcallData : register(u1);

//culled instance data
RWBuffer<float4> instanceDataOut : register(u2);

// Based on Parallel Prefix Sum (Scan) with CUDA by Mark Harris
groupshared uint temp[2048];

#define drawIndexedIndirect( \
	  _buffer                \
	, _offset                \
	, _numIndices            \
	, _numInstances          \
	, _startIndex            \
	, _startVertex           \
	, _startInstance         \
	)                        \
	_buffer[_offset*2+0] = uint4(_numIndices, _numInstances, _startIndex, _startVertex); \
	_buffer[_offset*2+1] = uint4(_startInstance, 0u, 0u, 0u)

[numthreads( 1024, 1, 1 )]
void main(ComputeShaderInput IN)
{
	uint tID = IN.dispatchThreadID.x;
	int NoofInstancesPowOf2 = int(u_cullingConfig.y);
	int NoofDrawcalls = int(u_cullingConfig.w);

	int offset = 1;
	temp[2 * tID    ] = uint(instancePredicates[2 * tID    ] ? 1 : 0); // load input into shared memory
	temp[2 * tID + 1] = uint(instancePredicates[2 * tID + 1] ? 1 : 0);

	int d;

	//perform reduction
	for (d = NoofInstancesPowOf2 >> 1; d > 0; d >>= 1)
	{
		GroupMemoryBarrierWithGroupSync();

		if (tID < d)
		{
			int ai = int(offset * (2 * tID + 1) - 1);
			int bi = int(offset * (2 * tID + 2) - 1);
			temp[bi] += temp[ai];
		}

		offset *= 2;
	}

	// clear the last element
	if (tID == 0)
	{
		temp[NoofInstancesPowOf2 - 1] = 0;
	}

	// perform downsweep and build scan
	for ( d = 1; d < NoofInstancesPowOf2; d *= 2)
	{
		offset >>= 1;

		GroupMemoryBarrierWithGroupSync();

		if (tID < d)
		{
			int ai = int(offset * (2 * tID + 1) - 1);
			int bi = int(offset * (2 * tID + 2) - 1);
			int t  = int(temp[ai]);
			temp[ai] = temp[bi];
			temp[bi] += t;
		}
	}

	GroupMemoryBarrierWithGroupSync();

	int index = int(2 * tID);

	// scatter results
	if (instancePredicates[index] > 0)
	{
		instanceDataOut[4 * temp[index]    ] = instanceDataIn[4 * index    ];
		instanceDataOut[4 * temp[index] + 1] = instanceDataIn[4 * index + 1];
		instanceDataOut[4 * temp[index] + 2] = instanceDataIn[4 * index + 2];
		instanceDataOut[4 * temp[index] + 3] = instanceDataIn[4 * index + 3];
	}

	index = int(2 * tID + 1);

	if (instancePredicates[index] > 0)
	{
		instanceDataOut[4 * temp[index]    ] = instanceDataIn[4 * index    ];
		instanceDataOut[4 * temp[index] + 1] = instanceDataIn[4 * index + 1];
		instanceDataOut[4 * temp[index] + 2] = instanceDataIn[4 * index + 2];
		instanceDataOut[4 * temp[index] + 3] = instanceDataIn[4 * index + 3];
	}

	if (tID == 0)
	{
		uint startInstance = 0;

		//copy data to indirect buffer, could possible be done in a different compute shader
		for (int k = 0; k < NoofDrawcalls; k++)
		{
			drawIndexedIndirect(
				drawcallData,
				k,
				drawcallConstData[ k * 3 ], 			//number of indices
				drawcallInstanceCount[k], 				//number of instances
				drawcallConstData[ k * 3 + 1 ],			//offset into the index buffer
				drawcallConstData[ k * 3 + 2 ],			//offset into the vertex buffer
				startInstance							//offset into the instance buffer
				);

			startInstance += drawcallInstanceCount[k];

			drawcallInstanceCount[k] = 0;
		}
	}

}
