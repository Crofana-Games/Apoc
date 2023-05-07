// Copyright Crofana Games. All Rights Reserved.


#include "ApcGameDomain.h"

#include "ReflectionContext.h"
#include "ApocalypseRuntimeModule.h"
#include "Interop.h"
#include "ReflectionProxies.h"

UApcGameDomain::UApcGameDomain()
	: ReflectionContext(MakeShareable<Apocalypse::IReflectionContext>(Apocalypse::IReflectionContext::New()))
{
}

void UApcGameDomain::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IApocalypseRuntimeModule& Module = IApocalypseRuntimeModule::Get();
	Module.RegisterDomain(this);

	const FString AssemblyDir = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Assemblies"));

	{
		struct
		{
			using FCallUE = bool(*)(const WIDECHAR*, Apocalypse::FManagedObject*, Apocalypse::FManagedValue*);
			FCallUE CallFunction = &ThisClass::CallFunction;
			FCallUE GetProperty = &ThisClass::GetProperty;
			FCallUE SetProperty = &ThisClass::SetProperty;
		} Userdata;
		
		FApcAssemblyLoadRequest Request;
		Request.Domain = this;
		Request.AssemblyPath = FPaths::Combine(AssemblyDir, TEXT("Engine.dll"));
		Request.EntryTypeName = "Engine.Entry";
		Request.EntryMethodName = TEXT("Setup");
		Request.Userdata = &Userdata;
	
		Module.LoadAssembly(Request);
	}

	{
		FApcAssemblyLoadRequest Request;
		Request.Domain = this;
		Request.AssemblyPath = FPaths::Combine(AssemblyDir, TEXT("Game.dll"));
		Request.EntryTypeName = "Game.Entry";
		Request.EntryMethodName = TEXT("Setup");
		Request.Userdata = nullptr;
	
		Module.LoadAssembly(Request);
	}
}

void UApcGameDomain::Deinitialize()
{
	IApocalypseRuntimeModule::Get().UnregisterDomain(this);
	
	Super::Deinitialize();
}

FName UApcGameDomain::GetDomainName() const
{
	static const FString Prefix = TEXT("Domain_");
	return FName(Prefix + GEngine->GetName());
}

bool UApcGameDomain::CallFunction(const WIDECHAR* FunctionName, Apocalypse::FManagedObject* ThisStub, Apocalypse::FManagedValue* Params)
{
	UApcGameDomain& Domain = Get();
	
	UObject* This = Domain.ReflectionContext->GetObject(ThisStub);
	if (!This)
	{
		return false;
	}

	Apocalypse::IClassProxy* Proxy = Domain.ReflectionContext->GetClassProxy(*This->GetClass()->GetPathName());
	if (!Proxy)
	{
		return false;
	}

	return Proxy->CallFunction(FunctionName, This, Params);
}

bool UApcGameDomain::GetProperty(const WIDECHAR* PropertyName, Apocalypse::FManagedObject* ThisStub, Apocalypse::FManagedValue* ReturnValue)
{
	UApcGameDomain& Domain = Get();
	
	UObject* This = Domain.ReflectionContext->GetObject(ThisStub);
	if (!This)
	{
		return false;
	}

	Apocalypse::IClassProxy* Proxy = Domain.ReflectionContext->GetClassProxy(*This->GetClass()->GetPathName());
	if (!Proxy)
	{
		return false;
	}

	return Proxy->SetProperty(PropertyName, This, ReturnValue);
}

bool UApcGameDomain::SetProperty(const WIDECHAR* PropertyName, Apocalypse::FManagedObject* ThisStub, Apocalypse::FManagedValue* NewValue)
{
	UApcGameDomain& Domain = Get();
	
	UObject* This = Domain.ReflectionContext->GetObject(ThisStub);
	if (!This)
	{
		return false;
	}

	Apocalypse::IClassProxy* Proxy = Domain.ReflectionContext->GetClassProxy(*This->GetClass()->GetPathName());
	if (!Proxy)
	{
		return false;
	}

	return Proxy->SetProperty(PropertyName, This, NewValue);
}
