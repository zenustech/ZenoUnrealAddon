#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "Components/SplineMeshComponent.h"
#include "ZenoWaterMeshComponent.generated.h"

class UZenoSplineComponent;
class FZenoMeshBufferAllocator;
class FZenoMeshIndexBuffer;
class FZenoMeshVertexBuffer;

struct FZenoWaterMeshRenderData
{
	FZenoWaterMeshRenderData(bool bInKeepCPUData = true);
	virtual ~FZenoWaterMeshRenderData();
	/** Upload data to gpu in any thread, not const at all */
	void UploadData_AnyThread() const;
	
	FZenoMeshBufferAllocator* BufferAllocator = nullptr;
	FZenoMeshVertexBuffer* VertexBuffer = nullptr;
	FZenoMeshIndexBuffer* IndexBuffer = nullptr;

	/** Render proxy of the material, nullable */
	FMaterialRenderProxy* MaterialProxy = nullptr;

	uint16 bKeepCPUData: 1;
};

USTRUCT(BlueprintType)
struct FZenoRiverBuildInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = 1, ClampMin = 1))
	int32 RiverWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UZenoSplineComponent> Spline;
};

UCLASS(ClassGroup=(Zeno), meta=(BlueprintSpawnableComponent), DefaultToInstanced)
class ZENOMESH_API UZenoWaterMeshComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UZenoWaterMeshComponent(const FObjectInitializer& Initializer);

	virtual ~UZenoWaterMeshComponent() override;

	/** Building river mesh with provided info */
	UFUNCTION(BlueprintCallable)
	virtual void BuildRiverMesh(const FZenoRiverBuildInfo& InBuildInfo);
	
protected:
	/** Water Material */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", DisplayName = "Material")
	UMaterialInterface* WaterMaterial = nullptr;

	// TODO [darc] : try to use mesh shader if supported :
	/** Precision of the mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", DisplayName = "Precision", meta = (UIMin = 1, ClampMin = 1))
	int32 Precision = 1;

	/** Should keeping uploaded buffers in cpu? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", DisplayName = "Keep Buffer In CPU")
	bool bKeepBufferInCPU = false;

	//////////////////////////////////////////////
	// Rendering Resources
	// We want to build the rendering resources manually
	//////////////////////////////////////////////
	/** Render Data, Sharable */
	TSharedPtr<FZenoWaterMeshRenderData> RenderData = nullptr;

protected:
	/** Return correct boundaries to avoid been culling */
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	
	virtual void CreateRenderState_Concurrent(FRegisterComponentContext* Context) override;
	
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const override;
	
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	
	friend class FZenoWaterMeshSceneProxy;
};
