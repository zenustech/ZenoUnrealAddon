// Copyright ZenusTech, Inc. All Rights Reserved.

#include "ZenoBridge.h"

#define LOCTEXT_NAMESPACE "FZenoBridgeModule"

void FZenoBridgeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FZenoBridgeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	StopClient();
}

bool FZenoBridgeModule::StartClient(const FString& EndpointString)
{
	if (Client) return false;
	
	FIPv4Endpoint Endpoint;
	FIPv4Endpoint::Parse(EndpointString, Endpoint);

	Client = NewObject<UZenoTcpClient>();
	Client->Setup(Endpoint);
	Client->StartClient();
	Client->AddToRoot();

	return true;
}

void FZenoBridgeModule::StopClient()
{
	if (Client)
	{
		Client->Stop();
		Client->RemoveFromRoot();
		Client->MarkAsGarbage();
		Client = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FZenoBridgeModule, ZenoBridge)