// Copyright Crofana Games. All Rights Reserved.

#include "ApocalypseRuntimeModule.h"

#include "ApocObjectModel.h"
#include "ApocTransferProtocol.h"
#include "coreclr_delegates.h"
#include "hostfxr.h"
#include "TestCompiledInClass.h"

#ifdef PLATFORM_WINDOWS
	#define HOSTFXR_PATH "Apocalypse/Source/ThirdParty/Win64/HostFXR/hostfxr.dll"
#else
	#error "Unknown platform"
#endif

using FInitializeHostFXR = hostfxr_initialize_for_runtime_config_fn;
using FHostFXRGetRuntimeDelegate = hostfxr_get_runtime_delegate_fn;
using FCloseHostFXR = hostfxr_close_fn;
using FHostFXRHandle = hostfxr_handle;

using FLoadAssemblyAndGetFunctionPointer = load_assembly_and_get_function_pointer_fn;

class FApocalypseRuntimeModule : public IApocalypseRuntimeModule
{
	// Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// End IModuleInterface

	void StartCLR();
};

IMPLEMENT_MODULE(FApocalypseRuntimeModule, ApocalypseRuntime)

enum class EAptpOpcode : uint8
{
	Call,
	Get,
	Set,
};

enum class EAptpSlotType : uint8
{
	UInt8,
	Int8,
	UInt16,
	Int16,
	UInt32,
	Int32,
	UInt64,
	Int64,
	Float,
	Double,
	Boolean,
	String,
	Pointer,
};

struct FAptpSlot
{
	EAptpSlotType Type;
};

struct FAptp
{
	EAptpOpcode Opcode;
	const WIDECHAR* TypeName;
	const WIDECHAR* MemberName;
	int32 SlotNum;
	FAptpSlot* Slots;

	FString ToString() const
	{
		return FString::Printf(TEXT("APTP: { OpCode: %d, TypeName: %s, MemberName: %s, Slot1: { Type: %d } }"), Opcode, TypeName, MemberName, Slots[1].Type);
	}
};

void Process(FAptp* Aptp)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Aptp->ToString());
	Aptp->Opcode = EAptpOpcode::Set;
}

void FApocalypseRuntimeModule::StartupModule()
{
	StartCLR();

	FAptp Aptp;
	Aptp.Opcode = EAptpOpcode::Call;
	Aptp.TypeName = TEXT("Player");
	Aptp.MemberName = TEXT("Attack");
	Aptp.SlotNum = 3;

	FAptpSlot Slots[3];
	Aptp.Slots = Slots;

	for (int32 i = 0; i < Aptp.SlotNum; i++)
	{
		Slots[i].Type = (EAptpSlotType)i;
	}
	
	Process(&Aptp);
}

void FApocalypseRuntimeModule::ShutdownModule()
{
}

void FApocalypseRuntimeModule::StartCLR()
{
	const FString HostFXRPath = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT(HOSTFXR_PATH));
	void* HostFXR = FPlatformProcess::GetDllHandle(*HostFXRPath);
	check(HostFXR);

	FInitializeHostFXR InitializeHostFxr = (FInitializeHostFXR)FPlatformProcess::GetDllExport(HostFXR, TEXT("hostfxr_initialize_for_runtime_config"));
	check(InitializeHostFxr);
	FHostFXRGetRuntimeDelegate GetRuntimeDelegate = (FHostFXRGetRuntimeDelegate)FPlatformProcess::GetDllExport(HostFXR, TEXT("hostfxr_get_runtime_delegate"));
	check(GetRuntimeDelegate);
	FCloseHostFXR CloseHostFXR = (FCloseHostFXR)FPlatformProcess::GetDllExport(HostFXR, TEXT("hostfxr_close"));
	check(CloseHostFXR);

	FHostFXRHandle Handle = nullptr;
	const FString RuntimeConfigPath = FPaths::Combine(FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("Apocalypse/Source/ThirdParty/RuntimeConfig.json")));
	InitializeHostFxr(*RuntimeConfigPath, nullptr, &Handle);
	check(Handle);

	void* FP = nullptr;
	GetRuntimeDelegate(Handle, hdt_load_assembly_and_get_function_pointer, &FP);
	FLoadAssemblyAndGetFunctionPointer LoadAssemblyAndGetFunctionPointer = (FLoadAssemblyAndGetFunctionPointer)FP;
	check(LoadAssemblyAndGetFunctionPointer);

	CloseHostFXR(Handle);
	
	const FString Assembly = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("Apocalypse/Content/Assemblies/Apocalypse.dll"));

	{
		const FString Type = TEXT("Apocalypse.Bootstrap, Apocalypse");
		const FString Method = TEXT("Startup");
		void(*Entry)(void*) = nullptr;
		LoadAssemblyAndGetFunctionPointer(*Assembly, *Type, *Method, UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&Entry);
		check(Entry);

		Entry(&FAptpEngine::Recv);
	}

	{
		const FString Type = TEXT("Apocalypse.AptpEngine, Apocalypse");
		const FString Method = TEXT("Recv");
		LoadAssemblyAndGetFunctionPointer(*Assembly, *Type, *Method, UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&FAptpEngine::ManagedRecvFunc);
		check(FAptpEngine::ManagedRecvFunc);
	}
}
