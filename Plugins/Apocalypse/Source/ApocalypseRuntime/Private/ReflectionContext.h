// Copyright Crofana Games. All Rights Reserved.

#pragma once

namespace Apocalypse
{
	class IClassProxy;
	class FManagedObject;
	
	class IReflectionContext
	{

	public:
		static IReflectionContext* New();

	public:
		virtual ~IReflectionContext() {  }
	
	public:
		virtual IClassProxy* GetClassProxy(FName ClassPath) = 0;
		virtual UObject* GetObject(FManagedObject* Stub) = 0;
		virtual FManagedObject* ToStub(UObject* Object) = 0;
	
	};
}
