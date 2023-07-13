#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "ZenoWaterMeshComponent.generated.h"

UCLASS(ClassGroup=(Zeno), meta=(BlueprintSpawnableComponent))
class ZENOMESH_API UZenoWaterMeshComponent : public UMeshComponent
{
	GENERATED_BODY()

public:
	UZenoWaterMeshComponent(const FObjectInitializer& Initializer);

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

protected:
	/** Water plane size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", DisplayName = "Surface Grid Size")
	FIntPoint GridSize = FIntPoint(7, 7);

	/** Decide how many quads in a 1x1 grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", DisplayName = "Precision")
	FIntPoint Precision = FIntPoint(10, 10);

protected:
	/** Return correct boundaries to avoid been culling */
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	friend class FZenoWaterMeshSceneProxy;
};
