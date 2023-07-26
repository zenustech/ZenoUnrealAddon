// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "VATImportSettings.generated.h"


UENUM(BlueprintType)
enum class EZenoVATType : uint8
{
	SoftBody,
	DynamicRemesh,
};


UCLASS(BlueprintType)
class ZENOEDITOR_API UVATImportSettings : public UObject
{
	GENERATED_BODY()

public:

	// VAT type to import
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = VAT, DisplayName = "VAT Type")
	EZenoVATType Type = EZenoVATType::SoftBody;

	// VAT file path
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = VAT, DisplayName = "VAT File Path")
	FFilePath FilePath;
	
};
