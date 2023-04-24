// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Roles/LiveLinkBasicRole.h"
#include "LiveLinkZenoRemoteRole.generated.h"

UCLASS()
class ZENOLIVELINKRUNTIME_API ULiveLinkZenoRemoteRole : public ULiveLinkBasicRole
{
	GENERATED_BODY()

	virtual FText GetDisplayName() const override;
	
};
