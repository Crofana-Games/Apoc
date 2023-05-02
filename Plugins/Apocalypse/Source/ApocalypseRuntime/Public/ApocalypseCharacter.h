// Copyright Crofana Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ApocalypseCharacter.generated.h"

USTRUCT(BlueprintType)
struct FTest
{
	GENERATED_BODY()
	FTest();
	~FTest();
	UPROPERTY()
	int32 x;
};

UCLASS()
class APOCALYPSERUNTIME_API AApocalypseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AApocalypseCharacter();
	
public:
	UFUNCTION(BlueprintCallable)
	void Test(FTest T) { }
	
	UFUNCTION(BlueprintPure)
	int32 GetHealth() const;

	UFUNCTION(BlueprintCallable)
	void SetHealth(int32 NewValue);

public:
	UFUNCTION(BlueprintCallable)
	void InjectToCSharp();
	
	UFUNCTION(BlueprintCallable)
	bool Attack(int32 SkillId);

public:
	UPROPERTY(BlueprintGetter = GetHealth, BlueprintSetter = SetHealth, EditAnywhere)
	int32 Health;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Atk;

};
