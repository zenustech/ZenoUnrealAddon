// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZenoLiveLinkSession.h"
#include "ZenoLiveLinkTypes.h"
#include "Subsystems/EngineSubsystem.h"
#include "ZenoLiveLinkClientSubsystem.generated.h"

class UZenoLiveLinkSession;
class FZenoLiveLinkSource;

UCLASS()
class ZENOLIVELINKRUNTIME_API UZenoLiveLinkClientSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FZenoLiveLinkSetting& GetZenoLiveLinkSetting();
	const FZenoLiveLinkSetting& GetZenoLiveLinkSetting() const;

	/**
	 * @brief Create a new session connection to zeno.
	 * @param InConnectionSetting information to connect
	 * @return New session id, can index session instance using GetSession(ID)
	 */
	FGuid NewSession(const FZenoLiveLinkSetting& InConnectionSetting);
	/**
	 * @brief Try close session.
	 * @param InGuid Guid of session
	 * @return false if failed.
	 */
	bool RequestCloseSession(FGuid InGuid);
	/**
	 * @brief Get session from guid
	 * @param InGuid guid
	 * @return Session instance. Can be nullptr if there is no session with guid given.
	 */
	UZenoLiveLinkSession* GetSession(FGuid InGuid);

	/**
	 * @brief Try to find subject in all sessions
	 * @param InName Subject name
	 * @return Session instance. nullptr if not found.
	 */
	UZenoLiveLinkSession* FindSessionWithSubject(const FName& InName);

	/**
	 * @brief Get a fallback session
	 * @return return nullptr if Sessions if empty
	 */
	UZenoLiveLinkSession* GetSessionFallback();

	template <typename T>
	TSharedPromise<T> TryLoadSubjectRemotely(const FName& InName);

	static struct FRawMesh ConvertZenoMeshToRawMesh(const zeno::remote::Mesh& InZenoMesh);

private:
	UPROPERTY(EditAnywhere, Category = Zeno, DisplayName = "Connection Settings")
	FZenoLiveLinkSetting ConnectionSetting;

	UPROPERTY(VisibleAnywhere, AdvancedDisplay, DisplayName = "Connection Sessions")
	TMap<FGuid, UZenoLiveLinkSession*> ConnectionSessions;

	friend FZenoLiveLinkSource;
};

template <typename T>
TSharedPromise<T> UZenoLiveLinkClientSubsystem::TryLoadSubjectRemotely(const FName& InName)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UZenoLiveLinkClientSubsystem::TryLoadSubjectRemotely);
	TSharedPromise<T> Promise = MakeShared<TPromise<TOptional<T>>>();
	
	CONSTEXPR zeno::remote::ESubjectType RequiredSubjectType = zeno::remote::TGetClassSubjectType<T>::Value;
	if (RequiredSubjectType == zeno::remote::ESubjectType::Invalid || RequiredSubjectType >= zeno::remote::ESubjectType::Num)
	{
		Promise->EmplaceValue();
		return Promise;
	}

	const UZenoLiveLinkSession* Session = FindSessionWithSubject(InName);
	if (!IsValid(Session))
	{
		Promise->EmplaceValue();
		return Promise;
	}

	const UZenoHttpClient* Client = Session->GetClient();
	if (!IsValid(Client))
	{
		Promise->EmplaceValue();
		return Promise;
	}

	TSharedPromise<zeno::remote::SubjectContainerList> List = Client->GetDataFromRemote({ InName.ToString() });
	List->GetFuture().Then([Promise, InName, RequiredSubjectType] (TResultFuture<zeno::remote::SubjectContainerList> Result)
	{
		if (!Result.IsReady() || !Result.Get().IsSet())
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to fetch from zeno."));
			Promise->EmplaceValue();
			return;
		}
		// Holding this with reference will lead to bug value. WTF?
		zeno::remote::SubjectContainerList DataList = Result.Get().GetValue();
		for (int32 Idx = 0; Idx < DataList.Data.size(); ++Idx)
		{
			const auto& [Name, Type, Data] = DataList.Data[Idx];
			const int16 FuckYouUE = Type - static_cast<int16>(RequiredSubjectType);
			if ( FString { Name.c_str() }.Equals(InName.ToString()) && FuckYouUE == 0)
			{
				std::error_code Err;
				T Res = msgpack::unpack<T>(Data.data(), Data.size(), Err);
				if (!Err)
				{
					Promise->EmplaceValue(MoveTempIfPossible(Res));
					return;
				}
			}
		}
		Promise->EmplaceValue();
	});

	return Promise;
}
