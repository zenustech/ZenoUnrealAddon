// Fill out your copyright notice in the Description page of Project Settings.


#include "Client/Role/LiveLinkZenoRemoteRole.h"

#define LOCTEXT_NAMESPACE "ULiveLinkZenoRemoteRole"

FText ULiveLinkZenoRemoteRole::GetDisplayName() const
{
	return LOCTEXT("RemoteSubject", "Remote Subject");
}

#undef LOCTEXT_NAMESPACE
