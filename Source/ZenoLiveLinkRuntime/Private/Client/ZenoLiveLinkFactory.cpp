// Fill out your copyright notice in the Description page of Project Settings.


#include "Client/ZenoLiveLinkFactory.h"

#include "Client/ZenoLiveLinkTypes.h"
#include "GenericPlatform/GenericPlatformOutputDevices.h"

#define LOCTEXT_NAMESPACE "UZenoLiveLinkFactory"

FText UZenoLiveLinkFactory::GetSourceDisplayName() const
{
	return LOCTEXT("ZenoSourceDisplayName", "Zeno");
}

FText UZenoLiveLinkFactory::GetSourceTooltip() const
{
	return LOCTEXT("ZenoSourceTooltip", "Start a new zeno remote session.");
}

#ifdef WITH_EDITORONLY_DATA
TSharedPtr<SWidget> UZenoLiveLinkFactory::BuildCreationPanel(FOnLiveLinkSourceCreated OnLiveLinkSourceCreated) const
{
	return TSharedPtr<SWidget>();
}
#endif // WITH_EDITORONLY_DATA

TSharedPtr<ILiveLinkSource> UZenoLiveLinkFactory::CreateSource(const FString& ConnectionString) const
{
	// TODO [darc] : Supporting init connection from presets :
	FZenoLiveLinkSetting Setting;
	FZenoLiveLinkSetting::StaticStruct()->ImportText(*ConnectionString, &Setting, const_cast<UZenoLiveLinkFactory*>(this), CPF_Transient, FGenericPlatformOutputDevices::GetError(), FZenoLiveLinkSetting::StaticStruct()->GetStructCPPName(), true);
	return nullptr;
}

ULiveLinkSourceFactory::EMenuType UZenoLiveLinkFactory::GetMenuType() const
{
	return EMenuType::SubPanel;
}

void UZenoLiveLinkFactory::CreateSourceFromSettings(FZenoLiveLinkSetting InSettings,
                                                    FOnLiveLinkSourceCreated OnSourceCreated) const
{
}

#undef LOCTEXT_NAMESPACE
