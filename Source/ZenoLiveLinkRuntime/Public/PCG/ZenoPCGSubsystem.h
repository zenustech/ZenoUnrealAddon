// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ZenoPCGSubsystem.generated.h"

class UZenoPCGLandscapeCache;
class UWorld;

UCLASS()
class ZENOLIVELINKRUNTIME_API UZenoPCGSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	UZenoPCGSubsystem(const FObjectInitializer& ObjectInitializer);
	
	/** Will return the subsystem from the World if it exists and if it is initialized */
	static UZenoPCGSubsystem* GetInstance(const UWorld* World);

	/** Contains all the PCG data required to query the landscape complete. Serialized in cooked builds only */
	UPROPERTY(VisibleAnywhere, Category = CachedData, meta = (DisplayName="Landscape Cache"))
	TObjectPtr<UZenoPCGLandscapeCache> LandscapeCacheObject = nullptr;
	
	FORCEINLINE UZenoPCGLandscapeCache* GetLandscapeCache() const { return LandscapeCacheObject.Get(); }

	virtual TStatId GetStatId() const override;
	
	//~Begin UObject Interface
#if WITH_EDITOR
	virtual void BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform) override;
#endif // WITH_EDITOR
	//~End UObject Interface
};
