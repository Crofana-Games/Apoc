// Copyright Crofana Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ApcDomainInterface.h"
#include "ApcGameDomain.generated.h"

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
class APOCALYPSERUNTIME_API UApcGameDomain : public UEngineSubsystem, public IApcDomainInterface
{
	GENERATED_BODY()

public:
	static ThisClass& Get() { return *GEngine->GetEngineSubsystem<ThisClass>(); }

public:
	UApcGameDomain();

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	virtual FName GetDomainName() const override;

private:
	static bool CallFunction(const WIDECHAR* FunctionName, Apocalypse::FManagedObject* ThisStub, Apocalypse::FManagedValue* Params);
	static bool GetProperty(const WIDECHAR* PropertyName, Apocalypse::FManagedObject* ThisStub, Apocalypse::FManagedValue* ReturnValue);
	static bool SetProperty(const WIDECHAR* PropertyName, Apocalypse::FManagedObject* ThisStub, Apocalypse::FManagedValue* NewValue);

private:
	TSharedPtr<Apocalypse::IReflectionContext> ReflectionContext;
	
};
