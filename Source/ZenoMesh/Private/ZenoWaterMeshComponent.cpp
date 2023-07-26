#include "ZenoWaterMeshComponent.h"

#ifdef UE_5_2_OR_LATER
#include "DataDrivenShaderPlatformInfo.h"
#include "MaterialDomain.h"
#include "Materials/MaterialRenderProxy.h"
#endif
#include "MeshDrawShaderBindings.h"
#include "MeshMaterialShader.h"
#include "ZenoMeshBuffer.h"
#include "ZenoMeshCommon.h"
#include "ZenoRiverActor.h"
#include "ZenoSplineComponent.h"
#include "Misc/ScopedSlowTask.h"
#include "UObject/GCObjectScopeGuard.h"

#define LOCTEXT_NAMESPACE "ZenoMesh"

class AZenoRiverActor;

void BeginInitResourceIfNotNull(FRenderResource* Resource)
{
	if (Resource != nullptr) BeginInitResource(Resource);
}

class FZenoWaterMeshVertexFactory : public FLocalVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FZenoWaterMeshVertexFactory);

public:
	FZenoWaterMeshVertexFactory(ERHIFeatureLevel::Type InFeatureLevel, const char* InDebugName,
	                            const TSharedPtr<FZenoWaterMeshRenderData>& InRenderData)
		: FLocalVertexFactory(InFeatureLevel, InDebugName)
		, RenderData(InRenderData)
	{
	}

	virtual FString GetFriendlyName() const override { return "Zeno Water Mesh Vertex Factory"; }

	virtual void InitRHI() override
	{
		RenderData->UploadData_AnyThread();

		ENQUEUE_RENDER_COMMAND(ZenoWaterMeshInitRHI)(
			[this](FRHICommandListImmediate&)
			{
				auto* VertexBuffer = RenderData->VertexBuffer;
				auto* IndexBuffer = RenderData->IndexBuffer;

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
			});

		FLocalVertexFactory::InitRHI();
	}

	virtual void ReleaseRHI() override
	{
		FLocalVertexFactory::ReleaseRHI();
	}

private:
	TSharedPtr<FZenoWaterMeshRenderData, ESPMode::ThreadSafe> RenderData;
};

class FZenoWaterMeshVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
	DECLARE_TYPE_LAYOUT(FZenoWaterMeshVertexFactoryShaderParameters, NonVirtual);

	void Bind(const FShaderParameterMap& InParameterMap)
	{
		WaterTime.Bind(InParameterMap, TEXT("WaterTime"));
	}

	void GetElementShaderBindings(
		const FSceneInterface* Scene,
		const FSceneView* View,
		const FMeshMaterialShader* Shader,
		const EVertexInputStreamType InputStreamType,
		ERHIFeatureLevel::Type FeatureLevel,
		const FVertexFactory* VertexFactory,
		const FMeshBatchElement& BatchElement,
		FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams) const
	{
		const FZenoWaterMeshVertexFactory* WaterMeshVertexFactory = static_cast<const FZenoWaterMeshVertexFactory*>(
			VertexFactory);

		// Bind LocalVertexFactoryUniformBuffer back.
		const FRHIUniformBuffer* VertexFactoryUniformBuffer = static_cast<FRHIUniformBuffer*>(BatchElement.
			VertexFactoryUserData);
		if (WaterMeshVertexFactory->SupportsManualVertexFetch(FeatureLevel) || UseGPUScene(
			GMaxRHIShaderPlatform, FeatureLevel))
		{
			ShaderBindings.Add(Shader->GetUniformBufferParameter<FLocalVertexFactoryUniformShaderParameters>(),
			                   VertexFactoryUniformBuffer);
		}

		ShaderBindings.Add(WaterTime, 0.0f);
	}

private:
	LAYOUT_FIELD(FShaderParameter, WaterTime);
};

