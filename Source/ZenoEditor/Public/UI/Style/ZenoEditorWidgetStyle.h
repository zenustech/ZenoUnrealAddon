// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZenoEditorWidgetStyle.generated.h"

USTRUCT()
struct ZENOEDITOR_API FZenoEditorWidgetStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FZenoEditorWidgetStyle();
	virtual ~FZenoEditorWidgetStyle() override;
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FZenoEditorWidgetStyle& GetDefault();
};

UCLASS(HideCategories=Object, MinimalAPI)
class UZenoEditorWidgetStyleWrapper : public USlateWidgetStyleContainerBase
{
	GENERATED_BODY()

public:
	 /** The actual data describing the widget appearance. */
	 UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	 FZenoEditorWidgetStyle WidgetStyle;

	 virtual const struct FSlateWidgetStyle* const GetStyle() const override
	 {
		 return static_cast<const struct FSlateWidgetStyle*>(&WidgetStyle);
	 }
};
