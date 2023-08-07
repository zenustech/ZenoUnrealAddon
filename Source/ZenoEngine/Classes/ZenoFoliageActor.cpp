// Zenustech © All Right Reserved


#include "ZenoFoliageActor.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"


AZenoFoliageActor::AZenoFoliageActor(const FObjectInitializer& InInitializer)
{
	FoliageMeshComponent = Cast<UInstancedStaticMeshComponent>(InInitializer.CreateDefaultSubobject(this, TEXT("FoliageMeshComponent"), UInstancedStaticMeshComponent::StaticClass(), UInstancedStaticMeshComponent::StaticClass()));
}

void AZenoFoliageActor::BeginPlay()
{
	Super::BeginPlay();

	FoliageMeshComponent->ClearInstances();
	FoliageMeshComponent->AddInstances(FoliageTransforms, false, false);
}

