// Copyright Crofana Games. All Rights Reserved.


#include "ApcGameDomain.h"

#include "ApocalypseRuntimeModule.h"

UApcGameDomain::UApcGameDomain()
{
}

void UApcGameDomain::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IApocalypseRuntimeModule& Module = IApocalypseRuntimeModule::Get();
	Module.RegisterDomain(this);

	const FString AssemblyDir = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Assemblies"));

	{
		FApcAssemblyLoadRequest Request;
		Request.Domain = this;
		Request.AssemblyPath = FPaths::Combine(AssemblyDir, TEXT("Game.dll"));
		Request.EntryTypeName = TEXT("Game.Entry");
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
	return FName(Prefix + GetOuter()->GetName());
}
