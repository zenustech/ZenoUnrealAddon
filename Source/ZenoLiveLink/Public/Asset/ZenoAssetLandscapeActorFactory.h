// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorFactories/ActorFactory.h"
#include "UObject/Object.h"
#include "ZenoAssetLandscapeActorFactory.generated.h"

class AActor;
struct FAssetData;

UCLASS(config = Editor)
class UZenoAssetLandscapeActorFactory : public UActorFactory
{
	GENERATED_BODY()

	UZenoAssetLandscapeActorFactory(const FObjectInitializer& ObjectInitializer);
	
	// UActorFactory interface
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual void PostCreateBlueprint(UObject* Asset, AActor* CDO) override;
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	// End of UActorFactory interface
};
