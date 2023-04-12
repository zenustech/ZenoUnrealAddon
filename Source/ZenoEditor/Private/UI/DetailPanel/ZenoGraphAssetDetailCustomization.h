#pragma once
#include "IDetailCustomization.h"
#include "ZenoDetailPanelService.h"
#include "ZenoGraphAsset.h"

class FZenoGraphAssetDetailCustomization : public IDetailCustomization
{
public:
	FZenoGraphAssetDetailCustomization();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static TSharedRef<IDetailCustomization> Create();
};

REGISTER_ZENO_DETAIL_CUSTOMIZATION(UZenoGraphAsset::StaticClass()->GetFName(), FZenoGraphAssetDetailCustomization);
