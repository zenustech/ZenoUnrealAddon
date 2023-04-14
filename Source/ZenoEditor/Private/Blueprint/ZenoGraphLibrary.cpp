﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprint/ZenoGraphLibrary.h"

#include <system_error>
#include <zeno/core/Graph.h>
#include <zeno/unreal/UnrealTool.h>

#include "ZenoGraphAsset.h"
#include "ThirdParty/msgpack.h"

std::shared_ptr<zeno::Graph> UZenoGraphLibrary::GetGraphFromJson(const char* Json)
{
	auto Graph = zeno::getSession().createGraph();

	ensure(zeno::LoadGraphChecked(Graph.get(), Json));
	
	return Graph;
}

bool UZenoGraphLibrary::IsValidGraph(UZenoGraphAsset* Graph)
{
	return IsValid(Graph) && zeno::IsValidZSL(TCHAR_TO_ANSI(* Graph->ZenoActionRecordExportedData));
}

zeno::unreal::SubnetNodeParamList  UZenoGraphLibrary::GetGraphParamList(UZenoGraphAsset* Graph)
{
	ensure(IsValidGraph(Graph));

	const std::shared_ptr<zeno::Graph> ZenoGraph = zeno::getSession().createGraph();
	ensure(zeno::LoadGraphChecked(ZenoGraph.get(), TCHAR_TO_ANSI(*Graph->ZenoActionRecordExportedData)));

	std::error_code Err;
	zeno::SimpleCharBuffer MsgBuffer = zeno::GetGraphInputParams(ZenoGraph.get());
	zeno::unreal::SubnetNodeParamList Params = msgpack::unpack<zeno::unreal::SubnetNodeParamList>(reinterpret_cast<uint8*>(MsgBuffer.data), MsgBuffer.length - 1, Err);
	ensure(!Err);

	return Params;
}