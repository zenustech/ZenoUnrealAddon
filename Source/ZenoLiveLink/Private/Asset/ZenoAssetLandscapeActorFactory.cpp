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

AActor* UZenoAssetLandscapeActorFactory::SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform,
	const FActorSpawnParameters& InSpawnParams)
{
	AZenoLandscapeAnchor* NewActor = CastChecked<AZenoLandscapeAnchor>(Super::SpawnActor(InAsset, InLevel, InTransform, InSpawnParams));
	if (const UZenoBridgeAsset* BridgeAsset = Cast<UZenoBridgeAsset>(InAsset); IsValid(BridgeAsset))
	{
		if (AZenoLandscapeAnchor* LandscapeAnchor = Cast<AZenoLandscapeAnchor>(NewActor); IsValid(LandscapeAnchor))
		{
			if (UZenoBridgeAssetData_Heightfield* Data_Heightfield = Cast<UZenoBridgeAssetData_Heightfield>(BridgeAsset->GetAssetData()); IsValid(Data_Heightfield))
			{
				LandscapeAnchor->Heightfield = Data_Heightfield;
			}
		}
	}
	return NewActor;
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
			if (nullptr != Asset
				&& Asset->GetAssetType() == EZenoAssetType::HeightField
				&& IsValid(Cast<UZenoBridgeAssetData_Heightfield>(Asset->GetAssetData()))
				)
			{
				return true;
			}
		}
	}
	
	return false;
}

#undef LOCTEXT_NAMESPACE
