// Fill out your copyright notice in the Description page of Project Settings.


#include "ZenoMeshCommon.h"

#include "MaterialDomain.h"
#include "MeshMaterialShader.h"

FZenoMeshVertexFactoryBase::FZenoMeshVertexFactoryBase(ERHIFeatureLevel::Type InFeatureLevel, const char* InDebugName, FZenoMeshVertexBuffer* InVertexBuffer/* = nullptr*/, FZenoMeshIndexBuffer* InIndexBuffer/* = nullptr*/)
	: FLocalVertexFactory(InFeatureLevel, InDebugName)
	, VertexBuffer(InVertexBuffer)
	, IndexBuffer(InIndexBuffer)
{
	check(VertexBuffer != nullptr);
	check(IndexBuffer != nullptr);
}

FZenoMeshVertexFactoryBase::~FZenoMeshVertexFactoryBase()
{
}

void FZenoMeshVertexFactoryBase::InitResource()
{
	BeginInitResource(VertexBuffer);
	BeginInitResource(IndexBuffer);

	ENQUEUE_RENDER_COMMAND(InitZenoCommonMeshVertexFactory)(
		[this](FRHICommandListImmediate& RHICmdList)
		{
			 FDataType Data;
			 Data.PositionComponent = FVertexStreamComponent(
				  &VertexBuffer->PositionBuffer,
				  0,
				  sizeof(FZenoMeshVertexBuffer::FPositionType),
				  VET_Float3
			 );

			 Data.NumTexCoords = VertexBuffer->GetNumTexCoords();
			 {
				  Data.LightMapCoordinateIndex = VertexBuffer->GetLightmapCoordinateIndex();
				  Data.PositionComponentSRV = VertexBuffer->PositionBufferSRV;
				  Data.TangentsSRV = VertexBuffer->TangentBufferSRV;
				  Data.TextureCoordinatesSRV = VertexBuffer->TexCoordBufferSRV;
				  Data.ColorComponentsSRV = VertexBuffer->ColorBufferSRV;
			 }

			 {
				  EVertexElementType UVDoubleWideVertexElementType = VET_None;
				  EVertexElementType UVVertexElementType = VET_None;
				  uint32 UVSizeInBytes = 0;
				  if (VertexBuffer->IsUse16BitTexCoord())
				  {
					  UVSizeInBytes = sizeof(FVector2DHalf);
					  UVDoubleWideVertexElementType = VET_Half4;
					  UVVertexElementType = VET_Half2;
				  }
				  else
				  {
					  UVSizeInBytes = sizeof(FVector2f);
					  UVDoubleWideVertexElementType = VET_Float4;
					  UVVertexElementType = VET_Float2;
				  }

				  const int32 NumTexCoord = VertexBuffer->GetNumTexCoords();
				  
				  int32 UVIndex;
				  const uint32 UVStride = UVSizeInBytes * NumTexCoord;
				  for (UVIndex = 0; UVIndex < NumTexCoord - 1; UVIndex += 2)
				  {
					  Data.TextureCoordinates.Add(
							FVertexStreamComponent(
								  &VertexBuffer->TexCoordBuffer,
								  UVSizeInBytes * UVIndex,
								  UVStride,
								  UVDoubleWideVertexElementType,
								  EVertexStreamUsage::ManualFetch
							)
					  );
				  }

				  if (UVIndex < NumTexCoord)
				  {
					  Data.TextureCoordinates.Add(
							FVertexStreamComponent(
									  &VertexBuffer->TexCoordBuffer,
									  UVSizeInBytes * UVIndex,
									  UVStride,
									  UVVertexElementType,
									  EVertexStreamUsage::ManualFetch
								  )
					  );
				  }

				  Data.TangentBasisComponents[0] = FVertexStreamComponent(
					  &VertexBuffer->TangentBuffer,
					  0,
					  2 * sizeof(FPackedNormal),
					  VET_PackedNormal,
					  EVertexStreamUsage::ManualFetch
				  );
				  Data.TangentBasisComponents[1] = FVertexStreamComponent(
					  &VertexBuffer->TangentBuffer,
					  sizeof(FPackedNormal),
					  2 * sizeof(FPackedNormal),
					  VET_PackedNormal,
					  EVertexStreamUsage::ManualFetch
				  );

				  Data.ColorComponent = FVertexStreamComponent(
					  &VertexBuffer->ColorBuffer,
					  0,
					  sizeof(FColor),
					  VET_Color,
					  EVertexStreamUsage::ManualFetch
				  );
			 }
			 SetData(Data);
		}
	);
	
	FLocalVertexFactory::InitResource();
}

void FZenoMeshVertexFactoryBase::InitRHI()
{
	FLocalVertexFactory::InitRHI();
}

void FZenoMeshVertexFactoryBase::ReleaseRHI()
{
	FLocalVertexFactory::ReleaseRHI();
}

void FZenoMeshVertexFactoryBase::ReleaseResource()
{
	FLocalVertexFactory::ReleaseResource();
	
	if (VertexBuffer != nullptr)
	{
		VertexBuffer->ReleaseResource();
		delete VertexBuffer;
	}

	if (IndexBuffer != nullptr)
	{
		IndexBuffer->ReleaseResource();
		delete IndexBuffer;
	}
}

bool FZenoMeshVertexFactoryBase::ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& InParameters)
{
	return InParameters.MaterialParameters.MaterialDomain != MD_UI;
}

void FZenoMeshVertexFactoryBase::ModifyCompilationEnvironment(
	const FVertexFactoryShaderPermutationParameters& InParameters, FShaderCompilerEnvironment& OutEnvironment)
{
	OutEnvironment.SetDefine(TEXT("ZENO_COMMON_MESH"), TEXT("1"));
}
