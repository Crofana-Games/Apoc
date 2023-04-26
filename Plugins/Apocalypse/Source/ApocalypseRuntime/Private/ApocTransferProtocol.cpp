// Copyright Crofana Games. All Rights Reserved.


#include "ApocTransferProtocol.h"

void FAptpEngine::Recv(FAptpMessage& Message)
{
	// C# calls C++, three cases:
	// 1. Call UFunction.
	// 2. Access UProperty.
	// 3. Call container or delegate exported function.
	UObject* This = nullptr;
	
	bool bFunction = true;
	bool bProperty = true;
	if (bFunction)
	{
		// TODO: Convert Message to UFunction params.
		void* Params = nullptr;

		UFunction* Function = nullptr;
		if (Function)
		{
			This->ProcessEvent(Function, Params);
		}

		FOutParmRec* Rec = nullptr;
		while (Rec)
		{
			// TODO: Copy values back to Message.
			Rec = Rec->NextOutParm;
		}
	}
	else if (bProperty)
	{
		FProperty* Property = nullptr;
		
		bool bRead = true;
		if (bRead)
		{
			void** Return = Property->ContainerPtrToValuePtr<void*>(This);
			// TODO: Copy Return back to Message.
		}
		else
		{
			*Property->ContainerPtrToValuePtr<void*>(This) = nullptr;
		}
	}
	else
	{
		// TODO: How?
	}
}
