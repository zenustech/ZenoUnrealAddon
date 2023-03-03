#pragma once
#include "LandscapeFileFormatInterface.h"

class FLandscapeHeightmapFileFormatZeno_Virtual : public ILandscapeHeightmapFileFormat
{
public:
	FLandscapeHeightmapFileFormatZeno_Virtual();
	
	virtual const FLandscapeFileTypeInfo& GetInfo() const override;
	virtual FLandscapeFileInfo Validate(const TCHAR* HeightmapFilename, FName LayerName) const override;
	virtual FLandscapeImportData<uint16> Import(const TCHAR* HeightmapFilename, FName LayerName, FLandscapeFileResolution ExpectedResolution) const override;
	virtual void Export(const TCHAR* HeightmapFilename, FName LayerName, TArrayView<const uint16> Data, FLandscapeFileResolution DataResolution, FVector Scale) const override;
	
private:
	FLandscapeFileTypeInfo FileTypeInfo;
};
