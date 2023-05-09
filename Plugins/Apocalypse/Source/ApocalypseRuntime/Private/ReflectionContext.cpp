// Copyright Crofana Games. All Rights Reserved.


#include "ReflectionContext.h"

#include "Interop.h"
#include "ReflectionProxies.h"

namespace Apocalypse
{
	class FReflectionContext : public IReflectionContext
	{

	public:
		FReflectionContext(FNewStub InNewStub)
			: NewStub(InNewStub)
		{
			
		}

	public:
		virtual IClassProxy* GetClassProxy(FName ClassPath) override;
		virtual UObject* GetObject(FManagedObject* Stub) override;
		virtual FManagedObject* ToStub(UObject* Object) override;

	private:
		TMap<FName, TUniquePtr<IClassProxy>> ClassProxyMap;
	
		TManagedObjectRegistry<FObjectKey> ObjectRegistry;

		FNewStub NewStub;
	
	};

	IClassProxy* FReflectionContext::GetClassProxy(FName ClassPath)
	{
		if (auto Proxy = ClassProxyMap.Find(ClassPath))
		{
			if ((*Proxy)->IsValid())
			{
				return Proxy->Get();
			}
		}

		UClass* Class = LoadObject<UClass>(nullptr, *ClassPath.ToString());
		if (!Class)
		{
			return nullptr;
		}

		return ClassProxyMap.Emplace(ClassPath, TUniquePtr<IClassProxy>(IClassProxy::New(this, Class))).Get();
	}

	UObject* FReflectionContext::GetObject(FManagedObject* Stub)
	{
		FObjectKey Unmanaged;
		if (ObjectRegistry.ToUnmanaged(Stub, Unmanaged))
		{
			return Unmanaged.ResolveObjectPtr();
		}

		return nullptr;
	}

	FManagedObject* FReflectionContext::ToStub(UObject* Object)
	{
		FManagedObject* Stub = ObjectRegistry.ToManaged(Object);
		if (!Stub)
		{
			FString Path = Object->GetClass()->GetPathName();
			const WIDECHAR* ClassPath = GetData(Path);
			Stub = NewStub(&ClassPath);
			if (Stub)
			{
				ObjectRegistry.Register(Stub, Object);
			}
		}
		
		return Stub;
	}

	IReflectionContext* IReflectionContext::New(FNewStub InNewStub)
	{
		return new FReflectionContext(InNewStub);
	}
}


