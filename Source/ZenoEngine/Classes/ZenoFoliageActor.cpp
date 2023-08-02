// Zenustech © All Right Reserved


#include "ZenoFoliageActor.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"


AZenoFoliageActor::AZenoFoliageActor(const FObjectInitializer& InInitializer)
{
	FoliageMeshComponent = Cast<UHierarchicalInstancedStaticMeshComponent>(InInitializer.CreateDefaultSubobject(this, TEXT("FoliageMeshComponent"), UHierarchicalInstancedStaticMeshComponent::StaticClass(), UHierarchicalInstancedStaticMeshComponent::StaticClass()));
}

void AZenoFoliageActor::BeginPlay()
{
	Super::BeginPlay();
	
}

