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


static TAutoConsoleVariable<float> CVarWaterTimeDelta(
	TEXT("z.Water.Delta"),
	0.01f,
	TEXT("Water time delta"),
	ECVF_RenderThreadSafe | ECVF_Scalability
);

class FZenoWaterMeshVertexFactory : public FZenoMeshVertexFactoryBase
{
	DECLARE_VERTEX_FACTORY_TYPE(FZenoWaterMeshVertexFactory);
	
public:
	using Super = FZenoMeshVertexFactoryBase;
	
	FZenoWaterMeshVertexFactory(const FIntPoint& InQuads, const FIntPoint& InPrecision, ERHIFeatureLevel::Type InFeatureLevel, const char* InDebugName = "WaterMeshVertexFactory")
		: FZenoMeshVertexFactoryBase(InFeatureLevel, InDebugName, new FZenoMeshVertexBuffer(1, 0, true, BufferAllocator), new FZenoMeshIndexBuffer(BufferAllocator))
		, QuadSize(InQuads)
		, Precision(InPrecision)
	{
	}

	virtual ~FZenoWaterMeshVertexFactory() override
	{
	}
	
	virtual FString GetFriendlyName() const override { return "ZenoWaterVertexFactory"; }

	virtual void InitResource() override
	{
		const FVector3f QuadCenter(QuadSize.X * 0.5, QuadSize.Y * 0.5, 0.0);
		const FIntPoint QuadNum(QuadSize.X + Precision.X - 1, QuadSize.Y + Precision.Y - 1);
		const FIntPoint VertexNum(QuadNum.X + 1, QuadNum.Y + 1);
		const FVector3f SubDelta(static_cast<double>(QuadSize.X) / FMath::Max(1.0, static_cast<double>(QuadNum.X)), static_cast<double>(QuadSize.Y) / FMath::Max(1.0, static_cast<double>(QuadNum.Y)), 0.0);

		VertexBuffer->Vertices.Reserve(VertexNum.X * VertexNum.Y);
		IndexBuffer->Indices.Reserve(QuadNum.X * QuadNum.Y * 6);
		for (int32 y = 0; y < VertexNum.Y; ++y)
		{
			for (int32 x = 0; x < VertexNum.X; ++x)
			{
				const FVector3f Position = FVector3f(x, y, 0.0) * SubDelta - QuadCenter;
				const FVector2f TexCoord = FVector2f(x, y) / FVector2f(VertexNum);
				VertexBuffer->Vertices.Emplace(Position, TexCoord);
			}
		}
		for (int32 y = 0; y < QuadNum.Y; ++y)
		{
			for (int32 x = 0; x < QuadNum.X; ++x)
			{
				const int32 Index = y * VertexNum.X + x;
				IndexBuffer->Indices.Emplace(Index);
				IndexBuffer->Indices.Emplace(Index + VertexNum.X);
				IndexBuffer->Indices.Emplace(Index + 1);
				IndexBuffer->Indices.Emplace(Index + 1);
				IndexBuffer->Indices.Emplace(Index + VertexNum.X);
				IndexBuffer->Indices.Emplace(Index + VertexNum.X + 1);
			}
		}
		
		Super::InitResource();
	}

	virtual bool SupportsPositionOnlyStream() const override
	{
		// return RHISupportsManualVertexFetch(GMaxRHIShaderPlatform);
		return false;
	}

	virtual bool SupportsPositionAndNormalOnlyStream() const override
	{
		// return RHISupportsManualVertexFetch(GMaxRHIShaderPlatform);
		return false;
	}
	
	FORCEINLINE float GetWaterTime() const
	{
		static float WaterTime = 0.0f;
		return WaterTime += CVarWaterTimeDelta.GetValueOnRenderThread();
	}

	FORCEINLINE float GetWaterDepth_RenderThread() const
	{
		return WaterDepth;
	}

	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& InParameters,
	                                         FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("ZENO_WATER_MESH"), TEXT("1"));
	}

protected:
	FIntPoint QuadSize;
	FIntPoint Precision;

	float WaterDepth;

	friend class FZenoWaterMeshSceneProxy;
};

class FZenoWaterMeshVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
	DECLARE_TYPE_LAYOUT(FZenoWaterMeshVertexFactoryShaderParameters, NonVirtual);

public:
	void Bind(const FShaderParameterMap& ParameterMap)
	{
		WaterTime.Bind(ParameterMap, TEXT("WaterTime"));
		WaterDepth.Bind(ParameterMap, TEXT("WaterDepth"));
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
		const FZenoWaterMeshVertexFactory* ZenoWaterMeshVertexFactory = static_cast<const FZenoWaterMeshVertexFactory*>(VertexFactory);
		
		// Bind LocalVertexFactoryUniformBuffer back.
		const FRHIUniformBuffer* VertexFactoryUniformBuffer = static_cast<FRHIUniformBuffer*>(BatchElement.VertexFactoryUserData);
		if (ZenoWaterMeshVertexFactory->SupportsManualVertexFetch(FeatureLevel) || UseGPUScene(GMaxRHIShaderPlatform, FeatureLevel))
		{
			ShaderBindings.Add(Shader->GetUniformBufferParameter<FLocalVertexFactoryUniformShaderParameters>(), VertexFactoryUniformBuffer);
		}

		ShaderBindings.Add(WaterTime, ZenoWaterMeshVertexFactory->GetWaterTime());
		ShaderBindings.Add(WaterDepth, ZenoWaterMeshVertexFactory->GetWaterDepth_RenderThread());
	}
