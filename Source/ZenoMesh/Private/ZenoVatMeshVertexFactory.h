#pragma once
#include "ZenoMeshBuffer.h"
struct FZenoMeshData;
class FZenoMeshIndexBuffer;
class FZenoMeshVertexBuffer;
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
	LAYOUT_FIELD(FShaderParameter, BoundsMin);
	LAYOUT_FIELD(FShaderParameter, BoundsMax);
	LAYOUT_FIELD(FShaderParameter, TotalFrame);
	LAYOUT_FIELD(FShaderParameter, TextureHeight);
	LAYOUT_FIELD(FShaderParameter, CurrentFrame);
	LAYOUT_FIELD(FShaderParameter, InstanceTransform);
	// LAYOUT_FIELD(FShaderParameter, PlaySpeed);
	// LAYOUT_FIELD(FShaderParameter, bAutoPlay);
	LAYOUT_FIELD(FShaderResourceParameter, PositionTexture);
	LAYOUT_FIELD(FShaderResourceParameter, PositionTextureSampler);
	LAYOUT_FIELD(FShaderResourceParameter, NormalTexture);
	LAYOUT_FIELD(FShaderResourceParameter, NormalTextureSampler);
};

class FZenoVatMeshVertexFactory final : public FLocalVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FZenoVatMeshVertexFactory);

public:
	FZenoVatMeshVertexFactory(const FZenoMeshData* InMesh, ERHIFeatureLevel::Type InFeatureLevel, const char* InDebugName);

	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& InParameters);

	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& InParameters,
	                                         FShaderCompilerEnvironment& OutEnvironment);

	FORCEINLINE void SetSceneProxy(FZenoVatMeshSceneProxy* InSceneProxy);
	
	virtual void InitResource() override;

	virtual void ReleaseResource() override;

	virtual bool SupportsPositionOnlyStream() const override;

	virtual bool SupportsPositionAndNormalOnlyStream() const override;

	FZenoMeshVertexBuffer* VertexBuffer = nullptr;
	FZenoMeshIndexBuffer* IndexBuffer = nullptr;
	
private:
	FZenoVatMeshSceneProxy* SceneProxy = nullptr;

	FZenoMeshBufferAllocator BufferAllocator;
};
