// Fill out your copyright notice in the Description page of Project Settings.


#include "Landscape/ZenoPCGVolume.h"

#include "Builders/CubeBuilder.h"
#include "Components/BrushComponent.h"
#include "Engine/Polys.h"

AZenoPCGVolume::AZenoPCGVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PCGComponent = ObjectInitializer.CreateDefaultSubobject<UZenoPCGVolumeComponent>(this, TEXT("PCG Component"));

	PolyFlags = 0;

	const UObject* That = this;
	while (That && That->HasAnyFlags(RF_DefaultSubObject))
	{
		That = That->GetOuter();
	}

	if (That && !That->HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad | RF_NeedPostLoad))
	{
		if (UBrushComponent* MyBrushComponent = GetBrushComponent())
		{
			MyBrushComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
			MyBrushComponent->SetMobility(EComponentMobility::Static);
			MyBrushComponent->SetGenerateOverlapEvents(false);
		}
	}
	SetActorScale3D({10.0, 10.0, 10.0});
}

#if WITH_EDITOR
void AZenoPCGVolume::PostActorCreated()
{
	Super::PostActorCreated();
	BuildBrush();
}

void AZenoPCGVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	BuildBrush();
}
#endif // WITH_EDITOR

bool AZenoPCGVolume::GetReferencedContentObjects(TArray<UObject*>& Objects) const
{
	Super::GetReferencedContentObjects(Objects);

	if (PCGComponent)
	{
	}

	return true;
}

#if WITH_EDITOR
void AZenoPCGVolume::BuildBrush()
{
	Brush = NewObject<UModel>(this, NAME_None, RF_Transactional);
	Brush->Initialize(nullptr, true);
	Brush->Polys = NewObject<UPolys>(this, NAME_None, RF_Transactional);
	GetBrushComponent()->Brush = Brush;
	BrushBuilder = NewObject<UCubeBuilder>(this, NAME_None, RF_Transactional);

	UCubeBuilder* CubeBuilder = static_cast<UCubeBuilder*>(BrushBuilder);

	CubeBuilder->X = GetActorScale().X;
	CubeBuilder->Y = GetActorScale().Y;
	CubeBuilder->Z = GetActorScale().Z;

	BrushBuilder->Build(GetWorld(), this);
}
#endif // WITH_EDITOR
