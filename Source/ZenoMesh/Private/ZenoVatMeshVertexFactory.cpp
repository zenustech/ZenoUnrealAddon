#include "ZenoVatMeshVertexFactory.h"

#include "MaterialDomain.h"
#include "MeshDrawShaderBindings.h"
#include "MeshMaterialShader.h"
#include "ZenoMeshBuffer.h"
#include "ZenoVATMeshComponent.h"

////////////////////////////////////////////////////////////////////////
/// FZenoVatMeshVertexFactoryShaderParameters
////////////////////////////////////////////////////////////////////////

void FZenoVatMeshVertexFactoryShaderParameters::Bind(const FShaderParameterMap& ParameterMap)
{
	BoundsMin.Bind(ParameterMap, TEXT("VatBoundsMin"));
	BoundsMax.Bind(ParameterMap, TEXT("VatBoundsMax"));
	TotalFrame.Bind(ParameterMap, TEXT("VatTotalFrame"));
	TextureHeight.Bind(ParameterMap, TEXT("VatTextureHeight"));
	PlaySpeed.Bind(ParameterMap, TEXT("VatPlaySpeed"));
	bAutoPlay.Bind(ParameterMap, TEXT("ShouldVatAutoPlay"));
	PositionTexture.Bind(ParameterMap, TEXT("VatPositionTexture"));
	PositionTextureSampler.Bind(ParameterMap, TEXT("VatPositionTextureSampler"));
	CurrentFrame.Bind(ParameterMap, TEXT("VatCurrentFrame"));
}

void FZenoVatMeshVertexFactoryShaderParameters::GetElementShaderBindings(const FSceneInterface* Scene,
	const FSceneView* View, const FMeshMaterialShader* Shader, const EVertexInputStreamType InputStreamType,
	ERHIFeatureLevel::Type FeatureLevel, const FVertexFactory* VertexFactory, const FMeshBatchElement& BatchElement,
	FMeshDrawSingleShaderBindings& ShaderBindings, FVertexInputStreamArray& VertexStreams) const
{
	const FZenoVatMeshVertexFactory* ZenoVatMeshVertexFactory = static_cast<const FZenoVatMeshVertexFactory*>(VertexFactory);

	// Bind LocalVertexFactoryUniformBuffer back.
	const FRHIUniformBuffer* VertexFactoryUniformBuffer = static_cast<FRHIUniformBuffer*>(BatchElement.VertexFactoryUserData);
	if(ZenoVatMeshVertexFactory->SupportsManualVertexFetch(FeatureLevel) || UseGPUScene(GMaxRHIShaderPlatform, FeatureLevel))
	{
		ShaderBindings.Add(Shader->GetUniformBufferParameter<FLocalVertexFactoryUniformShaderParameters>(), VertexFactoryUniformBuffer);
	}

	const FZenoVatMeshUniformDataWrapper* BatchUserData = static_cast<const FZenoVatMeshUniformDataWrapper*>(BatchElement.UserData);
	check(BatchUserData);

	ShaderBindings.Add(BoundsMin, BatchUserData->Data.BoundsMin);
	ShaderBindings.Add(BoundsMax, BatchUserData->Data.BoundsMax);
	ShaderBindings.Add(TotalFrame, BatchUserData->Data.TotalFrame);
	ShaderBindings.Add(TextureHeight, BatchUserData->Data.TextureHeight);
	ShaderBindings.Add(PlaySpeed, BatchUserData->Data.PlaySpeed);
	ShaderBindings.Add(bAutoPlay, BatchUserData->Data.bAutoPlay);
	ShaderBindings.Add(CurrentFrame, BatchUserData->Data.CurrentFrame);

	if (BatchUserData->Data.PositionTexture.IsValid())
	{
		ShaderBindings.AddTexture(PositionTexture, PositionTextureSampler, BatchUserData->Data.PositionTexture->GetResource()->SamplerStateRHI, BatchUserData->Data.PositionTexture->GetResource()->GetTexture2DRHI());
	}
}

////////////////////////////////////////////////////////////////////////
/// FZenoVatMeshVertexFactory
////////////////////////////////////////////////////////////////////////

FZenoVatMeshVertexFactory::FZenoVatMeshVertexFactory(ERHIFeatureLevel::Type InFeatureLevel, const char* InDebugName)
	: FLocalVertexFactory(InFeatureLevel, InDebugName)
{
}

bool FZenoVatMeshVertexFactory::ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& InParameters)
{
	return InParameters.MaterialParameters.MaterialDomain == MD_Surface && InParameters.MaterialParameters.ShadingModels.HasAnyShadingModel({ MSM_DefaultLit, MSM_Unlit });
}

void FZenoVatMeshVertexFactory::ModifyCompilationEnvironment(
	const FVertexFactoryShaderPermutationParameters& InParameters, FShaderCompilerEnvironment& OutEnvironment)
{
	OutEnvironment.SetDefine(TEXT("CLOTH_MESH"), TEXT("1"));
}

void FZenoVatMeshVertexFactory::SetSceneProxy(FZenoVatMeshSceneProxy* InSceneProxy)
{
	SceneProxy = InSceneProxy;
}

