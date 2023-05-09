// Copyright Crofana Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ApcDomainInterface.h"
#include "ApcEngineDomain.generated.h"

namespace Apocalypse
{
	union FManagedValue;
	class FManagedObject;
	class IReflectionContext;
}

/**
 * 
 */
UCLASS()
class APOCALYPSERUNTIME_API UApcEngineDomain : public UEngineSubsystem, public IApcDomainInterface
{
	GENERATED_BODY()

public:
	static ThisClass& Get() { return *GEngine->GetEngineSubsystem<ThisClass>(); }

public:
	UApcEngineDomain();

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	virtual FName GetDomainName() const override;

public:
	static Apocalypse::FManagedObject* NewString(FString Data);

private:
	static bool CallFunction(const WIDECHAR* FunctionName, Apocalypse::FManagedObject* ThisStub, Apocalypse::FManagedValue* Params);
	static bool GetProperty(const WIDECHAR* PropertyName, Apocalypse::FManagedObject* ThisStub, Apocalypse::FManagedValue* ReturnValue);
	static bool SetProperty(const WIDECHAR* PropertyName, Apocalypse::FManagedObject* ThisStub, Apocalypse::FManagedValue* NewValue);

	enum class EApcLogVerbosity
	{
		Verbose,
		Log,
		Warning,
		Error,
		Fatal,
	};
	static void Log(EApcLogVerbosity Verbosity, const WIDECHAR* Message);

	static Apocalypse::FManagedObject* FindObject(Apocalypse::FManagedObject* ClassStub, Apocalypse::FManagedObject* OuterStub, const WIDECHAR* Name, uint8 bExactClass);
	static Apocalypse::FManagedObject* GetClass(Apocalypse::FManagedObject* ThisStub);
	static Apocalypse::FManagedObject* GetOuter(Apocalypse::FManagedObject* ThisStub);
	static Apocalypse::FManagedObject* GetName(Apocalypse::FManagedObject* ThisStub);

	static Apocalypse::FManagedObject* GetDefaultObject(Apocalypse::FManagedObject* ThisStub);

private:
	static Apocalypse::FManagedObject*(*ManagedNewString)(const WIDECHAR*);
	
private:
	TSharedPtr<Apocalypse::IReflectionContext> ReflectionContext;
	
};