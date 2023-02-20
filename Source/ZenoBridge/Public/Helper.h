#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LogZeno, Log, All);

#include "SocketSubsystem.h"

struct FUnrealSocketHelper
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
};
