#pragma once
#include "ZenoObjTypes.h"
#include "ZenoObjLoader.generated.h"

enum class EWavefrontParseError : uint8;

DECLARE_DELEGATE_TwoParams(FZenoWavefrontObjectParserDelegate, const EWavefrontAttrType&, const FString&);

USTRUCT(BlueprintType)
struct ZENOEDITOR_API FWavefrontObjectCustomMetadata
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = Context)
	int32 TextureWidth = 0;

	UPROPERTY(BlueprintReadOnly, Category = Context)
	int32 FrameNum = 0;

	UPROPERTY(BlueprintReadOnly, Category = Context)
	int32 RowsPerFrame = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = Context)
	FVector3f BoundsMin = FVector3f::Zero();
	
	UPROPERTY(BlueprintReadOnly, Category = Context)
	FVector3f BoundsMax = FVector3f::Zero();
};

USTRUCT(BlueprintType)
struct ZENOEDITOR_API FWavefrontObjectContextCreateArgs
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Properties)
	bool bTreatingNormalAsUV = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Properties)
	bool bIsUpwardY = false;
};



USTRUCT(BlueprintType)
struct ZENOEDITOR_API FWavefrontObjectContext
{
	GENERATED_BODY()
	
	explicit FWavefrontObjectContext(const FWavefrontObjectContextCreateArgs& InArgs = FWavefrontObjectContextCreateArgs());

	/** Vertex positions. Array[WedgeId]=float3(x,y,z) */
	UPROPERTY(BlueprintReadOnly, Category = Context)
	TArray<FVector> VertexBuffer;
	/** Texture coordinates. Array[WedgeId]=float2(u,v) */
	UPROPERTY(BlueprintReadOnly, Category = Context)
	TArray<FVector2f> UVChannel0;
	/** UV from vn attr (if enable bTreatingNormalAsUV) */
	UPROPERTY(BlueprintReadOnly, Category = Context)
	TArray<FVector2f> UVChannel1;
	/** Vertex normals. Array[WedgeId]=float3(x,y,z) */
	UPROPERTY(BlueprintReadOnly, Category = Context)
	TArray<FVector3f> NormalChannel;
	/** Index of the vertex at this wedge. Array[FaceId] = VertexId */
	UPROPERTY(BlueprintReadOnly, Category = Context)
	TArray<int32> FaceBuffer;
	UPROPERTY(BlueprintReadOnly, Category = Context)
	FWavefrontObjectCustomMetadata Metadata;

	// Properties
	/** Scale apply to vertices while import */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
	FVector3f ImportScale = FVector3f::Zero();
	/** Flip Y axis while import */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
	bool bIsUpwardY = false;
	/** Parse normal into uv1 channel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
	bool bTreatingNormalAsUV = false;

	template <EWavefrontAttrType T>
	void ParseLine(const FString& Data);

	void Parse(EWavefrontAttrType InType, const FString& InData);

	void CompleteParse();

private:
	TArray<FVector2f> UVBuffer0;
	TArray<FVector2f> UVBuffer1;
	TArray<FVector3f> NormalBuffer;
	TMap<int32, uint32> FaceIdToUVIndex[2];
	TMap<int32, uint32> FaceIdToNormalIndex;
};

	
template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::Vertex>(const FString& Data);

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::Face>(const FString& Data);

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::VertexTexture>(const FString& Data);

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::VertexNormal>(const FString& Data);

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::MetadataWidth>(const FString& Data);

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::MetadataFrameNum>(const FString& Data);

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::MetadataRowsPerFrame>(const FString& Data);

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::MetadataBMax>(const FString& Data);

template <>
void FWavefrontObjectContext::ParseLine<EWavefrontAttrType::MetadataBMin>(const FString& Data);

template <EWavefrontAttrType T>
void FWavefrontObjectContext::ParseLine(const FString& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("Skipping %d data: %s"), T, *Data);
}


class ZENOEDITOR_API FWavefrontFileParser : public TSharedFromThis<FWavefrontFileParser>
{
public:
	explicit FWavefrontFileParser(const TArray<FString>& InContent);

	/**
	 * Try to parse current context.
	 * Return a ['FRawMesh'] instance if no errors.
	 */
	TSharedPtr<FRawMesh> ParseDynamicMesh(EWavefrontParseError& OutError) const;

	/**
	 * @brief Parse each line using specificed delegate
	 * @param Delegate Tag processor
	 */
	void ParseFile(const FZenoWavefrontObjectParserDelegate& Delegate) const;

protected:
	static EWavefrontAttrType GetLabelFromLine(const FString& InLine, FString& OutLineData);
	
private:
	TArray<FString> Lines;
};
