// Copyright Crofana Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

enum class EAptpSlotType
{
	None,
	
	UInt8,
	Int8,
	UInt16,
	Int16,
	UInt32,
	Int32,
	UInt64,
	Int64,

	Float,
	Double,

	Boolean,

	String,

	Object,
	Pointer,

	Num,
};

enum class EAptpSlotFlags
{
	None = 0,

	Parameter = 1 << 10,
	
	ThisParameter = 1 << 11 | Parameter, 
	OutParameter = 1 << 12 | Parameter,
	ReturnParameter = 1 << 13 | Parameter,

	All = -1,
};

struct FAptpHeader
{
	int64 Token;

	FString ToString() const
	{
		return FString::Printf(TEXT("{ Token: %llu }"), Token);
	}
};

struct FAptpBody
{
	int32 Length;
	struct FAptpSlot* Slots;

	FString ToString() const;
};

union FAptpSlotContent
{
	uint8 UInt8;
	int8 Int8;
	uint16 UInt16;
	int16 Int16;
	uint32 UInt32;
	int32 Int32;
	uint64 UInt64;
	int64 Int64;

	float Float;
	double Double;

	uint8 Boolean;

	const WIDECHAR* String;

	void* Pointer;
};

struct FAptpSlot
{
	EAptpSlotType Type;
	EAptpSlotFlags Flags;
	FAptpSlotContent Content;

	FString ToString() const;
};

struct FAptpMessage
{
	FAptpHeader Header;
	FAptpBody Body;

	FString ToString() const
	{
		return FString::Printf(TEXT("{ Header: %s, Body: %s }"), *Header.ToString(), *Body.ToString());
	}
};

struct APOCALYPSERUNTIME_API IAptpProcessor
{
	virtual ~IAptpProcessor() {  }
	virtual int32 Process(FAptpMessage& Message) = 0;
};

class APOCALYPSERUNTIME_API FAptpProcessor_StaticUFunction : public IAptpProcessor
{
	
};

class APOCALYPSERUNTIME_API FAptpProcessor_MemberUFunction : public IAptpProcessor
{
public:
	FAptpProcessor_MemberUFunction(FName InFunctionName);
	virtual int32 Process(FAptpMessage& Message) override;
private:
	FName FunctionName;
};

class APOCALYPSERUNTIME_API FAptpProcessor_UProperty : public IAptpProcessor
{
	
};

class APOCALYPSERUNTIME_API FAptpProcessor_GlobalFunction : public IAptpProcessor
{
	
};

class APOCALYPSERUNTIME_API FAptpProcessor_MemberFunction : public IAptpProcessor
{
	
};

class APOCALYPSERUNTIME_API FAptpProcessorRegistry
{
public:
	static int64 AllocateToken();
	static void RegisterProcessor(int64 Token, IAptpProcessor& Processor);
	static IAptpProcessor* GetProcessor(int64 Token);
private:
	static TMap<int64, IAptpProcessor*>& GetLookup();
};

class FAptpEngine
{
	friend class FApocalypseRuntimeModule;
public:
	static int32 Send(FAptpMessage& Message) { return ManagedRecvFunc(Message); }
private:
	static int32 Recv(FAptpMessage& Message);
private:
	using FManagedRecvFunc = int32(*)(FAptpMessage&);
	inline static FManagedRecvFunc ManagedRecvFunc = nullptr;
};
