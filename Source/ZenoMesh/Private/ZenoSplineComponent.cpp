// Zenustech © All Right Reserved


#include "ZenoSplineComponent.h"


UZenoSplineComponent::UZenoSplineComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UZenoSplineComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UZenoSplineComponent::UpdateSpline()
{
	Super::UpdateSpline();
	OnSplineUpdated.Broadcast();
}
