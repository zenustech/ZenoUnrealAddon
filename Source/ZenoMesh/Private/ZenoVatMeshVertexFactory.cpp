#include "ZenoVatMeshVertexFactory.h"

#include "MeshDrawShaderBindings.h"

////////////////////////////////////////////////////////////////////////
/// FZenoVatMeshVertexFactoryShaderParameters
////////////////////////////////////////////////////////////////////////

void FZenoVatMeshVertexFactoryShaderParameters::Bind(const FShaderParameterMap& ParameterMap)
{
	PositionBuffer.Bind(ParameterMap, TEXT("PositionBuffer"));
}

void FZenoVatMeshVertexFactoryShaderParameters::GetElementShaderBindings(const FSceneInterface* Scene,
	const FSceneView* View, const FMeshMaterialShader* Shader, const EVertexInputStreamType InputStreamType,
	ERHIFeatureLevel::Type FeatureLevel, const FVertexFactory* VertexFactory, const FMeshBatchElement& BatchElement,
	FMeshDrawSingleShaderBindings& ShaderBindings, FVertexInputStreamArray& VertexStreams) const
{
	const FZenoVatMeshVertexFactory* ZenoVatMeshVertexFactory = static_cast<const FZenoVatMeshVertexFactory*>(VertexFactory);

	ShaderBindings.Add(PositionBuffer, ZenoVatMeshVertexFactory->GetPositionBuffer().GetSRV());
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
	return InParameters.MaterialParameters.MaterialDomain != MD_UI;
}

void FZenoVatMeshVertexFactory::ModifyCompilationEnvironment(
	const FVertexFactoryShaderPermutationParameters& InParameters, FShaderCompilerEnvironment& OutEnvironment)
{
	if (const bool ContainsManualVertexFetch = OutEnvironment.GetDefinitions().Contains("MANUAL_VERTEX_FETCH"); !ContainsManualVertexFetch)
	{
		OutEnvironment.SetDefine(TEXT("MANUAL_VERTEX_FETCH"), TEXT("0"));
	}

	OutEnvironment.SetDefine(TEXT("CLOTH_MESH"), TEXT("1"));
}

void FZenoVatMeshVertexFactory::SetSceneProxy(FZenoVatMeshSceneProxy* InSceneProxy)
{
	SceneProxy = InSceneProxy;
}

void FZenoVatMeshVertexFactory::InitRHI()
{
	FLocalVertexFactory::InitRHI();

	PositionBuffer.Init(0);
	ENQUEUE_RENDER_COMMAND(FZenoVatMeshVertexFactoryInitRHI) (
		[this] (FRHICommandListImmediate& CmdList)
		{
			InitVertexResources_RenderThread(CmdList);
		}
	);
}

void FZenoVatMeshVertexFactory::ReleaseRHI()
{
	PositionBuffer.ReleaseRHI();
	
	FLocalVertexFactory::ReleaseRHI();
}

void FZenoVatMeshVertexFactory::InitVertexResources_RenderThread(FRHICommandListImmediate& CmdList)
{
	PositionBuffer.InitRHI();
}

FPositionVertexBuffer& FZenoVatMeshVertexFactory::GetPositionBuffer() const
{
	return PositionBuffer;
}

IMPLEMENT_TYPE_LAYOUT(FZenoVatMeshVertexFactoryShaderParameters);

IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FZenoVatMeshVertexFactory, SF_Vertex, FZenoVatMeshVertexFactoryShaderParameters);

IMPLEMENT_VERTEX_FACTORY_TYPE(FZenoVatMeshVertexFactory, "/Engine/Private/LocalVertexFactory.ush",
                              EVertexFactoryFlags::SupportsPositionOnly);
