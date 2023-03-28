#pragma once

#include <array>
// #include "VATTypes.generated.h"

class UVATUtility;

struct FVATInfo final
{
public:
	inline ~FVATInfo()
	{
		FMemory::Free(this->FrameVertexNum);
	}
	
	// Magic
	std::array<uint8, 4> Magic;
	// Bounding Box Min
	std::array<float, 3> BoundingBoxMin;
	// Bounding Box Max
	std::array<float, 3> BoundingBoxMax;
	// Frame count
	int32 FrameNum;
	// Max vertex count
	int32 MaxVertexNum;
	// Image height
	int32 ImageHeight;
	// Vertex per frame array. Element num == Frame count
	int32* FrameVertexNum;

	friend UVATUtility;
};
