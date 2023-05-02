// Copyright Crofana Games. All Rights Reserved.

#include "ApocalypseRuntimeModule.h"

#include "ApocTransferProtocol.h"
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
using FGetFunctionPointer = get_function_pointer_fn;

class FApocalypseRuntimeModule : public IApocalypseRuntimeModule
{

public:
	virtual void* GetDotNetFunctionPointer(FString Type, FString Method) override;
	
	// Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// End IModuleInterface

	void StartCLR();

private:
	FLoadAssemblyAndGetFunctionPointer LoadAssemblyAndGetFunctionPointer;
	FGetFunctionPointer GetFunctionPointer;
};

IMPLEMENT_MODULE(FApocalypseRuntimeModule, ApocalypseRuntime)

void* FApocalypseRuntimeModule::GetDotNetFunctionPointer(FString Type, FString Method)
{
	const FString Assembly = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("Apocalypse/Content/Assemblies/Apocalypse.dll"));
	
	void* Result = nullptr;
	FString s = FString::Printf(TEXT("Apocalypse.%s, Apocalypse"), *Type);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *s);
	LoadAssemblyAndGetFunctionPointer(*Assembly, TEXT("Apocalypse.Bootstrap, Apocalypse"), *Method, UNMANAGEDCALLERSONLY_METHOD, nullptr, &Result);
	return Result;
}

void FApocalypseRuntimeModule::StartupModule()
{
	StartCLR();
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

	GetRuntimeDelegate(Handle, hdt_load_assembly_and_get_function_pointer, (void**)&LoadAssemblyAndGetFunctionPointer);
	check(LoadAssemblyAndGetFunctionPointer);

	GetRuntimeDelegate(Handle, hdt_get_function_pointer, (void**)&GetFunctionPointer);
	check(GetFunctionPointer);

	CloseHostFXR(Handle);
	
	const FString Assembly = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("Apocalypse/Content/Assemblies/Apocalypse.dll"));

	{
		struct FInitArgs
		{
			void* UnmanagedRecvFunc;
		};
		
		const FString Type = TEXT("Apocalypse.Bootstrap, Apocalypse");
		const FString Method = TEXT("Startup");
		void(*Entry)(FInitArgs) = nullptr;
		LoadAssemblyAndGetFunctionPointer(*Assembly, *Type, *Method, UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&Entry);
		check(Entry);

		FInitArgs InitArgs;
		InitArgs.UnmanagedRecvFunc = &FAptpEngine::Recv;

		Entry(InitArgs);
	}

	{
		const FString Type = TEXT("Apocalypse.AptpEngine, Apocalypse");
		const FString Method = TEXT("Recv");
		LoadAssemblyAndGetFunctionPointer(*Assembly, *Type, *Method, UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&FAptpEngine::ManagedRecvFunc);
		check(FAptpEngine::ManagedRecvFunc);
	}
}
