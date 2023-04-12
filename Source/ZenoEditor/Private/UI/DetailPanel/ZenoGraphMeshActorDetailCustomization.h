#pragma once
#include <zeno/unreal/ZenoUnrealTypes.h>

#include "IDetailCustomization.h"

class AZenoGraphMeshActor;
class SZenoGenericInputBox;

class FZenoGraphMeshActorDetailCustomization : public IDetailCustomization
{
public:
	FZenoGraphMeshActorDetailCustomization();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	
	static TSharedRef<IDetailCustomization> Create();

private:
	void BuildParamList_Slate(const zeno::unreal::SubnetNodeParamList& InParamList);
	FReply DoMeshGenerate(AZenoGraphMeshActor* TargetActor) const;

private:
	TMap<FName, zeno::unreal::EParamType> ParamTypeMap;
	TMap<FName, TSharedRef<SZenoGenericInputBox>> Slate_ParamInputBox;
	TSharedPtr<SVerticalBox> Slate_InputPanel;

	inline static FCriticalSection CriticalSection {};
};
