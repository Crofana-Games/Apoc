// Copyright Crofana Games. All Rights Reserved.


#include "ReflectionProxies.h"

#include "ReflectionContext.h"
#include "Interop.h"

namespace Apocalypse
{
	class IPropertyVisitor
	{
	public:
		virtual ~IPropertyVisitor()
		{
		}

	public:
		virtual IReflectionContext& GetContext() const = 0;

	public:
		virtual void Initialize(void* Dest) const = 0;
		virtual void Copy(FManagedValue* Src, void* Dest) const = 0;
		virtual void CopyBack(void* Src, FManagedValue* Dest) const = 0;
		virtual void Destroy(void* Dest) const = 0;

		virtual void* ContainerPtrToValuePtr(void* Container) const = 0;

	public:
		void Initialize_InContainer(void* Container) const { Initialize(ContainerPtrToValuePtr(Container)); }

		void Copy_InContainer(FManagedValue* Src, void* Container) const
		{
			Copy(Src, ContainerPtrToValuePtr(Container));
		}

		void CopyBack_InContainer(void* Container, FManagedValue* Dest) const
		{
			CopyBack(ContainerPtrToValuePtr(Container), Dest);
		}

		void Destroy_InContainer(void* Container) const { Destroy(ContainerPtrToValuePtr(Container)); }
	};

	class IPropertyVisitorOwner : public IPropertyVisitor
	{
	public:
		virtual IReflectionContext& GetContext() const override { return GetInnerVisitor().GetContext(); }

	public:
		virtual void Initialize(void* Dest) const override { return GetInnerVisitor().Initialize(Dest); }
		virtual void Copy(FManagedValue* Src, void* Dest) const override { return GetInnerVisitor().Copy(Src, Dest); }

		virtual void CopyBack(void* Src, FManagedValue* Dest) const override
		{
			return GetInnerVisitor().CopyBack(Src, Dest);
		}

		virtual void Destroy(void* Dest) const override { return GetInnerVisitor().Destroy(Dest); }

		virtual void* ContainerPtrToValuePtr(void* Container) const override
		{
			return GetInnerVisitor().ContainerPtrToValuePtr(Container);
		}

	protected:
		virtual IPropertyVisitor& GetInnerVisitor() const = 0;

	protected:
		static IPropertyVisitor* CreateInnerVisitor(IReflectionContext* Context, FProperty* Property);
	};


	class FParameterProxy : public IPropertyVisitorOwner
	{
	public:
		explicit FParameterProxy(IReflectionContext* Context, FProperty* Property);

	public:
		bool IsRef() const { return bRef; }
		bool IsOut() const { return bOut; }
		bool IsReturn() const { return bReturn; }

	protected:
		virtual IPropertyVisitor& GetInnerVisitor() const override { return *InnerVisitor; }

	private:
		TUniquePtr<IPropertyVisitor> InnerVisitor;

		bool bRef;
		bool bOut;
		bool bReturn;
	};


	class FPropertyProxy : public IPropertyVisitorOwner
	{
	public:
		explicit FPropertyProxy(IReflectionContext* Context, FProperty* Property);

	protected:
		virtual IPropertyVisitor& GetInnerVisitor() const override { return *InnerVisitor; }

	private:
		TUniquePtr<IPropertyVisitor> InnerVisitor;
	};


	class FFunctionProxy
	{
	public:
		explicit FFunctionProxy(IReflectionContext* InContext, UFunction* Function);

	public:
		bool ProcessEvent(UObject* This, FManagedValue* Params) const;

	private:
		void Link();

		bool HasReturnParameter() const { return Parameters.IsValidIndex(ReturnParameterIndex); }
		FParameterProxy* GetParameter(int32 Index) const { return Parameters[Index].Get(); }

		void SetupBuffer(void* Buffer, FManagedValue* Params) const;
		void CopyBufferBack(void* Buffer, FManagedValue* Params) const;
		void CleanupBuffer(void* Buffer) const;

