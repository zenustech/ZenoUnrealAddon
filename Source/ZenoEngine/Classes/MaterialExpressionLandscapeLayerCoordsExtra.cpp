// Zenustech © All Right Reserved


#include "MaterialExpressionLandscapeLayerCoordsExtra.h"

#include "MaterialCompiler.h"

#define LOCTEXT_NAMESPACE "UMaterialExpressionLandscapeLayerCoordsExtra"

UMaterialExpressionLandscapeLayerCoordsExtra::UMaterialExpressionLandscapeLayerCoordsExtra(
	const FObjectInitializer& ObjectInitializer)
	: UMaterialExpressionLandscapeLayerCoords(ObjectInitializer)
{
	static FText STRING_Zeno = LOCTEXT("Zeno", "Zeno");
	
	MenuCategories.Add(STRING_Zeno);
}

#if WITH_EDITOR
int32 UMaterialExpressionLandscapeLayerCoordsExtra::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{
	int32 ScaleInput = ScaleInputNode.IsConnected() ? ScaleInputNode.Compile(Compiler) : Compiler->Constant(1.0f);
	
	switch (CustomUVType)
	{
	case LCCT_CustomUV0:
		return Compiler->TextureCoordinate(0, false, false);
	case LCCT_CustomUV1:
		return Compiler->TextureCoordinate(1, false, false);
	case LCCT_CustomUV2:
		return Compiler->TextureCoordinate(2, false, false);
	case LCCT_WeightMapUV:
		return Compiler->TextureCoordinate(3, false, false);
	default:
		break;
	}

	int32 BaseUV;

	switch (MappingType)
	{
	case TCMT_Auto:
	case TCMT_XY: BaseUV = Compiler->TextureCoordinate(0, false, false); break;
	case TCMT_XZ: BaseUV = Compiler->TextureCoordinate(1, false, false); break;
	case TCMT_YZ: BaseUV = Compiler->TextureCoordinate(2, false, false); break;
	default: UE_LOG(LogTemp, Fatal, TEXT("Invalid mapping type %u"), (uint8)MappingType); return INDEX_NONE;
	};

	// float Scale = (MappingScale == 0.0f) ? 1.0f : 1.0f / MappingScale;
	// int32 RealScale = Compiler->Constant(Scale);
	int32 Scale = Compiler->Div(Compiler->Constant(1.0f), Compiler->Max(Compiler->Constant(1.0f), ScaleInput));
	int32 RealScale = Scale;

	const float Cos = FMath::Cos(MappingRotation * PI / 180.0f);
	const float Sin = FMath::Sin(MappingRotation * PI / 180.0f);

	int32 TransformedUV = Compiler->Add(
		Compiler->Mul(RealScale,
		Compiler->AppendVector(
		Compiler->Dot(BaseUV, Compiler->Constant2(+Cos, +Sin)),
		Compiler->Dot(BaseUV, Compiler->Constant2(-Sin, +Cos)))
		),
		Compiler->Constant2(MappingPanU, MappingPanV)
		);
	
	return TransformedUV;
}

FExpressionInput* UMaterialExpressionLandscapeLayerCoordsExtra::GetInput(int32 InputIndex)
{
	return &ScaleInputNode;
}

const TArray<FExpressionInput*> UMaterialExpressionLandscapeLayerCoordsExtra::GetInputs()
{
	TArray<FExpressionInput*> OutInputs;
	OutInputs.Add(&ScaleInputNode);
	return OutInputs;
}

FName UMaterialExpressionLandscapeLayerCoordsExtra::GetInputName(int32 InputIndex) const
{
	return FName("Scale");
}

uint32 UMaterialExpressionLandscapeLayerCoordsExtra::GetInputType(int32 InputIndex)
{
	return MCT_Float;
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE
