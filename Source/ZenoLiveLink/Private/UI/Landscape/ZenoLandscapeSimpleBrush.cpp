// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Landscape/ZenoLandscapeSimpleBrush.h"

#include "Landscape.h"
#include "LandscapeModule.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UI/Landscape/ZenoLandscapeHelper.h"

DEFINE_LOG_CATEGORY(LogZenoLandscapeSimpleBrush);

#define LOCTEXT_NAMESPACE "AZenoLandscapeSimpleBrush"

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
	BaseHeightmapRT = FZenoLandscapeHelper::GetOrCreateTransientRenderTarget2D(BaseHeightmapRT, TEXT("BaseHeightmapRT"), LandscapeRTRes, RTF_RGBA8);
	HeightmapRTA = FZenoLandscapeHelper::GetOrCreateTransientRenderTarget2D(HeightmapRTA, TEXT("HeightmapRTA"), LandscapeRTRes, RTF_RGBA8);

	return nullptr != HeightmapRTA && nullptr != BaseHeightmapRT;
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

	// Write color data into texture pixels
	ENQUEUE_RENDER_COMMAND(UpdateBaseHeightRT) (
	[&](FRHICommandListImmediate& RHICmdList)
	{
		const int32 Size = sqrt(BaseHeightData.Num());
		
		const TRefCountPtr<FRHITexture2D> Texture = BaseHeightmapRT->GetRenderTargetResource()->GetRenderTargetTexture();
		uint32 DestStride = 0;
		uint8* RawData = static_cast<uint8*>(RHICmdList.LockTexture2D(Texture, 0, EResourceLockMode::RLM_WriteOnly, DestStride, false, true));

		const uint32 RowNum = FMath::Min(BaseHeightmapRT->SizeY, Size);
		const uint32 ColNum = FMath::Min(BaseHeightmapRT->SizeX, Size);
		uint32 BaseHeightDataOffset = 0;
		
		for (uint32 Row = 0; Row < RowNum; ++Row)
		{
			for (uint32 Col = 0; Col < ColNum; ++Col)
			{
				FColor* PixelAddress = reinterpret_cast<FColor*>(RawData + Col * sizeof(FColor) + Row * DestStride);
				
				const uint16 Height = BaseHeightData[BaseHeightDataOffset++];
				const uint8 R = Height >> 8;
				const uint8 G = Height & 0xFF;
				const FColor PixelColor { R, G, 0, 0 };
				*PixelAddress = PixelColor;
			}
		}
		
		RHICmdList.UnlockTexture2D(Texture, 0, false, true);
	});
	FlushRenderingCommands();

	BrushHeightMapEncoderMID->SetTextureParameterValue(FName("InputHeightRT"), BaseHeightmapRT);

	return true;
}

void AZenoLandscapeSimpleBrush::SetTargetLandscape(ALandscape* InTargetLandscape)
{
	// if (OwningLandscape != InTargetLandscape)
	// {
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
	// }
}

void AZenoLandscapeSimpleBrush::SetBaseHeightmapData(const TArray<uint16>& InHeightmapData)
{
	// TODO [darc] : support shapes other than rect and remove this check :
	{
		const int32 Size = sqrt(InHeightmapData.Num());
		check(Size * Size == InHeightmapData.Num());
	}
	BaseHeightData.Empty();
	BaseHeightData.Append(InHeightmapData);
	RequestLandscapeUpdate(true);
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
	if (IsValid(BrushHeightMapEncoderMaterial))
	{
		OutDependencies.Add(BrushHeightMapEncoderMaterial);
	}
}

bool AZenoLandscapeSimpleBrush::IsAffectingWeightmapLayer(const FName& InLayerName) const
{
	return Super::IsAffectingWeightmapLayer(InLayerName);
}

#undef LOCTEXT_NAMESPACE
