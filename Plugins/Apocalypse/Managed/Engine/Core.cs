


using System.Runtime.InteropServices;

namespace Engine;

public static class Core
{
    [UnmanagedCallersOnly]
    internal static IntPtr NewString(IntPtr data)
    {
        var str = Marshal.PtrToStringUni(data);
        var handle = GCHandle.Alloc(str, GCHandleType.WeakTrackResurrection);

        return GCHandle.ToIntPtr(handle);
    }
}