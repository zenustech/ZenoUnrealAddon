// Zenustech © All Right Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZenoFoliageActor.generated.h"

UCLASS()
class ZENOMESH_API AZenoFoliageActor : public AActor
{
	GENERATED_BODY()

public:
	AZenoFoliageActor(const FObjectInitializer& InInitializer);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Foliage Mesh")
	UInstancedStaticMeshComponent* FoliageMeshComponent = nullptr;
};
