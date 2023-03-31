#pragma once

enum class EWavefrontParseError : uint8
{
	Success = 0,
	BadVertexData,
	BadVertexTextureData,
	BadFaceData,
};

enum class EWavefrontAttrType : uint8
{
	Invalid = 0,
	Vertex,
	VertexTexture,
	Face,
	Max,
};

struct FWavefrontUtilities
{
	inline static const TMap<EWavefrontAttrType, FString> LabelMap {
		{EWavefrontAttrType::Invalid, "_"},
		{EWavefrontAttrType::Vertex, "v"},
		{EWavefrontAttrType::VertexTexture, "vt"},
		{EWavefrontAttrType::Face, "f"}
	};
};

