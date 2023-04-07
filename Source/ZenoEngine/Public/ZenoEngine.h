// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FZenoEngineModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void MapShaderDirectory(const FString& BaseDir);

private:
	FString ShaderDirectory;
	static FString VirtualShaderPath;
};
