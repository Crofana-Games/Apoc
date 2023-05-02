using System.Runtime.InteropServices;

namespace Apocalypse;

public unsafe struct InitArgs
{
    public delegate* unmanaged<AptpMessage*, int> UnmanagedRecvFunc;
}

public unsafe class Bootstrap
{
    
    [UnmanagedCallersOnly]
    public static void Startup(InitArgs args)
    {
        AptpEngine.NativeProcess = args.UnmanagedRecvFunc;
        
        Console.WriteLine("Apocalypse Startup!!!");
    }

    [UnmanagedCallersOnly]
    public static void InjectApocalypseCharacter(IntPtr nativePtr)
    {
        ApocalypseCharacter character = new() { NativePtr = nativePtr };
        
        System.Runtime.InteropServices.Marshal
        
        Console.WriteLine(character.Attack(15) ? "Success" : "Fail");

        // Console.WriteLine($"Health: {character.Health}");
        // character.Health = 500;
        // Console.WriteLine($"Health: {character.Health}");
        //
        // Console.WriteLine($"Atk: {character.Atk}");
        // character.Atk = 100;
        // Console.WriteLine($"Atk: {character.Atk}");
    }
}
