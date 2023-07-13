#include "ZenoWaterMeshComponent.h"

#include "DataDrivenShaderPlatformInfo.h"
#include "MaterialDomain.h"
#include "MeshDrawShaderBindings.h"
#include "MeshMaterialShader.h"
#include "ZenoMeshBuffer.h"
#include "Materials/MaterialRenderProxy.h"

class FZenoWaterMeshVertexFactory : public FLocalVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FZenoWaterMeshVertexFactory);
	
public:
	using Super = FLocalVertexFactory;
	
	FZenoWaterMeshVertexFactory(const FIntPoint& InQuads, const FIntPoint& InPrecision, ERHIFeatureLevel::Type InFeatureLevel, const char* InDebugName = "WaterMeshVertexFactory")
		: FLocalVertexFactory(InFeatureLevel, InDebugName)
		, QuadSize(InQuads)
		, Precision(InPrecision)
	{
		VertexBuffer = new FZenoMeshVertexBuffer(1, 0, true, BufferAllocator);
		IndexBuffer = new FZenoMeshIndexBuffer(BufferAllocator);
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
				IndexBuffer->Indices.Emplace(Index + 1);
				IndexBuffer->Indices.Emplace(Index + VertexNum.X);
				IndexBuffer->Indices.Emplace(Index + 1);
				IndexBuffer->Indices.Emplace(Index + VertexNum.X + 1);
				IndexBuffer->Indices.Emplace(Index + VertexNum.X);
			}
		}
		
		BeginInitResource(VertexBuffer);
		BeginInitResource(IndexBuffer);

		ENQUEUE_RENDER_COMMAND(InitZenoWaterMeshVertexFactory)(
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
		
		Super::InitResource();
	}

	virtual void ReleaseResource() override
	{
		Super::ReleaseResource();
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

	virtual bool SupportsPositionOnlyStream() const override
	{
		return RHISupportsManualVertexFetch(Get)
	}

	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& InParameters)
	{
		return InParameters.MaterialParameters.MaterialDomain != MD_UI;
	}

	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& InParameters,
	                                         FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("ZENO_WATER_MESH"), TEXT("1"));
	}

protected:
	FIntPoint QuadSize;
	FIntPoint Precision;
	
	FZenoMeshVertexBuffer* VertexBuffer = nullptr;
	FZenoMeshIndexBuffer* IndexBuffer = nullptr;
	FZenoMeshBufferAllocator BufferAllocator;

	friend class FZenoWaterMeshSceneProxy;
};

class FZenoWaterMeshVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
	DECLARE_TYPE_LAYOUT(FZenoWaterMeshVertexFactoryShaderParameters, NonVirtual);

public:
	void Bind(const FShaderParameterMap& ParameterMap) {}
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
	}
private:
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
		VertexFactory = new FZenoWaterMeshVertexFactory(QuadSize, Precision, GetScene().GetFeatureLevel(), TCHAR_TO_ANSI(*WaterComponent->GetName()));
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
		// TODO [darc] : memory footprint :
		return 0;
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
														: UMaterial::GetDefaultMaterial(MD_Surface)->GetRenderProxy();

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];
				
				FMeshBatch& MeshBatch = Collector.AllocateMesh();
				{
					MeshBatch.VertexFactory = VertexFactory;
					MeshBatch.MaterialRenderProxy = MaterialProxy;

					MeshBatch.CastShadow = IsShadowCast(View);
					MeshBatch.bWireframe = bWireframe;
					MeshBatch.bCanApplyViewModeOverrides = true;
					MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();
					MeshBatch.bDisableBackfaceCulling = true;
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
		// MaterialRelevance.SetPrimitiveViewRelevance(Result);
		Result.bVelocityRelevance = DrawsVelocity() && Result.bOpaque && Result.bRenderInMainPass;
		return Result;
	}

protected:

	/** How many quad into rendering */
	FIntPoint QuadSize = FIntPoint(10, 10);
	
	FIntPoint Precision = FIntPoint(10, 10);

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

FBoxSphereBounds UZenoWaterMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
#if 0
	const FVector Extent(GridSize.X, GridSize.Y, 128.0);
	return FBoxSphereBounds(LocalToWorld.GetLocation(), Extent, Extent.GetMax());
#else // For debug
	return FBoxSphereBounds(LocalToWorld.GetLocation(), FVector(100000.0f), 100000.0f);
#endif
}


IMPLEMENT_TYPE_LAYOUT(FZenoWaterMeshVertexFactoryShaderParameters);

IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FZenoWaterMeshVertexFactory, SF_Vertex, FZenoWaterMeshVertexFactoryShaderParameters);

IMPLEMENT_VERTEX_FACTORY_TYPE(
	FZenoWaterMeshVertexFactory,
	"/Plugin/ZenoMesh/Private/WaterVertexFactory.ush",
	EVertexFactoryFlags::UsedWithMaterials
	| EVertexFactoryFlags::SupportsPositionOnly
	| EVertexFactoryFlags::SupportsCachingMeshDrawCommands
	| EVertexFactoryFlags::SupportsDynamicLighting
	| EVertexFactoryFlags::SupportsStaticLighting
);

