// Copyright Crofana Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ApcDomainInterface.generated.h"


// This class does not need to be modified.
UINTERFACE(NotBlueprintable)
class UApcDomainInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class APOCALYPSERUNTIME_API IApcDomainInterface : public IInterface
{
	GENERATED_BODY()

public:
	virtual FName GetDomainName() const = 0;
	//virtual bool InvokeMethod(FName TypeName, FName MethodName, FManagedObject* This, ) const = 0;
	
};
