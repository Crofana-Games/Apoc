// Copyright Crofana Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class IApcDomainInterface;

struct FApcAssemblyLoadRequest
{
	IApcDomainInterface* Domain;
	FString AssemblyPath;
	FString EntryTypeName;
	FString EntryMethodName;
	void* Userdata = nullptr;
};

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

public:
	virtual void RegisterDomain(IApcDomainInterface* Domain) = 0;
	virtual void LoadAssembly(const FApcAssemblyLoadRequest& Request) = 0;
	virtual void UnregisterDomain(IApcDomainInterface* Domain) = 0;
	
};
