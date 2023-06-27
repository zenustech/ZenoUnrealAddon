#include "UI/Toolkit/ZenoLandscapeEditor.h"

#include "EditorModeManager.h"
#include "EditorModes.h"
#include "ZenoEditorCommand.h"

#define LOCTEXT_NAMESPACE "UZenoLandscapeEditor"

UZenoLandscapeEditor::UZenoLandscapeEditor()
	: UZenoEditorToolkitBase()
{
}

FText UZenoLandscapeEditor::GetDisplayName() const
{
	return LOCTEXT("ZenoLandscape", "Zeno Landscape");
}

bool UZenoLandscapeEditor::CanBeCreate(const FSpawnTabArgs& Args) const
{
	return GLevelEditorModeTools().IsModeActive(FBuiltinEditorModes::EM_Landscape);
}

FName UZenoLandscapeEditor::GetUniqueName() const
{
	const static FName NAME_Landscape = "Z_Landscape";
	return NAME_Landscape;
}

void UZenoLandscapeEditor::Init()
{
	State = NewObject<UZenoLandscapeObject>();
	UZenoEditorToolkitBase::SetPropertyObject(State)
	.AddSubMode("Import", FZenoEditorCommand::Get().ImportHeightField.ToSharedRef())
	.AddSubMode("Export", FZenoEditorCommand::Get().ExportHeightField.ToSharedRef())
	;
}

void UZenoLandscapeEditor::Register()
{
	Super::Register();
	GLevelEditorModeTools().OnEditorModeIDChanged().AddUObject(this, &UZenoLandscapeEditor::OnEditorModeChanged);
}

void UZenoLandscapeEditor::OnEditorModeChanged(const FEditorModeID& InModeID, const bool bIsEnteringMode)
{
	if (InModeID == FBuiltinEditorModes::EM_Landscape)
	{
		if (bIsEnteringMode)
		{
			FGlobalTabmanager::Get()->TryInvokeTab(GetUniqueName());
		}
		else
		{
			if (const auto& Tab = Slate_ToolkitDockTab; Tab.IsValid())
			{
				Tab->RequestCloseTab();
			}
			MakeDockTab();
		}
	}
}

#undef LOCTEXT_NAMESPACE
