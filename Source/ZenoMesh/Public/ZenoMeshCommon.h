#pragma once

#include "CoreMinimal.h"
#include "ZenoMeshBuffer.h"
#include "UObject/Object.h"
#include "ZenoMeshCommon.generated.h"


class FZenoMeshIndexBuffer;
class FZenoMeshVertexBuffer;

UCLASS()
class ZENOMESH_API UZenoMeshCommonLibrary : public UObject
{
	GENERATED_BODY()
};

class FZenoMeshVertexFactoryBase : public FLocalVertexFactory
{
public:
	using Super = FLocalVertexFactory;

	FZenoMeshVertexFactoryBase(ERHIFeatureLevel::Type InFeatureLevel, const char* InDebugName, FZenoMeshVertexBuffer* InVertexBuffer = nullptr, FZenoMeshIndexBuffer* InIndexBuffer = nullptr);
	virtual ~FZenoMeshVertexFactoryBase() override;

	virtual void InitResource() override;
	virtual void InitRHI() override;

	virtual void ReleaseRHI() override;
	virtual void ReleaseResource() override;

	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& InParameters);

	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& InParameters,
	                                         FShaderCompilerEnvironment& OutEnvironment);

protected:
	FZenoMeshVertexBuffer* VertexBuffer = nullptr;
	FZenoMeshIndexBuffer* IndexBuffer = nullptr;
	FZenoMeshBufferAllocator BufferAllocator;
};
