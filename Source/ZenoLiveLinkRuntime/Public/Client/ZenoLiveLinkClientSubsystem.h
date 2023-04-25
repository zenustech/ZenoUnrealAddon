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

	template <typename T>
	TSharedPromise<T> TryLoadSubjectRemotely(const FName& InName);

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

	UZenoHttpClient::TAsyncResult<zeno::remote::SubjectContainerList> List = Client->GetDataFromRemote({ InName.ToString() });

	AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [List, Promise, InName, RequiredSubjectType]
	{
		List.WaitFor(FTimespan::FromSeconds(5));
		if (!List.IsReady())
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to fetch from zeno."));
			Promise->EmplaceValue();
			return;
		}
		
		for (const auto& Subject : List.Get()->Data)
		{
			if (FString{ Subject.Name.c_str() }.Equals(InName.ToString()) && Subject.Type == static_cast<uint16>(RequiredSubjectType))
			{
				std::error_code Err;
				T Res = msgpack::unpack<T>(Subject.Data.data(), Subject.Data.size(), Err);
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
