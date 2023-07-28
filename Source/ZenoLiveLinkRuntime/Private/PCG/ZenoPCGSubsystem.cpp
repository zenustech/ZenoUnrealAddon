// Fill out your copyright notice in the Description page of Project Settings.


#include "PCG/ZenoPCGSubsystem.h"

#include "PCG/Grid/ZenoPCGLandscapeCache.h"

UZenoPCGSubsystem::UZenoPCGSubsystem(const FObjectInitializer& ObjectInitializer)
{
	LandscapeCacheObject = ObjectInitializer.CreateDefaultSubobject<UZenoPCGLandscapeCache>(this, TEXT("LandscapeCache"));
}

UZenoPCGSubsystem* UZenoPCGSubsystem::GetInstance(const UWorld* World)
{
	if (World)
	{
		UZenoPCGSubsystem* Subsystem = World->GetSubsystem<UZenoPCGSubsystem>();
		return (Subsystem && Subsystem->IsInitialized()) ? Subsystem : nullptr;
	}
	return nullptr;
}

TStatId UZenoPCGSubsystem::GetStatId() const
{
	return TStatId();
}

#if WITH_EDITOR
void UZenoPCGSubsystem::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	Super::BeginCacheForCookedPlatformData(TargetPlatform);
	LandscapeCacheObject->AdvanceCache();
}
#endif // WITH_EDITOR
