// Fill out your copyright notice in the Description page of Project Settings.


#include "Asset/ZenoAssetLandscapeActorFactory.h"

#include "Classes/ZenoBridgeAsset.h"
#include "Landscape/Classes/ZenoLandscapeAnchor.h"

#define LOCTEXT_NAMESPACE "ZenoAssetLandscapeActorFactory"

UZenoAssetLandscapeActorFactory::UZenoAssetLandscapeActorFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("LandscapeActorName", "Zeno Landscape");
	NewActorClass = AZenoLandscapeAnchor::StaticClass();
}

void UZenoAssetLandscapeActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);
}

void UZenoAssetLandscapeActorFactory::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	Super::PostCreateBlueprint(Asset, CDO);
}

bool UZenoAssetLandscapeActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (AssetData.IsValid())
	{
		const UClass* AssetClass = AssetData.GetClass();
		if (nullptr != AssetClass && AssetClass->IsChildOf(UZenoBridgeAsset::StaticClass()))
		{
			const UZenoBridgeAsset* Asset = Cast<UZenoBridgeAsset>(AssetData.GetAsset());
			if (nullptr != Asset && Asset->GetAssetType() == EZenoAssetType::Invalid)
			{
				return true;
			}
		}
	}
	
	return false;
}

#undef LOCTEXT_NAMESPACE
