


using System.Runtime.InteropServices;

namespace Engine;

public unsafe abstract partial class Object : IStub
{

    public static T? FromHandle<T>(IntPtr handle) where T : Object => GCHandle.FromIntPtr(handle).Target as T;
    public static Object? FromHandle(IntPtr handle) => FromHandle<Object>(handle);



    public static T? FindObject<T>(Class? @class, Object? outer, string name, bool exactClass = false) where T : Object
    {
        IntPtr classPtr = @class is null ? IntPtr.Zero : @class.Handle;
        IntPtr outerPtr = outer is null ? IntPtr.Zero : outer.Handle;
        IntPtr namePtr = Marshal.StringToHGlobalUni(name);
        byte bExactClass = Convert.ToByte(exactClass);
        T? result = FromHandle<T>(UnmanagedFindObject(classPtr, outerPtr, namePtr, bExactClass));
        Marshal.FreeHGlobal(namePtr);
        return result;
    }
    public static Object? FindObject(Class? @class, Object? outer, string name, bool exactClass = false) => FindObject<Object>(@class, outer, name, exactClass);


    public Object()
    {
        Handle = GCHandle.ToIntPtr(GCHandle.Alloc(this, GCHandleType.WeakTrackResurrection));
    }



    public Class Class => FromHandle<Class>(UnmanagedGetClass(Handle))!;
    public Object Outer => FromHandle(UnmanagedGetOuter(Handle))!;
    public string Name => (string)GCHandle.FromIntPtr(UnmanagedGetName(Handle)).Target!;



    public IntPtr Handle { get; }



    internal static delegate* unmanaged<IntPtr, IntPtr, IntPtr, byte, IntPtr> UnmanagedFindObject;



    internal static delegate* unmanaged<IntPtr, IntPtr> UnmanagedGetClass;
    internal static delegate* unmanaged<IntPtr, IntPtr> UnmanagedGetOuter;
    internal static delegate* unmanaged<IntPtr, IntPtr> UnmanagedGetName;

}