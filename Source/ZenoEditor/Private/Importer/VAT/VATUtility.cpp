// Fill out your copyright notice in the Description page of Project Settings.


#include "Importer/VAT/VATUtility.h"

#include "Misc/FileHelper.h"

bool UVATUtility::ParseBinaryInfo(const FString& InFilePath, FVATInfo& OutInfo)
{
	TArray<uint8> BinaryBuffer;
	FFileHelper::LoadFileToArray(BinaryBuffer, *InFilePath);
	constexpr size_t VATInfoSize = sizeof(FVATInfo) - sizeof(FVATInfo::FrameVertexNum);
	if (BinaryBuffer.Num() < VATInfoSize)
	{
		return false;
	}
	FMemory::Memcpy(&OutInfo, BinaryBuffer.GetData(), VATInfoSize);
	const size_t VertexPerFrameSize = OutInfo.FrameNum * sizeof(int32);
	if (VertexPerFrameSize + VATInfoSize > BinaryBuffer.Num())
	{
		return false;
	}
	int32* VertexPerFrameArray = static_cast<int32*>(FMemory::Malloc(VertexPerFrameSize));
	FMemory::Memcpy(VertexPerFrameArray, BinaryBuffer.GetData() + VATInfoSize, VertexPerFrameSize);
	OutInfo.FrameVertexNum = VertexPerFrameArray;
	
	return true;
}
