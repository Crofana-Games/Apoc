// Copyright Crofana Games. All Rights Reserved.


#include "TestCompiledInClass.h"

#include "ApocObjectModel.h"



BEGIN_APOCALYPSE_CLASS(FRecord)
	APOCALYPSE_STATIC_FUNCTION(StaticPrint)
	APOCALYPSE_MEMBER_FUNCTION(Print)
END_APOCALYPSE_CLASS()
IMPLEMENT_APOCALYPSE_CLASS(FRecord)


