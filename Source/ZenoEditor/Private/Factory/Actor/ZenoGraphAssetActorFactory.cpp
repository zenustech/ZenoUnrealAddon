// Fill out your copyright notice in the Description page of Project Settings.


#include "Factory/Actor/ZenoGraphAssetActorFactory.h"

AActor* UZenoGraphAssetActorFactory::SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform,
	const FActorSpawnParameters& InSpawnParams)
{
	return Super::SpawnActor(InAsset, InLevel, InTransform, InSpawnParams);
}

void UZenoGraphAssetActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);
}

void UZenoGraphAssetActorFactory::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	Super::PostCreateBlueprint(Asset, CDO);
}

bool UZenoGraphAssetActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	return Super::CanCreateActorFrom(AssetData, OutErrorMsg);
}
