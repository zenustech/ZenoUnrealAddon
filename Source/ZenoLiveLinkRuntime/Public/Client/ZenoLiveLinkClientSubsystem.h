// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZenoLiveLinkTypes.h"
#include "Subsystems/EngineSubsystem.h"
#include "ZenoLiveLinkClientSubsystem.generated.h"

class FZenoLiveLinkSource;

UCLASS()
class ZENOLIVELINKRUNTIME_API UZenoLiveLinkClientSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	UPROPERTY(EditAnywhere, Category = Zeno, DisplayName = "Connection Settings")
	FZenoLiveLinkSetting ConnectionSetting;

	friend FZenoLiveLinkSource;
};
