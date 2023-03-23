#include "ZenoLandscapeVisualDataPropertyView.h"

#include "DetailWidgetRow.h"
#include "EditorDialogLibrary.h"
#include "Helper.h"
#include "PropertyCustomizationHelpers.h"
#include "ZenoBridge.h"
#include "3rd/msgpack.h"
#include "Command/ZenoTextureExportCommand.h"
#include "model/subject.h"
#include "UI/Landscape/ZenoLandscapeEditorObject.h"
#include "UI/Landscape/ZenoLandscapeHelper.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "FZenoLandscapeVisualDataPropertyView"


FZenoLandscapeTexture2DExportPropertyView::FZenoLandscapeTexture2DExportPropertyView(): ExportSetting(NewObject<UZenoTextureExportSetting>())
{
	ExportSetting->AddToRoot();
		
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bShowScrollBar = true;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(ExportSetting);
}

FZenoLandscapeTexture2DExportPropertyView::~FZenoLandscapeTexture2DExportPropertyView()
{
	ExportSetting->RemoveFromRoot();
}

TSharedRef<IPropertyTypeCustomization> FZenoLandscapeTexture2DExportPropertyView::MakeInstance()
{
	return MakeShared<FZenoLandscapeTexture2DExportPropertyView>();
}

void FZenoLandscapeTexture2DExportPropertyView::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle,
                                                                FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	HeaderRow.ValueContent()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SNew(SProperty, PropertyHandle).ShouldDisplayName(false)
		]
		+ SVerticalBox::Slot()
		.Padding(FMargin { .0, 10.0, .0, .0 })
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				// Export button
				SNew(SComboButton)
				.ButtonContent()
				[
					SNew(STextBlock).Text(LOCTEXT("Export", "Export")).Font(CustomizationUtils.GetRegularFont())
				]
				.OnGetMenuContent(FOnGetContent::CreateSP(this, &FZenoLandscapeTexture2DExportPropertyView::MakeExportPanelContent, PropertyHandle))
			]
		]
	];
}

TSharedRef<SWidget> FZenoLandscapeTexture2DExportPropertyView::MakeExportPanelContent(
	TSharedRef<IPropertyHandle> PropertyHandle)
{
	return SAssignNew(Slate_ExportPanel, SScrollBox)
		+ SScrollBox::Slot()
		[
			DetailsView.ToSharedRef()
		]
		+ SScrollBox::Slot()
		.Padding(FMargin { .0, 10.0, .0, .0 })
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("DoExport", "Export"))
				.OnClicked(FOnClicked::CreateSP(this, &FZenoLandscapeTexture2DExportPropertyView::ExportData, PropertyHandle))
			]
		]
		;
}

