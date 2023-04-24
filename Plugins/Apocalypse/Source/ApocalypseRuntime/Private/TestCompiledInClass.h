// Copyright Crofana Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


struct FRecord
{
	FRecord()
	{
		UE_LOG(LogTemp, Warning, TEXT("[FRecord::Ctor] [%d]"), MyA);
	}
	
	~FRecord()
	{
		UE_LOG(LogTemp, Warning, TEXT("[FRecord::Dtor]"));
	}
	
	static void StaticPrint(int32 A)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FRecord::StaticPrint] [%d]"), A);
	}

	void Print(int32 A)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FRecord::Print] [%d] [%d]"), MyA, A);
	}

	int32 MyA = FMath::RandRange(100, 1000);
};
