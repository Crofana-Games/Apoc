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
};

IMPLEMENT_MODULE(FApocalypseRuntimeModule, ApocalypseRuntime)

void FApocalypseRuntimeModule::StartupModule()
{
	Apocalypse::GetRegistry(); // Create registry and process auto registration.
	
	for (auto Entry : Apocalypse::GetRegistry().TypeMap)
	{
		TSharedPtr<Apocalypse::IClass> Class = Entry.Value->AsClass();
		if (!Class)
		{
			continue;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("Class [%s] Registered!!!"), *Entry.Key.ToString());
		TMap<FName, TSharedPtr<Apocalypse::IFunction>> FunctionMap;
		Class->GetFunctionMap(FunctionMap);
		for (auto FuncEntry : FunctionMap)
		{
			UE_LOG(LogTemp, Warning, TEXT("    Function [%s] Registered!!!"), *FuncEntry.Key.ToString());
		}


		// (*FunctionMap["StaticPrint"])(&Rand);
		//
		// uint8* Parm = (uint8*)FMemory::Malloc(sizeof(void*) + sizeof(int32));
		// (*FunctionMap["__Construct"])(Parm);
		// *(int32*)(Parm + sizeof(void*)) = Rand;
		// (*FunctionMap["Print"])(Parm);
		// (*FunctionMap["__Destruct"])(Parm);
		// FMemory::Free(Parm);
	}

	int32 Rand = FMath::RandRange(100, 1000);
	
	Apocalypse::FAptpMessage Message;
	Message.OpCode = Apocalypse::EAptpOpCode::Call;
	Message.TypeName = "FRecord";
	Message.MemberName = "StaticPrint";
	Message.Buffer = &Rand;
	Apocalypse::GetAptpProcessor().Process(Message);
	
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
	const FString Type = TEXT("Apocalypse.Bootstrap, Apocalypse");
	const FString Method = TEXT("Startup");
	void(*Entry)() = nullptr;
	LoadAssemblyAndGetFunctionPointer(*Assembly, *Type, *Method, UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&Entry);
	check(Entry);

	Entry();
}

void FApocalypseRuntimeModule::ShutdownModule()
{
}
