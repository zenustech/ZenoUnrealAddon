// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorFactories/ActorFactory.h"
#include "UObject/Object.h"
#include "ZenoGraphAssetActorFactory.generated.h"

/**
 * Allow Zeno Graph Asset to place on world
 */
UCLASS()
class ZENOEDITOR_API UZenoGraphAssetActorFactory : public UActorFactory
{
	GENERATED_BODY()

	explicit UZenoGraphAssetActorFactory(const FObjectInitializer& Initializer);

	//~ Begin UActorFactory interface
	virtual AActor* SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform, const FActorSpawnParameters& InSpawnParams) override;
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual void PostCreateBlueprint(UObject* Asset, AActor* CDO) override;
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	//~ End UActorFactory interface
};
