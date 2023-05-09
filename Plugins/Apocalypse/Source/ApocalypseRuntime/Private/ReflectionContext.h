// Copyright Crofana Games. All Rights Reserved.

#pragma once

namespace Apocalypse
{
	class IClassProxy;
	class FManagedObject;
	
	class IReflectionContext
	{

	public:
		using FNewStub = FManagedObject*(*)(const WIDECHAR**);

	public:
		static IReflectionContext* New(FNewStub InNewStub);

	public:
		virtual ~IReflectionContext() {  }
	
	public:
		virtual IClassProxy* GetClassProxy(FName ClassPath) = 0;
		virtual UObject* GetObject(FManagedObject* Stub) = 0;
		virtual FManagedObject* ToStub(UObject* Object) = 0;

	public:
		template <typename T>
		T* GetObject(FManagedObject* Stub)
		{
			return Cast<T>(GetObject(Stub));
		}
		
	};
}
