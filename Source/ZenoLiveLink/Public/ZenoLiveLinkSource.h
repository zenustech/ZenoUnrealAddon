// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "ILiveLinkSource.h"
#include "ZenoLiveLinkSetting.h"

class ZENOLIVELINK_API FZenoLiveLinkSource : public ILiveLinkSource, public TSharedFromThis<FZenoLiveLinkSource>
{
public:
	explicit FZenoLiveLinkSource(const FZenoLiveLinkSetting InConnectionSettings);
	virtual ~FZenoLiveLinkSource() override;

	// ~start impl ILiveLinkSource
	virtual void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) override;
	virtual bool IsSourceStillValid() const override;
	virtual bool RequestSourceShutdown() override;

	// Update function would block game thread
	virtual void Update() override;

	// for display
	virtual FText GetSourceType() const override { return SourceType; }
	virtual FText GetSourceMachineName() const override { return SourceMachineName; }
	virtual FText GetSourceStatus() const override { return SourceStatus; }
	// ~end impl ILiveLinkSource

private:
	FText SourceType;
	FText SourceMachineName;
	FText SourceStatus;
	
	ILiveLinkClient* Client;
	FGuid SourceGuid;
};
