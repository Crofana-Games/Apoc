// Copyright Crofana Games. All Rights Reserved.

#pragma once

namespace Apocalypse
{

	template <typename T>
	constexpr bool bPrimitive = false;

#define PRIMITIVE_TYPE(Type) template <> constexpr bool bPrimitive<Type> = true;
	PRIMITIVE_TYPE(uint8)
	PRIMITIVE_TYPE(uint16)
	PRIMITIVE_TYPE(uint32)
	PRIMITIVE_TYPE(uint64)
	PRIMITIVE_TYPE(int8)
	PRIMITIVE_TYPE(int16)
	PRIMITIVE_TYPE(int32)
	PRIMITIVE_TYPE(int64)
	PRIMITIVE_TYPE(float)
	PRIMITIVE_TYPE(double)
	PRIMITIVE_TYPE(bool)
	PRIMITIVE_TYPE(int64)
	PRIMITIVE_TYPE(uint8)
	PRIMITIVE_TYPE(int16)
	PRIMITIVE_TYPE(int32)
	PRIMITIVE_TYPE(int64)
	PRIMITIVE_TYPE(uint8)
#undef PRIMITIVE_TYPE

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
		virtual bool IsValueSemantics() const = 0; // 决定Buffer中非基元类型的解析策略，值语义解包时默认拷贝一个新对象，除非Property显式指定传引用；对象语义解包时不会发生拷贝。
		
		bool IsEnum() const { return !IsClass(); }
		TSharedPtr<struct IClass> AsClass();
		TSharedPtr<struct IEnum> AsEnum();
	};

	struct IClass : IType
	{
		virtual bool IsClass() const override final { return true; }
		virtual bool IsPrimitive() const = 0;
		virtual TSharedPtr<struct IFunction> GetFunction(FName InName) const = 0;
		virtual void GetFunctionMap(TMap<FName, TSharedPtr<struct IFunction>>& OutFunctionMap) const = 0;
	};

	struct IEnum : IType
	{
		virtual bool IsClass() const override final { return false; }
		virtual bool IsValueSemantics() const override { return true; }
	};

	struct IMember : ITypeInfo
	{
		virtual TSharedPtr<IClass> GetOwner() = 0;
	};

	struct IFunction : IMember
	{
		virtual int32 GetBufferSize() const = 0;
		/**
		 * Buffer中包含了函数调用所需的上下文信息，函数会根据自己的参数信息将Buffer分成多个Chunk，每个Chunk根据参数类型不同有不同的解析策略：
		 * 1. 基元类型：参数的值直接存在Buffer中，会直接解析出来。
		 * 2. 值类型：Buffer中存的是一个固定大小的（8-byte）内存地址，解析时会去对应地址找到原型，并且拷贝一个新对象。
		 * 3. 引用类型：Buffer中存的是一个固定大小的（8-byte）内存地址，解析时会去对应地址找到对象，并且直接操作该对象。
		 */
		virtual bool operator()(void* Buffer) const = 0;
	};

	struct IProperty : IMember
	{
		
	};

	struct IEnumValue : ITypeInfo
	{
		
	};




	


	// Classes
	template <typename CompiledInType, bool bUnrealField, bool bValueSemantics>
	class TCompiledInClass : public IClass
	{

		static_assert(bValueSemantics || !(bUnrealField && !TPointerIsConvertibleFromTo<CompiledInType, const volatile UObject>::Value), "Unreal script structs must be value semantics!");

	public:
		static constexpr bool bDestructible = std::is_destructible_v<CompiledInType> && !TIsTriviallyDestructible<CompiledInType>::Value;
		
	public:
		TCompiledInClass(FName InName)
			: Name(InName)
		{

		}

		virtual FName GetName() override { return Name; }
		virtual bool IsValueSemantics() const override { return bValueSemantics; }
		virtual bool IsPrimitive() const override { return false; }
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

	// template <typename CompiledInType>
	// class TPrimitiveClass : public TCompiledInClass<false, CompiledInType>
	// {
	// public:
	// 	static constexpr bool bPrimitive = false;
	// 	static constexpr bool bDestructible = false;
	//
	// public:
	// 	TPrimitiveClass(FName InName)
	// 		: TCompiledInClass(InName)
	// 	{
	//
	// 	}
	//
	// 	virtual bool IsPrimitive() const override { return bPrimitive; } // Note: bPrimitive is static so even it looks like the same we must override.
	// };



	// class FDynamicClass : public IClass
	// {
	// 	virtual bool IsPrimitive() const override { return false; }
	// };



	

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
	template <typename CompiledInType, bool bUnrealField, bool bValueSemantics, typename DerivedType>
	struct TAutoCompiledInClass
	{
		using CompiledInClassType = TCompiledInClass<CompiledInType, bUnrealField, bValueSemantics>;
		
		TAutoCompiledInClass()
		{
			auto Class = MakeShared<CompiledInClassType>(GetClassName());
			SetupClass(Class);
			GetTypedThis()->SetupClass(Class);
			GetRegistry().TypeMap.Emplace(GetClassName(), Class);
		}

		~TAutoCompiledInClass()
		{
			GetRegistry().TypeMap.Remove(GetClassName());
		}

	private:
		void SetupClass(TSharedPtr<CompiledInClassType> Class)
		{
			Class->ConditionalRegisterDefaultConstructor();
			Class->ConditionalRegisterDestructor();
		}

	private:
		DerivedType* GetTypedThis() { return StaticCast<DerivedType*>(this); }
		FName GetClassName() { return GetTypedThis()->StaticClassName; }
	};
	
}


