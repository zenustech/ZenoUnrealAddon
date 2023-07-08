#pragma once

class ZENOEDITOR_API FZenoDetailPanelServiceManager
{
public:
	static FZenoDetailPanelServiceManager& Get();

private:
	static void Register();
	static void Unregister();

public:
	void RegisterDetailCustomization(const FName& InName, const FOnGetDetailCustomizationInstance& DetailCustomization);

private:
	TMap<FName, FOnGetDetailCustomizationInstance> DetailCustomizations;

	bool bInitialized = false;

	friend class FZenoEditorModule;
};

#define REGISTER_ZENO_DETAIL_CUSTOMIZATION(Name, Service) struct FStaticInitForZenoEditorDetailCustomizationNamed##Service \
                                           	{ \
                                           		FStaticInitForZenoEditorDetailCustomizationNamed##Service() { \
                                           			if (!IsRunningCommandlet()) \
                                           			FZenoDetailPanelServiceManager::Get().RegisterDetailCustomization(Name, FOnGetDetailCustomizationInstance::CreateStatic(&##Service::Create));\
                                           		} \
                                           	}; \
                                           	static FStaticInitForZenoEditorDetailCustomizationNamed##Service StaticInitForZenoEditorDetailCustomizationNamed##Service;

