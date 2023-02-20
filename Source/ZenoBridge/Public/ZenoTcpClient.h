#pragma once

#define DEFAULT_RECEIVE_BUFFER_SIZE 1024
#define DEFAULT_SEND_BUFFER_SIZE 2048

#include "CoreMinimal.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "ZenoTcpClient.generated.h"

const auto GTextZenoSocketName = TEXT("ZenoTcpSocket");

class UZenoTcpClient;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FClientLostConnectionDelegate, UZenoTcpClient*, Thread);

UCLASS()
class UZenoTcpClient : public UObject, public FRunnable
{
	GENERATED_BODY()
	
public:
	// ~start FRunnable interface
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	// ~end FRunnable interface

	void StartClient();
	void Setup(const FIPv4Endpoint& InBindingAddr);

public:
	FClientLostConnectionDelegate LostConnectionDelegate;

private:
	FIPv4Endpoint BindingAddr;

	FCriticalSection CriticalSection;

	class FSocket* CurrentSocket = nullptr;

	std::atomic<bool> bIsThreadStop;

	TArray<uint8> ReceiveBuffer;
};
