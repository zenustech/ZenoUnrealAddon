#include "ZenoGraphAssetDetailCustomization.h"

#include "DetailLayoutBuilder.h"

FZenoGraphAssetDetailCustomization::FZenoGraphAssetDetailCustomization()
{
}

void FZenoGraphAssetDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
}

TSharedRef<IDetailCustomization> FZenoGraphAssetDetailCustomization::Create()
{
	return MakeShared<FZenoGraphAssetDetailCustomization>();
}
