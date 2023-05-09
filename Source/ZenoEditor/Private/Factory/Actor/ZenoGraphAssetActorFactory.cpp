#include "Factory/Actor/ZenoGraphAssetActorFactory.h"
#include "ZenoGraphActor.h"
#include "ZenoGraphAsset.h"
#include "Utilities/ZenoEngineTypes.h"

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
	NewActor->ZenoGraphAsset->ClearFlags(RF_Standalone | RF_Transient); // Asset object must be savable
	// Create input parameters
	TArray<UZenoInputParameter*>& InputParameters = NewActor->InputParameters;
	for (const FZenoInputParameterDescriptor& Descriptor : Asset->InputParameterDescriptors)
	{
		UZenoInputParameter* InputParameter = Descriptor.CreateInputParameter(NewActor);
		if (nullptr != InputParameter)
		{
			InputParameters.Add(InputParameter);
		}
	}

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
#ifdef UE_5_2_OR_LATER
	return UZenoGraphAsset::StaticClass() == AssetData.GetClass(EResolveClass::Yes);
#else
	return UZenoGraphAsset::StaticClass() == AssetData.GetClass();
#endif
}

#undef LOCTEXT_NAMESPACE
