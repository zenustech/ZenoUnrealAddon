#pragma once

class ZENOENGINE_API FZenoBasicVertexFactory final : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FZenoBasicVertexFactory)
public:
	FZenoBasicVertexFactory(ERHIFeatureLevel::Type InFeatureLevel, const char* InDebugName);

	/**
	 * Should we cache the material's shadertype on this platform with this vertex factory? 
	 */
	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters);
	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);
	static void GetPSOPrecacheVertexFetchElements(EVertexInputStreamType VertexInputStreamType, FVertexDeclarationElementList& Elements);

	virtual void InitResource() override;
	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;

private:

	bool bIsInitialized = false;
	
	struct FDebugName
	{
		FDebugName(const char* InDebugName)
#if !UE_BUILD_SHIPPING
			: DebugName(InDebugName)
#endif
		{}
	private:
#if !UE_BUILD_SHIPPING
		const char* DebugName;
#endif
	} DebugName;
};
