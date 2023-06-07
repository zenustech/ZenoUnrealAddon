// Fill out your copyright notice in the Description page of Project Settings.


#include "ZenoSimpleMesh.h"

FZenoIndexBuffer::FZenoIndexBuffer(const TArray<uint32>& InIndices, bool bInUse32BitIndices/** = false*/, bool bInKeepDataInCPU/** = false*/)
	: bUse32BitIndices(bInUse32BitIndices)
	, bKeepDataInCPU(bInKeepDataInCPU)
{
	CPUIndicesData.Append(InIndices);
}

void FZenoIndexBuffer::InitRHI()
{
	if (bUse32BitIndices)
	{
		IndexBufferRHI = CreateIndexBuffer(CPUIndicesData);
	}
	else
	{
		TArray<uint16> Tmp;
		Tmp.Reserve(CPUIndicesData.Num());
		for (const uint32 Index : CPUIndicesData)
		{
			Tmp.Add(Index);
		}
		IndexBufferRHI = CreateIndexBuffer(Tmp);
	}
	
	if (!bKeepDataInCPU)
	{
		CPUIndicesData.Reset();
	}
}

FZenoVertexBuffer::FZenoVertexBuffer(const TArray<FVector4>& InVertices, bool bInKeepDataInCPU)
	: CPUSideVerticesData(InVertices)
	, bKeepDataInCPU(bInKeepDataInCPU)
{
}

void FZenoVertexBuffer::InitRHI()
{
	TResourceArray<FVector4, VERTEXBUFFER_ALIGNMENT> Vertices;
	
	NumVertices = CPUSideVerticesData.Num();
	const uint32 Size = sizeof(FVector4) * NumVertices;

	Vertices.AddUninitialized(NumVertices);
	for (int32 Idx = 0; Idx < NumVertices; Idx++)
	{
		Vertices[Idx] = CPUSideVerticesData[Idx];
	}
	
	FRHIResourceCreateInfo CreateInfo(TEXT("ZenoVertexBuffer"), &Vertices);
	VertexBufferRHI = RHICreateVertexBuffer(Size, BUF_Static, CreateInfo);

	if (!bKeepDataInCPU)
	{
		CPUSideVerticesData.Reset();
	}
}
