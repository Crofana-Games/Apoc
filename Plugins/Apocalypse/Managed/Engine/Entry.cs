
using System.Runtime.InteropServices;

namespace Engine;

internal class Entry
{
    private unsafe struct InitArgs
    {
        public delegate* unmanaged<IntPtr, IntPtr, ManagedValue*, bool> callFunction;
        public delegate* unmanaged<IntPtr, IntPtr, ManagedValue*, bool> getProperty;
        public delegate* unmanaged<IntPtr, IntPtr, ManagedValue*, bool> setProperty;
    }

    public unsafe static void Setup(IntPtr userdata)
    {
        InitArgs* args = (InitArgs*)userdata;
        Reflection.CallFunction = args->callFunction;
        Reflection.GetProperty = args->getProperty;
        Reflection.SetProperty = args->setProperty;

        Console.WriteLine("Engine Module Setup!");
    }
}