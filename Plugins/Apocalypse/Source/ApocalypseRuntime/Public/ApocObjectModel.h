// Copyright Crofana Games. All Rights Reserved.

#pragma once

namespace Apocalypse
{

	struct FRegistry
	{
		TMap<FName, TSharedPtr<struct IType>> TypeMap;
	};

	FRegistry& GetRegistry();
	
	// Interfaces
	
	struct ITypeInfo : public TSharedFromThis<ITypeInfo>
	{
		virtual ~ITypeInfo() = default;
		virtual FName GetName() = 0;
	};

	struct IType : ITypeInfo
	{
		virtual bool IsClass() const = 0;
		bool IsEnum() const { return !IsClass(); }
		TSharedPtr<struct IClass> AsClass();
		TSharedPtr<struct IEnum> AsEnum();
	};

	struct IClass : IType
	{
		virtual bool IsClass() const override final { return true; }
		virtual TSharedPtr<struct IFunction> GetFunction(FName InName) const = 0;
		virtual void GetFunctionMap(TMap<FName, TSharedPtr<struct IFunction>>& OutFunctionMap) const = 0;
	};

	struct IEnum : IType
	{
		virtual bool IsClass() const override final { return false; }
	};

	struct IMember : ITypeInfo
	{
		virtual TSharedPtr<IClass> GetOwner() = 0;
	};

	struct IFunction : IMember
	{
		virtual int32 GetBufferSize() const = 0;
		virtual bool operator()(void* Buffer) const = 0;
	};

	struct IProperty : IMember
	{
		
	};

	struct IEnumValue : ITypeInfo
	{
		
	};




	


	// Classes
	template <bool bUnrealField, typename CompiledInType>
	class TCompiledInClass : public IClass
	{

		static constexpr bool bDefaultConstructible = TIsConstructible<CompiledInType>::Value;
		static constexpr bool bDestructible = std::is_destructible_v<CompiledInType>;
		
	public:
		TCompiledInClass(FName InName)
			: Name(InName)
		{

		}

		virtual FName GetName() override { return Name; }
		virtual TSharedPtr<IFunction> GetFunction(FName InName) const override
		{
			const TSharedPtr<IFunction>* Function = FunctionMap.Find(InName);
			return Function ? *Function : nullptr;
		}
		virtual void GetFunctionMap(TMap<FName, TSharedPtr<IFunction>>& OutFunctionMap) const override { OutFunctionMap = FunctionMap; }

	public:
		void ConditionalRegisterDefaultConstructor();
		void ConditionalRegisterDestructor();
		
		template <typename ReturnType, typename... ArgTypes>
		void RegisterStaticFunction(FName InFunctionName, ReturnType(*InFunction)(ArgTypes...));
		
		template <typename ReturnType, typename ThisType, typename... ArgTypes>
        void RegisterMemberFunction(FName InFunctionName, ReturnType(ThisType::*InFunction)(ArgTypes...));

		void RegisterUnrealFunction(UFunction& InFunction);
		
	private:
		FName Name;
		TMap<FName, TSharedPtr<IFunction>> FunctionMap;
		
	};



	class FDynamicClass : public IClass
	{
		
	};



	

	// Functions

	class FFunctionBase : public IFunction
	{
	public:
		FFunctionBase(TSharedPtr<IClass> InOwner)
			: Owner(InOwner)
		{
			
		}

	public:
		virtual TSharedPtr<IClass> GetOwner() override { return LIKELY(Owner.IsValid()) ? Owner.Pin() : nullptr; }

	private:
		TWeakPtr<IClass> Owner;
	};

	template <typename CompiledInType, typename... ArgTypes>
	class TConstructor : public FFunctionBase
	{
	public:
		TConstructor(FName InName, TSharedPtr<IClass> InOwner)
			: FFunctionBase(InOwner)
			, Name(InName)
			, BufferSize(0)
		{
			
		}
		
		virtual FName GetName() override { return Name; }
		virtual int32 GetBufferSize() const override { return 0; }
		
	public:
		virtual bool operator()(void* Buffer) const override
		{
			*(CompiledInType**)Buffer = new CompiledInType(*(ArgTypes*)Buffer...); // TODO
			return true;
		}