template <typename CompiledInType, bool bUnrealField, bool bValueSemantics>
template <typename ReturnType, typename... ArgTypes>
void Apocalypse::TCompiledInClass<CompiledInType, bUnrealField, bValueSemantics>::RegisterStaticFunction(FName InFunctionName, ReturnType(*InFunction)(ArgTypes...))
{
	FunctionMap.Emplace(InFunctionName, MakeShared<TStaticFunction<ReturnType, ArgTypes...>>(InFunctionName, StaticCastSharedRef<IClass>(AsShared()), InFunction));
}

template <typename CompiledInType, bool bUnrealField, bool bValueSemantics>
template <typename ReturnType, typename ThisType, typename ... ArgTypes>
void Apocalypse::TCompiledInClass<CompiledInType, bUnrealField, bValueSemantics>::RegisterMemberFunction(FName InFunctionName, ReturnType(ThisType::* InFunction)(ArgTypes...))
{
	FunctionMap.Emplace(InFunctionName, MakeShared<TMemberFunction<ReturnType, ThisType, ArgTypes...>>(InFunctionName, StaticCastSharedRef<IClass>(AsShared()), InFunction));
}

template <typename CompiledInType, bool bUnrealField, bool bValueSemantics>
void Apocalypse::TCompiledInClass<CompiledInType, bUnrealField, bValueSemantics>::RegisterUnrealFunction(UFunction& InFunction)
{
	static_assert(bUnrealField, "Cannot register UFunction on non-unreal class!");
	
	FunctionMap.Emplace(InFunction.GetFName(), MakeShared<FUnrealFunction>(StaticCastSharedRef<IClass>(AsShared()), InFunction));
}

template <typename CompiledInType, bool bUnrealField, bool bValueSemantics>
void Apocalypse::TCompiledInClass<CompiledInType, bUnrealField, bValueSemantics>::ConditionalRegisterDefaultConstructor()
{
	if constexpr (!(bUnrealField && TPointerIsConvertibleFromTo<CompiledInType, const volatile UObject>::Value))
	{
		static_assert(TIsConstructible<CompiledInType>::Value, "Non-unreal class must have default constructor!");
		
		FunctionMap.Emplace("__Construct", MakeShared<TConstructor<CompiledInType>>("__Construct", StaticCastSharedRef<IClass>(AsShared())));
	}
}

template <typename CompiledInType, bool bUnrealField, bool bValueSemantics>
void Apocalypse::TCompiledInClass<CompiledInType, bUnrealField, bValueSemantics>::ConditionalRegisterDestructor()
{
	if constexpr (!bUnrealField && bDestructible)
	{
		FunctionMap.Emplace("__Destruct", MakeShared<TDestructor<CompiledInType>>(StaticCastSharedRef<IClass>(AsShared())));
	}
}






// Helper macros

#define BEGIN_APOCALYPSE_CLASS_EX(ClassName, bUnrealField, bValueSemantics) \
struct FAutoCompiledInClass_##ClassName : Apocalypse::TAutoCompiledInClass<ClassName, bUnrealField, bValueSemantics, FAutoCompiledInClass_##ClassName> \
{ \
	using StaticClass = ClassName; \
	inline static FName StaticClassName = #ClassName; \
	void SetupClass(TSharedPtr<CompiledInClassType> Class) \
	{ \

#define BEGIN_APOCALYPSE_CLASS(ClassName) BEGIN_APOCALYPSE_CLASS_EX(ClassName, false, false)
#define BEGIN_APOCALYPSE_STRUCT(ClassName) BEGIN_APOCALYPSE_CLASS_EX(ClassName, false, true)
#define BEGIN_APOCALYPSE_UCLASS(ClassName) BEGIN_APOCALYPSE_CLASS_EX(ClassName, true, false)
#define BEGIN_APOCALYPSE_USTRUCT(ClassName) BEGIN_APOCALYPSE_CLASS_EX(ClassName, true, true)

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