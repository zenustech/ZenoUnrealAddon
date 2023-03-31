#pragma once
#include "ZenoObjTypes.h"

enum class EWavefrontParseError : uint8;

class ZENOEDITOR_API FWavefrontFileParser : public TSharedFromThis<FWavefrontFileParser>
{
public:
	explicit FWavefrontFileParser(const TArray<FString>& InContent);

	/**
	 * Try to parse current context.
	 * Return a ['FRawMesh'] instance if no errors.
	 */
	TSharedPtr<FRawMesh> Parse(EWavefrontParseError& OutError) const;

protected:
	static EWavefrontAttrType GetLabelFromLine(const FString& InLine, FString& OutLineData);
	
private:
	TArray<FString> Lines;
};
