// Fill out your copyright notice in the Description page of Project Settings.


#include "Client/ZenoLiveLinkClientSubsystem.h"

#include "RawMesh.h"
#include "Client/ZenoLiveLinkSession.h"

void UZenoLiveLinkClientSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
}

void UZenoLiveLinkClientSubsystem::Deinitialize()
{
}

FZenoLiveLinkSetting& UZenoLiveLinkClientSubsystem::GetZenoLiveLinkSetting()
{
	return ConnectionSetting;
}

const FZenoLiveLinkSetting& UZenoLiveLinkClientSubsystem::GetZenoLiveLinkSetting() const
{
	return ConnectionSetting;
}

FGuid UZenoLiveLinkClientSubsystem::NewSession(const FZenoLiveLinkSetting& InConnectionSetting)
{
	const FGuid NewId = FGuid::NewGuid();
	UZenoLiveLinkSession* Session = UZenoLiveLinkSession::CreateSession(InConnectionSetting, NewId);
	ConnectionSessions.Add(NewId, Session);
	return NewId;
}

bool UZenoLiveLinkClientSubsystem::RequestCloseSession(FGuid InGuid)
{
	if (!ConnectionSessions.Contains(InGuid))
	{
		return true;
	}
	// TODO [darc] : sending stop session request to zeno :
	ConnectionSessions.Remove(InGuid);
	return true;
}

UZenoLiveLinkSession* UZenoLiveLinkClientSubsystem::GetSession(FGuid InGuid)
{
	if (ConnectionSessions.Contains(InGuid))
	{
		return ConnectionSessions[InGuid];
	}
	return nullptr;
}

UZenoLiveLinkSession* UZenoLiveLinkClientSubsystem::FindSessionWithSubject(const FName& InName)
{
	for (const auto& Pair : ConnectionSessions)
	{
		if (IsValid(Pair.Value) && Pair.Value->IsInitialized() && Pair.Value->HasSubject(InName.ToString()))
		{
			return Pair.Value;
		}
	}
	return nullptr;
}

UZenoLiveLinkSession* UZenoLiveLinkClientSubsystem::GetSessionFallback()
{
	if (ConnectionSessions.IsEmpty()) return nullptr;
	return ConnectionSessions.begin()->Value;
}

FRawMesh UZenoLiveLinkClientSubsystem::ConvertZenoMeshToRawMesh(const zeno::remote::Mesh& InZenoMesh)
{
	FRawMesh RawMesh;

	// Fill vertices and triangles
	RawMesh.VertexPositions.Reserve(InZenoMesh.vertices.size());
	RawMesh.WedgeIndices.Reserve(InZenoMesh.triangles.size());
	for (const auto& Vertex : InZenoMesh.vertices)
	{
		const auto& X = Vertex[0];
		const auto& Y = Vertex[1];
		const auto& Z = Vertex[2];
		RawMesh.VertexPositions.Add( { X.data(), Y.data(), Z.data() });
	}
	for (const auto& Triangle : InZenoMesh.triangles)
	{
		RawMesh.WedgeIndices.Add(Triangle[0]);
		RawMesh.WedgeIndices.Add(Triangle[1]);
		RawMesh.WedgeIndices.Add(Triangle[2]);
	}
	const SIZE_T NumFace = InZenoMesh.triangles.size();
	const SIZE_T NumWedge = RawMesh.WedgeIndices.Num();
	// TODO [darc] : check if we need to fill other data like normals, tangents, UVs, colors, etc. :
	// TODO [darc] : check NumFace == NumWedge / 3 :
	// Leave other data empty for now. Set zeroed to bypass check.
	RawMesh.FaceMaterialIndices.SetNumZeroed(NumFace);
	RawMesh.FaceSmoothingMasks.SetNumZeroed(NumFace);
	RawMesh.WedgeTangentX.SetNumZeroed(NumWedge);
	RawMesh.WedgeTangentY.SetNumZeroed(NumWedge);
	RawMesh.WedgeTangentZ.SetNumZeroed(NumWedge);
	RawMesh.WedgeColors.SetNumZeroed(NumWedge);
	RawMesh.WedgeTexCoords[0].SetNumZeroed(NumWedge);

	return RawMesh;
}
