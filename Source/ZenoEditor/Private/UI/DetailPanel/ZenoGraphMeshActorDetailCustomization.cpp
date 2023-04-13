#include "ZenoGraphMeshActorDetailCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "ZenoDetailPanelService.h"
#include "ZenoGraphActor.h"
#include "ZenoGraphAsset.h"
#include "Blueprint/ZenoGraphLibrary.h"
#include <map>
#include <string>
#include <utility>
#include <zeno/core/Graph.h>
#include <zeno/unreal/UnrealTool.h>

#include "DetailWidgetRow.h"
#include "RawMesh.h"
#include "Async/Async.h"
#include "Slate/ZenoGenericInputBox.h"
#include "UObject/GCObjectScopeGuard.h"

#define LOCTEXT_NAMESPACE "FZenoGraphMeshActorDetailCustomization"

FZenoGraphMeshActorDetailCustomization::FZenoGraphMeshActorDetailCustomization()
{
}

void FZenoGraphMeshActorDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	IDetailCategoryBuilder& ZenoBuilder = DetailBuilder.EditCategory("Zeno");
	if (Objects.IsEmpty()) return;
	const TWeakObjectPtr<UObject>& Object = Objects[0];
	AZenoGraphMeshActor* MeshActor = Cast<AZenoGraphMeshActor>(Object.Get());
	if (!IsValid(MeshActor) || !UZenoGraphLibrary::IsValidGraph(MeshActor->ZenoGraphAsset))
		return;

	ZenoBuilder.AddCustomRow(LOCTEXT("Zeno", "Zeno"))
	[
		SAssignNew(Slate_InputPanel, SVerticalBox)
	];
	BuildParamList_Slate(UZenoGraphLibrary::GetGraphParamList(MeshActor->ZenoGraphAsset));
	Slate_InputPanel->AddSlot()
	[
		SNew(SButton)
		.Text(LOCTEXT("Generate", "Generate"))
		.OnClicked_Raw(this, &FZenoGraphMeshActorDetailCustomization::DoMeshGenerate, MeshActor)
	];
}

TSharedRef<IDetailCustomization> FZenoGraphMeshActorDetailCustomization::Create()
{
	return MakeShared<FZenoGraphMeshActorDetailCustomization>();
}

void FZenoGraphMeshActorDetailCustomization::BuildParamList_Slate(const zeno::unreal::SubnetNodeParamList& InParamList)
{
	ParamTypeMap.Reset();
	Slate_ParamInputBox.Reset();

	for (const std::pair<std::string, int8_t>& Pair : InParamList.params)
	{
		FString ParamName = FString(Pair.first.c_str());
		const zeno::unreal::EParamType ParamType = static_cast<zeno::unreal::EParamType>(Pair.second);
		ParamTypeMap.Add(FName(ParamName), ParamType);
		TSharedPtr<SZenoGenericInputBox> Box;
		Slate_InputPanel
		->AddSlot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock)
				.Text(FText::FromString(ParamName))
			]
			+ SHorizontalBox::Slot()
			[
				SAssignNew(Box, SZenoGenericInputBox)
				.ParamType(Pair.second)
			]
		];
		Slate_ParamInputBox.Add(FName(ParamName), Box.ToSharedRef());
	}
}

