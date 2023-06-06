// Fill out your copyright notice in the Description page of Project Settings.


#include "ZenoVATMeshComponent.h"

#include "MaterialDomain.h"
#include "MeshDrawShaderBindings.h"
#include "MeshMaterialShader.h"
#include "Materials/MaterialRenderProxy.h"
#include "UObject/SoftObjectPtr.h"


////////////////////////////////////////////////////////////////////////////
/// Uniform Shader Parameters of ZenoVATMeshComponent
///////////////////////////////////////////////////////////////////////////
BEGIN_SHADER_PARAMETER_STRUCT(FZenoVatMeshVertexFactoryUniformShaderParameters, ZENOMESH_API)
	SHADER_PARAMETER(uint32, VertexPositionTextureHeight)
	SHADER_PARAMETER(uint32, VertexAnimationFrameNumber)
	SHADER_PARAMETER(FVector3f, PositionBoundsMin)
	SHADER_PARAMETER(FVector3f, PositionBoundsMax)
END_SHADER_PARAMETER_STRUCT()

////////////////////////////////////////////////////////////////////////////
/// VertexFactory of ZenoVATMeshComponent
///////////////////////////////////////////////////////////////////////////
struct FZenoVatMeshVertexFactory : FLocalVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FZenoVatMeshVertexFactory);
public:
	FZenoVatMeshVertexFactory(ERHIFeatureLevel::Type InFeatureLevel)
		: FLocalVertexFactory(InFeatureLevel, "FZenoVatMeshVertexFactory")
		, OuterSceneProxy(nullptr)
	{
	}

	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters)
	{
		return (Parameters.MaterialParameters.MaterialDomain == EMaterialDomain::MD_Surface && Parameters.MaterialParameters.ShadingModels == MSM_DefaultLit) || Parameters.MaterialParameters.bIsDefaultMaterial;
	}

	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		const bool ContainsManualVertexFetch = OutEnvironment.GetDefinitions().Contains("MANUAL_VERTEX_FETCH");
		if (!ContainsManualVertexFetch)
		{
			OutEnvironment.SetDefine(TEXT("MANUAL_VERTEX_FETCH"), TEXT("0"));
		}
		OutEnvironment.SetDefine(TEXT("ZENO_VAT_MESH"), TEXT("1"));
	}

	virtual void InitRHI() override
	{
		check(HasValidFeatureLevel());
		//The vertex declaration element lists (Nothing but an array of FVertexElement)
		FVertexDeclarationElementList Elements; //Used for the Default vertex stream
		FVertexDeclarationElementList PosOnlyElements; // Used for the PositionOnly vertex stream

		if (Data.PositionComponent.VertexBuffer != NULL)
		{
			//We add the position stream component to both element lists
			Elements.Add(AccessStreamComponent(Data.PositionComponent, 0));
			PosOnlyElements.Add(AccessStreamComponent(Data.PositionComponent, 0, EVertexInputStreamType::PositionOnly));
		}

		//Initialize the Position Only vertex declaration which will be used in the depth pass
		InitDeclaration(PosOnlyElements, EVertexInputStreamType::PositionOnly);

		//We add all the available texcoords to the default element list, that's all what we'll need for unlit shading
		if (Data.TextureCoordinates.Num())
		{
			const int32 BaseTexCoordAttribute = 4;
			for (int32 CoordinateIndex = 0; CoordinateIndex < Data.TextureCoordinates.Num(); CoordinateIndex++)
			{
				Elements.Add(AccessStreamComponent(
					Data.TextureCoordinates[CoordinateIndex],
					BaseTexCoordAttribute + CoordinateIndex
				));
			}

			for (int32 CoordinateIndex = Data.TextureCoordinates.Num(); CoordinateIndex < MAX_STATIC_TEXCOORDS / 2;
			     CoordinateIndex++)
			{
				Elements.Add(AccessStreamComponent(
					Data.TextureCoordinates[Data.TextureCoordinates.Num() - 1],
					BaseTexCoordAttribute + CoordinateIndex
				));
			}
		}

		check(Streams.Num() > 0);

		InitDeclaration(Elements);
		check(IsValidRef(GetDeclaration()));

		FZenoVatMeshVertexFactoryUniformShaderParameters MetadataParameters;
		MetadataParameters.VertexPositionTextureHeight = TextureHeight;
		MetadataParameters.VertexAnimationFrameNumber = FrameNum;
		MetadataParameters.PositionBoundsMin = Bounds[0];
		MetadataParameters.PositionBoundsMax = Bounds[1];
		FZenoVatMeshVertexFactoryUniformShaderParameters::CreateUniformBuffer(MetadataParameters, UniformBuffer_MultiFrame);
	}

	virtual void ReleaseRHI() override
	{
		MetadataUniformBuffer.SafeRelease();
	}

	void SetSceneProxy(class FZenoVatMeshSceneProxy* InSceneProxy)
	{
		OuterSceneProxy = InSceneProxy;
	}