	private:
		IReflectionContext* Context;
		UFunction* Inner;

		TArray<TUniquePtr<FParameterProxy>> Parameters;

		bool bStatic; // Currently used only for check.
		bool bInterface; // Currently not used.

		int32 ParamSize;
		int32 Alignment;
		int32 ReturnParameterIndex;
		TArray<int32> OutParameterIndices;
	};


	class FClassProxy : public IClassProxy
	{
	public:
		FClassProxy(IReflectionContext* InContext, UClass* Class);

	public:
		virtual bool CallFunction(FName FunctionName, UObject* This, FManagedValue* Params) const override;
		virtual bool GetProperty(FName PropertyName, UObject* This, FManagedValue* ReturnValue) const override;
		virtual bool SetProperty(FName PropertyName, UObject* This, FManagedValue* NewValue) const override;

		virtual IReflectionContext& GetContext() const override { return *Context; }
		virtual bool IsValid() const override { return Inner.IsValid(); }

	protected:
		virtual const IClassProxy* GetSuper() const override;
		virtual const FFunctionProxy* GetFunctionExcludeSuper(FName FunctionName) const override;
		virtual const FPropertyProxy* GetPropertyExcludeSuper(FName PropertyName) const override;

	private:
		void Link();

	private:
		const UClass* operator*() const;

	private:
		IReflectionContext* Context;
		TWeakObjectPtr<UClass> Inner;

		IClassProxy* Super;
		TMap<FName, TUniquePtr<FFunctionProxy>> FunctionMap;
		TMap<FName, TUniquePtr<FPropertyProxy>> PropertyMap;
	};


	// ------------------------------------------------- BEGIN PROPERTY VISITORS -------------------------------------------------

	class FPropertyVisitorBase : public IPropertyVisitor
	{
	public:
		explicit FPropertyVisitorBase(IReflectionContext* InContext, FProperty* Property);

	public:
		virtual IReflectionContext& GetContext() const override { return *Context; }

	public:
		virtual void Initialize(void* Dest) const override { GetInnerProperty()->InitializeValue(Dest); }

		virtual void Copy(FManagedValue* Src, void* Dest) const override
		{
			GetInnerProperty()->CopyCompleteValue(Dest, Src);
		}

		virtual void CopyBack(void* Src, FManagedValue* Dest) const override
		{
			GetInnerProperty()->CopyCompleteValue(Dest, Src);
		}

		virtual void Destroy(void* Dest) const override { GetInnerProperty()->DestroyValue(Dest); }

		virtual void* ContainerPtrToValuePtr(void* Container) const override
		{
			return GetInnerProperty()->ContainerPtrToValuePtr<void>(Container);
		}

	protected:
		virtual FProperty* GetInnerProperty() const = 0;

	private:
		IReflectionContext* Context;
	};

#define DECLARE_PROPERTY_VISITOR_EX(PropertyType, VisitorType, SuperPropertyType) \
class VisitorType : public SuperPropertyType \
{ \
	using Super = SuperPropertyType; \
public: \
	explicit VisitorType(IReflectionContext* Context, PropertyType* Property) \
		: Super(Context, Property) \
		, Inner(Property) \
	{ \
	} \
public: \
	virtual void Initialize(void* Dest) const override; \
	virtual void Copy(FManagedValue* Src, void* Dest) const override; \
	virtual void CopyBack(void* Src, FManagedValue* Dest) const override; \
	virtual void Destroy(void* Dest) const override; \
protected: \
	virtual FProperty* GetInnerProperty() const override { return Inner; } \
private: \
	PropertyType* Inner; \
};

#define DECLARE_PROPERTY_VISITOR(PropertyType) DECLARE_PROPERTY_VISITOR_EX(PropertyType, PropertyType##Visitor, FPropertyVisitorBase)


	// Primitive types
	DECLARE_PROPERTY_VISITOR(FNumericProperty)

