// Copyright Crofana Games. All Rights Reserved.


#include "ApocTransferProtocol.h"

FString FAptpBody::ToString() const
{
	FString Result = FString::Printf(TEXT("{ Length: %d, Slots: [ "), Length);
	for (int32 i = 0; i < Length; i++)
	{
		Result.Append(Slots[i].ToString()).Append(" ");
	}
	Result.Append("] }");
	return Result;
}

FString FAptpSlot::ToString() const
{
	FString TypeStr;
	FString ContentStr;
	switch (Type)
	{
		case EAptpSlotType::None:
		{
			TypeStr = "None";
			ContentStr = "None";
		} break;
		case EAptpSlotType::UInt8:
		{
			TypeStr = "UInt8";
			ContentStr = FString::Printf(TEXT("%u"), Content.UInt8);
		} break;
		case EAptpSlotType::Int8:
		{
			TypeStr = "Int8";
			ContentStr = FString::Printf(TEXT("%d"), Content.Int8);
		} break;
		case EAptpSlotType::UInt16:
        {
        	TypeStr = "UInt16";
        	ContentStr = FString::Printf(TEXT("%u"), Content.UInt16);
        } break;
		case EAptpSlotType::Int16:
		{
			TypeStr = "Int16";
			ContentStr = FString::Printf(TEXT("%d"), Content.Int16);
		} break;
		case EAptpSlotType::UInt32:
		{
			TypeStr = "UInt32";
			ContentStr = FString::Printf(TEXT("%u"), Content.UInt32);
		} break;
		case EAptpSlotType::Int32:
		{
			TypeStr = "Int32";
			ContentStr = FString::Printf(TEXT("%d"), Content.Int32);
		} break;
		case EAptpSlotType::UInt64:
		{
			TypeStr = "UInt64";
			ContentStr = FString::Printf(TEXT("%llu"), Content.UInt64);
		} break;
		case EAptpSlotType::Int64:
		{
			TypeStr = "Int64";
			ContentStr = FString::Printf(TEXT("%lld"), Content.Int64);
		} break;

		case EAptpSlotType::Float:
		{
			TypeStr = "Float";
			ContentStr = FString::Printf(TEXT("%f"), Content.Float);
		} break;
		case EAptpSlotType::Double:
		{
			TypeStr = "Double";
			ContentStr = FString::Printf(TEXT("%f"), Content.Double);
		} break;

		case EAptpSlotType::Boolean:
		{
			TypeStr = "Boolean";
			ContentStr = Content.Boolean ? "true" : "false";
		} break;

		case EAptpSlotType::String:
		{
			TypeStr = "String";
			ContentStr = Content.String;
		} break;

		case EAptpSlotType::Object:
		{
			TypeStr = "Object";
			ContentStr = ((UObject*)Content.Pointer)->GetName();
		} break;

		case EAptpSlotType::Pointer:
		{
			TypeStr = "Pointer";
			ContentStr = FString::Printf(TEXT("%p"), Content.Pointer);
		} break;

		default:
		{
			TypeStr = FString::Printf(TEXT("%d???"), Type);
			ContentStr = "???";
		} break;
	}

	return FString::Printf(TEXT("{ Type: %s, Flags: %d, Content: %s }"), *TypeStr, Flags, *ContentStr);
}

FAptpProcessor_MemberUFunction::FAptpProcessor_MemberUFunction(FName InFunctionName)
{
	FunctionName = InFunctionName;
}

int32 FAptpProcessor_MemberUFunction::Process(FAptpMessage& Message)
{
	UObject* This = (UObject*)Message.Body.Slots->Content.Pointer;
	if (!This)
	{
		return -1;
	}
	
	UFunction* Func = This->FindFunction(FunctionName);
	if (!Func)
	{
		return -1;
	}
	
	void* Params = FMemory::Malloc(Func->ParmsSize, Func->MinAlignment);
	TMap<int32, FProperty*> OutProperties;
	int32 Index = 1;
	for (TFieldIterator<FProperty> It(Func); It && (It->PropertyFlags & CPF_Parm); ++It)
	{
		FProperty *Property = *It;
		
		if (!Property->HasAnyPropertyFlags(CPF_ReturnParm))
		{
			Property->SetValue_InContainer(Params, &Message.Body.Slots[Index].Content);
		}
		
		if (Property->HasAnyPropertyFlags(CPF_OutParm | CPF_ReturnParm) && !Property->HasAnyPropertyFlags(CPF_ConstParm))
		{
			OutProperties.Emplace(Index, Property);
		}

		Index++;
	}

	This->ProcessEvent(Func, Params);

	for (auto OutProperty : OutProperties)
	{
		OutProperty.Value->CopyCompleteValue(&Message.Body.Slots[OutProperty.Key].Content, OutProperty.Value->ContainerPtrToValuePtr<void>(Params));
	}
	
	FMemory::Free(Params);
	return 0;
}

int64 FAptpProcessorRegistry::AllocateToken()
{
	static int64 Token = 0;
	return --Token;
}

void FAptpProcessorRegistry::RegisterProcessor(int64 Token, IAptpProcessor& Processor)
{
	GetLookup().Emplace(Token, &Processor);
}

IAptpProcessor* FAptpProcessorRegistry::GetProcessor(int64 Token)
{
	IAptpProcessor** Processor = GetLookup().Find(Token);
	return Processor ? *Processor : nullptr;
}

TMap<int64, IAptpProcessor*>& FAptpProcessorRegistry::GetLookup()
{
	static TMap<int64, IAptpProcessor*> Registry;
	return Registry;
}

int32 FAptpEngine::Recv(FAptpMessage& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("APTP Recv: %s"), *Message.ToString());

	IAptpProcessor* Processor = FAptpProcessorRegistry::GetProcessor(Message.Header.Token);
	return Processor ? Processor->Process(Message) : -1;
	
	// C# calls C++, three cases:
	// 1. Call UFunction.
	// 2. Access UProperty.
	// 3. Call container or delegate exported function.
	// UObject* This = nullptr;
	//
	// bool bFunction = true;
	// bool bProperty = true;
	// if (bFunction)
	// {
	// 	// TODO: Convert Message to UFunction params.
	// 	void* Params = nullptr;
	//
	// 	UFunction* Function = nullptr;
	// 	if (Function)
	// 	{
	// 		This->ProcessEvent(Function, Params);
	// 	}
	//
	// 	FOutParmRec* Rec = nullptr;
	// 	while (Rec)
	// 	{
	// 		// TODO: Copy values back to Message.
	// 		Rec = Rec->NextOutParm;
	// 	}
	// }
	// else if (bProperty)
	// {
	// 	FProperty* Property = nullptr;
	// 	
	// 	bool bRead = true;
	// 	if (bRead)
	// 	{
	// 		void** Return = Property->ContainerPtrToValuePtr<void*>(This);
	// 		// TODO: Copy Return back to Message.
	// 	}
	// 	else
	// 	{
	// 		*Property->ContainerPtrToValuePtr<void*>(This) = nullptr;
	// 	}
	// }
	// else
	// {
	// 	// TODO: How?
	// }
	//
	// return 0;
}
