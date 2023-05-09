

using Engine;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

namespace Game;

internal class Entry
{
    public unsafe static void Setup(IntPtr userdata)
    {
        var obj = Engine.Object.FindObject(null, null, Class.StaticClassPath);
        Logger.Log($"Class: {obj.Class.Name} Name: {obj.Name}");

        Logger.Log("Game Module Setup!");

        Task.Delay(1000).ContinueWith(_ =>
        {
            var gameplayStaticsClass = Engine.Object.FindObject<Class>(null, null, "Script/Engine.GameplayStatics")!;
            var gameplayStaticsCDO = gameplayStaticsClass.GetDefaultObject();

            var functionName = Marshal.StringToHGlobalUni("GetPlayerCharacter");
            var worldContext = new ManagedValue();
            worldContext.Object = IntPtr.Zero;
            var playerIndex = new ManagedValue();
            playerIndex.I4 = 0;
            ManagedValue* Params = stackalloc ManagedValue[]
            {
                worldContext,
                playerIndex,
                new ManagedValue(),
            };
            Reflection.CallFunction(functionName, gameplayStaticsCDO.Handle, null);
            Marshal.FreeHGlobal(functionName);

            var mainPlayer = Engine.Object.FromHandle(Params[2].Object);
        });
    }

}