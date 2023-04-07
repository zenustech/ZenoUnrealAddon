#include "Render/ZenoBasicVertexFactory.h"

#include "MaterialDomain.h"
#include "MeshMaterialShader.h"

FZenoBasicVertexFactory::FZenoBasicVertexFactory(ERHIFeatureLevel::Type InFeatureLevel, const char* InDebugName)
	: FVertexFactory(InFeatureLevel)
	, DebugName(InDebugName)
{
}

bool FZenoBasicVertexFactory::ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters)
{
	return Parameters.MaterialParameters.MaterialDomain == EMaterialDomain::MD_Surface;
}

void FZenoBasicVertexFactory::ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters,
	FShaderCompilerEnvironment& OutEnvironment)
{
	OutEnvironment.SetDefine(TEXT("IS_ZENO_PASS"), TEXT("1"));
	OutEnvironment.SetDefine(TEXT("IS_ZENO_BASIC_PASS"), TEXT("1"));
}

/**
 * TODO [darc] : support EVertexFactoryFlags::SupportsPSOPrecaching :
 * This function will be called when set EVertexFactoryFlags::SupportsPSOPrecaching
 */
void FZenoBasicVertexFactory::GetPSOPrecacheVertexFetchElements(EVertexInputStreamType VertexInputStreamType,
	FVertexDeclarationElementList& Elements)
{
}

void FZenoBasicVertexFactory::InitResource()
{
	if (bIsInitialized)
		return;
	
	FVertexFactory::InitResource();

	bIsInitialized = true;
}

void FZenoBasicVertexFactory::InitRHI()
{
}

void FZenoBasicVertexFactory::ReleaseRHI()
{
	FVertexFactory::ReleaseRHI();
}

IMPLEMENT_VERTEX_FACTORY_TYPE(FZenoBasicVertexFactory, "/Plugin/ZenoEngine/Private/ZenoBasicVertexFactory.ush",
	EVertexFactoryFlags::UsedWithMaterials
	| EVertexFactoryFlags::SupportsDynamicLighting
);
