// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Importer/VAT/VATTypes.h"
#include "VATUtility.generated.h"

/**
 *
 */
UCLASS()
class ZENOEDITOR_API UVATUtility : public UObject
{
	GENERATED_BODY()

public:
	static bool ParseBinaryInfo(const FString& InFilePath, FVATInfo& OutInfo);
};
