// Fill out your copyright notice in the Description page of Project Settings.


#include "Client/ZenoLiveLinkClientSubsystem.h"

#include "Client/ZenoLiveLinkSession.h"

void UZenoLiveLinkClientSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
}

void UZenoLiveLinkClientSubsystem::Deinitialize()
{
}

FZenoLiveLinkSetting& UZenoLiveLinkClientSubsystem::GetZenoLiveLinkSetting()
{
	return ConnectionSetting;
}

const FZenoLiveLinkSetting& UZenoLiveLinkClientSubsystem::GetZenoLiveLinkSetting() const
{
	return ConnectionSetting;
}

FGuid UZenoLiveLinkClientSubsystem::NewSession(const FZenoLiveLinkSetting& InConnectionSetting)
{
	const FGuid NewId = FGuid::NewGuid();
	UZenoLiveLinkSession* Session = UZenoLiveLinkSession::CreateSession(InConnectionSetting, NewId);
	ConnectionSessions.Add(NewId, Session);
	return NewId;
}

bool UZenoLiveLinkClientSubsystem::RequestCloseSession(FGuid InGuid)
{
	if (!ConnectionSessions.Contains(InGuid))
	{
		return true;
	}
	// TODO [darc] : sending stop session request to zeno :
	ConnectionSessions.Remove(InGuid);
	return true;
}
