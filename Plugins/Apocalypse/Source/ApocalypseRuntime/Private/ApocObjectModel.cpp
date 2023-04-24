// Copyright Crofana Games. All Rights Reserved.


#include "ApocObjectModel.h"

Apocalypse::FRegistry& Apocalypse::GetRegistry()
{
	static FRegistry Registry;
	return Registry;
}

TSharedPtr<Apocalypse::IClass> Apocalypse::IType::AsClass()
{
	return IsClass() ? StaticCastSharedPtr<IClass>(AsShared().ToSharedPtr()) : nullptr;
}

TSharedPtr<Apocalypse::IEnum> Apocalypse::IType::AsEnum()
{
	return IsEnum() ? StaticCastSharedPtr<IEnum>(AsShared().ToSharedPtr()) : nullptr;
}
