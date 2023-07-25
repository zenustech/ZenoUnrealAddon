#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZenoRiverActor.generated.h"

class UZenoSplineComponent;
class UZenoWaterMeshComponent;

UCLASS()
class ZENOMESH_API AZenoRiverActor : public AActor
{
	GENERATED_BODY()

public:
	AZenoRiverActor(const FObjectInitializer& InObjectInitializer);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void OnSplineUpdated();

protected:
	/** Water mesh to rendering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Water Mesh")
	UZenoWaterMeshComponent* WaterMeshComponent = nullptr;

	/** Spline to control the flow dir */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Spline Component")
	UZenoSplineComponent* SplineComponent = nullptr;

	/** River width */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "River Width")
	int32 RiverWidth = 10;

	friend class UZenoWaterMeshComponent;
};
