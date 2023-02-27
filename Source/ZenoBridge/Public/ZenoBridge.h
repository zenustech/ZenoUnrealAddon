// Copyright ZenusTech, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZenoTcpClient.h"
#include "Modules/ModuleManager.h"

class ZENOBRIDGE_API FZenoBridgeModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	bool StartClient(const FString& EndpointString);
	void StopClient();

	TObjectPtr<UZenoTcpClient> GetTcpClient() const { return Client; }
	
private:
	TObjectPtr<UZenoTcpClient> Client;
};