public:
	TUniformBufferRef<FZenoVatMeshVertexFactoryUniformShaderParameters> MetadataUniformBuffer;
	
private:
	uint32 TextureHeight = 0;
	uint32 FrameNum = 0;
	float PlaySpeed = .0f;
	FVector3f Bounds[2] { FVector3f::Zero(), FVector3f::Zero() };
	
	class FZenoVatMeshSceneProxy* OuterSceneProxy;

	friend class FZenoVatMeshSceneProxy;
};

/* 
 * Helper function that initializes the vertex buffers of the vertex factory's Data member from the static mesh vertex buffers
 * We're using this so we can initialize only the data that we're interested in.
*/
static void InitVertexFactoryData(FZenoVatMeshVertexFactory* VertexFactory, FStaticMeshVertexBuffers* VertexBuffers)
{
	ENQUEUE_RENDER_COMMAND(StaticMeshVertexBuffersLegacyInit)(
		[VertexFactory, VertexBuffers](FRHICommandListImmediate& RHICmdList)
		{
			// Initialize or update the RHI vertex buffers
			InitOrUpdateResource(&VertexBuffers->PositionVertexBuffer);
			InitOrUpdateResource(&VertexBuffers->StaticMeshVertexBuffer);

			// Use the RHI vertex buffers to create the needed Vertex stream components in an FDataType instance, and then set it as the data of the vertex factory
			FLocalVertexFactory::FDataType Data;
			VertexBuffers->PositionVertexBuffer.BindPositionVertexBuffer(VertexFactory, Data);
			VertexBuffers->StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(VertexFactory, Data);
			VertexFactory->SetData(Data);

			// Initialize the vertex factory using the data that we just set, this will call the InitRHI() method that we implemented in out vertex factory
			InitOrUpdateResource(VertexFactory);
		});
}

////////////////////////////////////////////////////////////////////////////
/// SceneProxy of ZenoVATMeshComponent
///////////////////////////////////////////////////////////////////////////
class FZenoVatMeshSceneProxy final : public FPrimitiveSceneProxy
{
public:
	virtual SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	explicit  FZenoVatMeshSceneProxy(const UZenoVATMeshComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, MaterialRelevance(InComponent->GetMaterialRelevance(GetScene().GetFeatureLevel()))
		, TextureHeight(InComponent->TextureHeight)
		, FrameNum(InComponent->TotalFrame)
		, PlaySpeed(InComponent->PlaySpeed)
		, Bounds{ InComponent->MinBounds, InComponent->MaxBounds }
		, MaterialInterface(InComponent->GetMaterial(0))
		, VertexFactory(GetScene().GetFeatureLevel())
	{
		// TODO [darc] : fill data
		VertexFactory.SetSceneProxy(this);
		VertexFactory.TextureHeight = TextureHeight;
		VertexFactory.Bounds[0] = Bounds[0];
		VertexFactory.Bounds[1] = Bounds[1];
		VertexFactory.FrameNum = FrameNum;
		VertexFactory.PlaySpeed = PlaySpeed;
	}

	virtual ~FZenoVatMeshSceneProxy() override
	{
		// TODO [darc] : release data
		VertexFactory.ReleaseResource();
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;
		FColoredMaterialRenderProxy* WireframeMaterialInstance = nullptr;
		if (bWireframe)
		{
			WireframeMaterialInstance = new FColoredMaterialRenderProxy(
				GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr,
				FLinearColor(.0f, .5f, 1.f)
			);
			Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
		}

		const UMaterialInterface* MeshMaterial = IsValid(MaterialInterface.LoadSynchronous()) ? MaterialInterface.LoadSynchronous() : GEngine->DebugMeshMaterial;
		FMaterialRenderProxy* MaterialProxy = bWireframe ? WireframeMaterialInstance : MeshMaterial->GetRenderProxy();
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];
				// Allocate a new batch for this mesh
				FMeshBatch& Mesh = Collector.AllocateMesh();
				{
					Mesh.bWireframe = bWireframe;
					Mesh.VertexFactory = &VertexFactory;
					Mesh.MaterialRenderProxy = MaterialProxy;
					Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
					Mesh.Type = PT_TriangleList;
					Mesh.DepthPriorityGroup = SDPG_World;
					Mesh.bCanApplyViewModeOverrides = false;
				}
				FMeshBatchElement& BatchElement = Mesh.Elements[0];
				BatchElement.FirstIndex = 0;
			}
		}
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
		Result.bVelocityRelevance = IsMovable() && Result.bOpaque && Result.bRenderInMainPass;
		return Result;
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}
	
	virtual uint32 GetMemoryFootprint(void) const override
	{
		return(sizeof(*this) + GetAllocatedSize());
	}

	uint32 GetAllocatedSize(void) const
	{
		return(FPrimitiveSceneProxy::GetAllocatedSize());
	}