	DECLARE_PROPERTY_VISITOR(FBoolProperty)

	DECLARE_PROPERTY_VISITOR(FEnumProperty)

	// String types
	DECLARE_PROPERTY_VISITOR(FStrProperty)

	DECLARE_PROPERTY_VISITOR(FNameProperty)

	DECLARE_PROPERTY_VISITOR(FTextProperty)

	// Object types
	DECLARE_PROPERTY_VISITOR_EX(FObjectPropertyBase, FObjectPropertyVisitor, FPropertyVisitorBase)

	DECLARE_PROPERTY_VISITOR(FInterfaceProperty)
	
	DECLARE_PROPERTY_VISITOR(FStructProperty)

	// Container types
	DECLARE_PROPERTY_VISITOR(FArrayProperty)

	DECLARE_PROPERTY_VISITOR(FMapProperty)

	DECLARE_PROPERTY_VISITOR(FSetProperty)

	// Delegate types
	DECLARE_PROPERTY_VISITOR(FDelegateProperty)

	DECLARE_PROPERTY_VISITOR(FMulticastDelegateProperty)

#undef DECLARE_PROPERTY_VISITOR
#undef DECLARE_PROPERTY_VISITOR_EX

	// -------------------------------------------------  END PROPERTY VISITORS  -------------------------------------------------

#define CONDITIONAL_CREATE_VISITOR_EX(PropertyType, VisitorType) if (PropertyType* TypedProperty = CastField<PropertyType>(Property)) { return new VisitorType(Context, TypedProperty); }
#define CONDITIONAL_CREATE_VISITOR(PropertyType) CONDITIONAL_CREATE_VISITOR_EX(PropertyType, PropertyType##Visitor)
	
	IPropertyVisitor* IPropertyVisitorOwner::CreateInnerVisitor(IReflectionContext* Context, FProperty* Property)
	{
		// Primitive types
		CONDITIONAL_CREATE_VISITOR(FNumericProperty)
		CONDITIONAL_CREATE_VISITOR(FBoolProperty)
		CONDITIONAL_CREATE_VISITOR(FEnumProperty)

		// String types
		CONDITIONAL_CREATE_VISITOR(FStrProperty)
		CONDITIONAL_CREATE_VISITOR(FNameProperty)
		CONDITIONAL_CREATE_VISITOR(FTextProperty)

		// Object types
		CONDITIONAL_CREATE_VISITOR_EX(FObjectPropertyBase, FObjectPropertyVisitor)
		CONDITIONAL_CREATE_VISITOR(FInterfaceProperty)
		CONDITIONAL_CREATE_VISITOR(FStructProperty)

		// Container types
		CONDITIONAL_CREATE_VISITOR(FArrayProperty)
		CONDITIONAL_CREATE_VISITOR(FMapProperty)
		CONDITIONAL_CREATE_VISITOR(FSetProperty)

		// Delegate types
		CONDITIONAL_CREATE_VISITOR(FDelegateProperty)
		CONDITIONAL_CREATE_VISITOR(FMulticastDelegateProperty)

		checkNoEntry();
		return nullptr;
	}

	FParameterProxy::FParameterProxy(IReflectionContext* Context, FProperty* Property)
		: InnerVisitor(CreateInnerVisitor(Context, Property))
		  , bRef(Property->HasAnyPropertyFlags(CPF_ReferenceParm))
		  , bOut(Property->HasAnyPropertyFlags(CPF_OutParm))
		  , bReturn(Property->HasAnyPropertyFlags(CPF_ReturnParm))
	{
	}

	FPropertyProxy::FPropertyProxy(IReflectionContext* Context, FProperty* Property)
		: InnerVisitor(CreateInnerVisitor(Context, Property))
	{
	}

