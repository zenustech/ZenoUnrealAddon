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

UZenoLandscapeActorFactory::UZenoLandscapeActorFactory(const FObjectInitializer& Initializer)
{
	DisplayName = LOCTEXT("Name", "Zeno Landscape");
	NewActorClass = AZenoLandscapeBundleActor::StaticClass();
}

// AActor* UZenoLandscapeActorFactory::SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform,
// 	const FActorSpawnParameters& InSpawnParams)
// {
// 	const UZenoAssetBundle* Asset = Cast<UZenoAssetBundle>(InAsset);
// 	if (!IsValid(Asset)) { return nullptr; }
//
// 	if (Asset->Landscapes.IsEmpty()) { return nullptr; }
// 	
// 	AZenoLandscapeBundleActor* NewActor = Cast<AZenoLandscapeBundleActor>(Super::SpawnActor(InAsset, InLevel, InTransform, InSpawnParams));
//
// 	NewActor->SetActorScale3D({ 128.f, 128.f, 256.f });
//
// 	return NewActor;
// }

void UZenoLandscapeActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	// UE using this function to generate a preview actor, so we need to check if the actor is transient
	if (NewActor->HasAllFlags(RF_Transient))
	{
		return;
	}

	const FName FolderPath = FName(FString::Printf(TEXT("/Game/Zeno/%s"), *FGuid::NewGuid().ToString()));
	
	UZenoAssetBundle* AssetBundle = Cast<UZenoAssetBundle>(Asset);
	AZenoLandscapeBundleActor* LandscapeActor = Cast<AZenoLandscapeBundleActor>(NewActor);
	if (IsValid(AssetBundle) && IsValid(LandscapeActor))
	{
		LandscapeActor->SetActorScale3D({ 128.f, 128.f, 256.f });
		ALandscapeProxy* LandscapeProxy = nullptr;
		for (const auto& Landscape : AssetBundle->Landscapes)
		{
			ALandscapeProxy* Actor = AddLandscape(LandscapeActor, Landscape);
			Actor->SetActorScale3D({ 128.f, 128.f, 256.f });
			// TODO [darc] : Generating the material :
			Actor->UpdateAllComponentMaterialInstances();
			LandscapeProxy = Actor;
			Actor->SetFolderPath(FolderPath);
		}

		// TODO [darc] : support multiple landscape :
		check(IsValid(LandscapeProxy));
		FVector LandscapeOrigin = FVector::Zero();
		FVector LandscapeExtent = FVector::Zero();
		LandscapeProxy->GetActorBounds(false, LandscapeOrigin, LandscapeExtent, true);
		const FVector LandscapePosition = LandscapeProxy->GetActorLocation();
		LandscapeOrigin.Z = LandscapePosition.Z;
		LandscapeActor->SetActorLocation(LandscapeOrigin);

		for (const auto& PointSet : AssetBundle->PointSet)
		{
			AActor* Actor = AddFoliage(LandscapeActor, PointSet);
		}
		
		NewActor->SetFolderPath_Recursively(FolderPath);

		FEditorDelegates::RefreshEditor.Broadcast();
	}
}

bool UZenoLandscapeActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	return AssetData.IsInstanceOf(UZenoAssetBundle::StaticClass());
}

ALandscapeProxy* UZenoLandscapeActorFactory::AddLandscape(AZenoLandscapeBundleActor* NewActor,
	const UZenoLandscapeAsset* InLandscapeData)
{
	check(IsValid(InLandscapeData));

	const FIntPoint Extent = InLandscapeData->Extent;
	int32 QuadsPerSection = 31;
	int32 SectionsPerComponent = 7;
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
	Landscape->SetActorScale3D(FVector {1.f, 1.f, 1.f});
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

	Landscape->AttachToActor(NewActor, FAttachmentTransformRules::SnapToTargetIncludingScale);
	
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
	FoliageActor->AttachToActor(NewActor, FAttachmentTransformRules::SnapToTargetIncludingScale);

	return FoliageActor;
}

#undef LOCTEXT_NAMESPACE
