#pragma once
#include "IDetailCustomization.h"
#include "ZenoLandscapeActorDetailCustomization.generated.h"

UCLASS()
class UZenoLandscapeActorSetting : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, NonTransactional, Category = "Create", DisplayName = "New Layer Name")
	FName NewLayerName = "Mask";

	UPROPERTY(EditAnywhere, NonTransactional, Category = "Heightmap", DisplayName = "Heightmap Textures", AdvancedDisplay)
	TMap<FGuid, TWeakObjectPtr<UTexture2D>> EncounteredTextures;
};

class FZenoLandscapeActorDetailCustomization : public IDetailCustomization
{
public:
	FZenoLandscapeActorDetailCustomization();
	virtual ~FZenoLandscapeActorDetailCustomization() override;
	
	static TSharedRef<IDetailCustomization> MakeInstance();
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	UZenoLandscapeActorSetting* UISettings;

	TSharedPtr<IDetailsView> UISettingDetailsView;
};