	FFunctionProxy::FFunctionProxy(IReflectionContext* InContext, UFunction* Function)
		: Context(InContext)
		  , Inner(Function)
		  , bStatic(Function->HasAnyFunctionFlags(FUNC_Static))
		  , bInterface(Function->GetOuterUClassUnchecked()->HasAnyClassFlags(CLASS_Interface))
		  , ParamSize(Function->ParmsSize)
		  , Alignment(Function->MinAlignment)
		  , ReturnParameterIndex(INDEX_NONE)
	{
		Link();
	}

	bool FFunctionProxy::ProcessEvent(UObject* This, FManagedValue* Params) const
	{
		const int32 Callspace = This->GetFunctionCallspace(Inner, nullptr);
		if (Callspace != FunctionCallspace::Local)
		{
			ensure(Callspace != FunctionCallspace::Remote); // RPC is not supported yet.
			return false;
		}

		void* Buffer = FMemory_Alloca_Aligned(ParamSize, Alignment);
		SetupBuffer(Buffer, Params);

		This->ProcessEvent(Inner, Buffer);

		CopyBufferBack(Buffer, Params);
		CleanupBuffer(Buffer);

		return true;
	}

	void FFunctionProxy::Link()
	{
		Parameters.Reserve(Inner->NumParms);
		for (TFieldIterator<FProperty> It(Inner); It && (It->PropertyFlags & CPF_Parm); ++It)
		{
			FParameterProxy* Proxy = new FParameterProxy(Context, *It);
			int32 Index = Parameters.Emplace(Proxy);

			if (Proxy->IsReturn())
			{
				ReturnParameterIndex = Index;
			}
			else if (Proxy->IsOut())
			{
				OutParameterIndices.Emplace(Index);
			}
		}

		ensure(!HasReturnParameter() || ReturnParameterIndex == Parameters.Num() - 1);
	}

	void FFunctionProxy::SetupBuffer(void* Buffer, FManagedValue* Params) const
	{
		for (int32 Index = 0; Index < Parameters.Num(); Index++)
		{
			const FParameterProxy* Property = GetParameter(Index);

			if (Property->IsOut() && !Property->IsRef() || Property->IsReturn())
			{
				Property->Initialize_InContainer(Buffer);
			}
			else
			{
				Property->Copy_InContainer(Params + Index, Buffer);
			}
		}
	}

	void FFunctionProxy::CopyBufferBack(void* Buffer, FManagedValue* Params) const
	{
		for (int32 Index = 0; Index < Parameters.Num(); Index++)
		{
			const FParameterProxy* Property = GetParameter(Index);

			if (Property->IsOut() || Property->IsReturn())
			{
				Property->CopyBack_InContainer(Buffer, Params + Index);
			}
		}
	}

	void FFunctionProxy::CleanupBuffer(void* Buffer) const
	{
		for (int32 Index = 0; Index < Parameters.Num(); Index++)
		{
			const FParameterProxy* Property = GetParameter(Index);

			Property->Destroy_InContainer(Buffer);
		}
	}


	FClassProxy::FClassProxy(IReflectionContext* InContext, UClass* Class)
		: Context(InContext)
		  , Inner(Class)
	{
		Link();
	}

	bool FClassProxy::CallFunction(FName FunctionName, UObject* This, FManagedValue* Params) const
	{
		const FFunctionProxy* Function = GetFunction(FunctionName);
		return Function ? Function->ProcessEvent(This, Params) : false;
	}

	bool FClassProxy::GetProperty(FName PropertyName, UObject* This, FManagedValue* ReturnValue) const
	{
		if (const FPropertyProxy* Property = IClassProxy::GetProperty(PropertyName))
		{
			Property->CopyBack_InContainer(&This, ReturnValue);
			return true;
		}

		return false;
	}

	bool FClassProxy::SetProperty(FName PropertyName, UObject* This, FManagedValue* NewValue) const
	{
		if (const FPropertyProxy* Property = IClassProxy::GetProperty(PropertyName))
		{
			Property->Copy_InContainer(NewValue, &This);
			return true;
		}

		return false;
	}

