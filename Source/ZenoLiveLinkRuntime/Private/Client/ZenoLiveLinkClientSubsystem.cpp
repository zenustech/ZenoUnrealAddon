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

UZenoLiveLinkSession* UZenoLiveLinkClientSubsystem::GetSession(FGuid InGuid)
{
	if (ConnectionSessions.Contains(InGuid))
	{
		return ConnectionSessions[InGuid];
	}
	return nullptr;
}

UZenoLiveLinkSession* UZenoLiveLinkClientSubsystem::FindSessionWithSubject(const FName& InName)
{
	for (const auto& Pair : ConnectionSessions)
	{
		if (IsValid(Pair.Value) && Pair.Value->IsInitialized() && Pair.Value->HasSubject(InName.ToString()))
		{
			return Pair.Value;
		}
	}
	return nullptr;
}

UZenoLiveLinkSession* UZenoLiveLinkClientSubsystem::GetSessionFallback()
{
	if (ConnectionSessions.IsEmpty()) return nullptr;
	return ConnectionSessions.begin()->Value;
}
