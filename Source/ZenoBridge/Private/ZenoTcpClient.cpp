#include "ZenoTcpClient.h"

#include "Helper.h"
#include "Common/TcpSocketBuilder.h"
#include "3rd/msgpack.h"
#include "model/transform.h"

bool UZenoTcpClient::Init()
{
	CurrentSocket = FTcpSocketBuilder(GTextZenoSocketName)
		.AsReusable()
		.AsBlocking()
		.WithReceiveBufferSize(DEFAULT_RECEIVE_BUFFER_SIZE)
		.WithSendBufferSize(DEFAULT_SEND_BUFFER_SIZE)
	;

	if (nullptr == CurrentSocket)
	{
		UE_LOG(LogZeno, Error, TEXT("Failed to build a new tcp socket."));
		return false;
	}

	if (const TSharedRef<FInternetAddr> InternetAddr =
		FUnrealSocketHelper::NewInternetAddr(BindingAddr.Address.ToString(), BindingAddr.Port);
		!CurrentSocket->Connect(*InternetAddr))
	{
		const ESocketErrors LastErr = FUnrealSocketHelper::GetLastError();
		UE_LOG(LogZeno, Error, TEXT("Failed to connect to zeno host with error code (%d).\nError description: %s"), LastErr, FUnrealSocketHelper::GetErrorDesc(LastErr));
		return false;
	}

	return true;
}

uint32 UZenoTcpClient::Run()
{
	while (!bIsThreadStop.load() && nullptr != CurrentSocket)
	{
		// blocking read socket status for up to 5s
		bool bDisconnected = false;
		CurrentSocket->HasPendingConnection(bDisconnected);
		CurrentSocket->Wait(ESocketWaitConditions::WaitForReadOrWrite, FTimespan(0, 0, 5));
		if (bDisconnected)
		{
			UE_LOG(LogZeno, Warning, TEXT("Previous connection has lost, stopping tcp client."));
			Stop();
			LostConnectionDelegate.Broadcast(this);
			continue;
		}

		if (uint32 DataSize; nullptr != CurrentSocket && CurrentSocket->HasPendingData(DataSize))
		{
			ReceiveBuffer.Init(0, DataSize);
			int32 ReadCnt;
			CurrentSocket->Recv(ReceiveBuffer.GetData(), DataSize, ReadCnt);
			// TODO: darc split packets
			const auto [x, y, z] = msgpack::unpack<Translation>(ReceiveBuffer.GetData(), ReadCnt);
			UE_LOG(LogZeno, Warning, TEXT("x(%f) y(%f) z(%f)"), x, y, z);
		}

		if (nullptr != CurrentSocket)
		{
			constexpr uint8 Data = 0x10;
			int32 Size;
			CurrentSocket->Send(&Data, 1, Size);
		}
	}
	return 0;
}

void UZenoTcpClient::Stop()
{
	bIsThreadStop = true;
	CurrentSocket->Close();
	FUnrealSocketHelper::DestroySocket(CurrentSocket);
	CurrentSocket = nullptr;
}

void UZenoTcpClient::Exit()
{
	if (nullptr != CurrentSocket)
	{
		CurrentSocket->Close();
		FUnrealSocketHelper::DestroySocket(CurrentSocket);
	}
}

void UZenoTcpClient::StartClient()
{
	FRunnableThread::Create(this, GTextZenoSocketName);
}

void UZenoTcpClient::Setup(const FIPv4Endpoint& InBindingAddr)
{
	BindingAddr = InBindingAddr;
}
