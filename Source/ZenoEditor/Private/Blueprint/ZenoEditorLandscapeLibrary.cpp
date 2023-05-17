// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprint/ZenoEditorLandscapeLibrary.h"

#include "LandscapeImportHelper.h"
#include "LiveLinkTypes.h"
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
