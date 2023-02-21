// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Roles/LiveLinkTransformRole.h"
#include "UObject/Object.h"
#include "LiveLinkTranslationRole.generated.h"

/**
 * Role associated for Translation data.
 */
UCLASS()
class ZENOLIVELINK_API ULiveLinkTranslationRole : public ULiveLinkTransformRole
{
	GENERATED_BODY()
	
public:
	//~ Begin ULiveLinkRole interface
	virtual UScriptStruct* GetStaticDataStruct() const override;
	virtual UScriptStruct* GetFrameDataStruct() const override;
	virtual UScriptStruct* GetBlueprintDataStruct() const override;

	virtual bool InitializeBlueprintData(const FLiveLinkSubjectFrameData& InSourceData, FLiveLinkBlueprintDataStruct& OutBlueprintData) const override;

	virtual FText GetDisplayName() const override;
	//~ End ULiveLinkRole interface
};
