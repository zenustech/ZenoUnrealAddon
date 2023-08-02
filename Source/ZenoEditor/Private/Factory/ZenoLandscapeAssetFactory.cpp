// Fill out your copyright notice in the Description page of Project Settings.


#include "Factory/ZenoLandscapeAssetFactory.h"

#include "IImageWrapperModule.h"
#include "ZenoAssetBundle.h"
#include "ZenoLandscapeAsset.h"
#include "ZenoPointSetAsset.h"
#include "alpaca/alpaca.h"
#include "zeno/unreal/ZenoAssetTypes.h"

#define LOCTEXT_NAMESPACE "ZenoLandscapeAssetFactory"

UZenoLandscapeAssetFactory::UZenoLandscapeAssetFactory(const FObjectInitializer& ObjectInitializer)
{
	Formats.Add("zlasset;Zeno Asset File");
	bText = false;
	bEditorImport = true;
	SupportedClass = UZenoAssetBundle::StaticClass();
	bCreateNew = false;
}

UObject* UZenoLandscapeAssetFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName,
                                                         EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd,
                                                         FFeedbackContext* Warn)
{
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, InClass, InParent, InName, Type);

	check( InClass == UZenoAssetBundle::StaticClass() );

	UZenoAssetBundle* AssetBundle = NewObject<UZenoAssetBundle>(InParent, InClass, InName, Flags);

	std::error_code Err;
	std::vector<uint8> Data(Buffer, BufferEnd);
	zeno::unreal::AssetBundle RawBundle = alpaca::deserialize<alpaca::options::with_checksum, zeno::unreal::AssetBundle>(Data, Err);

	if (Err)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to deserialize asset bundle: %hs"), Err.message().c_str());
	}

	TMap<FString, UTexture2D**> NameToTextureMap;
	TMap<FString, UTexture2D*> NameToTexture;
	
	for (const auto& [Name, Value] : RawBundle.Assets)
	{
		if (std::holds_alternative<zeno::unreal::PointSet>(Value))
		{
			auto& PointSet = std::get<zeno::unreal::PointSet>(Value);
			UZenoPointSetAsset* PointSetAsset = NewObject<UZenoPointSetAsset>(AssetBundle, UZenoPointSetAsset::StaticClass(), FName(Name.c_str()), Flags);
			PointSetAsset->PointType = static_cast<EZenoPointType>(PointSet.PointType);
			PointSetAsset->Points.Reserve(PointSet.Points.size());
			for (const auto& Point : PointSet.Points)
			{
				PointSetAsset->Points.Emplace(FRotator { Point.Rotation[0], Point.Rotation[2], Point.Rotation[1] }, FVector { Point.Position[0], Point.Position[2], Point.Position[1] }, FVector { Point.Scale[0], Point.Scale[2], Point.Scale[1] });
			}
			AssetBundle->NameToAssetMap.Add(FString(Name.c_str()), PointSetAsset);
			AssetBundle->PointSet.Add(PointSetAsset);
		}
		else if (std::holds_alternative<zeno::unreal::LandscapeData>(Value))
		{
			const auto& [Nx, Ny, HeightField, Transform, BaseColorTextureRef, _Flags] = std::get<zeno::unreal::LandscapeData>(Value);
			UZenoLandscapeAsset* LandscapeAsset = NewObject<UZenoLandscapeAsset>(AssetBundle, UZenoLandscapeAsset::StaticClass(), FName(Name.c_str()), Flags);
			LandscapeAsset->Extent = FIntPoint { static_cast<int32>(Nx), static_cast<int32>(Ny) };
			LandscapeAsset->HeightData.Reserve(HeightField.size());
			for (const auto& Height : HeightField)
			{
				LandscapeAsset->HeightData.Emplace(Height);
			}
			LandscapeAsset->Transform.Position = FVector(Transform.Position[0], Transform.Position[2], Transform.Position[1]);
			LandscapeAsset->Transform.Rotation = FVector(Transform.Rotation[0], Transform.Rotation[2], Transform.Rotation[1]);
			LandscapeAsset->Transform.Scale = FVector(Transform.Scale[0], Transform.Scale[2], Transform.Scale[1]);
			if (!BaseColorTextureRef.Guid.empty())
			{
				FString Key(BaseColorTextureRef.Guid.c_str());
				if (UTexture2D* Texture = NameToTexture.FindRef(Key))
				{
					LandscapeAsset->BaseColorTexture = Texture;
				}
				else
				{
					NameToTextureMap.Add(Key, &LandscapeAsset->BaseColorTexture);
				}
			}

			AssetBundle->Landscapes.Add(LandscapeAsset);
			AssetBundle->NameToAssetMap.Add(FString(Name.c_str()), LandscapeAsset);
		}
		else if (std::holds_alternative<zeno::unreal::PngTextureData>(Value))
		{
			const auto& [TextureData, Width, Height, _Flags] = std::get<zeno::unreal::PngTextureData>(Value);

			IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
			TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
			ImageWrapper->SetCompressed(TextureData.data(), TextureData.size());
			TArray64<uint8> RawData;
			if (ImageWrapper->GetBitDepth() == 8 && ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
			{
				UTexture2D* Texture = NewObject<UTexture2D>(AssetBundle, UTexture2D::StaticClass(), FName(Name.c_str()), RF_Public);
				Texture->Source.Init(Width, Height, 1, 1, ETextureSourceFormat::TSF_BGRA8, RawData.GetData());
				Texture->SRGB = true;
				Texture->CompressionSettings = TC_Default;
				Texture->Filter = TF_Nearest;
				
				Texture->UpdateResource();

				FString Key(Name.c_str());
				NameToTexture.Add(Key, Texture);
				if (UTexture2D** Ptr = NameToTextureMap.FindRef(Key))
				{
					*Ptr = Texture;
				}

				AssetBundle->UsedTextures.Add(Texture);
			}
			else if (ImageWrapper->GetBitDepth() == 16 && ImageWrapper->GetRaw(ERGBFormat::Gray, 16, RawData))
			{
				UTexture2D* Texture = NewObject<UTexture2D>(AssetBundle, UTexture2D::StaticClass(), FName(Name.c_str()), RF_Public);
				Texture->Source.Init(Width, Height, 1, 1, ETextureSourceFormat::TSF_G16, RawData.GetData());
				Texture->SRGB = false;
				Texture->CompressionSettings = TC_Default;
				Texture->Filter = TF_Nearest;
				
				Texture->UpdateResource();
				
				FString Key(Name.c_str());
				NameToTexture.Add(Key, Texture);
				if (UTexture2D** Ptr = NameToTextureMap.FindRef(Key))
				{
					*Ptr = Texture;
				}
				
				AssetBundle->UsedTextures.Add(Texture);
			}
		}
	}
	
	return AssetBundle;
}

bool UZenoLandscapeAssetFactory::CanCreateNew() const
{
	return false;
}

FText UZenoLandscapeAssetFactory::GetDisplayName() const
{
	return LOCTEXT("Name", "Zeno Landscape Asset");
}

#undef LOCTEXT_NAMESPACE
