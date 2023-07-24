#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "ZenoWaterMeshComponent.generated.h"

UCLASS(ClassGroup=(Zeno), meta=(BlueprintSpawnableComponent))
class ZENOMESH_API UZenoWaterMeshComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UZenoWaterMeshComponent(const FObjectInitializer& Initializer);

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const override;

	virtual void CreateRenderState_Concurrent(FRegisterComponentContext* Context) override;

protected:
	/** Water plane size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", DisplayName = "Surface Grid Size")
	FIntPoint GridSize = FIntPoint(7, 7);

	/** Decide how many quads in a 1x1 grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", DisplayName = "Precision")
	FIntPoint Precision = FIntPoint(10, 10);

	/** Water Material */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", DisplayName = "Material")
	UMaterialInterface* WaterMaterial = nullptr;

	/** Depth of water */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", DisplayName = "Wave Depth")
	float WaterDepth = 1.0f;

protected:
	/** Return correct boundaries to avoid been culling */
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	
	friend class FZenoWaterMeshSceneProxy;
};
