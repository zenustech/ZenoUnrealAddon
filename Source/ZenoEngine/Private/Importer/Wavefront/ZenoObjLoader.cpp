#include "Importer/Wavefront/ZenoObjLoader.h"

#include "MeshUtilities.h"
#include "RawMesh.h"
#include "Chaos/Pair.h"

FWavefrontFileParser::FWavefrontFileParser(const TArray<FString>& InContent)
{
	Lines.Empty();
	Lines.Append(InContent);
}

TSharedPtr<FRawMesh> FWavefrontFileParser::Parse(EWavefrontParseError& OutError) const
{
	TSharedRef<FRawMesh> RawMesh = MakeShared<FRawMesh>();
	OutError = EWavefrontParseError::Success;

	TArray<FVector2f> UVIndex;
	TMap<uint32, uint32> EdgeIdToUVIndex;
	
	for (const FString& Line : Lines)
	{
		FString Data;
		TArray<FString> TempArr;
		const EWavefrontAttrType Type = GetLabelFromLine(Line, Data);
		if (Type == EWavefrontAttrType::Invalid)
		{
			continue;
		}
		Data.ParseIntoArray(TempArr, TEXT(" "), true);
		if (Type == EWavefrontAttrType::Vertex)
		{
			if (TempArr.Num() != 3)
			{
				OutError = EWavefrontParseError::BadVertexData;
				return nullptr;
			}
			float X = FCString::Atof(*TempArr[0]);
			float Y = FCString::Atof(*TempArr[1]);
			float Z = FCString::Atof(*TempArr[2]);
			RawMesh->VertexPositions.Add({ X, Y, Z });
		} else if (Type == EWavefrontAttrType::VertexTexture)
		{
			if (TempArr.Num() != 2)
			{
				OutError = EWavefrontParseError::BadVertexTextureData;
				return nullptr;
			}
			float U = FCString::Atof(*TempArr[0]);
			float V = FCString::Atof(*TempArr[1]);
			UVIndex.Add({ U, V });
		} else if (Type == EWavefrontAttrType::Face)
		{
			if (TempArr.Num() != 3)
			{
				OutError = EWavefrontParseError::BadFaceData;
				return nullptr;
			}
			TArray<FString> VertexObject0, VertexObject1, VertexObject2;
			TempArr[0].ParseIntoArray(VertexObject0, TEXT("/"), true);
			TempArr[1].ParseIntoArray(VertexObject1, TEXT("/"), true);
			TempArr[2].ParseIntoArray(VertexObject2, TEXT("/"), true);
			if (VertexObject0.Num() != 2 || VertexObject1.Num() != 2 || VertexObject2.Num() != 2)
			{
				OutError = EWavefrontParseError::BadFaceData;
				return nullptr;
			}
			const uint32 VertexIndex0 = FCString::Atoi(*VertexObject0[0]);
			const uint32 VertexIndex1 = FCString::Atoi(*VertexObject1[0]);
			const uint32 VertexIndex2 = FCString::Atoi(*VertexObject2[0]);
			const uint32 TextureIndex0 = FCString::Atoi(*VertexObject0[1]);
			const uint32 TextureIndex1 = FCString::Atoi(*VertexObject1[1]);
			const uint32 TextureIndex2 = FCString::Atoi(*VertexObject2[1]);
			uint32 EdgeId0 = RawMesh->WedgeIndices.Add(VertexIndex0 - 1);
			uint32 EdgeId1 = RawMesh->WedgeIndices.Add(VertexIndex1 - 1);
			uint32 EdgeId2 = RawMesh->WedgeIndices.Add(VertexIndex2 - 1);
			// UV might load after face data
			// Insert it later
			EdgeIdToUVIndex.Add(EdgeId0, TextureIndex0 - 1);
			EdgeIdToUVIndex.Add(EdgeId1, TextureIndex1 - 1);
			EdgeIdToUVIndex.Add(EdgeId2, TextureIndex2 - 1);
		}
	}

	const uint32 EdgeNum = RawMesh->WedgeIndices.Num();
	RawMesh->WedgeTexCoords[0].SetNumZeroed(EdgeNum);
	for (const auto& Pair : EdgeIdToUVIndex)
	{
		RawMesh->WedgeTexCoords[0][Pair.Key] = UVIndex[Pair.Value];
	}

	// IMeshUtilities& MeshUtilities = FModuleManager::LoadModuleChecked<IMeshUtilities>("MeshUtilities");
	// ETangentOptions::Type TangentOptions = static_cast<ETangentOptions::Type>(ETangentOptions::BlendOverlappingNormals | ETangentOptions::UseMikkTSpace);
	// TArray<uint32> SmoothGroup;
	// SmoothGroup.SetNumZeroed(EdgeNum);
	// TArray<FVector3f> TangentX, TangentY, Normals;
	// MeshUtilities.CalculateTangents(RawMesh->VertexPositions, RawMesh->WedgeIndices, RawMesh->WedgeTexCoords[0], SmoothGroup, TangentOptions, TangentX, TangentY, Normals);
	// RawMesh->WedgeTangentX.Append(MoveTempIfPossible(TangentX));
	// RawMesh->WedgeTangentY.Append(MoveTempIfPossible(TangentY));
	RawMesh->WedgeTangentX.SetNumUninitialized(EdgeNum);
	RawMesh->WedgeTangentY.SetNumUninitialized(EdgeNum);
	RawMesh->WedgeTangentZ.SetNumUninitialized(EdgeNum);
	RawMesh->WedgeColors.SetNumUninitialized(EdgeNum);
	for (size_t Idx = 0; Idx < EdgeNum; ++Idx)
	{
		RawMesh->WedgeTangentX[Idx] = { 1.0f, .0f, .0f};
		RawMesh->WedgeTangentY[Idx] = { .0f, 1.0f, .0f};
		RawMesh->WedgeTangentZ[Idx] = { .0f, .0f, 1.0f};
		RawMesh->WedgeColors[Idx] = FColor::Blue;
	}

	RawMesh->FaceMaterialIndices.SetNumZeroed(EdgeNum / 3);
	RawMesh->FaceSmoothingMasks.SetNumZeroed(EdgeNum / 3);
	
	return RawMesh.ToSharedPtr();
}

EWavefrontAttrType FWavefrontFileParser::GetLabelFromLine(const FString& InLine, FString& OutLineData)
{
	const FString TrimLine = InLine.TrimStart();
	FString ResultData;
	EWavefrontAttrType ResultType =  EWavefrontAttrType::Invalid;
	for (const auto& Pair : FWavefrontUtilities::LabelMap)
	{
		if (TrimLine.StartsWith(Pair.Value) && Pair.Value.Len() >= FWavefrontUtilities::LabelMap[ResultType].Len())
		{
			ResultData = TrimLine.Replace(*Pair.Value, TEXT("")).TrimStartAndEnd();
			ResultType = Pair.Key;
		}
	}

	if (ResultType != EWavefrontAttrType::Invalid)
	{
		OutLineData = ResultData;
	}
	
	return ResultType;
}
