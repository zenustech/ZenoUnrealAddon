#include "Importer/Wavefront/ZenoObjLoader.h"

#include "RawMesh.h"
#include "Blueprint/ZenoEditorSettings.h"
#include "Internationalization/Regex.h"
#include <regex>

FWavefrontObjectContext::FWavefrontObjectContext(const FWavefrontObjectContextCreateArgs& InArgs/* = FCreateArgs() **/)
{
	const FVector3f& Scale = GetDefault<UZenoEditorSettings>()->VATImportScale;
	ImportScale = Scale;
	bIsUpwardY = InArgs.bIsUpwardY;
	bTreatingNormalAsUV = InArgs.bTreatingNormalAsUV;
}

void FWavefrontObjectContext::Parse(EWavefrontAttrType InType, const FString& InData)
{
	if (bIsCompleted)
	{
		return;
	}
	
	if (InType == EWavefrontAttrType::Face)
	{
		ParseLine<EWavefrontAttrType::Face>(InData);
	}
	else if (InType == EWavefrontAttrType::Vertex)
	{
		ParseLine<EWavefrontAttrType::Vertex>(InData);
	}
	else if (InType == EWavefrontAttrType::VertexNormal)
	{
		ParseLine<EWavefrontAttrType::VertexNormal>(InData);
	}
	else if (InType == EWavefrontAttrType::VertexTexture)
	{
		ParseLine<EWavefrontAttrType::VertexTexture>(InData);
	}
	else if (InType == EWavefrontAttrType::MetadataWidth)
	{
		ParseLine<EWavefrontAttrType::MetadataWidth>(InData);
	}
	else if (InType == EWavefrontAttrType::MetadataRowsPerFrame)
	{
		ParseLine<EWavefrontAttrType::MetadataRowsPerFrame>(InData);
	}
	else if (InType == EWavefrontAttrType::MetadataFrameNum)
	{
		ParseLine<EWavefrontAttrType::MetadataFrameNum>(InData);
	}
	else if (InType == EWavefrontAttrType::MetadataBMax)
	{
		ParseLine<EWavefrontAttrType::MetadataBMax>(InData);
	}
	else if (InType == EWavefrontAttrType::MetadataBMin)
	{
		ParseLine<EWavefrontAttrType::MetadataBMin>(InData);
	}
	else
	{
		ParseLine<EWavefrontAttrType::Invalid>(InData);
	}
}