class FZenoWaterMeshSceneProxy : public FPrimitiveSceneProxy
{
public:
	FZenoWaterMeshSceneProxy(const UPrimitiveComponent* InComponent, const FName& ResourceName)
		: FPrimitiveSceneProxy(InComponent, ResourceName)
	{
		const UZenoWaterMeshComponent* WaterMeshComponent = Cast<UZenoWaterMeshComponent>(InComponent);
		check(WaterMeshComponent->RenderData.IsValid());

		RenderData = WaterMeshComponent->RenderData;

		VertexFactory = new FZenoWaterMeshVertexFactory(GetScene().GetFeatureLevel(), "FZenoWaterMeshSceneProxy",
		                                                RenderData);

		MaterialInterface = WaterMeshComponent->WaterMaterial;
	}

	virtual ~FZenoWaterMeshSceneProxy() override
	{
		delete VertexFactory;
	}

	virtual SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	virtual uint32 GetMemoryFootprint() const override
	{
		return sizeof(*this) + GetAllocatedSize();
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		// const bool bValid = true;
		// const bool bIsHiddenInEditor = View->Family->EngineShowFlags.Editor;
		//
		// FPrimitiveViewRelevance Result;
		// Result.bDrawRelevance = bValid && IsShown(View) && !bIsHiddenInEditor;
		// Result.bShadowRelevance = bValid && IsShadowCast(View) && ShouldRenderInMainPass() && !bIsHiddenInEditor;
		// Result.bDynamicRelevance = true;
		// Result.bStaticRelevance = false;
		// Result.bRenderInMainPass = ShouldRenderInMainPass();
		// Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		// Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		// Result.bTranslucentSelfShadow = false;
		// ViewRelevance.SetPrimitiveViewRelevance(Result);
		// Result.bVelocityRelevance = DrawsVelocity() && Result.bOpaque && Result.bRenderInMainPass;
		// return Result;
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bDynamicRelevance = true;
		Result.bRenderInMainPass = ShouldRenderInMainPass();
		Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
		ViewRelevance.SetPrimitiveViewRelevance(Result);
		Result.bVelocityRelevance = DrawsVelocity() && Result.bOpaque && Result.bRenderInMainPass;
		return Result;
	}

	virtual void CreateRenderThreadResources() override
	{
		BeginInitResource(VertexFactory);
	}

	virtual void DestroyRenderThreadResources() override
	{
		BeginReleaseResource(VertexFactory);
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily,
	                                    uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FZenoWaterMeshSceneProxy::GetDynamicMeshElements);
		// Set up wireframe material (if needed)
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		FColoredMaterialRenderProxy* WireframeMaterialInstance = nullptr;
		if (bWireframe)
		{
			WireframeMaterialInstance = new FColoredMaterialRenderProxy(
				GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr,
				FLinearColor(0.5f, 0.5f, 1.f)
			);

			Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
		}

		const FMaterialRenderProxy* MaterialProxy = bWireframe
			                                            ? WireframeMaterialInstance
			                                            : MaterialInterface.IsValid()
			                                            ? MaterialInterface->GetRenderProxy()
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
					GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld,
					SingleCaptureIndex,
					bOutputVelocity);
				bOutputVelocity |= AlwaysHasVelocity();

				FMeshBatch& MeshBatch = Collector.AllocateMesh();
				{
					MeshBatch.VertexFactory = VertexFactory;
					MeshBatch.MaterialRenderProxy = MaterialProxy;

					MeshBatch.CastShadow = IsShadowCast(View);
					MeshBatch.bWireframe = bWireframe;
					MeshBatch.bCanApplyViewModeOverrides = true;
					MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();
					MeshBatch.bDisableBackfaceCulling = ViewRelevance.bTwoSided;
					MeshBatch.Type = PT_TriangleList;
					MeshBatch.DepthPriorityGroup = SDPG_World;
					MeshBatch.bUseForDepthPass = true;
					MeshBatch.bUseAsOccluder = false;
					MeshBatch.bUseForMaterial = true;
					MeshBatch.bDitheredLODTransition = true;
					MeshBatch.bUseSelectionOutline = IsSelected();
				}

				MeshBatch.Elements.SetNumZeroed(1);
				{
					FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
					BatchElement.IndexBuffer = RenderData->IndexBuffer;
					BatchElement.FirstIndex = 0;
					BatchElement.NumPrimitives = RenderData->IndexBuffer->Indices.Num() / 3;
					BatchElement.MinVertexIndex = 0;
					BatchElement.MaxVertexIndex = RenderData->VertexBuffer->Vertices.Num() - 1;
					
					BatchElement.VertexFactoryUserData = VertexFactory->GetUniformBuffer();

					if (BatchElement.PrimitiveIdMode != PrimID_FromPrimitiveSceneInfo)
					{
						FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.
							AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
						DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(),
						                                  GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap,
						                                  bOutputVelocity);
						BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;
					}
					else
					{
						BatchElement.PrimitiveUniformBuffer = GetUniformBuffer();
					}
				}

				Collector.AddMesh(ViewIndex, MeshBatch);
			}
		}
	}

