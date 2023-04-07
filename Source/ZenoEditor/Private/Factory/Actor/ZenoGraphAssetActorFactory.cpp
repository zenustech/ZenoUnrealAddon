#include "Factory/Actor/ZenoGraphAssetActorFactory.h"

#define LOCTEXT_NAMESPACE "UZenoGraphAssetActorFactory"

UZenoGraphAssetActorFactory::UZenoGraphAssetActorFactory(const FObjectInitializer& Initializer)
{
	DisplayName = LOCTEXT("ZenoGraphAssetActorName", "Zeno Graph");
}

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

#undef LOCTEXT_NAMESPACE