FReply FZenoGraphMeshActorDetailCustomization::DoMeshGenerate(AZenoGraphMeshActor* TargetActor) const
{
	if (CriticalSection.TryLock())
	{
		AsyncTask(ENamedThreads::GameThread, [this, TargetActor]
		{
			if (!IsValid(TargetActor))
			{
				return;
			}
			FScopeLock Lock { &FZenoGraphMeshActorDetailCustomization::CriticalSection };
			FGCObjectScopeGuard Guard {TargetActor};
			std::shared_ptr<zeno::Graph> Graph = zeno::getSession().createGraph();
			if (zeno::LoadGraphChecked(Graph.get(), TCHAR_TO_ANSI(*TargetActor->ZenoGraphAsset->ZenoActionRecordExportedData)))
			{
				zeno::unreal::NodeParamInput Inputs;
				for (const auto& SlateUI : Slate_ParamInputBox)
				{
					 try { Inputs.data.try_emplace(TCHAR_TO_ANSI(*SlateUI.Key.ToString()), SlateUI.Value->GetDataUnsafe<float>()); } catch (...) {}
					 try { Inputs.data.try_emplace(TCHAR_TO_ANSI(*SlateUI.Key.ToString()), SlateUI.Value->GetDataUnsafe<int32>()); } catch (...) {}
				}
				std::vector<uint8> Buffer = msgpack::pack(Inputs);
				zeno::SimpleCharBuffer OutBuffer = zeno::Run_Mesh(Graph.get(), { reinterpret_cast<char*>(Buffer.data()), Buffer.size() });
				std::error_code Err;
				zeno::unreal::Mesh MeshData = msgpack::unpack<zeno::unreal::Mesh>(reinterpret_cast<uint8*>(OutBuffer.data), OutBuffer.length, Err);
				FRawMesh RawMesh;
				RawMesh.VertexPositions.Reserve(MeshData.vertices.size());
				RawMesh.WedgeIndices.Reserve(MeshData.triangles.size());
				for (const auto& Vertex : MeshData.vertices)
				{
					RawMesh.VertexPositions.Add( { Vertex[0], Vertex[1], Vertex[2] } );
				}
				for (const auto& Triangle : MeshData.triangles)
				{
					RawMesh.WedgeIndices.Append( { static_cast<uint32>(Triangle[0]), static_cast<uint32>(Triangle[1]), static_cast<uint32>(Triangle[2]) } );
				}
				const size_t NumFace = MeshData.triangles.size();
				const size_t NumWedge = RawMesh.WedgeIndices.Num();
				RawMesh.FaceMaterialIndices.SetNumZeroed(NumFace);
				RawMesh.FaceSmoothingMasks.SetNumZeroed(NumFace);
				RawMesh.WedgeTangentX.SetNumZeroed(NumWedge);
				RawMesh.WedgeTangentY.SetNumZeroed(NumWedge);
				RawMesh.WedgeTangentZ.SetNumZeroed(NumWedge);
				RawMesh.WedgeColors.SetNumZeroed(NumWedge);
				// TODO [darc] : support UV :
				RawMesh.WedgeTexCoords[0].SetNumZeroed(NumWedge);
				TargetActor->StaticMesh = NewObject<UStaticMesh>(TargetActor, UStaticMesh::StaticClass(), MakeUniqueObjectName(TargetActor, UStaticMesh::StaticClass(), FName("StaticMesh")), RF_Public|RF_Standalone);
				TargetActor->StaticMesh->PreEditChange(nullptr);
				TargetActor->StaticMesh->ImportVersion = LastVersion;
				TargetActor->StaticMesh->NaniteSettings.bEnabled = false;
				FStaticMeshSourceModel& SourceModel = TargetActor->StaticMesh->AddSourceModel();
				{
					 SourceModel.BuildSettings.bRecomputeNormals = false;
					 SourceModel.BuildSettings.bRecomputeTangents = false;
					 SourceModel.BuildSettings.bRemoveDegenerates = false;
					 SourceModel.BuildSettings.bComputeWeightedNormals = false;
					 SourceModel.BuildSettings.bUseMikkTSpace = false;
					 SourceModel.BuildSettings.bUseFullPrecisionUVs = true;
					 SourceModel.BuildSettings.bUseHighPrecisionTangentBasis = true;
					 SourceModel.SaveRawMesh(RawMesh);
				}
				TargetActor->StaticMesh->PostEditChange();
				TargetActor->StaticMesh->Build();
				TargetActor->StaticMeshComponent->SetStaticMesh(TargetActor->StaticMesh);
			}
		});
		CriticalSection.Unlock();
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to generate zeno mesh because another task is running."))
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE

REGISTER_ZENO_DETAIL_CUSTOMIZATION(AZenoGraphMeshActor::StaticClass()->GetFName(), FZenoGraphMeshActorDetailCustomization);