private:
	TSharedPtr<FZenoWaterMeshRenderData, ESPMode::ThreadSafe> RenderData;

	FZenoWaterMeshVertexFactory* VertexFactory = nullptr;

	FMaterialRelevance ViewRelevance;
	
	TSoftObjectPtr<UMaterialInterface> MaterialInterface;
};

FZenoWaterMeshRenderData::FZenoWaterMeshRenderData(bool bInKeepCPUData/* = true*/)
	: bKeepCPUData(bInKeepCPUData)
{
	BufferAllocator = new FZenoMeshBufferAllocator;
	VertexBuffer = new FZenoMeshVertexBuffer(1, 0, true, *BufferAllocator);
	IndexBuffer = new FZenoMeshIndexBuffer(*BufferAllocator);
}

FZenoWaterMeshRenderData::~FZenoWaterMeshRenderData()
{
	if (IsInRenderingThread())
	{
		VertexBuffer->ReleaseResource();
		IndexBuffer->ReleaseResource();
		delete IndexBuffer;
		delete VertexBuffer;
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(FZenoWaterMeshRenderData_UploadData)([IndexBuffer = IndexBuffer, VertexBuffer = VertexBuffer](FRHICommandListImmediate&)
		{
			VertexBuffer->ReleaseResource();
			IndexBuffer->ReleaseResource();
			delete IndexBuffer;
			delete VertexBuffer;
		});
	}
	delete BufferAllocator;
}

void FZenoWaterMeshRenderData::UploadData_AnyThread() const
{
	auto UploadOperator = [this]()
	{
		check(VertexBuffer != nullptr);
		check(IndexBuffer != nullptr);
		BeginInitResource(VertexBuffer);
		BeginInitResource(IndexBuffer);

		// Release cpu data if needed
		// if (!bKeepCPUData)
		// {
		// 	VertexBuffer->Vertices.Empty();
		// 	IndexBuffer->Indices.Empty();
		// }
	};

	if (IsInRenderingThread())
	{
		UploadOperator();
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(FZenoWaterMeshRenderData_UploadData)([UploadOperator](FRHICommandListImmediate&)
		{
			UploadOperator();
		});
	}
}

UZenoWaterMeshComponent::UZenoWaterMeshComponent(const FObjectInitializer& Initializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bAutoRegister = true;
	bWantsInitializeComponent = true;
	bAutoActivate = true;
}

UZenoWaterMeshComponent::~UZenoWaterMeshComponent()
{
}

