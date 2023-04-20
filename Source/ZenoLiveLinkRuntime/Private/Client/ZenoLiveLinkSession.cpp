// Fill out your copyright notice in the Description page of Project Settings.


#include "Client/ZenoLiveLinkSession.h"

UZenoLiveLinkSession::UZenoLiveLinkSession()
{
}

void UZenoLiveLinkSession::Init(const FZenoLiveLinkSetting& ConnectionSetting)
{
	check(!bInitialized.load());
	bInitialized = true;
	
	Settings = ConnectionSetting;

	// TODO [darc] : Request for session key :
}

UZenoLiveLinkSession* UZenoLiveLinkSession::CreateSession(const FZenoLiveLinkSetting& ConnectionSetting, const FGuid Guid)
{
	UZenoLiveLinkSession* NewSession = NewObject<UZenoLiveLinkSession>();
	NewSession->Init(ConnectionSetting);
	NewSession->Guid = Guid;
	return NewSession;
}