	const IClassProxy* FClassProxy::GetSuper() const
	{
		return Super;
	}

	const FFunctionProxy* FClassProxy::GetFunctionExcludeSuper(FName FunctionName) const
	{
		if (const auto Function = FunctionMap.Find(FunctionName))
		{
			return Function->Get();
		}

		return nullptr;
	}

	const FPropertyProxy* FClassProxy::GetPropertyExcludeSuper(FName PropertyName) const
	{
		if (const auto Property = PropertyMap.Find(PropertyName))
		{
			return Property->Get();
		}

		return nullptr;
	}

	void FClassProxy::Link()
	{
		const UClass* Class = **this;

		if (const UClass* SuperClass = Class->GetSuperClass())
		{
			Super = Context->GetClassProxy(*SuperClass->GetPathName());
		}

		for (TFieldIterator<UFunction> It(Class, EFieldIteratorFlags::ExcludeSuper,
										  EFieldIteratorFlags::ExcludeDeprecated,
										  EFieldIteratorFlags::ExcludeInterfaces); It; ++It)
		{
			UFunction* Function = *It;
			FunctionMap.Emplace(Function->GetFName(), MakeUnique<FFunctionProxy>(Context, Function));
		}

		for (TFieldIterator<FProperty> It(Class, EFieldIteratorFlags::ExcludeSuper,
										  EFieldIteratorFlags::ExcludeDeprecated); It; ++It)
		{
			FProperty* Property = *It;
			PropertyMap.Emplace(Property->GetFName(), MakeUnique<FPropertyProxy>(Context, Property));
		}
	}

	const UClass* FClassProxy::operator*() const
	{
		ensure(Inner.IsValid());
		return Inner.Get();
	}


	// ------------------------------------------------- BEGIN PROPERTY VISITORS -------------------------------------------------

	// Base
	FPropertyVisitorBase::FPropertyVisitorBase(IReflectionContext* InContext, FProperty* Property)
		: Context(InContext)
	{
	}


	// FNumericPropertyVisitor
	void FNumericPropertyVisitor::Initialize(void* Dest) const
	{
		Super::Initialize(Dest);
	}

	void FNumericPropertyVisitor::Copy(FManagedValue* Src, void* Dest) const
	{
		Super::Copy(Src, Dest);
	}

	void FNumericPropertyVisitor::CopyBack(void* Src, FManagedValue* Dest) const
	{
		Super::CopyBack(Src, Dest);
	}

	void FNumericPropertyVisitor::Destroy(void* Dest) const
	{
		Super::Destroy(Dest);
	}


	// FBoolPropertyVisitor
	void FBoolPropertyVisitor::Initialize(void* Dest) const
	{
		Super::Initialize(Dest);
	}

	void FBoolPropertyVisitor::Copy(FManagedValue* Src, void* Dest) const
	{
		Inner->SetPropertyValue(Dest, (bool)Src->Bool);
	}

	void FBoolPropertyVisitor::CopyBack(void* Src, FManagedValue* Dest) const
	{
		Dest->Bool = (uint8)Inner->GetPropertyValue(Src);
	}

	void FBoolPropertyVisitor::Destroy(void* Dest) const
	{
		Super::Destroy(Dest);
	}


	// FEnumPropertyVisitor
	void FEnumPropertyVisitor::Initialize(void* Dest) const
	{
		Super::Initialize(Dest);
	}

	void FEnumPropertyVisitor::Copy(FManagedValue* Src, void* Dest) const
	{
		Super::Copy(Src, Dest);
	}

	void FEnumPropertyVisitor::CopyBack(void* Src, FManagedValue* Dest) const
	{
		Super::CopyBack(Src, Dest);
	}

	void FEnumPropertyVisitor::Destroy(void* Dest) const
	{
		Super::Destroy(Dest);
	}


