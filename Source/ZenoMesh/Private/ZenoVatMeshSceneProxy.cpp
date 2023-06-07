#include "ZenoVatMeshSceneProxy.h"

#include "MaterialDomain.h"
#include "ZenoVatMeshVertexFactory.h"
#include "Materials/MaterialRenderProxy.h"

FZenoVatMeshSceneProxy::FZenoVatMeshSceneProxy(const UPrimitiveComponent* InComponent, const FName& ResourceName)
	: FPrimitiveSceneProxy(InComponent, ResourceName)
{
	if (MaterialInterface = InComponent->GetMaterial(0); MaterialInterface.IsValid())
	{
		MaterialRelevance = MaterialInterface->GetRelevance(GetScene().GetFeatureLevel());
	}
	else
	{
		const UMaterialInterface* DefaultMaterial = UMaterial::GetDefaultMaterial(MD_Surface);
		MaterialRelevance = DefaultMaterial->GetRelevance(GetScene().GetFeatureLevel());
	}
	
	VertexFactory = new FZenoVatMeshVertexFactory(GetScene().GetFeatureLevel(), "ZenoVatMeshVertexFactory");
	
	IndexBuffer.Indices = { 0, 1, 2, 2, 1, 3 };
	TArray Vertices {
		FDynamicMeshVertex { FVector3f { .0f, .0f, 1.f } * 100.f },
		FDynamicMeshVertex { FVector3f { .5f, .0f, .5f } * 100.f },
		FDynamicMeshVertex { FVector3f { .0f, .5f, .5f } * 100.f },
		FDynamicMeshVertex { FVector3f { .5f, .5f, .0f } * 100.f },
	};
	VertexBuffers.InitFromDynamicVertex(VertexFactory, Vertices, 4);

	BeginInitResource(&VertexBuffers.PositionVertexBuffer);
	BeginInitResource(&VertexBuffers.StaticMeshVertexBuffer);
	BeginInitResource(&VertexBuffers.ColorVertexBuffer);
	BeginInitResource(&IndexBuffer);
	BeginInitResource(VertexFactory);
}

FZenoVatMeshSceneProxy::~FZenoVatMeshSceneProxy()
{
	VertexBuffers.PositionVertexBuffer.ReleaseResource();
	VertexBuffers.ColorVertexBuffer.ReleaseResource();
	VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
	IndexBuffer.ReleaseResource();
	
	if (VertexFactory)
	{
		VertexFactory->ReleaseResource();
	}
	delete VertexFactory;
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
		                                            : UMaterial::GetDefaultMaterial(MD_Surface)->GetRenderProxy();

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

			FMeshBatch& Mesh = Collector.AllocateMesh();
			{
				Mesh.bWireframe = bWireframe;
				Mesh.VertexFactory = VertexFactory;
				Mesh.MaterialRenderProxy = MaterialProxy;
				Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
				Mesh.Type = PT_TriangleList;
				Mesh.DepthPriorityGroup = SDPG_World;
				Mesh.bCanApplyViewModeOverrides = false;
			}
			{
				FMeshBatchElement& BatchElement = Mesh.Elements[0];
				BatchElement.IndexBuffer = &IndexBuffer;

				FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
				DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(),
				                                  GetLocalBounds(), GetLocalBounds(), true,
				                                  bHasPrecomputedVolumetricLightmap, bOutputVelocity,
				                                  GetCustomPrimitiveData());
				BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = IndexBuffer.Indices.Num() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;
			}

			Collector.AddMesh(ViewIndex, Mesh);
		}
	}
}

void FZenoVatMeshSceneProxy::DestroyRenderThreadResources()
{
	FPrimitiveSceneProxy::DestroyRenderThreadResources();
}

void FZenoVatMeshSceneProxy::UpdateBuffer()
{
	ENQUEUE_RENDER_COMMAND(FZenoVatMeshSceneProxyUpdateBuffer)(
		[this] (FRHICommandListImmediate& RHICmdList)
		{
			TArray Vertices {
				FDynamicMeshVertex { FVector3f { .0f, .0f, 1.f } * 100.f },
				FDynamicMeshVertex { FVector3f { .5f, .0f, .5f } * 100.f },
				FDynamicMeshVertex { FVector3f { .0f, .5f, .5f } * 100.f },
				FDynamicMeshVertex { FVector3f { .5f, .5f, .0f } * 100.f },
			};

			for (int32 Idx = 0; Idx < Vertices.Num(); Idx++)
			{
				VertexBuffers.PositionVertexBuffer.VertexPosition(Idx) = Vertices[Idx].Position;
				VertexBuffers.ColorVertexBuffer.VertexColor(Idx) = FColor::White;
			}

			{
				auto& VertexBuffer = VertexBuffers.PositionVertexBuffer;
				void* VertexBufferData = RHILockBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
				FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
				RHIUnlockBuffer(VertexBuffer.VertexBufferRHI);
			}

			{
				auto& VertexBuffer = VertexBuffers.ColorVertexBuffer;
				void* VertexBufferData = RHILockBuffer(VertexBuffer.VertexBufferRHI, 0,
				                                       VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(),
				                                       RLM_WriteOnly);
				FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(),
				                VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
				RHIUnlockBuffer(VertexBuffer.VertexBufferRHI);
			}
		}
	);
}
