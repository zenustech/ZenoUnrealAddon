#pragma once
#include "MaterialDomain.h"
#include "MeshMaterialShader.h"

class FZenoVatMeshSceneProxy;

class FZenoVatMeshVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
	DECLARE_TYPE_LAYOUT(FZenoVatMeshVertexFactoryShaderParameters, NonVirtual);

public:
	void Bind(const FShaderParameterMap& ParameterMap);
	
	void GetElementShaderBindings(
    		const FSceneInterface* Scene,
    		const FSceneView* View,
    		const FMeshMaterialShader* Shader,
    		const EVertexInputStreamType InputStreamType,
    		ERHIFeatureLevel::Type FeatureLevel,
    		const FVertexFactory* VertexFactory,
    		const FMeshBatchElement& BatchElement,
    		FMeshDrawSingleShaderBindings& ShaderBindings,
    		FVertexInputStreamArray& VertexStreams) const;

private:
	LAYOUT_FIELD(FShaderResourceParameter, PositionBuffer);
};

struct FZenoVatMeshVertexFactory final : public FLocalVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FZenoVatMeshVertexFactory);

public:
	FZenoVatMeshVertexFactory(ERHIFeatureLevel::Type InFeatureLevel, const char* InDebugName);

	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& InParameters);

	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& InParameters,
	                                         FShaderCompilerEnvironment& OutEnvironment);

	FORCEINLINE void SetSceneProxy(FZenoVatMeshSceneProxy* InSceneProxy);
	
	virtual void InitRHI() override;

	virtual void ReleaseRHI() override;

	void InitVertexResources_RenderThread(FRHICommandListImmediate& CmdList);

	FPositionVertexBuffer& GetPositionBuffer() const;

private:
	FZenoVatMeshSceneProxy* SceneProxy = nullptr;

	mutable FPositionVertexBuffer PositionBuffer;
};