	// FStrPropertyVisitor
	void FStrPropertyVisitor::Initialize(void* Dest) const
	{
		Super::Initialize(Dest);
	}

	void FStrPropertyVisitor::Copy(FManagedValue* Src, void* Dest) const
	{
		Inner->SetPropertyValue(Dest, Src->String);
	}

	void FStrPropertyVisitor::CopyBack(void* Src, FManagedValue* Dest) const
	{
		Dest->String = GetData(Inner->GetPropertyValue(Src));
	}

	void FStrPropertyVisitor::Destroy(void* Dest) const
	{
		Super::Destroy(Dest);
	}


	// FNamePropertyVisitor
	void FNamePropertyVisitor::Initialize(void* Dest) const
	{
		Super::Initialize(Dest);
	}

	void FNamePropertyVisitor::Copy(FManagedValue* Src, void* Dest) const
	{
		Inner->SetPropertyValue(Dest, Src->String);
	}

	void FNamePropertyVisitor::CopyBack(void* Src, FManagedValue* Dest) const
	{
		Dest->String = GetData(Inner->GetPropertyValue(Src).ToString());
	}

	void FNamePropertyVisitor::Destroy(void* Dest) const
	{
		Super::Destroy(Dest);
	}


	// FTextPropertyVisitor
	void FTextPropertyVisitor::Initialize(void* Dest) const
	{
		Super::Initialize(Dest);
	}

	void FTextPropertyVisitor::Copy(FManagedValue* Src, void* Dest) const
	{
		Inner->SetPropertyValue(Dest, FText::FromString(Src->String));
	}

	void FTextPropertyVisitor::CopyBack(void* Src, FManagedValue* Dest) const
	{
		Dest->String = GetData(Inner->GetPropertyValue(Src).ToString());
	}

	void FTextPropertyVisitor::Destroy(void* Dest) const
	{
		Super::Destroy(Dest);
	}


	// FObjectPropertyVisitor
	void FObjectPropertyVisitor::Initialize(void* Dest) const
	{
		Super::Initialize(Dest);
	}

	void FObjectPropertyVisitor::Copy(FManagedValue* Src, void* Dest) const
	{
		Inner->SetObjectPropertyValue(Dest, GetContext().GetObject(Src->Object));
	}

	void FObjectPropertyVisitor::CopyBack(void* Src, FManagedValue* Dest) const
	{
		Dest->Object = GetContext().ToStub(Inner->LoadObjectPropertyValue(Src)); // Use Load instead of Get to be compatible with SoftObjectPtr.
	}

	void FObjectPropertyVisitor::Destroy(void* Dest) const
	{
		Super::Destroy(Dest);
	}


	
	// FInterfacePropertyVisitor
	void FInterfacePropertyVisitor::Initialize(void* Dest) const
	{
		Super::Initialize(Dest);
	}

	void FInterfacePropertyVisitor::Copy(FManagedValue* Src, void* Dest) const
	{
		FScriptInterface Interface;
		if (UObject* Object = GetContext().GetObject(Src->Object))
		{
			Interface.SetObject(Object);
			Interface.SetInterface(Object->GetInterfaceAddress(Inner->InterfaceClass));
		}
		Inner->SetPropertyValue(Dest, Interface);
	}

	void FInterfacePropertyVisitor::CopyBack(void* Src, FManagedValue* Dest) const
	{
		const FScriptInterface& Interface = Inner->GetPropertyValue(Src);
		Dest->Object = GetContext().ToStub(Interface.GetObject());
	}

	void FInterfacePropertyVisitor::Destroy(void* Dest) const
	{
		Super::Destroy(Dest);
	}

	

	// FStructPropertyVisitor
	void FStructPropertyVisitor::Initialize(void* Dest) const
	{
		Super::Initialize(Dest);
	}

	void FStructPropertyVisitor::Copy(FManagedValue* Src, void* Dest) const
	{
		ensure(false);
	}

