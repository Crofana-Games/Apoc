


namespace Engine;

public unsafe partial class Class
{

    public Object GetDefaultObject() => FromHandle(UnmanagedGetDefaultObject(Handle))!;

    internal static delegate* unmanaged<IntPtr, IntPtr> UnmanagedGetDefaultObject;

}