using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

namespace Linker;

internal unsafe class Entry
{
    public struct ApcAssemblyLoadRequest
    {
        public IntPtr Domain { get; }
        public IntPtr AssemblyPath { get; }
        public IntPtr EntryTypeName { get; }
        public IntPtr EntryMethodName { get; }
        public IntPtr Userdata { get; }
    }

    public unsafe struct FInitArgs
    {
        public delegate* unmanaged<IntPtr, IntPtr> CreateDomain;
        public delegate* unmanaged<ApcAssemblyLoadRequest, void> LoadAssembly;
        public delegate* unmanaged<IntPtr, void> UnloadDomain;
    }

    [UnmanagedCallersOnly]
    public static void Setup(FInitArgs* args)
    {
        args->CreateDomain = &CreateDomain;
        args->LoadAssembly = &LoadAssembly;
        args->UnloadDomain = &UnloadDomain;

        Debugger.Launch();

        Console.WriteLine("Kernel Module Setup!");
    }

    [UnmanagedCallersOnly]
    public static IntPtr CreateDomain(IntPtr domainNamePtr)
    {
        string? domainName = Marshal.PtrToStringUni(domainNamePtr);
        if (domainName is null)
        {
            return IntPtr.Zero;
        }

        var domain = new ApcDomain(domainName);
        return GCHandle.ToIntPtr(domain.Handle);
    }

    [UnmanagedCallersOnly]
    public static void LoadAssembly(ApcAssemblyLoadRequest request)
    {
        var domain = request.Domain != IntPtr.Zero ? GCHandle.FromIntPtr(request.Domain).Target as ApcDomain : AssemblyLoadContext.Default;
        if (domain is null)
        {
            return;
        }

        string? assemblyPath = Marshal.PtrToStringUni(request.AssemblyPath);
        if (assemblyPath is null)
        {
            return;
        }

        Assembly asm;
        using (var stream = new FileStream(assemblyPath, FileMode.Open))
        {
            asm = domain.LoadFromStream(stream);
        }

        if (request.EntryTypeName == IntPtr.Zero || request.EntryMethodName == IntPtr.Zero)
        {
            return;
        }

        string? entryTypeName = Marshal.PtrToStringUni(request.EntryTypeName);
        if (entryTypeName is null)
        {
            return;
        }

        string? entryMethodName = Marshal.PtrToStringUni(request.EntryMethodName);
        if (entryMethodName is null)
        {
            return;
        }


        Type? entry = asm.GetType(entryTypeName);
        if (entry is null)
        {
            return;
        }

        MethodInfo? setup = entry.GetMethod(entryMethodName);
        if (setup is null)
        {
            return;
        }

        if (!setup.IsStatic)
        {
            return;
        }

        ParameterInfo[] parameters = setup.GetParameters();
        if (parameters.Length != 1)
        {
            return;
        }

        if (parameters[0].ParameterType != typeof(IntPtr))
        {
            return;
        }

        setup.Invoke(null, new object[] { request.Userdata });
    }

    [UnmanagedCallersOnly]
    public static void UnloadDomain(IntPtr domainHandle)
    {
        var domain = GCHandle.FromIntPtr(domainHandle).Target as ApcDomain;
        if (domain is null)
        {
            return;
        }
        
        domain.Unload();
    }
}