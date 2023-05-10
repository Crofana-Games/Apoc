// Copyright Crofana Games. All Rights Reserved.

#pragma once

namespace Apocalypse
{
	union FManagedValue;
	class IReflectionContext;

	class FFunctionProxy;
	class FPropertyProxy;

	class IClassProxy : FNoncopyable
	{

	public:
		static IClassProxy* New(IReflectionContext* Context, UClass* Class);

	public:
		virtual ~IClassProxy() {  }
	
	public:
		virtual bool CallFunction(FName FunctionName, UObject* This, FManagedValue* Params) const = 0;
		virtual bool GetProperty(FName PropertyName, UObject* This, FManagedValue* ReturnValue) const = 0;
		virtual bool SetProperty(FName PropertyName, UObject* This, FManagedValue* NewValue) const = 0;

		virtual IReflectionContext& GetContext() const = 0;
		virtual bool IsValid() const = 0;

	public:
		const FFunctionProxy* GetFunction(FName FunctionName) const;
		const FPropertyProxy* GetProperty(FName PropertyName) const;

	protected:
		virtual const IClassProxy* GetSuper() const = 0;
		virtual const FFunctionProxy* GetFunctionExcludeSuper(FName FunctionName) const = 0;
		virtual const FPropertyProxy* GetPropertyExcludeSuper(FName PropertyName) const = 0;
	
	};
}

