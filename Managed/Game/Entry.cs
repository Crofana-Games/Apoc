

using Engine;
using System.Runtime.Loader;

namespace Game;

internal class Entry
{
    public static void Setup(IntPtr userdata)
    {
        var obj = Engine.Object.FindObject(null, null, Class.StaticClassPath);
        Logger.Log($"Class: {obj.Class.Name} Name: {obj.Name}");

        Logger.Log("Game Module Setup!");
    }

}