private:
	LAYOUT_FIELD(FShaderParameter, WaterTime);
	LAYOUT_FIELD(FShaderParameter, WaterDepth);
};

class FZenoWaterMeshSceneProxy final : public FPrimitiveSceneProxy
{
public:
	FZenoWaterMeshSceneProxy(const UPrimitiveComponent* InComponent, const FName& ResourceName)
		: FPrimitiveSceneProxy(InComponent, ResourceName)
	{
		const UZenoWaterMeshComponent* WaterComponent = Cast<UZenoWaterMeshComponent>(InComponent);
		check(IsValid(WaterComponent));

		QuadSize = WaterComponent->GridSize;
		Precision =  WaterComponent->Precision;
		MaterialInterface = WaterComponent->WaterMaterial;
		VertexFactory = new FZenoWaterMeshVertexFactory(QuadSize, Precision, GetScene().GetFeatureLevel(), TCHAR_TO_ANSI(*WaterComponent->GetName()));
		VertexFactory->WaterDepth = WaterComponent->WaterDepth;
	}

	explicit FZenoWaterMeshSceneProxy(FPrimitiveSceneProxy const& PrimitiveSceneProxy) = delete;

	virtual ~FZenoWaterMeshSceneProxy() override
	{
		if (VertexFactory)
		{
			VertexFactory->ReleaseResource();
			delete VertexFactory;
		}
	}

	virtual SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<SIZE_T>(&UniquePointer);
	}
	
	virtual uint32 GetMemoryFootprint() const override
	{
		return sizeof(*this) + GetAllocatedSize();
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FZenoWaterMeshSceneProxy::GetDynamicMeshElements);
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
				
				FMeshBatch& MeshBatch = Collector.AllocateMesh();
				{
					MeshBatch.VertexFactory = VertexFactory;
					MeshBatch.MaterialRenderProxy = MaterialProxy;

					MeshBatch.CastShadow = false;
					MeshBatch.bWireframe = bWireframe;
					MeshBatch.bCanApplyViewModeOverrides = true;
					MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();
					MeshBatch.bDisableBackfaceCulling = MaterialRelevance.bTwoSided;
					MeshBatch.Type = PT_TriangleList;
					MeshBatch.DepthPriorityGroup = SDPG_World;
					MeshBatch.bUseForDepthPass = true;
					MeshBatch.bUseAsOccluder = true;
					MeshBatch.bUseForMaterial = true;
					MeshBatch.bDitheredLODTransition = true;
					MeshBatch.bUseSelectionOutline = IsSelected();
				}

				{
					FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
					BatchElement.PrimitiveUniformBuffer = GetUniformBuffer();

					// Pass the vertex factory's uniform buffer to FZenoVatMeshVertexFactoryShaderParameters::GetElementShaderBindings
					BatchElement.VertexFactoryUserData = VertexFactory->GetUniformBuffer();

					BatchElement.IndexBuffer = VertexFactory->IndexBuffer;
					BatchElement.FirstIndex = 0;
					BatchElement.NumPrimitives = VertexFactory->IndexBuffer->Indices.Num() / 3;
					BatchElement.MinVertexIndex = 0;
					BatchElement.MaxVertexIndex = VertexFactory->VertexBuffer->Vertices.Num() - 1;
				}

				Collector.AddMesh(ViewIndex, MeshBatch);
			}
		}
		
	}

	virtual void CreateRenderThreadResources() override
	{
		check(VertexFactory);
		VertexFactory->InitResource();
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
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

protected:

	/** How many quad into rendering */
	FIntPoint QuadSize = FIntPoint(10, 10);
	
	FIntPoint Precision = FIntPoint(10, 10);
	
	TSoftObjectPtr<UMaterialInterface> MaterialInterface;
	
	FMaterialRelevance MaterialRelevance;

	/** One vertex factory per proxy */
	FZenoWaterMeshVertexFactory* VertexFactory = nullptr;
};

UZenoWaterMeshComponent::UZenoWaterMeshComponent(const FObjectInitializer& Initializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bAutoRegister	= true;
	bWantsInitializeComponent = true;
	bAutoActivate	= true;
}

FPrimitiveSceneProxy* UZenoWaterMeshComponent::CreateSceneProxy()
{
	return new FZenoWaterMeshSceneProxy(this, FName("ZenoWaterMesh"));
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
	FVector Extent(GridSize.X, GridSize.Y, WaterDepth);
	Extent *= BoundsScale;
	return FBoxSphereBounds(FVector::Zero(), Extent, Extent.GetMax()).TransformBy(LocalToWorld);
#else // For debug
    return FBoxSphereBounds(LocalToWorld.GetLocation(), FVector(1000000.0f, 1000000.0f, 1000000.0f), 1000000.0f);
#endif
}


IMPLEMENT_TYPE_LAYOUT(FZenoWaterMeshVertexFactoryShaderParameters);

IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FZenoWaterMeshVertexFactory, SF_Vertex, FZenoWaterMeshVertexFactoryShaderParameters);

IMPLEMENT_VERTEX_FACTORY_TYPE(
	FZenoWaterMeshVertexFactory,
	"/Plugin/ZenoMesh/Private/WaterVertexFactory.ush",
	EVertexFactoryFlags::UsedWithMaterials
	// | EVertexFactoryFlags::SupportsPositionOnly
	// | EVertexFactoryFlags::SupportsCachingMeshDrawCommands
	// | EVertexFactoryFlags::SupportsDynamicLighting
	// | EVertexFactoryFlags::SupportsStaticLighting
);

