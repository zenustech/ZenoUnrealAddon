// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Landscape/ZenoLandscapeSimpleBrush.h"

#include "Landscape.h"
#include "LandscapeModule.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UI/Landscape/ZenoLandscapeHelper.h"

DEFINE_LOG_CATEGORY(LogZenoLandscapeSimpleBrush);

UZenoSimpleBrushSettings::UZenoSimpleBrushSettings()
	: DefaultBrushHeightMapEncoderMaterial(FSoftObjectPath(TEXT("/ZenoBridge/Material/Mat_HeightMapEncoder.Mat_HeightMapEncoder")))
{
}

AZenoLandscapeSimpleBrush::AZenoLandscapeSimpleBrush(const FObjectInitializer& ObjectInitializer)
	: LandscapeRTRes(0, 0)
{
	SetAffectsHeightmap(true);
}

// Called when the game starts or when spawned
void AZenoLandscapeSimpleBrush::BeginPlay()
{
	Super::BeginPlay();
}

void AZenoLandscapeSimpleBrush::PostLoad()
{
	Super::PostLoad();
}

void AZenoLandscapeSimpleBrush::SetupDefaultMaterials()
{
	const UZenoSimpleBrushSettings* ZenoSimpleBrushSettings = GetDefault<UZenoSimpleBrushSettings>();
	check(ZenoSimpleBrushSettings != nullptr);
	
	BrushHeightMapEncoderMaterial = ZenoSimpleBrushSettings->DefaultBrushHeightMapEncoderMaterial.LoadSynchronous();
}

bool AZenoLandscapeSimpleBrush::AllocateRTs()
{
	HeightmapRTA = FZenoLandscapeHelper::GetOrCreateTransientRenderTarget2D(HeightmapRTA, TEXT("HeightmapRTA"), LandscapeRTRes, RTF_RGBA8);

	return nullptr != HeightmapRTA;
}

bool AZenoLandscapeSimpleBrush::CreateMIDs()
{
	BrushHeightMapEncoderMID = FZenoLandscapeHelper::GetOrCreateTransientMid(BrushHeightMapEncoderMID, TEXT("BrushHeightMapEncoderMID"), BrushHeightMapEncoderMaterial);

	return true;
}

bool AZenoLandscapeSimpleBrush::BrushRenderSetup()
{
	if (!AllocateRTs())
	{
		UE_LOG(LogZenoLandscapeSimpleBrush, Error, TEXT("Invalid Render Target for Water Brush. Aborting BrushRenderSetup."));
		return false;
	}

	if (!CreateMIDs())
	{
		UE_LOG(LogZenoLandscapeSimpleBrush, Error, TEXT("Invalid material for Water Brush. Aborting BrushRenderSetup."));
		return false;
	}

	return true;
}

void AZenoLandscapeSimpleBrush::SetTargetLandscape(ALandscape* InTargetLandscape)
{
	if (OwningLandscape != InTargetLandscape)
	{
		if (OwningLandscape)
		{
			OwningLandscape->RemoveBrush(this);
		}

		if (InTargetLandscape && InTargetLandscape->CanHaveLayersContent())
		{
			static const FName SimpleLayerName = FName("SimpleZenoBrushLayer");
			const ILandscapeModule& LandscapeModule = FModuleManager::GetModuleChecked<ILandscapeModule>("Landscape");
			const int32 SimpleLayerIndex = LandscapeModule.GetLandscapeEditorServices()->GetOrCreateEditLayer(SimpleLayerName, InTargetLandscape);

			InTargetLandscape->AddBrushToLayer(SimpleLayerIndex, this);
		}
	}
}

void AZenoLandscapeSimpleBrush::PostActorCreated()
{
	Super::PostActorCreated();
	if (!BrushHeightMapEncoderMaterial)
	{
		SetupDefaultMaterials();
	}
}

// Called every frame
void AZenoLandscapeSimpleBrush::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UTextureRenderTarget2D* AZenoLandscapeSimpleBrush::Render_Native(const bool InIsHeightmap, UTextureRenderTarget2D* InCombinedResult, const FName& InWeightmapLayerName)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AZenoLandscapeSimpleBrush::Render_Native);

	LandscapeRTRef = InCombinedResult;

	if (BrushRenderSetup())
	{
		if (InIsHeightmap)
		{
			UKismetRenderingLibrary::ClearRenderTarget2D(this, HeightmapRTA, FLinearColor::Black);
			UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, HeightmapRTA, BrushHeightMapEncoderMID);
			return HeightmapRTA;
		}
	}
	
	return nullptr;
}

void AZenoLandscapeSimpleBrush::Initialize_Native(const FTransform& InLandscapeTransform,
	const FIntPoint& InLandscapeSize, const FIntPoint& InLandscapeRenderTargetSize)
{
	LandscapeRTRes = InLandscapeRenderTargetSize;
	LandscapeQuads = InLandscapeSize;
}

void AZenoLandscapeSimpleBrush::CheckForErrors()
{
	Super::CheckForErrors();
}

void AZenoLandscapeSimpleBrush::GetRenderDependencies(TSet<UObject*>& OutDependencies)
{
	Super::GetRenderDependencies(OutDependencies);
}

bool AZenoLandscapeSimpleBrush::IsAffectingWeightmapLayer(const FName& InLayerName) const
{
	return Super::IsAffectingWeightmapLayer(InLayerName);
}