	private:
		FName Name;
		int32 BufferSize;
		
	};


	
	template <typename CompiledInType>
	class TDestructor : public FFunctionBase
	{
	public:
		TDestructor(TSharedPtr<IClass> InOwner)
			: FFunctionBase(InOwner)
		{
			
		}
		
		virtual FName GetName() override { return "__Destruct"; }
		virtual int32 GetBufferSize() const override { return sizeof(void*); }
		
		virtual bool operator()(void* Buffer) const override
		{
			delete *(CompiledInType**)Buffer; // TODO
			return true;
		}
		
	};


	
	template <typename ReturnType, typename... ArgTypes>
	class TStaticFunction : public FFunctionBase
	{
		using FuncType = ReturnType(*)(ArgTypes...);

	public:
		TStaticFunction(FName InName, TSharedPtr<IClass> InOwner, FuncType InFunc)
			: FFunctionBase(InOwner)
			, Name(InName)
			, Func(InFunc)
		{
			
		}

	public:
		virtual FName GetName() override { return Name; }
		virtual int32 GetBufferSize() const override { return 0; }

		virtual bool operator()(void* Buffer) const override
		{
			Func(*(ArgTypes*)Buffer...); // TODO
			return true;
		}

	private:
		FName Name;
		FuncType Func;

	};



	template <typename ReturnType, typename ThisType, typename... ArgTypes>
	class TMemberFunction : public FFunctionBase
	{
		using FuncType = ReturnType(ThisType::*)(ArgTypes...);

	public:
		TMemberFunction(FName InName, TSharedPtr<IClass> InOwner, FuncType InFunc)
			: FFunctionBase(InOwner)
			, Name(InName)
			, Func(InFunc)
		{
			
		}

	public:
		virtual FName GetName() override { return Name; }
		virtual int32 GetBufferSize() const override { return 0; }
		
		virtual bool operator()(void* Buffer) const override
		{
			ThisType* This = *(ThisType**)Buffer;
			(This->*Func)(*(ArgTypes*)((uint8*)Buffer + sizeof(void*))...); // TODO
			return true;
		}

	private:
		FName Name;
		FuncType Func;

	};


	class FUnrealFunction : public FFunctionBase
	{
	public:
		FUnrealFunction(TSharedPtr<IClass> InOwner, UFunction& InFunction)
			: FFunctionBase(InOwner)
			, Function(&InFunction)
		{

		}

		virtual FName GetName() override { return Function->GetFName(); }
		virtual int32 GetBufferSize() const override { return (IsStatic() ? sizeof(UObject*) : 0) + Function->ParmsSize; }

		virtual bool operator()(void* Buffer) const override
		{
			const bool bStatic = IsStatic();
			UObject* This = bStatic ? Function->GetOuterUClass()->GetDefaultObject() : *(UObject**)Buffer;
			void* Params = bStatic ? Buffer : (uint8*)Buffer + sizeof(UObject*);

			This->ProcessEvent(Function.Get(), Params);
			
			return true;
		}

	private:
		bool IsStatic() const { return Function->HasAnyFunctionFlags(FUNC_Static); }

	private:
		TWeakObjectPtr<UFunction> Function;
		
	};



	

	// Auto registration
	template <bool bUnrealField, typename CompiledInType, typename DerivedType>
	struct TAutoCompiledInClass
	{
		TAutoCompiledInClass()
		{
			auto Class = MakeShared<TCompiledInClass<bUnrealField, CompiledInType>>(GetClassName());
			SetupClass(Class);
			GetTypedThis()->SetupClass(Class);
			GetRegistry().TypeMap.Emplace(GetClassName(), Class);
		}

		~TAutoCompiledInClass()
		{
			GetRegistry().TypeMap.Remove(GetClassName());
		}

	private:
		void SetupClass(TSharedPtr<TCompiledInClass<bUnrealField, CompiledInType>> Class)
		{
			Class->ConditionalRegisterDefaultConstructor();
			Class->ConditionalRegisterDestructor();
		}

	private:
		DerivedType* GetTypedThis() { return StaticCast<DerivedType*>(this); }
		FName GetClassName() { return GetTypedThis()->StaticClassName; }
	};
	
}


