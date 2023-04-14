﻿#pragma once

class IZenoEditorExtenderService
{
public:
	virtual ~IZenoEditorExtenderService() = default;
	virtual void Register() = 0;
	virtual void Unregister() = 0;
};

class FZenoEditorExtenderServiceBase : public IZenoEditorExtenderService
{
public:
	FZenoEditorExtenderServiceBase();

	virtual void Register() override;
	virtual void Unregister() override;
	virtual void MapAction() {};

protected:
	TSharedPtr<FUICommandList> CommandList;
	TSharedPtr<FExtender> MenuBarExtender;

	inline static FName ZenoVATHookLabel = "ZenoVAT";
	inline static FName ZenoGraphHookLabel = "ZenoGraph";
};

class FZenoEditorMenuExtender final : public IZenoEditorExtenderService
{
public:
	virtual void Register() override;
	virtual void Unregister() override;

	void Add(const FName& InName, IZenoEditorExtenderService& Service);

private:
	TMap<FName, IZenoEditorExtenderService*> Services;
	
public:
	static FZenoEditorMenuExtender& Get();
};

#define REGISTER_EDITOR_EXTENDER_SERVICE(Name, Service) struct FStaticInitForZenoEditorExtenderNamed##Service \
	{ \
		FStaticInitForZenoEditorExtenderNamed##Service() { \
			FZenoEditorMenuExtender::Get().Add(Name, Service::Get());\
		} \
	}; \
	static FStaticInitForZenoEditorExtenderNamed##Service StaticInitForZenoEditorExtenderNamed##Service;
