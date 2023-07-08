// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZenoCommonBlueprintLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ZENOEDITOR_API UZenoCommonBlueprintLibrary : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @param OutFiles Files that user picked
	 * @param Title Picker title
	 * @param DefaultPath Picker default path when opened
	 * @param DefaultFile Default file selected
	 * @param FileTypes Allowed extensions
	 * @param Flags
	 * @return Return 'false' if failed to load DesktopPlatformModule
	 */
	UFUNCTION(BlueprintCallable)
	static bool OpenSystemFilePicker(TArray<FString>& OutFiles, const FString& Title = "Open File", const FString& DefaultPath = "", const FString& DefaultFile = "", const FString& FileTypes = "", const int32 Flags = 0);

	/**
	 * @param AssetName Default asset name
	 * @param PackagePath Default path
	 * @return Selected path. Might be empty if nothing selected
	 * @return const FString SavePackageName = FPackageName::ObjectPathToPackageName(SaveObjectPath);
	 * @return const FString SavePackagePath = FPaths::GetPath(SavePackageName);
	 * @return const FString SaveAssetName = FPaths::GetBaseFilename(SavePackageName);
	 */
	UFUNCTION(BlueprintCallable)
	static FString OpenContentPicker(const FString& AssetName = "", const FString& PackagePath = "/Game");

	/**
	 * @brief Open settings modal, return after close window.
	 * @param InObject Object to edit
	 * @param InTitle Modal title
	 * @return return true if user clicked apply button
	 */
	UFUNCTION(BlueprintCallable)
	static bool OpenSettingsModal(UObject* InObject, const FText& InTitle = NSLOCTEXT("UZenoCommonBlueprintLibrary", "DefualtTitle", "Settings Panel"));
	
	/**
	 * @brief Open settings modal, return after close window.
	 * @param InObject Object to edit
	 * @param InTitle Modal title
	 * @return return true if user clicked apply button
	 */
	static bool OpenSettingsModal(TSharedRef<FStructOnScope> InObject, const FText& InTitle = NSLOCTEXT("UZenoCommonBlueprintLibrary", "DefualtTitle", "Settings Panel"));

	/**
	 * @brief Open settings modal, return after close window.
	 * @param InText Text to show
	 * @param InDuration Duration to show
	 * @param InColor Color to show
	 * @return return true if user clicked apply button
	 */
	UFUNCTION(BlueprintCallable)
	static void ShowNotification(const FText& InText, const float InDuration = 3.0f);
};
