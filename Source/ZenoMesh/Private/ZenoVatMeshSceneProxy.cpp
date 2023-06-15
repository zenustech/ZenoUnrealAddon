#include "ZenoVatMeshSceneProxy.h"

#include "MaterialDomain.h"
#include "ZenoMeshDescriptor.h"
#include "ZenoVATMeshComponent.h"
#include "ZenoVatMeshVertexFactory.h"
#include "Materials/MaterialRenderProxy.h"

FZenoVatMeshSceneProxy::FZenoVatMeshSceneProxy(const UPrimitiveComponent* InComponent, const FName& ResourceName)
	: FPrimitiveSceneProxy(InComponent, ResourceName)
{
	const UZenoVATMeshComponent* Component = Cast<UZenoVATMeshComponent>(InComponent);
	if (MaterialInterface = Component->MeshMaterial; MaterialInterface.IsValid())
	{
		MaterialRelevance = MaterialInterface->GetRelevance(GetScene().GetFeatureLevel());
	}
	else
	{
		MaterialInterface = UMaterial::GetDefaultMaterial(MD_Surface);
		MaterialRelevance = MaterialInterface->GetRelevance(GetScene().GetFeatureLevel());
	}

	if (const UZenoMeshInstance* MeshInstance = Component->MeshData; IsValid(MeshInstance))
	{
		const FZenoMeshData MeshData = MeshInstance->MeshData;
		VertexFactory = new FZenoVatMeshVertexFactory(&MeshData, GetScene().GetFeatureLevel(), "ZenoVatMeshVertexFactory");
	}
	else
	{
		VertexFactory = new FZenoVatMeshVertexFactory(nullptr, GetScene().GetFeatureLevel(), "ZenoVatMeshVertexFactory");
	}

	UniformData = new FZenoVatMeshUniformData();
	UniformData->bAutoPlay = Component->bAutoPlay;
	UniformData->BoundsMax = Component->MaxBounds;
	UniformData->BoundsMin = Component->MinBounds;
	UniformData->PlaySpeed = Component->PlaySpeed;
	UniformData->TextureHeight = Component->TextureHeight;
	UniformData->TotalFrame = Component->TotalFrame;
	UniformData->CurrentFrame = Component->CurrentFrame;
	if (!Component->PositionTexturePath.IsNull())
	{
		UniformData->PositionTexture = Component->PositionTexturePath.LoadSynchronous();
	}
}

FZenoVatMeshSceneProxy::~FZenoVatMeshSceneProxy()
{
	if (VertexFactory)
	{
		VertexFactory->ReleaseResource();
	}
	delete VertexFactory;
	delete UniformData;
}

SIZE_T FZenoVatMeshSceneProxy::GetTypeHash() const
{
	static uint8 UniquePointer = 0;
	return reinterpret_cast<SIZE_T>(&UniquePointer);
}

uint32 FZenoVatMeshSceneProxy::GetMemoryFootprint() const
{
	return sizeof(*this) + GetAllocatedSize();
}

FPrimitiveViewRelevance FZenoVatMeshSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View);
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bDynamicRelevance = true;
	Result.bRenderInMainPass = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
	MaterialRelevance.SetPrimitiveViewRelevance(Result);
	Result.bVelocityRelevance = DrawsVelocity() && Result.bOpaque && Result.bRenderInMainPass;
	return Result;
}

void FZenoVatMeshSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
                                                    const FSceneViewFamily& ViewFamily, uint32 VisibilityMap,
                                                    FMeshElementCollector& Collector) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FZenoVatMeshSceneProxy::GetDynamicMeshElements);
	if (VertexFactory == nullptr || VertexFactory->VertexBuffer == nullptr || VertexFactory->IndexBuffer == nullptr || VertexFactory->VertexBuffer->Vertices.IsEmpty())
	{
		ensureMsgf(false, TEXT("ZenoVatMeshSceneProxy has invalid vertex data"));
		return;
	}
	
	// Set up wireframe material (if needed)
	const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

	FColoredMaterialRenderProxy* WireframeMaterialInstance = nullptr;
	if (bWireframe)
	{
		WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr,
			FLinearColor(0, 0.5f, 1.f)
		);

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
	}
	
	const FMaterialRenderProxy* MaterialProxy = bWireframe
		                                            ? WireframeMaterialInstance
		                                            : MaterialInterface.IsValid() ? MaterialInterface->GetRenderProxy() : UMaterial::GetDefaultMaterial(MD_Surface)->GetRenderProxy();

	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			const FSceneView* View = Views[ViewIndex];

			bool bHasPrecomputedVolumetricLightmap;
			FMatrix PreviousLocalToWorld;
			int32 SingleCaptureIndex;
			bool bOutputVelocity;
			GetScene().GetPrimitiveUniformShaderParameters_RenderThread(
				GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex,
				bOutputVelocity);
			bOutputVelocity |= AlwaysHasVelocity();
			
			FZenoVatMeshUniformDataWrapper& UserData = Collector.AllocateOneFrameResource<FZenoVatMeshUniformDataWrapper>();
			UserData.Data = *UniformData;
			
			FMeshBatch& Mesh = Collector.AllocateMesh();
			FMeshBatchElement& BatchElement = Mesh.Elements[0];
			{
				Mesh.VertexFactory = VertexFactory;
				Mesh.MaterialRenderProxy = MaterialProxy;

				Mesh.CastShadow = IsShadowCast(View);
				Mesh.bWireframe = bWireframe;
				Mesh.bCanApplyViewModeOverrides = false;
				Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
				Mesh.bDisableBackfaceCulling = MaterialRelevance.bTwoSided;
				Mesh.Type = PT_TriangleList;
				Mesh.DepthPriorityGroup = SDPG_World;
				Mesh.bUseSelectionOutline = IsSelected();
			}
			{
				if (BatchElement.PrimitiveIdMode != PrimID_FromPrimitiveSceneInfo)
				{
					 FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
					 DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, bOutputVelocity);
					 BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;
				}
				else
				{
					BatchElement.PrimitiveUniformBuffer = GetUniformBuffer();
				}
				
				BatchElement.IndexBuffer = VertexFactory->IndexBuffer;
				BatchElement.UserData = &UserData;

				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = VertexFactory->IndexBuffer->Indices.Num() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = VertexFactory->VertexBuffer->Vertices.Num() - 1;

				// Pass the vertex factory's uniform buffer to FZenoVatMeshVertexFactoryShaderParameters::GetElementShaderBindings
				BatchElement.VertexFactoryUserData = VertexFactory->GetUniformBuffer();
			}
			
			Collector.AddMesh(ViewIndex, Mesh);
		}
	}
}

void FZenoVatMeshSceneProxy::CreateRenderThreadResources()
{
	VertexFactory->InitResource();
	FPrimitiveSceneProxy::CreateRenderThreadResources();
}

void FZenoVatMeshSceneProxy::DestroyRenderThreadResources()
{
	FPrimitiveSceneProxy::DestroyRenderThreadResources();
}

void FZenoVatMeshSceneProxy::SetVatInfo_RenderThread(const FZenoVatMeshUniformData& InUniformData) const
{
	*UniformData = InUniformData;
}

void FZenoVatMeshSceneProxy::SetMaterial_RenderThread(const UMaterialInterface* InMaterialInterface)
{
	if (IsValid(InMaterialInterface))
	{
		MaterialInterface = InMaterialInterface;
	}
}