FReply FZenoLandscapeTexture2DExportPropertyView::ExportData(TSharedRef<IPropertyHandle> PropertyHandle)
{
	TWeakObjectPtr<UTexture2D>* Ptr;
	PropertyHandle->GetValueData(reinterpret_cast<void*&>(Ptr));
		
	if (Ptr && Ptr->IsValid())
	{
		if (!ExportSetting->SubjectName.IsEmpty())
		{
			UTexture2D* Texture2D = Ptr->Get();
			const FZenoBridgeModule& ZenoBridgeModule = FModuleManager::LoadModuleChecked<FZenoBridgeModule>("ZenoBridge");
			if (TObjectPtr<UZenoTcpClient> Client = ZenoBridgeModule.GetTcpClient(); Client)
			{
				// Update texture2d options to make sure there is a mipmap data
				// Keeping old settings
				const TextureCompressionSettings OldCompressSettings = Texture2D->CompressionSettings;
				const TextureMipGenSettings OldMipGenSettings = Texture2D->MipGenSettings;
				const bool OldSRGB = Texture2D->SRGB;
				// Update settings
				Texture2D->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
				Texture2D->SRGB = false;
				Texture2D->UpdateResource();
					
				UnrealHeightFieldSubject Subject;
				Subject.m_name = TCHAR_TO_UTF8(*ExportSetting->SubjectName);
				FTexture2DMipMap* MipMap = &Texture2D->GetPlatformData()->Mips[0];
				ensure(MipMap && MipMap->SizeX == MipMap->SizeY);
				Subject.m_resolution = MipMap->SizeX; // assume that x == y for now
				FByteBulkData& RawTextureData = MipMap->BulkData;
				const FColor* FormattedImageData = static_cast<const FColor*>(RawTextureData.LockReadOnly());
				const size_t HeightFieldSize = MipMap->SizeX * MipMap->SizeY;
				Subject.m_height.resize(HeightFieldSize);

				for (size_t Idx = 0; Idx < HeightFieldSize; Idx++)
				{
					const FColor& PixelColor = FormattedImageData[Idx];
					uint16 RawHeight = (PixelColor.R << 8) + PixelColor.G;
					Subject.m_height[Idx] = FZenoLandscapeHelper::RemapUint16ToFloat(RawHeight);
				}
				RawTextureData.Unlock();

				// Reset settings
				Texture2D->CompressionSettings = OldCompressSettings;
				Texture2D->MipGenSettings = OldMipGenSettings;
				Texture2D->SRGB = OldSRGB;
				Texture2D->UpdateResource();
					
				const TArray<TArray<uint8>> Data = FUnrealSocketHelper::MakeSendFileData(Subject, ZBFileType::HeightField);
				Client->SendUdpDatagrams(Data);
			}
			else
			{
				UEditorDialogLibrary::ShowMessage(
					LOCTEXT("ActionWarning", "Warning"),
					LOCTEXT("ZenoNotConnected", "You have to connect to zeno first. You could find it on LiveLink panel."),
					EAppMsgType::Ok
				);
			}
		}
		else
		{
			UEditorDialogLibrary::ShowMessage(
				LOCTEXT("ActionWarning", "Warning"),
				LOCTEXT("SubjectNameIsEmpty", "Subject's name could not be empty."),
				EAppMsgType::Ok
			);
		}
	}
	else
	{
		UEditorDialogLibrary::ShowMessage(
			LOCTEXT("ActionWarning", "Warning"),
			LOCTEXT("BadTargetPtr", "Bad target pointer."),
			EAppMsgType::Ok
		);
	}
		
	return FReply::Handled();
}

FZenoLandscapeVisualDataPropertyView::FZenoLandscapeVisualDataPropertyView()
{
	CommandList = MakeShared<FUICommandList>();

	CommandList->MapAction(FZenoTextureExportCommand::Get().ExportTexture, FExecuteAction::CreateLambda([]
	{
		UE_LOG(LogTemp, Warning, TEXT("123"));
	}));

	// Create a custom detail panel for our VisualData object
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.HostCommandList = CommandList;
	DetailsViewArgs.bShowObjectLabel = true;
	DetailsViewArgs.bShowScrollBar = true;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	// Register view hook for TWeakObjectPtr<_>
	DetailsView->RegisterInstancedCustomPropertyTypeLayout("WeakObjectProperty", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FZenoLandscapeTexture2DExportPropertyView::MakeInstance));
	DetailsView->SetObject(UZenoLandscape_VisualData::StaticClass()->GetDefaultObject());
}

void FZenoLandscapeVisualDataPropertyView::CustomizeHeader(
	TSharedRef<IPropertyHandle> PropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{

	PropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([View = DetailsView.ToWeakPtr(), PropertyHandle]
	{
		if (View.IsValid())
		{
			UObject* Object;
			PropertyHandle->GetValue(Object);
			UZenoLandscape_VisualData* VisualDataObject = Cast<UZenoLandscape_VisualData>(Object);
			View.Pin()->SetObject(VisualDataObject);
		}
	}));

	HeaderRow
	.NameContent()
	[
		SNew(STextBlock).Text(PropertyHandle->GetPropertyDisplayName()).Font(CustomizationUtils.GetRegularFont())
	]
	.ValueContent()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.MaxHeight(300)
		[
			DetailsView.ToSharedRef()
		]
	];
}

void FZenoLandscapeVisualDataPropertyView::CustomizeChildren(
	TSharedRef<IPropertyHandle> PropertyHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

TSharedRef<IPropertyTypeCustomization> FZenoLandscapeVisualDataPropertyView::MakeInstance()
{
	return MakeShared<FZenoLandscapeVisualDataPropertyView>();
}

#undef LOCTEXT_NAMESPACE