	void FStructPropertyVisitor::CopyBack(void* Src, FManagedValue* Dest) const
	{
		ensure(false);
	}

	void FStructPropertyVisitor::Destroy(void* Dest) const
	{
		Super::Destroy(Dest);
	}


	// FArrayPropertyVisitor
	void FArrayPropertyVisitor::Initialize(void* Dest) const
	{
		Super::Initialize(Dest);
	}

	void FArrayPropertyVisitor::Copy(FManagedValue* Src, void* Dest) const
	{
		ensure(false);
	}

	void FArrayPropertyVisitor::CopyBack(void* Src, FManagedValue* Dest) const
	{
		ensure(false);
	}

	void FArrayPropertyVisitor::Destroy(void* Dest) const
	{
		Super::Destroy(Dest);
	}


	// FMapPropertyVisitor
	void FMapPropertyVisitor::Initialize(void* Dest) const
	{
		Super::Initialize(Dest);
	}

	void FMapPropertyVisitor::Copy(FManagedValue* Src, void* Dest) const
	{
		ensure(false);
	}

	void FMapPropertyVisitor::CopyBack(void* Src, FManagedValue* Dest) const
	{
		ensure(false);
	}

	void FMapPropertyVisitor::Destroy(void* Dest) const
	{
		Super::Destroy(Dest);
	}


	// FSetPropertyVisitor
	void FSetPropertyVisitor::Initialize(void* Dest) const
	{
		Super::Initialize(Dest);
	}

	void FSetPropertyVisitor::Copy(FManagedValue* Src, void* Dest) const
	{
		ensure(false);
	}

	void FSetPropertyVisitor::CopyBack(void* Src, FManagedValue* Dest) const
	{
		ensure(false);
	}

	void FSetPropertyVisitor::Destroy(void* Dest) const
	{
		Super::Destroy(Dest);
	}


	// FDelegatePropertyVisitor
	void FDelegatePropertyVisitor::Initialize(void* Dest) const
	{
		Super::Initialize(Dest);
	}

	void FDelegatePropertyVisitor::Copy(FManagedValue* Src, void* Dest) const
	{
		ensure(false);
	}

	void FDelegatePropertyVisitor::CopyBack(void* Src, FManagedValue* Dest) const
	{
		ensure(false);
	}

	void FDelegatePropertyVisitor::Destroy(void* Dest) const
	{
		Super::Destroy(Dest);
	}


	// FMulticastDelegatePropertyVisitor
	void FMulticastDelegatePropertyVisitor::Initialize(void* Dest) const
	{
		Super::Initialize(Dest);
	}

	void FMulticastDelegatePropertyVisitor::Copy(FManagedValue* Src, void* Dest) const
	{
		ensure(false);
	}

	void FMulticastDelegatePropertyVisitor::CopyBack(void* Src, FManagedValue* Dest) const
	{
		ensure(false);
	}

	void FMulticastDelegatePropertyVisitor::Destroy(void* Dest) const
	{
		Super::Destroy(Dest);
	}

	// ------------------------------------------------- END PROPERTY VISITORS  -------------------------------------------------


	IClassProxy* IClassProxy::New(IReflectionContext* Context, UClass* Class)
	{
		return new FClassProxy(Context, Class);
	}

	const FFunctionProxy* IClassProxy::GetFunction(FName FunctionName) const
	{
		const IClassProxy* Class = this;
		while (Class)
		{
			if (const FFunctionProxy* Function = Class->GetFunctionExcludeSuper(FunctionName))
			{
				return Function;
			}

			Class = Class->GetSuper();
		}

		return nullptr;
	}

	const FPropertyProxy* IClassProxy::GetProperty(FName PropertyName) const
	{
		const IClassProxy* Class = this;
		while (Class)
		{
			if (const FPropertyProxy* Property = Class->GetPropertyExcludeSuper(PropertyName))
			{
				return Property;
			}

			Class = Class->GetSuper();
		}

		return nullptr;
	}
}
