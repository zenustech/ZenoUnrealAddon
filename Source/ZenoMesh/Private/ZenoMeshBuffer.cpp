#include "..\Public\ZenoMeshBuffer.h"

#ifdef UE_5_2_OR_LATER
#include "DataDrivenShaderPlatformInfo.h"
#endif

#if PLATFORM_USES_GLES
typedef uint16 DynamicMeshIndexType;
#else
typedef int32 DynamicMeshIndexType;
#endif

FZenoMeshVertex::FZenoMeshVertex() = default;

FZenoMeshVertex::FZenoMeshVertex(const FVector3f& InPosition):
	Position(InPosition)
	, Tangent(FVector3f(1, 0, 0))
	, Normal(FVector3f(0, 0, 1))
	, Color(FColor::White)
{
	Normal.Vector.W = 127;
		
	for (int32 Idx = 0; Idx < MAX_STATIC_TEXCOORDS; Idx++)
	{
		TextureCoordinate[Idx] = FVector2f::Zero();
	}
}

FZenoMeshVertex::FZenoMeshVertex(const FVector3f& InPosition, const FVector2f& InTexCoord):
	Position(InPosition)
	, Tangent(FVector3f(1, 0, 0))
	, Normal(FVector3f(0, 0, 1))
	, Color(FColor::White)
{
	Normal.Vector.W = 127;

	for (int32 Idx = 0; Idx < MAX_STATIC_TEXCOORDS; Idx++)
	{
		TextureCoordinate[Idx] = InTexCoord;
	}
}

FBufferRHIRef FZenoMeshBufferAllocator::AllocVertexBuffer(uint32 Stride, uint32 NumElements)
{
	const uint32 SizeInBytes = GetVertexBufferSize(Stride, NumElements);

	FRHIResourceCreateInfo CreateInfo(TEXT("FDynamicMeshBufferAllocator"));
	return RHICreateVertexBuffer(SizeInBytes, BUF_Static | BUF_ShaderResource, CreateInfo);
}

FBufferRHIRef FZenoMeshBufferAllocator::AllocIndexBuffer(uint32 NumElements)
{
	const uint32 SizeInBytes = GetIndexBufferSize(NumElements);

	FRHIResourceCreateInfo CreateInfo(TEXT("FDynamicMeshBufferAllocator"));
	return RHICreateIndexBuffer(sizeof(DynamicMeshIndexType), SizeInBytes, BUF_Static, CreateInfo);
}

void FZenoMeshPrimitiveUniformBuffer::InitPrimitiveResource()
{
	InitResource();
}

void FZenoMeshPrimitiveUniformBuffer::ReleasePrimitiveResource()
{
	ReleaseResource();
	delete this;
}

FZenoMeshVertexBuffer::FZenoMeshVertexBuffer(uint32 InNumTexCoords, uint32 InLightmapCoordinateIndex, const bool bInUse16BitTexCoord, FZenoMeshBufferAllocator& InAllocator) :
	NumTexCoords(InNumTexCoords)
	, LightmapCoordinateIndex(InLightmapCoordinateIndex)
	, bUse16BitTexCoord(bInUse16BitTexCoord)
	, BufferAllocator(InAllocator)
{
	check(NumTexCoords > 0 && NumTexCoords <= MAX_STATIC_TEXCOORDS);
	check(LightmapCoordinateIndex < NumTexCoords);
}