void UZenoWaterMeshComponent::BuildRiverMesh(const FZenoRiverBuildInfo& InBuildInfo)
{
	if (!InBuildInfo.Spline.IsValid())
	{
		UE_LOG(LogZenoMesh, Error, TEXT("Invalid spline component, aborted building river mesh."));
		return;
	}
	RenderData = nullptr;
	TSharedRef<FZenoWaterMeshRenderData, ESPMode::ThreadSafe> TempRenderData = MakeShared<FZenoWaterMeshRenderData>(bKeepBufferInCPU);

	UZenoSplineComponent* SplineComponent = InBuildInfo.Spline.Get();
	// Prevent spline from GC
	FGCObjectScopeGuard SplineGuard(SplineComponent);

	TArray<FVector> PathPoints;
	const int32 NumPoints = SplineComponent->GetNumberOfSplinePoints() * InBuildInfo.Precision;
	const float Step = SplineComponent->Duration / static_cast<float>(NumPoints);
	PathPoints.Reserve(InBuildInfo.Precision);
	for (int32 i = 0; i < NumPoints; ++i)
	{
		PathPoints.Add(SplineComponent->GetLocationAtTime(i * Step, ESplineCoordinateSpace::Local));
	}

	for (const FVector& Point : PathPoints)
	{
		FVector RightVector = FVector::CrossProduct(Point, FVector::UpVector).GetSafeNormal();
		for (float Offset = -InBuildInfo.RiverWidth * 0.5f; Offset <= InBuildInfo.RiverWidth * 0.5f; Offset += InBuildInfo.Precision)
		{
			FVector Position = Point + RightVector * Offset;
			FVector2f UV = FVector2f(Offset / InBuildInfo.RiverWidth + 0.5f, 0.0f);
			TempRenderData->VertexBuffer->Vertices.Emplace(FVector3f(Position), UV);
		}
	}

	const int32 NumVertices = TempRenderData->VertexBuffer->Vertices.Num();
	const int32 NumColumns = InBuildInfo.RiverWidth / InBuildInfo.Precision + 1;

	for (int32 i = 0; i < NumVertices - NumColumns - 1; i++)
	{
		if ((i+1) % NumColumns == 0) continue;
		TempRenderData->IndexBuffer->Indices.Add(i);
		TempRenderData->IndexBuffer->Indices.Add(i + NumColumns);
		TempRenderData->IndexBuffer->Indices.Add(i + 1);
		
		TempRenderData->IndexBuffer->Indices.Add(i + NumColumns);
		TempRenderData->IndexBuffer->Indices.Add(i + NumColumns + 1);
		TempRenderData->IndexBuffer->Indices.Add(i + 1);
	}

	RenderData = TempRenderData;

	MarkRenderStateDirty();
	// call this in scene proxy
	// RenderData->UploadData_AnyThread();
}

FPrimitiveSceneProxy* UZenoWaterMeshComponent::CreateSceneProxy()
{
	if (RenderData.IsValid() && RenderData->IndexBuffer->Indices.Num() > 0)
	{
		return new FZenoWaterMeshSceneProxy(this, TEXT("FZenoWaterMeshSceneProxy"));
	}
	return nullptr;
}

void UZenoWaterMeshComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	Super::GetUsedMaterials(OutMaterials, bGetDebugMaterials);
	OutMaterials.Add(WaterMaterial);
}

void UZenoWaterMeshComponent::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
	Super::CreateRenderState_Concurrent(Context);
}

FBoxSphereBounds UZenoWaterMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
#if 1
	if (AZenoRiverActor* RiverActor = Cast<AZenoRiverActor>(GetAttachParentActor()); IsValid(RiverActor))
	{
		const float LengthMax = FMath::Max(RiverActor->SplineComponent->GetSplineLength(),
		                                   static_cast<float>(RiverActor->RiverWidth));
		FVector Extent(LengthMax, LengthMax, 10.f);
		Extent *= FVector(BoundsScale);
		return FBoxSphereBounds(FVector::Zero(), Extent, Extent.GetMax()).TransformBy(LocalToWorld);
	}
	return FBoxSphereBounds(LocalToWorld.GetLocation(), FVector(100.0f, 100.0f, 10.0f), 100.0f);
#else // For debug
    return FBoxSphereBounds(LocalToWorld.GetLocation(), FVector(1000000.0f, 1000000.0f, 1000000.0f), 1000000.0f);
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_TYPE_LAYOUT(FZenoWaterMeshVertexFactoryShaderParameters);

IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FZenoWaterMeshVertexFactory, SF_Vertex,
                                        FZenoWaterMeshVertexFactoryShaderParameters);

IMPLEMENT_VERTEX_FACTORY_TYPE(
	FZenoWaterMeshVertexFactory,
	"/Engine/Private/LocalVertexFactory.ush",
	EVertexFactoryFlags::UsedWithMaterials
	// | EVertexFactoryFlags::SupportsComputeShading
);
