using System.Runtime.InteropServices;

namespace Apocalypse;

public enum EType
{
    None,
    A,
    B,
}

public class Bootstrap
{
    [UnmanagedCallersOnly]
    public static void Startup()
    {
        Console.WriteLine("Apocalypse Startup!!!");
    }
}
