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
class APOCALYPSERUNTIME_API UApcGameDomain : public UGameInstanceSubsystem, public IApcDomainInterface
{
	GENERATED_BODY()

public:
	UApcGameDomain();

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	virtual FName GetDomainName() const override;
	
};