template <bool bUnrealField, typename CompiledInType>
template <typename ReturnType, typename... ArgTypes>
void Apocalypse::TCompiledInClass<bUnrealField, CompiledInType>::RegisterStaticFunction(FName InFunctionName, ReturnType(*InFunction)(ArgTypes...))
{
	FunctionMap.Emplace(InFunctionName, MakeShared<TStaticFunction<ReturnType, ArgTypes...>>(InFunctionName, StaticCastSharedRef<IClass>(AsShared()), InFunction));
}

template <bool bUnrealField, typename CompiledInType>
template <typename ReturnType, typename ThisType, typename ... ArgTypes>
void Apocalypse::TCompiledInClass<bUnrealField, CompiledInType>::RegisterMemberFunction(FName InFunctionName, ReturnType(ThisType::* InFunction)(ArgTypes...))
{
	FunctionMap.Emplace(InFunctionName, MakeShared<TMemberFunction<ReturnType, ThisType, ArgTypes...>>(InFunctionName, StaticCastSharedRef<IClass>(AsShared()), InFunction));
}

template <bool bUnrealField, typename CompiledInType>
void Apocalypse::TCompiledInClass<bUnrealField, CompiledInType>::RegisterUnrealFunction(UFunction& InFunction)
{
	static_assert(bUnrealField, "Cannot register UFunction on non-unreal class!");
	
	FunctionMap.Emplace(InFunction.GetFName(), MakeShared<FUnrealFunction>(StaticCastSharedRef<IClass>(AsShared()), InFunction));
}

template <bool bUnrealField, typename CompiledInType>
void Apocalypse::TCompiledInClass<bUnrealField, CompiledInType>::ConditionalRegisterDefaultConstructor()
{
	if constexpr (!bUnrealField && bDefaultConstructible)
	{
		FunctionMap.Emplace("__Construct", MakeShared<TConstructor<CompiledInType>>("__Construct", StaticCastSharedRef<IClass>(AsShared())));
	}
}

template <bool bUnrealField, typename CompiledInType>
void Apocalypse::TCompiledInClass<bUnrealField, CompiledInType>::ConditionalRegisterDestructor()
{
	if constexpr (!bUnrealField && bDestructible)
	{
		FunctionMap.Emplace("__Destruct", MakeShared<TDestructor<CompiledInType>>(StaticCastSharedRef<IClass>(AsShared())));
	}
}






// Helper macros

#define BEGIN_APOCALYPSE_CLASS_EX(ClassName, bUnrealField) \
struct FAutoCompiledInClass_##ClassName : Apocalypse::TAutoCompiledInClass<bUnrealField, ClassName, FAutoCompiledInClass_##ClassName> \
{ \
	using StaticClass = ClassName; \
	inline static FName StaticClassName = #ClassName; \
	void SetupClass(TSharedPtr<Apocalypse::TCompiledInClass<bUnrealField, ClassName>> Class) \
	{ \

#define BEGIN_APOCALYPSE_CLASS(ClassName) BEGIN_APOCALYPSE_CLASS_EX(ClassName, false)
#define BEGIN_UNREAL_APOCALYPSE_CLASS(ClassName) BEGIN_APOCALYPSE_CLASS_EX(ClassName, true)

#define APOCALYPSE_STATIC_FUNCTION(FunctionName) Class->RegisterStaticFunction(#FunctionName, &StaticClass::FunctionName);

#define APOCALYPSE_MEMBER_FUNCTION(FunctionName) Class->RegisterMemberFunction(#FunctionName, &StaticClass::FunctionName);

#define END_APOCALYPSE_CLASS() \
	} \
};

#define IMPLEMENT_APOCALYPSE_CLASS(ClassName) FAutoCompiledInClass_##ClassName BODY_MACRO_COMBINE(AutoCompiledInClass_##ClassName, _, , __LINE__);



















// struct ITypeInfo
// {
// 	virtual FName GetName() = 0;
// };
//
// struct IType : ITypeInfo
// {
// 	virtual void GetMemberMap(const TMap<FName, TSharedPtr<struct IMember>>& MemberMap) = 0;
// };
//
// struct IClass : IType
// {
// 		
// };
//
// struct IStruct : IType
// {
// 		
// };
//
// struct IMember : ITypeInfo
// {
// 		
// };
//
// struct IFunction : IMember
// {
// 		
// };
//
// struct IProperty : IMember
// {
// 		
// };