// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZenoLiveLinkTypes.h"
#include "Subsystems/EngineSubsystem.h"
#include "ZenoLiveLinkClientSubsystem.generated.h"

class UZenoLiveLinkSession;
class FZenoLiveLinkSource;

UCLASS()
class ZENOLIVELINKRUNTIME_API UZenoLiveLinkClientSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FZenoLiveLinkSetting& GetZenoLiveLinkSetting();
	const FZenoLiveLinkSetting& GetZenoLiveLinkSetting() const;

	FGuid NewSession(const FZenoLiveLinkSetting& InConnectionSetting);
	bool RequestCloseSession(FGuid InGuid);

private:
	UPROPERTY(EditAnywhere, Category = Zeno, DisplayName = "Connection Settings")
	FZenoLiveLinkSetting ConnectionSetting;

	UPROPERTY(VisibleAnywhere, AdvancedDisplay, DisplayName = "Connection Sessions")
	TMap<FGuid, UZenoLiveLinkSession*> ConnectionSessions;

	friend FZenoLiveLinkSource;
};
