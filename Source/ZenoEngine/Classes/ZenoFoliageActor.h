// Zenustech © All Right Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZenoFoliageActor.generated.h"

UCLASS()
class ZENOENGINE_API AZenoFoliageActor : public AActor
{
	GENERATED_BODY()

public:
	AZenoFoliageActor(const FObjectInitializer& InInitializer);

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Foliage Type")
	int32 TypeId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Foliage Mesh")
	UHierarchicalInstancedStaticMeshComponent* FoliageMeshComponent = nullptr;
};
