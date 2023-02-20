// Copyright ZenusTech, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZenoTcpClient.h"
#include "Modules/ModuleManager.h"

class FZenoBridgeModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:
	TObjectPtr<UZenoTcpClient> Client;
};