void FZenoMeshVertexBuffer::InitRHI()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FZenoMeshVertexBuffer::InitRHI);

	uint32 TextureStride = sizeof(FVector2f);
	EPixelFormat TextureFormat = PF_G32R32F;

	if (bUse16BitTexCoord)
	{
		TextureStride = sizeof(FVector2DHalf);
		TextureFormat = PF_G16R16F;
	}

	PositionBuffer.VertexBufferRHI = BufferAllocator.AllocVertexBuffer(sizeof(FPositionType), Vertices.Num());
	// We will pass both tangent and normal as a single packed normal
	TangentBuffer.VertexBufferRHI = BufferAllocator.AllocVertexBuffer(sizeof(FPackedNormal), 2 * Vertices.Num());
	TexCoordBuffer.VertexBufferRHI = BufferAllocator.AllocVertexBuffer(TextureStride, NumTexCoords * Vertices.Num());
	ColorBuffer.VertexBufferRHI = BufferAllocator.AllocVertexBuffer(sizeof(FColor), Vertices.Num());

	if (RHISupportsManualVertexFetch(GMaxRHIShaderPlatform))
	{
		TangentBufferSRV = RHICreateShaderResourceView(TangentBuffer.VertexBufferRHI, 4, PF_R8G8B8A8_SNORM);
		TexCoordBufferSRV = RHICreateShaderResourceView(TexCoordBuffer.VertexBufferRHI, TextureStride, TextureFormat);
		ColorBufferSRV = RHICreateShaderResourceView(ColorBuffer.VertexBufferRHI, 4, PF_R8G8B8A8);
		PositionBufferSRV = RHICreateShaderResourceView(PositionBuffer.VertexBufferRHI, sizeof(float), PF_R32_FLOAT);
	}

	FVector3f* PositionBufferData = static_cast<FVector3f*>(RHILockBuffer(PositionBuffer.VertexBufferRHI, 0, sizeof(FVector3f) * Vertices.Num(), RLM_WriteOnly));
	FPackedNormal* TangentBufferData = static_cast<FPackedNormal*>(RHILockBuffer(TangentBuffer.VertexBufferRHI, 0, sizeof(FPackedNormal) * 2 * Vertices.Num(), RLM_WriteOnly));
	FColor* ColorBufferData = static_cast<FColor*>(RHILockBuffer(ColorBuffer.VertexBufferRHI, 0, sizeof(FColor) * Vertices.Num(), RLM_WriteOnly));
	
	void* TexCoordBufferData = RHILockBuffer(TexCoordBuffer.VertexBufferRHI, 0, NumTexCoords * TextureStride * Vertices.Num(), RLM_WriteOnly);
	FVector2f* TexCoordBufferData32 = !bUse16BitTexCoord ? static_cast<FVector2f*>(TexCoordBufferData) : nullptr;
	FVector2DHalf* TexCoordBufferData16 = bUse16BitTexCoord ? static_cast<FVector2DHalf*>(TexCoordBufferData) : nullptr;

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FZenoMeshVertexBuffer::InitRHI::CopyDataToVertexBuffers);

		for (int32 Idx = 0; Idx < Vertices.Num(); Idx++)
		{
			PositionBufferData[Idx] = Vertices[Idx].Position;
			TangentBufferData[2 * Idx + 0] = Vertices[Idx].Tangent;
			TangentBufferData[2 * Idx + 1] = Vertices[Idx].Normal;
			ColorBufferData[Idx] = Vertices[Idx].Color;

			for (uint32 TexCoordIdx = 0; TexCoordIdx < NumTexCoords; TexCoordIdx++)
			{
				if (!bUse16BitTexCoord)
				{
					TexCoordBufferData32[NumTexCoords * Idx + TexCoordIdx] = Vertices[Idx].TextureCoordinate[TexCoordIdx];
				}
				else
				{
					TexCoordBufferData16[NumTexCoords * Idx + TexCoordIdx] = FVector2DHalf(Vertices[Idx].TextureCoordinate[TexCoordIdx]);
				}
			}
		}
	}

	RHIUnlockBuffer(PositionBuffer.VertexBufferRHI);
	RHIUnlockBuffer(TangentBuffer.VertexBufferRHI);
	RHIUnlockBuffer(TexCoordBuffer.VertexBufferRHI);
	RHIUnlockBuffer(ColorBuffer.VertexBufferRHI);
}

void FZenoMeshVertexBuffer::InitResource()
{
	FRenderResource::InitResource();
	PositionBuffer.InitResource();
	TangentBuffer.InitResource();
	TexCoordBuffer.InitResource();
	ColorBuffer.InitResource();
}

void FZenoMeshVertexBuffer::ReleaseResource()
{
	ColorBuffer.ReleaseResource();
	TexCoordBuffer.ReleaseResource();
	TangentBuffer.ReleaseResource();
	PositionBuffer.ReleaseResource();
	FRenderResource::ReleaseResource();
}

void FZenoMeshVertexBuffer::ReleaseRHI()
{
	BufferAllocator.ReleaseVertexBuffer(PositionBuffer.VertexBufferRHI);
	BufferAllocator.ReleaseVertexBuffer(TangentBuffer.VertexBufferRHI);
	BufferAllocator.ReleaseVertexBuffer(TexCoordBuffer.VertexBufferRHI);
	BufferAllocator.ReleaseVertexBuffer(ColorBuffer.VertexBufferRHI);
}

uint32 FZenoMeshVertexBuffer::GetNumTexCoords() const { return NumTexCoords; }

uint32 FZenoMeshVertexBuffer::GetLightmapCoordinateIndex() const { return LightmapCoordinateIndex; }

bool FZenoMeshVertexBuffer::IsUse16BitTexCoord() const { return bUse16BitTexCoord; }

FZenoMeshIndexBuffer::FZenoMeshIndexBuffer(FZenoMeshBufferAllocator& InAllocator)
	: BufferAllocator(InAllocator)
{
}

void FZenoMeshIndexBuffer::InitRHI()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FZenoMeshIndexBuffer::InitRHI);

	IndexBufferRHI = BufferAllocator.AllocIndexBuffer(Indices.Num());

	void* Buffer;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(RHILockBuffer)
		Buffer = RHILockBuffer(IndexBufferRHI,0,Indices.Num() * sizeof(MeshIndexType),RLM_WriteOnly);
	}
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(Memcpy)
		FMemory::Memcpy(Buffer, Indices.GetData(),Indices.Num() * sizeof(MeshIndexType));
	}
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(RHIUnlockBuffer)
		RHIUnlockBuffer(IndexBufferRHI);
	}
}

void FZenoMeshIndexBuffer::ReleaseRHI()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FZenoMeshIndexBuffer::ReleaseRHI);

	BufferAllocator.ReleaseIndexBuffer(IndexBufferRHI);
	FIndexBuffer::ReleaseRHI();
}
