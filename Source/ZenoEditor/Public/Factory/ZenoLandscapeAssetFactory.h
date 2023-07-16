// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "ZenoLandscapeAssetFactory.generated.h"

/**
 * Handling file with extension .zlasset
 */
UCLASS()
class ZENOEDITOR_API UZenoLandscapeAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UZenoLandscapeAssetFactory(const FObjectInitializer& ObjectInitializer);

	//~ Begin UFactory Interface
	virtual UObject* FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override;
	//~ End UFactory Interface
	
};