void FWavefrontObjectContext::CompleteParse()
{
	// Fill uv channel from map
	UVChannel0.Reset();
	UVChannel0.AddZeroed(FaceBuffer.Num());
	if (bTreatingNormalAsUV)
	{
		UVChannel1.Reset();
		UVChannel1.AddZeroed(FaceBuffer.Num());
	}
	else
	{
		NormalChannel.Reset();
		NormalChannel.AddZeroed(FaceBuffer.Num());
	}
	
	for (const TPair<int32, uint32>& Pair : FaceIdToUVIndex[0])
	{
		UVChannel0[Pair.Key] = UVBuffer0[Pair.Value];
		if (bTreatingNormalAsUV)
		{
			UVChannel1[Pair.Key] = UVBuffer1[Pair.Value];
		}
		else
		{
			NormalChannel[Pair.Key] = NormalBuffer[Pair.Value];
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("qwq: %d / %d"), VertexBuffer.Num(), FaceBuffer.Num());
	
	bIsCompleted = true;
}

TSharedRef<FRawMesh> FWavefrontObjectContext::ToRawMesh() const
{
	TSharedRef<FRawMesh> RawMesh = MakeShared<FRawMesh>();

	RawMesh->VertexPositions = VertexBuffer;
	RawMesh->WedgeIndices.Reserve(FaceBuffer.Num());
	for (const int32 Face : FaceBuffer)
	{
		RawMesh->WedgeIndices.Add(static_cast<uint32>(Face));
	}
	RawMesh->WedgeTexCoords[0] = UVChannel0;
	if (RawMesh->WedgeTexCoords[0].Num() == 0)
	{
		RawMesh->WedgeTexCoords[0].AddZeroed(RawMesh->WedgeIndices.Num());
	}
	if (bTreatingNormalAsUV)
	{
		RawMesh->WedgeTexCoords[1] = UVChannel1;
	}
	else
	{
		RawMesh->WedgeTangentZ = NormalChannel;
	}

	const uint32 EdgeNum = RawMesh->WedgeIndices.Num();
	
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

	return RawMesh;
}

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::Vertex>(const FString& Data)
{
	TArray<FString> TempArr;
	Data.ParseIntoArray(TempArr, TEXT(" "), true);
	if (TempArr.Num() != 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skipping invalid vertex data: %s"), *Data);
		return;
	}
	
	float X = FCString::Atof(*TempArr[0]);
	float Y = FCString::Atof(*TempArr[1]);
	float Z = FCString::Atof(*TempArr[2]);
	if (bIsUpwardY)
	{
		Swap(Y, Z);
	}
	
	if (!ImportScale.IsNearlyZero())
	{
		X *= ImportScale.X;
		Y *= ImportScale.Y;
		Z *= ImportScale.Z;
	}
	
	VertexBuffer.Emplace(X, Y, Z);
}

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::Face>(const FString& Data)
{
	// Code to auto detecting face format
	// Just check following format:
	// v1 v2 v3
	const std::regex FaceFormat1("^(\\d+)\\s+(\\d+)\\s+(\\d+)$");
	// v1/vt1 v2/vt2 v3/vt3
	const std::regex FaceFormat2("^(\\d+)\\/(\\d+)\\s+(\\d+)\\/(\\d+)\\s+(\\d+)\\/(\\d+)$");
	// v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
	const std::regex FaceFormat3("^(\\d+)\\/(\\d+)\\/(\\d+)\\s+(\\d+)\\/(\\d+)\\/(\\d+)\\s+(\\d+)\\/(\\d+)\\/(\\d+)$");

	std::smatch Matches;
	const std::string TestStr(TCHAR_TO_ANSI(*Data));
	
	if (std::regex_search(TestStr, Matches, FaceFormat1))
	{
		const int32 VertexIndex1 = FCString::Atoi(ANSI_TO_TCHAR(Matches[1].str().c_str()));
		const int32 VertexIndex2 = FCString::Atoi(ANSI_TO_TCHAR(Matches[2].str().c_str()));
		const int32 VertexIndex3 = FCString::Atoi(ANSI_TO_TCHAR(Matches[3].str().c_str()));
		FaceBuffer.Add(VertexIndex1 - 1);
		FaceBuffer.Add(VertexIndex2 - 1);
		FaceBuffer.Add(VertexIndex3 - 1);
	}
	else if (std::regex_search(TestStr, Matches, FaceFormat2))
	{
		const int32 VertexIndex1 = FCString::Atoi(ANSI_TO_TCHAR(Matches[1].str().c_str()));
		const int32 VertexIndex2 = FCString::Atoi(ANSI_TO_TCHAR(Matches[3].str().c_str()));
		const int32 VertexIndex3 = FCString::Atoi(ANSI_TO_TCHAR(Matches[5].str().c_str()));
		const uint32 FaceId1 = FaceBuffer.Add(VertexIndex1 - 1);
		const uint32 FaceId2 = FaceBuffer.Add(VertexIndex2 - 1);
		const uint32 FaceId3 = FaceBuffer.Add(VertexIndex3 - 1);
		const int32 UVIndex1 = FCString::Atoi(ANSI_TO_TCHAR(Matches[2].str().c_str()));
		const int32 UVIndex2 = FCString::Atoi(ANSI_TO_TCHAR(Matches[4].str().c_str()));
		const int32 UVIndex3 = FCString::Atoi(ANSI_TO_TCHAR(Matches[6].str().c_str()));
		FaceIdToUVIndex[0].Add(FaceId1, UVIndex1 - 1);
		FaceIdToUVIndex[0].Add(FaceId2, UVIndex2 - 1);
		FaceIdToUVIndex[0].Add(FaceId3, UVIndex3 - 1);
	}
	else if (std::regex_search(TestStr, Matches, FaceFormat3))
	{
		const int32 VertexIndex1 = FCString::Atoi(ANSI_TO_TCHAR(Matches[1].str().c_str()));
		const int32 VertexIndex2 = FCString::Atoi(ANSI_TO_TCHAR(Matches[4].str().c_str()));
		const int32 VertexIndex3 = FCString::Atoi(ANSI_TO_TCHAR(Matches[7].str().c_str()));
		const uint32 FaceId1 = FaceBuffer.Add(VertexIndex1 - 1);
		const uint32 FaceId2 = FaceBuffer.Add(VertexIndex2 - 1);
		const uint32 FaceId3 = FaceBuffer.Add(VertexIndex3 - 1);
		const int32 UVIndex1 = FCString::Atoi(ANSI_TO_TCHAR(Matches[2].str().c_str()));
		const int32 UVIndex2 = FCString::Atoi(ANSI_TO_TCHAR(Matches[5].str().c_str()));
		const int32 UVIndex3 = FCString::Atoi(ANSI_TO_TCHAR(Matches[8].str().c_str()));
		FaceIdToUVIndex[0].Add(FaceId1, UVIndex1 - 1);
		FaceIdToUVIndex[0].Add(FaceId2, UVIndex2 - 1);
		FaceIdToUVIndex[0].Add(FaceId3, UVIndex3 - 1);
		const int32 NormalIndex1 = FCString::Atoi(ANSI_TO_TCHAR(Matches[3].str().c_str()));
		const int32 NormalIndex2 = FCString::Atoi(ANSI_TO_TCHAR(Matches[6].str().c_str()));
		const int32 NormalIndex3 = FCString::Atoi(ANSI_TO_TCHAR(Matches[9].str().c_str()));
		if (bTreatingNormalAsUV)
		{
			FaceIdToUVIndex[0].Add(FaceId1, NormalIndex1 - 1);
			FaceIdToUVIndex[0].Add(FaceId2, NormalIndex2 - 1);
			FaceIdToUVIndex[0].Add(FaceId3, NormalIndex3 - 1);
		} else
		{
			FaceIdToNormalIndex.Add(FaceId1, NormalIndex1 - 1);
			FaceIdToNormalIndex.Add(FaceId2, NormalIndex2 - 1);
			FaceIdToNormalIndex.Add(FaceId3, NormalIndex3 - 1);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Skipping invalid face data: %s"), *Data);
	}
}

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::VertexTexture>(const FString& Data)
{
	const std::regex UVFormat("^([+-]?(\\d*\\.)?\\d+)\\s+([+-]?(\\d*\\.)?\\d+)(\\s+([+-]?(\\d*\\.)?\\d+))?$");
	std::smatch Matches;
	const std::string TestStr(TCHAR_TO_ANSI(*Data));
	if (std::regex_search(TestStr, Matches, UVFormat))
	{
		const float U = FCString::Atof(ANSI_TO_TCHAR(Matches[1].str().c_str()));
		const float V = FCString::Atof(ANSI_TO_TCHAR(Matches[3].str().c_str()));
		// Not use of W component for now
		// const float W = FCString::Atof(*Matcher.GetCaptureGroup(5));
		UVBuffer0.Add(FVector2f(U, V));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Skipping invalid uv data: %s"), *Data);
	}
}

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::VertexNormal>(const FString& Data)
{
	const std::regex NormalFormat("^([+-]?(\\d*\\.)?\\d+)\\s+([+-]?(\\d*\\.)?\\d+)\\s+([+-]?(\\d*\\.)?\\d+)$");
	std::smatch Matches;
	const std::string TestStr(TCHAR_TO_ANSI(*Data));
	if (std::regex_search(TestStr, Matches, NormalFormat))
	{
		const float X = FCString::Atof(ANSI_TO_TCHAR(Matches[1].str().c_str()));
		const float Y = FCString::Atof(ANSI_TO_TCHAR(Matches[3].str().c_str()));
		const float Z = FCString::Atof(ANSI_TO_TCHAR(Matches[5].str().c_str()));
		if (bTreatingNormalAsUV)
		{
			UVBuffer1.Add(FVector2f(X, Y));
		}
		else
		{
			NormalBuffer.Add(FVector3f(X, Y, Z));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Skipping invalid normal data: %s"), *Data);
	}
}

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::MetadataWidth>(const FString& Data)
{
	Metadata.TextureWidth = FCString::Atoi(*Data);
}

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::MetadataRowsPerFrame>(const FString& Data)
{
	Metadata.RowsPerFrame = FCString::Atoi(*Data);
}

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::MetadataFrameNum>(const FString& Data)
{
	Metadata.FrameNum = FCString::Atoi(*Data);
}

FVector3f StringToVector3F(const FString& InString)
{
	TArray<FString> TempArr;
	InString.ParseIntoArray(TempArr, TEXT(" "), true);
	if (TempArr.Num() != 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skipping invalid vector data: %s"), *InString);
		return FVector3f::Zero();
	}
	return FVector3f(FCString::Atof(*TempArr[0]), FCString::Atof(*TempArr[1]), FCString::Atof(*TempArr[2]));
}

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::MetadataBMax>(const FString& Data)
{
	Metadata.BoundsMax = StringToVector3F(Data);
}

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::MetadataBMin>(const FString& Data)
{
	Metadata.BoundsMin = StringToVector3F(Data);
}

FWavefrontFileParser::FWavefrontFileParser(const TArray<FString>& InContent)
{
	Lines.Empty();
	Lines.Append(InContent);
}
	

TSharedPtr<FRawMesh> FWavefrontFileParser::ParseDynamicMesh(EWavefrontParseError& OutError) const
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
			const FVector3f& Scale = GetDefault<UZenoEditorSettings>()->VATImportScale;
			float X = FCString::Atof(*TempArr[0]) * Scale.X;
			float Y = FCString::Atof(*TempArr[2]) * Scale.Y;
			float Z = FCString::Atof(*TempArr[1]) * Scale.Z;
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
	
	return RawMesh;
}

void FWavefrontFileParser::ParseFile(const FZenoWavefrontObjectParserDelegate& Delegate) const
{
	for (const FString& Line : Lines)
	{
		FString Data;
		TArray<FString> TempArr;
		const EWavefrontAttrType Type = GetLabelFromLine(Line, Data);
		if (!Delegate.ExecuteIfBound(Type, Data))
		{
			break;
		}
	}
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
