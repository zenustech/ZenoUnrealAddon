// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "VATImportSettings.generated.h"


UENUM(BlueprintType)
enum class EZenoVATType
{
	SoftBody,
	DynamicRemesh,
};


UCLASS()
class ZENOEDITOR_API UVATImportSettings : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = VAT)
	EZenoVATType Type = EZenoVATType::SoftBody;
};
