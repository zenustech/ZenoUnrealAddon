// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Utilities/ZenoEngineTypes.h"
#include "ZenoMeshDescriptor.generated.h"

USTRUCT()
struct ZENOENGINE_API FZenoMeshData
{
	GENERATED_BODY()

	/** Vertex positions. Array[WedgeId]=float3(x,y,z) */
	UPROPERTY()
	TArray<FVector3f> VertexBuffer;
	/** Texture coordinates. Array[WedgeId]=float2(u,v) */
	UPROPERTY()
	TArray<FVector2f> UVChannel0;
	/** UV from vn attr (if enable bTreatingNormalAsUV) */
	UPROPERTY()
	TArray<FVector2f> UVChannel1;
	/** Vertex normals. Array[WedgeId]=float3(x,y,z) */
	UPROPERTY()
	TArray<FVector3f> NormalChannel;
	/** Index of the vertex at this wedge. Array[FaceId] = VertexId */
	UPROPERTY()
	TArray<int32> IndexBuffer;
};

USTRUCT()
struct ZENOENGINE_API FZenoVatMeshData : public FZenoMeshData
{
	GENERATED_BODY()

	UPROPERTY()
	FZenoVatMetadata VatMetadata;
};

/**
 * Object to keeping track of the mesh data
 */
UCLASS()
class ZENOENGINE_API UZenoMeshInstance : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	FZenoMeshData MeshData;

	friend struct FWavefrontObjectContext;
	friend class FZenoVatMeshSceneProxy;
	
};
