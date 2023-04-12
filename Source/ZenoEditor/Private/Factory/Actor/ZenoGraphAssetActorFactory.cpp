#include "Factory/Actor/ZenoGraphAssetActorFactory.h"
#include "ZenoGraphActor.h"
#include "ZenoGraphAsset.h"

#define LOCTEXT_NAMESPACE "UZenoGraphAssetActorFactory"

UZenoGraphAssetActorFactory::UZenoGraphAssetActorFactory(const FObjectInitializer& Initializer)
{
	DisplayName = LOCTEXT("ZenoGraphAssetActorName", "Zeno Graph");
	NewActorClass = AZenoGraphMeshActor::StaticClass();
}

AActor* UZenoGraphAssetActorFactory::SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform,
                                                const FActorSpawnParameters& InSpawnParams)
{
	const UZenoGraphAsset* Asset = Cast<UZenoGraphAsset>(InAsset);
	if (nullptr == Asset)
	{
		return nullptr;
	}
	AZenoGraphMeshActor* NewActor = Cast<AZenoGraphMeshActor>(Super::SpawnActor(InAsset, InLevel, InTransform, InSpawnParams));
	NewActor->ZenoGraphAsset = DuplicateObject(Asset, NewActor);

	return NewActor;
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
	return UZenoGraphAsset::StaticClass() == AssetData.GetClass(EResolveClass::Yes);
}

#undef LOCTEXT_NAMESPACE
