// Copyright Crofana Games. All Rights Reserved.


#include "ApcEngineDomain.h"

#include "ApcLogChannels.h"
#include "ReflectionContext.h"
#include "ApocalypseRuntimeModule.h"
#include "Interop.h"
#include "ReflectionProxies.h"

UApcEngineDomain::UApcEngineDomain()
{
	
}

void UApcEngineDomain::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IApocalypseRuntimeModule& Module = IApocalypseRuntimeModule::Get();
	Module.RegisterDomain(this);

	const FString AssemblyDir = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Assemblies"));

	{
		struct
		{
			Apocalypse::FManagedObject*(*NewString)(const WIDECHAR*);
			Apocalypse::IReflectionContext::FNewStub NewStub;
		
			using FCallUE = bool(*)(const WIDECHAR*, Apocalypse::FManagedObject*, Apocalypse::FManagedValue*);
			FCallUE CallFunction = &ThisClass::CallFunction;
			FCallUE GetProperty = &ThisClass::GetProperty;
			FCallUE SetProperty = &ThisClass::SetProperty;

			void(*Log)(EApcLogVerbosity, const WIDECHAR*) = &ThisClass::Log;

			Apocalypse::FManagedObject*(*FindObject)(Apocalypse::FManagedObject*, Apocalypse::FManagedObject*, const WIDECHAR*, uint8) = &ThisClass::FindObject;
			Apocalypse::FManagedObject*(*GetClass)(Apocalypse::FManagedObject*) = &ThisClass::GetClass;
			Apocalypse::FManagedObject*(*GetOuter)(Apocalypse::FManagedObject*) = &ThisClass::GetOuter;
			Apocalypse::FManagedObject*(*GetName)(Apocalypse::FManagedObject*) = &ThisClass::GetName;

			Apocalypse::FManagedObject*(*GetDefaultObject)(Apocalypse::FManagedObject*) = &ThisClass::GetDefaultObject;
		} Userdata;
		
		FApcAssemblyLoadRequest Request;
		//Request.Domain = this;
		Request.AssemblyPath = FPaths::Combine(AssemblyDir, TEXT("Engine.dll"));
		Request.EntryTypeName = TEXT("Engine.Entry");
		Request.EntryMethodName = TEXT("Setup");
		Request.Userdata = &Userdata;
	
		Module.LoadAssembly(Request);

		ManagedNewString = Userdata.NewString;

		ReflectionContext = MakeShareable<Apocalypse::IReflectionContext>(Apocalypse::IReflectionContext::New(Userdata.NewStub));
	}
}

void UApcEngineDomain::Deinitialize()
{
	IApocalypseRuntimeModule::Get().UnregisterDomain(this);
	
	Super::Deinitialize();
}

FName UApcEngineDomain::GetDomainName() const
{
	static const FString Prefix = TEXT("Domain_");
	return FName(Prefix + GEngine->GetName());
}

Apocalypse::FManagedObject* UApcEngineDomain::NewString(FString Data)
{
	return ManagedNewString(GetData(Data));
}

bool UApcEngineDomain::CallFunction(const WIDECHAR* FunctionName, Apocalypse::FManagedObject* ThisStub, Apocalypse::FManagedValue* Params)
{
	UApcEngineDomain& Domain = Get();
	
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

bool UApcEngineDomain::GetProperty(const WIDECHAR* PropertyName, Apocalypse::FManagedObject* ThisStub, Apocalypse::FManagedValue* ReturnValue)
{
	UApcEngineDomain& Domain = Get();
	
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

bool UApcEngineDomain::SetProperty(const WIDECHAR* PropertyName, Apocalypse::FManagedObject* ThisStub, Apocalypse::FManagedValue* NewValue)
{
	UApcEngineDomain& Domain = Get();
	
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

void UApcEngineDomain::Log(EApcLogVerbosity Verbosity, const WIDECHAR* Message)
{
	switch (Verbosity)
	{
		case EApcLogVerbosity::Verbose:
		{
			UE_LOG(LogApocalypse, Verbose, TEXT("%s"), Message);
		}
		case EApcLogVerbosity::Warning:
		{
			UE_LOG(LogApocalypse, Warning, TEXT("%s"), Message);
		}
		case EApcLogVerbosity::Error:
		{
			UE_LOG(LogApocalypse, Error, TEXT("%s"), Message);
		}
		case EApcLogVerbosity::Fatal:
		{
			UE_LOG(LogApocalypse, Fatal, TEXT("%s"), Message);
		}
		case EApcLogVerbosity::Log:
		default:
		{
			UE_LOG(LogApocalypse, Log, TEXT("%s"), Message);
		}
	}
}

Apocalypse::FManagedObject* UApcEngineDomain::FindObject(Apocalypse::FManagedObject* ClassStub, Apocalypse::FManagedObject* OuterStub, const WIDECHAR* Name, uint8 bExactClass)
{
	UApcEngineDomain& Domain = Get();
	
	UClass* Class = Domain.ReflectionContext->GetObject<UClass>(ClassStub);
	UObject* Outer = Domain.ReflectionContext->GetObject(OuterStub);

	return Domain.ReflectionContext->ToStub(StaticFindObject(Class, Outer, Name, (bool)bExactClass));
}

Apocalypse::FManagedObject* UApcEngineDomain::GetClass(Apocalypse::FManagedObject* ThisStub)
{
	UApcEngineDomain& Domain = Get();
	
	UObject* This = Domain.ReflectionContext->GetObject(ThisStub);

	return Domain.ReflectionContext->ToStub(This->GetClass());
}

Apocalypse::FManagedObject* UApcEngineDomain::GetOuter(Apocalypse::FManagedObject* ThisStub)
{
	UApcEngineDomain& Domain = Get();
	
	UObject* This = Domain.ReflectionContext->GetObject(ThisStub);

	return Domain.ReflectionContext->ToStub(This->GetOuter());
}

Apocalypse::FManagedObject* UApcEngineDomain::GetName(Apocalypse::FManagedObject* ThisStub)
{
	UApcEngineDomain& Domain = Get();
	
	UObject* This = Domain.ReflectionContext->GetObject(ThisStub);

	return NewString(This->GetName());
}

Apocalypse::FManagedObject* UApcEngineDomain::GetDefaultObject(Apocalypse::FManagedObject* ThisStub)
{
	UApcEngineDomain& Domain = Get();
	
	UClass* This = Domain.ReflectionContext->GetObject<UClass>(ThisStub);

	return Domain.ReflectionContext->ToStub(This->GetDefaultObject());
}

Apocalypse::FManagedObject*(*UApcEngineDomain::ManagedNewString)(const WIDECHAR*) = nullptr;