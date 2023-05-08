

using Engine;
using System.Runtime.Loader;

namespace Game;

internal class Entry
{
    public static void Setup(IntPtr userdata)
    {
        Print(AssemblyLoadContext.GetLoadContext(typeof(Entry).Assembly));
        Print(AssemblyLoadContext.Default);

        Logger.Log("Game Module Setup!");
    }

    private static void Print(AssemblyLoadContext alc)
    {
        var lst = alc.Assemblies.ToList();
        foreach (var asm in lst)
        {
            Logger.Log(asm.FullName);
        }
    }
}