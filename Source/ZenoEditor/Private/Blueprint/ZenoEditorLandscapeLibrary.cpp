// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprint/ZenoEditorLandscapeLibrary.h"

#include "Landscape.h"
#include "LandscapeImportHelper.h"
#include "LandscapeProxy.h"
#include "LandscapeStreamingProxy.h"
#include "LiveLinkTypes.h"
#include "ActorPartition/ActorPartitionSubsystem.h"
#include "Client/ZenoLiveLinkClientSubsystem.h"

void UZenoEditorLandscapeLibrary::ExpandHeightmapData(const TArray<uint16>& InHeightMap,
                                                      const FIntVector2& InTargetResolution, TArray<uint16>& OutHeightMap)
{
	check(InHeightMap != OutHeightMap);

	const int32 Size = sqrt(InHeightMap.Num());
	const FLandscapeImportResolution ImportResolution(Size, Size);
	const FLandscapeImportResolution TargetResolution(InTargetResolution.X, InTargetResolution.Y);

	FLandscapeImportHelper::TransformHeightmapImportData(InHeightMap, OutHeightMap, ImportResolution,
		TargetResolution,
		ELandscapeImportTransformType::Resample);
}

TSharedPromise<TTuple<int32, int32, FIntPoint>> UZenoEditorLandscapeLibrary::ChooseBestComponentSizeForSubject(const FLiveLinkSubjectKey& Key, const int32 InQuadsPerSection = 1, const int32 InSectionsPerComponent = 1)
{
	using FRequiredType = zeno::remote::HeightField;
	using FRetType = TTuple<int32, int32, FIntPoint>;
	TSharedPromise<FRetType> Promise = UZenoHttpClient::CreateNewPromise<FRetType>();
	
	UZenoLiveLinkClientSubsystem* Subsystem = GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>();
	if (IsValid(Subsystem))
	{
		Subsystem->TryLoadSubjectRemotely<FRequiredType>(Key.SubjectName)->GetFuture().Then(
		[Promise, InQPS = InQuadsPerSection, InSPC = InSectionsPerComponent] (const TResultFuture<FRequiredType>& Result) mutable
		{
			if (Result.IsReady() && Result.Get().IsSet())
			{
				const zeno::remote::HeightField HeightFieldData = Result.Get().GetValue();
				FIntPoint OutComponentCount;
				FLandscapeImportHelper::ChooseBestComponentSizeForImport(HeightFieldData.Nx, HeightFieldData.Ny, InQPS, InSPC, OutComponentCount);
				Promise->SetValue( TOptional<FRetType> { { InQPS, InSPC, OutComponentCount } });
				return;
			}
			Promise->EmplaceValue(); // Empty optional
		});
	}
	return Promise;
}

void UZenoEditorLandscapeLibrary::MakeLandscapeActorStreamable(ALandscape* LandscapeTarget)
{
	check(LandscapeTarget);
	ULandscapeInfo* LandscapeInfo = LandscapeTarget->GetLandscapeInfo();
	check(LandscapeInfo);
	
	LandscapeTarget->Modify();

	LandscapeTarget->InitializeProxyLayersWeightmapUsage();
	LandscapeTarget->bIncludeGridSizeInNameForLandscapeActors = true;

	FIntRect Extent;
	LandscapeInfo->GetLandscapeExtent(Extent.Min.X, Extent.Min.Y, Extent.Max.X, Extent.Max.Y);

	const UWorld* World = LandscapeTarget->GetWorld();
	check(World);

	UActorPartitionSubsystem* ActorPartitionSubsystem = World->GetSubsystem<UActorPartitionSubsystem>();
	check(ActorPartitionSubsystem);

	TArray<ULandscapeComponent*> LandscapeComponents;
	LandscapeComponents.Reserve(LandscapeInfo->XYtoComponentMap.Num());
	LandscapeInfo->ForAllLandscapeComponents([&LandscapeComponents](ULandscapeComponent* Component)
	{
		LandscapeComponents.Add(Component);
	});

	TSet<ALandscapeProxy*> ProxiesToDelete;
	FActorPartitionGridHelper::ForEachIntersectingCell(ALandscapeStreamingProxy::StaticClass(), Extent, World->PersistentLevel, [ActorPartitionSubsystem, LandscapeInfo, &LandscapeComponents, &ProxiesToDelete] (const UActorPartitionSubsystem::FCellCoord& CellCoord, const FIntRect& CellBounds)
	{
		
		TMap<ULandscapeComponent*, UMaterialInterface*> ComponentMaterials;
		TMap<ULandscapeComponent*, UMaterialInterface*> ComponentHoleMaterials;
		TMap <ULandscapeComponent*, TMap<int32, UMaterialInterface*>> ComponentLODMaterials;

		TArray<ULandscapeComponent*> ComponentsToMove;
		for (int32 i = 0; i < LandscapeComponents.Num();)
		{
			ULandscapeComponent* LandscapeComponent = LandscapeComponents[i];
			if (CellBounds.Contains(LandscapeComponent->GetSectionBase()))
			{
				ComponentMaterials.FindOrAdd(LandscapeComponent, LandscapeComponent->GetLandscapeMaterial());
				ComponentHoleMaterials.FindOrAdd(LandscapeComponent, LandscapeComponent->GetLandscapeHoleMaterial());
				TMap<int32, UMaterialInterface*>& LODMaterials = ComponentLODMaterials.FindOrAdd(LandscapeComponent);
				for (int32 LODIndex = 0; LODIndex <= 8; ++LODIndex)
				{
					LODMaterials.Add(LODIndex, LandscapeComponent->GetLandscapeMaterial(static_cast<int8>(LODIndex)));
				}

				ComponentsToMove.Add(LandscapeComponent);
				LandscapeComponents.RemoveAtSwap(i);
				ProxiesToDelete.Add(LandscapeComponent->GetTypedOuter<ALandscapeProxy>());
			}
			else
			{
				i++;
			}
		}

		if (ComponentsToMove.Num()) {}
		return true;
	});
}
