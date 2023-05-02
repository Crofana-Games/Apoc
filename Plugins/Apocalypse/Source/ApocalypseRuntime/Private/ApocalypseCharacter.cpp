// Copyright Crofana Games. All Rights Reserved.


#include "ApocalypseCharacter.h"

#include "ApocalypseRuntimeModule.h"
#include "ApocTransferProtocol.h"

FTest::FTest()
{
	x = 1;
	UE_LOG(LogTemp, Error, TEXT("CTOR! %p"), this);
}

FTest::~FTest()
{
	UE_LOG(LogTemp, Error, TEXT("DTOR! %p"), this);
}

AApocalypseCharacter::AApocalypseCharacter()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		IAptpProcessor* Processor = new FAptpProcessor_MemberUFunction(GET_FUNCTION_NAME_CHECKED(ThisClass, Attack));
		FAptpProcessorRegistry::RegisterProcessor(1, *Processor);
	}
}

int32 AApocalypseCharacter::GetHealth() const
{
	UE_LOG(LogTemp, Warning, TEXT("GetHealth: %d"), Health);
	return Health;
}

void AApocalypseCharacter::SetHealth(int32 NewValue)
{
	int32 OldValue = Health;
	Health = NewValue;
	UE_LOG(LogTemp, Warning, TEXT("SetHealth: %d -> %d"), OldValue, NewValue);
}

void AApocalypseCharacter::InjectToCSharp()
{
	auto FP = (void(*)(ThisClass*))IApocalypseRuntimeModule::Get().GetDotNetFunctionPointer("Bootstrap", "InjectApocalypseCharacter");
	FP(this);
}


bool AApocalypseCharacter::Attack(int32 SkillId)
{
	const bool bSuccess = FMath::RandBool();
	UE_LOG(LogTemp, Warning, TEXT("Attack! SkillId: %d! Success: %d!"), SkillId, bSuccess);
	return bSuccess;
}
