

namespace Engine;

internal class Entry
{
    private unsafe struct InitArgs
    {
        public delegate* unmanaged<IntPtr, IntPtr> Callback_CoreNewString;

        public delegate* unmanaged<IntPtr*, IntPtr> Callback_Reflection_NewStub;

        public delegate* unmanaged<IntPtr, IntPtr, ManagedValue*, bool> Reflection_CallFunction;
        public delegate* unmanaged<IntPtr, IntPtr, ManagedValue*, bool> Reflection_GetProperty;
        public delegate* unmanaged<IntPtr, IntPtr, ManagedValue*, bool> Reflection_SetProperty;

        public delegate* unmanaged<Logger.LogVerbosity, IntPtr, void> Logger_Log;

        public delegate* unmanaged<IntPtr, IntPtr, IntPtr, byte, IntPtr> Object_FindObject;
        public delegate* unmanaged<IntPtr, IntPtr> Object_UnmanagedGetClass;
        public delegate* unmanaged<IntPtr, IntPtr> Object_UnmanagedGetOuter;
        public delegate* unmanaged<IntPtr, IntPtr> Object_UnmanagedGetName;

        public delegate* unmanaged<IntPtr, IntPtr> Class_GetDefaultObject;
    }

    public unsafe static void Setup(IntPtr userdata)
    {
        InitArgs* args = (InitArgs*)userdata;

        // Callback
        args->Callback_CoreNewString = &Core.NewString;
        args->Callback_Reflection_NewStub = &Reflection.NewStub;

        // Reflection API
        Reflection.CallFunction = args->Reflection_CallFunction;
        Reflection.GetProperty = args->Reflection_GetProperty;
        Reflection.SetProperty = args->Reflection_SetProperty;

        // Log API
        Logger.UnmanagedLog = args->Logger_Log;

        // Core UObject API
        Object.UnmanagedFindObject = args->Object_FindObject;
        Object.UnmanagedGetClass = args->Object_UnmanagedGetClass;
        Object.UnmanagedGetOuter = args->Object_UnmanagedGetOuter;
        Object.UnmanagedGetName = args->Object_UnmanagedGetName;

        // UClass API
        Class.UnmanagedGetDefaultObject = args->Class_GetDefaultObject;

        Logger.Log("Engine Module Setup!");
    }
}