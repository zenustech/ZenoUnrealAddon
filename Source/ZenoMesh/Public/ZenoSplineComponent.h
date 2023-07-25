// Zenustech © All Right Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "ZenoSplineComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ZENOMESH_API UZenoSplineComponent : public USplineComponent
{
	GENERATED_BODY()

public:
	UZenoSplineComponent();

protected:
	virtual void BeginPlay() override;

public:
	DECLARE_MULTICAST_DELEGATE(FSplineSimpleEvent);

	FSplineSimpleEvent OnSplineUpdated;
	
	virtual void UpdateSpline() override;
};
