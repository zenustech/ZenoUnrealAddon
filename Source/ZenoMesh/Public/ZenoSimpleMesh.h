// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZenoSimpleMesh.generated.h"


////////////////////////////////////////////////////////////////////////////
/// Index Buffer of the Mesh
///////////////////////////////////////////////////////////////////////////
class ZENOMESH_API FZenoIndexBuffer : public FIndexBuffer
{
public:
	FZenoIndexBuffer(const TArray<uint32>& InIndices, bool bInUse32BitIndices = false, bool bInKeepDataInCPU = false);

	virtual void InitRHI() override;

	int32 GetNumIndices() const { return NumIndices; }

private:
	bool bUse32BitIndices;
	bool bKeepDataInCPU;

	template <typename IndexType>
	FBufferRHIRef CreateIndexBuffer(const TArray<IndexType>& InIndices);

	TArray<uint32> CPUIndicesData;
	int32 NumIndices = 0;
};

template <typename IndexType>
FBufferRHIRef FZenoIndexBuffer::CreateIndexBuffer(const TArray<IndexType>& InIndices)
{
	TResourceArray<IndexType, INDEXBUFFER_ALIGNMENT> Indices;
	Indices.AddUninitialized(InIndices.Num());
	for (int32 Idx = 0; Idx < InIndices.Num(); Idx++)
	{
		Indices[Idx] = InIndices[Idx];
	}

	NumIndices = Indices.Num();
	const uint32 Size = Indices.GetResourceDataSize();
	const uint32 Stride = sizeof(IndexType);

	FRHIResourceCreateInfo CreateInfo(TEXT("ZenoIndexBuffer"), &Indices);
	return RHICreateIndexBuffer(Stride, Size, BUF_Static, CreateInfo);
}


////////////////////////////////////////////////////////////////////////////
/// Vertex Buffer of the Mesh
///////////////////////////////////////////////////////////////////////////
class ZENOMESH_API FZenoVertexBuffer : public FVertexBuffer
{
public:
	FZenoVertexBuffer(const TArray<FVector4>& InVertices, bool bInKeepDataInCPU = false);

	virtual void InitRHI() override;

	int32 GetNumVertices() const { return NumVertices; }

private:
	TArray<FVector4> CPUSideVerticesData;
	bool bKeepDataInCPU;

	int32 NumVertices = 0;
};

UCLASS()
class ZENOMESH_API UZenoSimpleMesh : public UObject
{
	GENERATED_BODY()
};
