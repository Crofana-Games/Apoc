// Copyright Crofana Games. All Rights Reserved.

#include "ApocalypseRuntimeModule.h"

#include "ApcDomainInterface.h"
#include "inc/coreclr_delegates.h"
#include "inc/hostfxr.h"
#include "Interop.h"

#define BINARIES_DIR "Binaries"
#define HOSTFXR_PATH "dotnet/host/fxr/7.0.5/hostfxr.dll"

#ifdef PLATFORM_WINDOWS
	#define HOSTFXR_ROOT "Win64"
#else
	#error "Unknown platform"
#endif

struct FApcAssemblyLoadRequest_Interop
{
	Apocalypse::FManagedObject* Domain = nullptr;
	const WIDECHAR* AssemblyPath;
	const WIDECHAR* EntryTypeName = nullptr;
	const WIDECHAR* EntryMethodName = nullptr;
	void* Userdata;
};

using FInitializeHostFXR = hostfxr_initialize_for_runtime_config_fn;
using FHostFXRGetRuntimeDelegate = hostfxr_get_runtime_delegate_fn;
using FCloseHostFXR = hostfxr_close_fn;
using FHostFXRHandle = hostfxr_handle;

using FLoadAssemblyAndGetFunctionPointer = load_assembly_and_get_function_pointer_fn;

using FCreateDomain = Apocalypse::FManagedObject* (*)(const WIDECHAR*);
using FLoadAssembly = void (*)(FApcAssemblyLoadRequest_Interop);
using FUnloadDomain = void (*)(Apocalypse::FManagedObject*);

class FApocalypseRuntimeModule : public IApocalypseRuntimeModule
{

public:
	// Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// End IModuleInterface

	virtual void RegisterDomain(IApcDomainInterface* Domain) override;
	virtual void LoadAssembly(const FApcAssemblyLoadRequest& Request) override;
	virtual void UnregisterDomain(IApcDomainInterface* Domain) override;

private:
	void StartCLR();

private:
	Apocalypse::TManagedObjectRegistry<FObjectKey> DomainRegistry;

private:
	FCreateDomain CreateDomainFunction = nullptr;
	FLoadAssembly LoadAssemblyFunction = nullptr;
	FUnloadDomain UnloadDomainFunction = nullptr;
	
};

IMPLEMENT_MODULE(FApocalypseRuntimeModule, ApocalypseRuntime)

void FApocalypseRuntimeModule::StartupModule()
{
	StartCLR();
}

void FApocalypseRuntimeModule::ShutdownModule()
{
}

void FApocalypseRuntimeModule::RegisterDomain(IApcDomainInterface* Domain)
{
	FString DomainName = Domain->GetDomainName().ToString();
	Apocalypse::FManagedObject* DomainStub = CreateDomainFunction(GetData(DomainName));

	DomainRegistry.Register(DomainStub, Domain->_getUObject());
}

void FApocalypseRuntimeModule::LoadAssembly(const FApcAssemblyLoadRequest& Request)
{
	FApcAssemblyLoadRequest_Interop InteropRequest;
	if (Request.Domain)
	{
		InteropRequest.Domain = DomainRegistry.ToManaged(Request.Domain->_getUObject());
	}
	InteropRequest.AssemblyPath = GetData(Request.AssemblyPath);
	if (!Request.EntryTypeName.IsEmpty() && !Request.EntryMethodName.IsEmpty())
	{
		InteropRequest.EntryTypeName = GetData(Request.EntryTypeName);
		InteropRequest.EntryMethodName = GetData(Request.EntryMethodName);
	}
	InteropRequest.Userdata = Request.Userdata;

	LoadAssemblyFunction(InteropRequest);
}

void FApocalypseRuntimeModule::UnregisterDomain(IApcDomainInterface* Domain)
{
	const FObjectKey Unmanaged = Domain->_getUObject();
	
	UnloadDomainFunction(DomainRegistry.ToManaged(Unmanaged));
	
	DomainRegistry.Unregister(Unmanaged);
}

void FApocalypseRuntimeModule::StartCLR()
{
	const FString HostFXRPath = FPaths::Combine(FPaths::ProjectDir(), BINARIES_DIR, HOSTFXR_ROOT, HOSTFXR_PATH);
	void* HostFXR = FPlatformProcess::GetDllHandle(*HostFXRPath);
	check(HostFXR);

	FInitializeHostFXR InitializeHostFxr = (FInitializeHostFXR)FPlatformProcess::GetDllExport(HostFXR, TEXT("hostfxr_initialize_for_runtime_config"));
	check(InitializeHostFxr);
	FHostFXRGetRuntimeDelegate GetRuntimeDelegate = (FHostFXRGetRuntimeDelegate)FPlatformProcess::GetDllExport(HostFXR, TEXT("hostfxr_get_runtime_delegate"));
	check(GetRuntimeDelegate);
	FCloseHostFXR CloseHostFXR = (FCloseHostFXR)FPlatformProcess::GetDllExport(HostFXR, TEXT("hostfxr_close"));
	check(CloseHostFXR);
	
	const FString AssemblyPath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Assemblies"));

	FHostFXRHandle Handle = nullptr;
	const FString RuntimeConfigPath = FPaths::Combine(AssemblyPath, TEXT("Apocalypse.runtimeconfig.json"));
	InitializeHostFxr(*RuntimeConfigPath, nullptr, &Handle);
	check(Handle);

	FLoadAssemblyAndGetFunctionPointer LoadAssemblyAndGetFunctionPointer;
	GetRuntimeDelegate(Handle, hdt_load_assembly_and_get_function_pointer, (void**)&LoadAssemblyAndGetFunctionPointer);
	check(LoadAssemblyAndGetFunctionPointer);

	CloseHostFXR(Handle);
	
	struct FInitArgs
	{
		FCreateDomain OutCreateDomain;
		FLoadAssembly OutLoadAssembly;
		FUnloadDomain OutUnloadDomain;
	} InitArgs;

	const FString Assembly = FPaths::Combine(AssemblyPath, TEXT("Linker.dll"));
	const FString Type = TEXT("Linker.Entry, Linker");
	const FString Method = TEXT("Setup");
	void(*Setup)(FInitArgs&) = nullptr;
	LoadAssemblyAndGetFunctionPointer(*Assembly, *Type, *Method, UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&Setup);
	check(Setup);

	Setup(InitArgs);

	CreateDomainFunction = InitArgs.OutCreateDomain;
	LoadAssemblyFunction = InitArgs.OutLoadAssembly;
	UnloadDomainFunction = InitArgs.OutUnloadDomain;
}