private:
	FMaterialRelevance MaterialRelevance;

	uint32 TextureHeight = 0;
	uint32 FrameNum = 0;
	float PlaySpeed = .0f;
	FVector3f Bounds[2] { FVector3f::Zero(), FVector3f::Zero() };

	TSoftObjectPtr<UMaterialInterface> MaterialInterface = nullptr;
	FZenoVatMeshVertexFactory VertexFactory;
	
	friend class UZenoVATMeshComponent;
	// friend class FZenoVatMeshVertexFactoryShaderParameters;
};

////////////////////////////////////////////////////////////////////////////
/// Shader Parameters of ZenoVATMeshComponent
///////////////////////////////////////////////////////////////////////////
class FZenoVatMeshVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
	DECLARE_TYPE_LAYOUT(FZenoVatMeshVertexFactoryShaderParameters, NonVirtual);
public:
	void Bind(const FShaderParameterMap& ParameterMap)
	{
		AnimationMetadata.Bind(ParameterMap, TEXT("AnimationMetadata"), SPF_Optional);
	}

	void GetElementShaderBindings(
		const class FSceneInterface* Scene,
		const FSceneView* View,
		const class FMeshMaterialShader* Shader,
		const EVertexInputStreamType InputStreamType,
		ERHIFeatureLevel::Type FeatureLevel,
		const FVertexFactory* VertexFactory,
		const FMeshBatchElement& BatchElement,
		class FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams
	) const
	{
		if (BatchElement.bUserDataIsColorVertexBuffer)
		{
			const auto* LocalVertexFactory = static_cast<const FLocalVertexFactory*>(VertexFactory);
			const FColorVertexBuffer* ColorVertexBuffer = (FColorVertexBuffer*)(BatchElement.UserData);
			check(ColorVertexBuffer);

			if (!LocalVertexFactory->SupportsManualVertexFetch(FeatureLevel))
			{
				LocalVertexFactory->GetColorOverrideStream(ColorVertexBuffer, VertexStreams);
			}
		}

		const FZenoVatMeshVertexFactory* VATVertexFactory = static_cast<const FZenoVatMeshVertexFactory*>(VertexFactory);
	}

private:
	LAYOUT_FIELD(TShaderUniformBufferParameter<FZenoVatMeshVertexFactoryUniformShaderParameters>, AnimationMetadata);
};

////////////////////////////////////////////////////////////////////////////
IMPLEMENT_TYPE_LAYOUT(FZenoVatMeshVertexFactoryShaderParameters);
////////////////////////////////////////////////////////////////////////////
IMPLEMENT_VERTEX_FACTORY_TYPE(FZenoVatMeshVertexFactory, "/Plugin/ZenoMesh/Private/VATVertexFactory.ush", EVertexFactoryFlags::SupportsPositionOnly | EVertexFactoryFlags::SupportsPSOPrecaching);
////////////////////////////////////////////////////////////////////////////
IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FZenoVatMeshVertexFactory, SF_Vertex, FZenoVatMeshVertexFactoryShaderParameters);
////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////
/// Implementation of UZenoVATMeshComponent
///////////////////////////////////////////////////////////////////////////
UZenoVATMeshComponent::UZenoVATMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

FPrimitiveSceneProxy* UZenoVATMeshComponent::CreateSceneProxy()
{
	if (!SceneProxy)
		return SceneProxy = new FZenoVatMeshSceneProxy(this);
	else
		return SceneProxy;
}

#if WITH_EDITOR
void UZenoVATMeshComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property != nullptr)
	{
		if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UZenoVATMeshComponent, PositionTexturePath))
		{
			if (const UTexture2D* PositionTexture =  PositionTexturePath.LoadSynchronous(); IsValid(PositionTexture))
			{
				TextureHeight = PositionTexture->GetImportedSize().Y;
			}
		}
	}
}
#endif // WITH_EDITOR
