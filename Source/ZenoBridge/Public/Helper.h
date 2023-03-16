#pragma once
#include "3rd/msgpack.h"
#include "model/networktypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogZeno, Log, All);

#include "SocketSubsystem.h"
#include <string>

constexpr size_t GUdpPackCutSize = 40960;

struct ZENOBRIDGE_API FUnrealSocketHelper
{
	static ISocketSubsystem* GetSocketSubsystem()
	{
		return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	}
	
	static TSharedRef<FInternetAddr> NewInternetAddr()
	{
		ISocketSubsystem* SocketSubsystem = GetSocketSubsystem();
		check(SocketSubsystem);
		
		TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
		
		return MoveTemp(Addr);
	}
	
	static TSharedRef<FInternetAddr> NewInternetAddr(const uint32 Address, const uint16 Port = 0)
	{
		TSharedRef<FInternetAddr> Addr = NewInternetAddr();
		Addr->SetIp(Address);
		Addr->SetPort(Port);
		
		return MoveTemp(Addr);
	}
	
	static TSharedRef<FInternetAddr> NewInternetAddr(const FString& Address, const uint16 Port = 0)
	{
		TSharedRef<FInternetAddr> Addr = NewInternetAddr();
		bool bIsValid;
		Addr->SetIp(*Address, bIsValid);
		Addr->SetPort(Port);
		
		return MoveTemp(Addr);
	}

	static ESocketErrors GetLastError()
	{
		return GetSocketSubsystem()->GetLastErrorCode();
	}

	static const TCHAR* GetErrorDesc(const ESocketErrors Error)
	{
		return GetSocketSubsystem()->GetSocketError(Error);
	}

	static void DestroySocket(class FSocket* Socket)
	{
		GetSocketSubsystem()->DestroySocket(Socket);
	}

	static inline TOptional<std::string> AuthToken {};

	static inline TOptional<std::string> SessionName {};

	template <typename T>
	static TArray<TArray<uint8>> MakeSendFileData(T& DataToSend, const ZBFileType FileType)
	{
		static std::atomic<uint32> CurrentFileId = 0;
		const static uint8 Timestamp = time(nullptr);

		TArray<TArray<uint8>> Datagrams;

		auto Data = msgpack::pack(DataToSend);
		const uint16 DataPartSize = ceil(Data.size() / static_cast<double>(GUdpPackCutSize));
		uint32 FileId = CurrentFileId++;
		FileId <<= 8;
		FileId += Timestamp;
		for (uint16 Idx = 0; Idx < DataPartSize; Idx++)
		{
			uint32 Size = GUdpPackCutSize;
			if ((Idx + 1) * GUdpPackCutSize > Data.size())
			{
				Size = Data.size() - (GUdpPackCutSize * Idx);
			}

#ifdef PLATFORM_LITTLE_ENDIAN
			ZBUFileMessageHeader Header {
				FileType,
				Size,
				FileId,
				DataPartSize,
				Idx,
			};
#else // PLATFORM_BIT_ENDIAN
			ZBUFileMessageHeader Header {
				ByteSwap(FileType),
				ByteSwap(Size),
				ByteSwap(FileId),
				ByteSwap(DataPartSize),
				ByteSwap(Idx),
			};
#endif // PLATFORM_LITTLE_ENDIAN

			TArray<uint8> Temp;
			Temp.SetNumUninitialized(sizeof(ZBUFileMessageHeader) + Size);
			FGenericPlatformMemory::Memmove(Temp.GetData(), &Header, sizeof(ZBUFileMessageHeader));
			FGenericPlatformMemory::Memmove(Temp.GetData() + sizeof(ZBUFileMessageHeader), Data.data() + (Idx * GUdpPackCutSize), Size);
			Datagrams.Push(MoveTempIfPossible(Temp));
		}

		return Datagrams;
	}
};
