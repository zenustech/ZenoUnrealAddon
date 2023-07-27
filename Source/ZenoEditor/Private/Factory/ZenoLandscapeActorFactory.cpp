// Zenustech © All Right Reserved


#include "Factory/ZenoLandscapeActorFactory.h"

#include "Landscape.h"
#include "LandscapeImportHelper.h"
#include "LandscapeProxy.h"
#include "ZenoAssetBundle.h"
#include "ZenoLandscapeAsset.h"
#include "ZenoLandscapeBundleActor.h"
#include "ZenoPointSetAsset.h"
#include "Blueprint/ZenoEditorLandscapeLibrary.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ZenoFoliageActor.h"

#define LOCTEXT_NAMESPACE "ZenoLandscapeActorFactory"

UZenoLandscapeActorFactory::UZenoLandscapeActorFactory()
{
	DisplayName = LOCTEXT("Name", "Zeno Landscape");
	NewActorClass = AZenoLandscapeBundleActor::StaticClass();
}

AActor* UZenoLandscapeActorFactory::SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform,
	const FActorSpawnParameters& InSpawnParams)
{
	const UZenoAssetBundle* Asset = Cast<UZenoAssetBundle>(InAsset);
	if (!IsValid(Asset)) { return nullptr; }

	if (Asset->Landscapes.IsEmpty()) { return nullptr; }
	
	AZenoLandscapeBundleActor* NewActor = Cast<AZenoLandscapeBundleActor>(Super::SpawnActor(InAsset, InLevel, InTransform, InSpawnParams));
	
	return NewActor;
}

void UZenoLandscapeActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);
	UZenoAssetBundle* AssetBundle = Cast<UZenoAssetBundle>(Asset);
	AZenoLandscapeBundleActor* LandscapeActor = Cast<AZenoLandscapeBundleActor>(NewActor);
	if (IsValid(AssetBundle) && IsValid(LandscapeActor))
	{
		for (const auto& Landscape : AssetBundle->Landscapes)
		{
			AddLandscape(LandscapeActor, Landscape);
			// TODO [darc] : Generating the material :
		}

		for (const auto& PointSet : AssetBundle->PointSet)
		{
			AddFoliage(LandscapeActor, PointSet);
		}
	}
}

bool UZenoLandscapeActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	return Super::CanCreateActorFrom(AssetData, OutErrorMsg);
}

ALandscapeProxy* UZenoLandscapeActorFactory::AddLandscape(AZenoLandscapeBundleActor* NewActor,
	const UZenoLandscapeAsset* InLandscapeData)
{
	check(IsValid(InLandscapeData));

	const FIntPoint Extent = InLandscapeData->Extent;
	int32 QuadsPerSection;
	int32 SectionsPerComponent;
	FIntPoint ComponentCount;
	FLandscapeImportHelper::ChooseBestComponentSizeForImport(Extent.X, Extent.Y, QuadsPerSection, SectionsPerComponent,
	                                                         ComponentCount);
	// Gird count + 1 = Point count
	const int32 SizeX = ComponentCount.X * QuadsPerSection + 1;
	const int32 SizeY = ComponentCount.Y * QuadsPerSection + 1;

	// Scale the landscape to match the point size
	TArray<uint16> ExpandedHeightData;
	UZenoEditorLandscapeLibrary::ExpandHeightmapData(InLandscapeData->HeightData, {SizeX, SizeY}, ExpandedHeightData);

	TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
	TArray<FLandscapeImportLayerInfo> LandscapeImportLayerInfos;

	HeightDataPerLayers.Add({FGuid(), ExpandedHeightData});
	MaterialLayerDataPerLayers.Add({FGuid(), LandscapeImportLayerInfos});
	
	FScopedTransaction Transaction(LOCTEXT("Undo", "Creating New Landscape"));

	ALandscape* Landscape = NewActor->GetWorld()->SpawnActor<ALandscape>();
	Landscape->bCanHaveLayersContent = true;
	// Use uniform scale of the parent
	// Landscape->SetActorRelativeScale3D(FVector {50.f, 50.f, 50.f});
	Landscape->StaticLightingLOD = FMath::DivideAndRoundUp(
		 FMath::CeilLogTwo((SizeX * SizeY) / (2048 * 2048) + 1),
		 static_cast<uint32>(2));

	Landscape->Import(FGuid::NewGuid(), 0, 0, SizeX - 1, SizeY - 1, SectionsPerComponent, QuadsPerSection, HeightDataPerLayers,
	                  nullptr, MaterialLayerDataPerLayers, ELandscapeImportAlphamapType::Additive);

	ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
	check(LandscapeInfo);

	FActorLabelUtilities::SetActorLabelUnique(Landscape, LandscapeLabelName);
	FActorLabelUtilities::SetActorLabelUnique(Landscape, ALandscape::StaticClass()->GetName());

	LandscapeInfo->UpdateLayerInfoMap(Landscape);

	Landscape->AttachToActor(NewActor, FAttachmentTransformRules::KeepRelativeTransform);
	
	return Landscape;
}

AZenoFoliageActor* UZenoLandscapeActorFactory::AddFoliage(AZenoLandscapeBundleActor* NewActor, const UZenoPointSetAsset* InPointSetData)
{
	check(IsValid(InPointSetData));

	AZenoFoliageActor* FoliageActor = NewActor->GetWorld()->SpawnActor<AZenoFoliageActor>();
	FoliageActor->TypeId = static_cast<int32>(InPointSetData->PointType);

	TArray<FTransform> Transforms;
	Transforms.Reserve(InPointSetData->Points.Num());
	for (const FVector& Point : InPointSetData->Points)
	{
		Transforms.Add(FTransform {Point});
	}
	
	FoliageActor->FoliageMeshComponent->AddInstances(Transforms, false);

	NewActor->AddOwned(FoliageActor);
	FoliageActor->AttachToActor(NewActor, FAttachmentTransformRules::KeepRelativeTransform);

	return FoliageActor;
}

#undef LOCTEXT_NAMESPACE
