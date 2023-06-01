#pragma once

UENUM(BlueprintType)
enum class EWavefrontParseError : uint8
{
	Success = 0,
	BadVertexData,
	BadVertexTextureData,
	BadFaceData,
};

UENUM(BlueprintType)
enum class EWavefrontAttrType : uint8
{
	Invalid = 0,
	Vertex,
	VertexTexture,
	VertexNormal,
	Face,
	MetadataWidth,
	MetadataRowsPerFrame,
	MetadataFrameNum,
	MetadataBMin,
	MetadataBMax,
	Max,
};

struct FWavefrontUtilities
{
	inline static const TMap<EWavefrontAttrType, FString> LabelMap {
		{EWavefrontAttrType::Invalid, "_"},
		{EWavefrontAttrType::VertexNormal, "vn"},
		{EWavefrontAttrType::Vertex, "v"},
		{EWavefrontAttrType::VertexTexture, "vt"},
		{EWavefrontAttrType::Face, "f"},
		{EWavefrontAttrType::MetadataWidth, "# metadata VATWidth"},
		{EWavefrontAttrType::MetadataRowsPerFrame, "# metadata RowsPerFrame"},
		{EWavefrontAttrType::MetadataFrameNum, "# metadata FrameNum"},
		{EWavefrontAttrType::MetadataBMin, "# metadata BMin"},
		{EWavefrontAttrType::MetadataBMax, "# metadata BMax"},
	};
};
