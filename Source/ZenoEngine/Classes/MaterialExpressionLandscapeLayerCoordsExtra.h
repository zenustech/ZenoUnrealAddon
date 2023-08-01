// Zenustech © All Right Reserved

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialExpressionLandscapeLayerCoords.h"
#include "MaterialExpressionLandscapeLayerCoordsExtra.generated.h"

/**
 * 
 */
UCLASS()
class ZENOENGINE_API UMaterialExpressionLandscapeLayerCoordsExtra : public UMaterialExpressionLandscapeLayerCoords
{
	GENERATED_BODY()

protected:

	UPROPERTY(meta = (RequiredInput = "true"))
	FExpressionInput ScaleInputNode;

	explicit UMaterialExpressionLandscapeLayerCoordsExtra(const FObjectInitializer& ObjectInitializer);
	
#if WITH_EDITOR
	virtual int32 Compile(FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual FExpressionInput* GetInput(int32 InputIndex) override;
	virtual const TArray<FExpressionInput*> GetInputs() override;
	virtual FName GetInputName(int32 InputIndex) const override;

	virtual uint32 GetInputType(int32 InputIndex) override;
#endif

};
