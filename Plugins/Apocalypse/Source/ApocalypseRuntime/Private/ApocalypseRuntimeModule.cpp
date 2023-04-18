// Copyright Crofana Games. All Rights Reserved.

#include "ApocalypseRuntimeModule.h"

#include "coreclr_delegates.h"
#include "hostfxr.h"

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

	const FString Assembly = FPaths::Combine(FPaths::ProjectDir(), TEXT("Managed/Entry.dll"));
	const FString Type = TEXT("Apoc.Entry, Entry");
	const FString Method = TEXT("Launch");
	void(*Entry)() = nullptr;
	LoadAssemblyAndGetFunctionPointer(*Assembly, *Type, *Method, UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&Entry);
	check(Entry);

	Entry();
}

void FApocalypseRuntimeModule::ShutdownModule()
{
}
