#include "ZenoRiverActor.h"

#include "ZenoSplineComponent.h"
#include "ZenoWaterMeshComponent.h"
#include "Components/SplineComponent.h"


AZenoRiverActor::AZenoRiverActor(const FObjectInitializer& InObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false; // y the f**king rider template enable tick by default?

	SplineComponent = Cast<UZenoSplineComponent>(InObjectInitializer.CreateDefaultSubobject(this, TEXT("SplineComponent"), UZenoSplineComponent::StaticClass(), UZenoSplineComponent::StaticClass()));
	WaterMeshComponent = Cast<UZenoWaterMeshComponent>(InObjectInitializer.CreateDefaultSubobject(this, TEXT("WaterMeshComponent"), UZenoWaterMeshComponent::StaticClass(), UZenoWaterMeshComponent::StaticClass()));

	// Explicitly set the root component
	SetRootComponent(SplineComponent);
	WaterMeshComponent->SetupAttachment(SplineComponent);

	SplineComponent->OnSplineUpdated.AddUObject(this, &AZenoRiverActor::OnSplineUpdated);
}

void AZenoRiverActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AZenoRiverActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AZenoRiverActor::OnSplineUpdated()
{
	// TODO [darc] : Update water mesh :
	FZenoRiverBuildInfo BuildInfo;
	BuildInfo.Spline = SplineComponent;
	BuildInfo.RiverWidth = RiverWidth;
	BuildInfo.Precision = Precision;
	WaterMeshComponent->BuildRiverMesh(BuildInfo);
}

