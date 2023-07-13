#pragma once
#include "DynamicMeshBuilder.h"

struct ZENOMESH_API FZenoMeshVertex
{
	FVector3f Position;
	FVector2f TextureCoordinate[MAX_STATIC_TEXCOORDS];
	// a.k.a. TangentX
	FPackedNormal Tangent;
	// a.k.a. TangentZ
	FPackedNormal Normal;
	FColor Color;

	FZenoMeshVertex();

	explicit FZenoMeshVertex(const FVector3f& InPosition);

	FZenoMeshVertex(const FVector3f& InPosition, const FVector2f& InTexCoord);
};

class ZENOMESH_API FZenoMeshBufferAllocator : public FDynamicMeshBufferAllocator
{
public:
	virtual FBufferRHIRef AllocVertexBuffer(uint32 Stride, uint32 NumElements) override;
	virtual FBufferRHIRef AllocIndexBuffer(uint32 NumElements) override;
};

class FZenoMeshPrimitiveUniformBuffer : public FDynamicPrimitiveResource, public TUniformBuffer<FPrimitiveUniformShaderParameters>
{
public:
	
	// ~Begin FDynamicPrimitiveResource API
	virtual void InitPrimitiveResource() override;

	virtual void ReleasePrimitiveResource() override;
	// ~End FDynamicPrimitiveResource API
};

class ZENOMESH_API FZenoMeshVertexBuffer : public FRenderResource
{
public:
	using FPositionType = FVector3f;
	
	FVertexBuffer PositionBuffer;
	FVertexBuffer TangentBuffer;
	FVertexBuffer TexCoordBuffer;
	FVertexBuffer ColorBuffer;

	FShaderResourceViewRHIRef PositionBufferSRV;
	FShaderResourceViewRHIRef TangentBufferSRV;
	FShaderResourceViewRHIRef TexCoordBufferSRV;
	FShaderResourceViewRHIRef ColorBufferSRV;

	TArray<FZenoMeshVertex> Vertices;

	FZenoMeshVertexBuffer(uint32 InNumTexCoords, uint32 InLightmapCoordinateIndex, bool bInUse16BitTexCoord, FZenoMeshBufferAllocator& InAllocator);

	// ~Begin FRenderResource API
	virtual void InitRHI() override;
	virtual void InitResource() override;
	virtual void ReleaseResource() override;
	virtual void ReleaseRHI() override;
	// ~End FRenderResource API

	uint32 GetNumTexCoords() const;
	uint32 GetLightmapCoordinateIndex() const;
	bool IsUse16BitTexCoord() const;

private:
	const uint32 NumTexCoords;
	const uint32 LightmapCoordinateIndex;
	const bool bUse16BitTexCoord;
	FZenoMeshBufferAllocator& BufferAllocator;
};

class FZenoMeshIndexBuffer : public FIndexBuffer
{
public:

#if PLATFORM_USES_GLES
	using MeshIndexType = uint16;
#else
	using MeshIndexType = int32;
#endif
	
	TArray<MeshIndexType> Indices;

	explicit FZenoMeshIndexBuffer(FZenoMeshBufferAllocator& InAllocator);

	// ~Begin FRenderResource API
	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;
	// ~End FRenderResource API

private:
	FZenoMeshBufferAllocator& BufferAllocator;
};
