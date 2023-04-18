// Copyright Crofana Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class IApocalypseRuntimeModule : public IModuleInterface
{
public:
	static FORCEINLINE IApocalypseRuntimeModule& Get()
	{
		static IApocalypseRuntimeModule& Singleton = FModuleManager::LoadModuleChecked<IApocalypseRuntimeModule>("ApocalypseRuntime");
		return Singleton;
	}

	static FORCEINLINE bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ApocalypseRuntime");
	}
};
