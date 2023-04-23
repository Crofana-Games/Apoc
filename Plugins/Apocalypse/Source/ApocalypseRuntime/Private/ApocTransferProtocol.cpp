// Copyright Crofana Games. All Rights Reserved.


#include "ApocTransferProtocol.h"

#include "ApocObjectModel.h"

namespace Apocalypse::Aptp_Private
{
	void ProcessCall(IClass& Class, IFunction& Function, void* Buffer)
	{
		Function(Buffer);
	}
	
	void ProcessGetPropertyValue(IClass& Class, IProperty& PropertyName, void* Buffer)
	{
		
	}

	void ProcessGetEnumValue(IEnum& Enum, IEnumValue& ValueName, void* Buffer)
	{
		
	}
	
	void ProcessSet(IClass& Class, IProperty& PropertyName, void* Buffer)
	{
		
	}
	
}

void Apocalypse::FAptpProcessor::Process(const FAptpMessage& Message) const
{
	TSharedPtr<IType>* Type = GetRegistry().TypeMap.Find(Message.TypeName);
	if (!Type)
	{
		return;
	}
	
	switch (Message.OpCode)
	{
	case EAptpOpCode::Call:
		{
			if (IClass* Class = (*Type)->AsClass().Get())
			{
				if (IFunction* Function = Class->GetFunction(Message.MemberName).Get())
				{
					Aptp_Private::ProcessCall(*Class, *Function, Message.Buffer);
				}
			}
		} break;
	case EAptpOpCode::Get:
		{
			if (IClass* Class = (*Type)->AsClass().Get())
			{
				
			}
			else
			{
				
			}
		} break;
	case EAptpOpCode::Set:
		{
			if (IClass* Class = (*Type)->AsClass().Get())
			{
				
			}
		} break;
	default: break;
	}
}

const Apocalypse::FAptpProcessor& Apocalypse::GetAptpProcessor()
{
	static FAptpProcessor Processor;
	return Processor;
}
