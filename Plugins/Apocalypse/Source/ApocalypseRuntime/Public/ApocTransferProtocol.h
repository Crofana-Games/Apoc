// Copyright Crofana Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ApocTransferProtocol.generated.h"

enum class EAptpSlotType : uint8
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

	Pointer,

	Num,
};

enum class EAptpSlotSubtype : uint8
{
	None,

	Object,
	Struct,
	
	Array,
	Map,
	Set,
	
	Delegate,
	MulticastDelegate,
};

enum class EAptpSlotFlags : uint64
{
	None = 0,

	Parameter = 1 << 0,
	
	ConstParameter = 1 << 1 | Parameter,
	RefParameter = 1 << 2 | Parameter,
	OutParameter = 1 << 3 | Parameter,
	ReturnParameter = 1 << 4 | Parameter,

	All = -1,
};

struct FAptpHeader
{
	uint32 Version;
};

struct FAptpBody
{
	int32 Length;
	struct FAptpSlot* Slots;
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

	const WIDECHAR* String;

	void* Pointer;
};

struct FAptpSlot
{
	EAptpSlotType Type;
	EAptpSlotSubtype Subtype;
	EAptpSlotFlags Flags;
	FAptpSlotContent Content;
};

struct FAptpMessage
{
	FAptpHeader Header;
	FAptpBody Body;
};

class FAptpEngine
{
	friend class FApocalypseRuntimeModule;
public:
	static void Send(FAptpMessage& Message) { ManagedRecvFunc(Message); }
	static void Recv(FAptpMessage& Message);
private:
	using FManagedRecvFunc = void(*)(FAptpMessage&);
	static FManagedRecvFunc ManagedRecvFunc;
};
