using System.Runtime.InteropServices;

namespace Apocalypse;

public unsafe struct InitArgs
{
    public delegate* unmanaged<AptpMessage*, void> UnmanagedRecvFunc;
}

public unsafe class Bootstrap
{
    
    [UnmanagedCallersOnly]
    public static void Startup(InitArgs args)
    {
        AptpEngine.NativeProcess = args.UnmanagedRecvFunc;
        
        Console.WriteLine("Apocalypse Startup!!!");
    }
}
