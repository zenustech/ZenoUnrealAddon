#include "UI/Menu/ZenoEditorMenuExtender.h"

void FZenoEditorMenuExtender::Register()
{
	for (const auto Pair : Services)
	{
		if (Pair.Value != nullptr)
		{
			Pair.Value->Register();
		}
	}
}

void FZenoEditorMenuExtender::Unregister()
{
	for (const auto Pair : Services)
	{
		if (Pair.Value != nullptr)
		{
			Pair.Value->Unregister();
		}
	}
}

void FZenoEditorMenuExtender::Add(const FName& InName, IZenoEditorExtenderService& Service)
{
	Services.Add(InName, &Service);
}

FZenoEditorMenuExtender& FZenoEditorMenuExtender::Get()
{
	static FZenoEditorMenuExtender sExtender;
	return sExtender;
}
