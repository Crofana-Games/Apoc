

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
            var gameplayStaticsClass = Engine.Object.FindObject<Class>(null, null, "/Script/Engine.GameplayStatics")!;
            var gameplayStaticsCDO = gameplayStaticsClass.GetDefaultObject();

            var functionName = Marshal.StringToHGlobalUni("GetPlayerCharacter");
            var worldContext = new ManagedValue();
            worldContext.Object = Engine.Object.FindObject(null, null,
                "/Game/ThirdPerson/Maps/UEDPIE_0_ThirdPersonMap.ThirdPersonMap:PersistentLevel.BP_ThirdPersonCharacter_C_0")!.Handle;
            var playerIndex = new ManagedValue();
            playerIndex.I4 = 0;
            ManagedValue* Params = stackalloc ManagedValue[]
            {
                worldContext,
                playerIndex,
                new ManagedValue(),
            };
            Reflection.CallFunction(functionName, gameplayStaticsCDO.Handle, Params);
            Marshal.FreeHGlobal(functionName);

            var mainPlayer = Engine.Object.FromHandle(Params[2].Object);
            Logger.Error(mainPlayer.Name);
            var jumpName = Marshal.StringToHGlobalUni("Jump");
            Reflection.CallFunction(jumpName, mainPlayer.Handle, null);
            Marshal.FreeHGlobal(jumpName);
        });
    }

}