void FZenoVatMeshVertexFactory::InitResource()
{
	FLocalVertexFactory* VertexFactory = this;
	VertexBuffer = new FZenoMeshVertexBuffer(3, 2, false, BufferAllocator);
	IndexBuffer = new FZenoMeshIndexBuffer(BufferAllocator);

#if 1
	// Debug
	{
		VertexBuffer->Vertices.Append({
			FZenoMeshVertex { FVector3f { -100.f, .0f, .0f } },
			FZenoMeshVertex { FVector3f { 0.f, 100.0f, .0f } },
			FZenoMeshVertex { FVector3f { 100.f, .0f, .0f } },
		});
		IndexBuffer->Indices.Append({ 0, 1, 2 });
	}
#endif

	BeginInitResource(VertexBuffer);
	BeginInitResource(IndexBuffer);
	
	const FZenoMeshVertexBuffer* MeshVertexBuffer = VertexBuffer;

	ENQUEUE_RENDER_COMMAND(InitZenoVatMeshVertexFactory) (
		[VertexFactory, MeshVertexBuffer] (FRHICommandListImmediate& RHICmdList)
		{
			FDataType Data;
			Data.PositionComponent = FVertexStreamComponent(
				&MeshVertexBuffer->PositionBuffer,
				0,
				sizeof(FZenoMeshVertexBuffer::FPositionType),
				VET_Float3
			);

			Data.NumTexCoords = MeshVertexBuffer->GetNumTexCoords();
			{
				Data.LightMapCoordinateIndex = MeshVertexBuffer->GetLightmapCoordinateIndex();
				Data.PositionComponentSRV = MeshVertexBuffer->PositionBufferSRV;
				Data.TangentsSRV = MeshVertexBuffer->TangentBufferSRV;
				Data.TextureCoordinatesSRV = MeshVertexBuffer->TexCoordBufferSRV;
				Data.ColorComponentsSRV = MeshVertexBuffer->ColorBufferSRV;
			}

			{
				EVertexElementType UVDoubleWideVertexElementType = VET_None;
				EVertexElementType UVVertexElementType = VET_None;
				uint32 UVSizeInBytes = 0;
				if (MeshVertexBuffer->IsUse16BitTexCoord())
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

				const int32 NumTexCoord = MeshVertexBuffer->GetNumTexCoords();
				int32 UVIndex;
				const uint32 UVStride = UVSizeInBytes * NumTexCoord;
				for (UVIndex = 0; UVIndex < NumTexCoord - 1; UVIndex += 2)
				{
					Data.TextureCoordinates.Add(
						FVertexStreamComponent(
							&MeshVertexBuffer->TexCoordBuffer,
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
								&MeshVertexBuffer->TexCoordBuffer,
								UVSizeInBytes * UVIndex,
								UVStride,
								UVVertexElementType,
								EVertexStreamUsage::ManualFetch
							)
					);
				}

				Data.TangentBasisComponents[0] = FVertexStreamComponent(
					&MeshVertexBuffer->TangentBuffer,
					0,
					2 * sizeof(FPackedNormal),
					VET_PackedNormal,
					EVertexStreamUsage::ManualFetch
			    );
				Data.TangentBasisComponents[1] = FVertexStreamComponent(
					&MeshVertexBuffer->TangentBuffer,
					sizeof(FPackedNormal),
					2 * sizeof(FPackedNormal),
					VET_PackedNormal,
					EVertexStreamUsage::ManualFetch
				);

				Data.ColorComponent = FVertexStreamComponent(
					&MeshVertexBuffer->ColorBuffer,
					0,
					sizeof(FColor),
					VET_Color,
					EVertexStreamUsage::ManualFetch
				);
			}
			VertexFactory->SetData(Data);
		}
	);
	
	FLocalVertexFactory::InitResource();
}

void FZenoVatMeshVertexFactory::ReleaseResource()
{
	FLocalVertexFactory::ReleaseResource();
	if (VertexBuffer)
	{
		VertexBuffer->ReleaseResource();
		delete VertexBuffer;
	}
	if (IndexBuffer)
	{
		IndexBuffer->ReleaseResource();
		delete IndexBuffer;
	}
}

IMPLEMENT_TYPE_LAYOUT(FZenoVatMeshVertexFactoryShaderParameters);

IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FZenoVatMeshVertexFactory, SF_Vertex, FZenoVatMeshVertexFactoryShaderParameters);

IMPLEMENT_VERTEX_FACTORY_TYPE(
	FZenoVatMeshVertexFactory,
	"/Plugin/ZenoMesh/Private/VATVertexFactory.ush",
	// "/Engine/Private/LocalVertexFactory.ush",
	  EVertexFactoryFlags::UsedWithMaterials
	| EVertexFactoryFlags::SupportsStaticLighting
	| EVertexFactoryFlags::SupportsDynamicLighting
	| EVertexFactoryFlags::SupportsPositionOnly
	| EVertexFactoryFlags::SupportsManualVertexFetch
);
