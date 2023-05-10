

using System.Runtime.InteropServices;

namespace Engine;

public interface IStub
{
    IntPtr Handle { get; }
}

[StructLayout(LayoutKind.Explicit)]
public struct ManagedValue
{
    [FieldOffset(0)] public byte U1;
    [FieldOffset(0)] public ushort U2;
    [FieldOffset(0)] public uint U4;
    [FieldOffset(0)] public ulong U8;

    [FieldOffset(0)] public sbyte I1;
    [FieldOffset(0)] public short I2;
    [FieldOffset(0)] public int I4;
    [FieldOffset(0)] public long I8;

    [FieldOffset(0)] public float F4;
    [FieldOffset(0)] public double F8;

    [FieldOffset(0)][MarshalAs(UnmanagedType.U1)] public bool Bool;

    [FieldOffset(0)] public IntPtr String;

    [FieldOffset(0)] public IntPtr Object;
}

public unsafe static class Reflection
{

    static Reflection()
    {
        // Register special types
        _stubTypeMap["$DynamicObject"] = typeof(DynamicObject);

        // Register primitive types

        // Register exported types
        _stubTypeMap[Object.StaticClassPath] = typeof(Object);
        _stubTypeMap[Class.StaticClassPath] = typeof(Class);
    }

    [UnmanagedCallersOnly]
    internal static IntPtr NewStub(IntPtr* typeNamePtrs)
    {
        var typeName = Marshal.PtrToStringUni(*typeNamePtrs);
        if (typeName is null)
        {
            return IntPtr.Zero;
        }

        if (!_stubTypeMap.TryGetValue(typeName, out Type? type))
        {
            type = typeof(DynamicObject);
        }

        if (type.IsAbstract)
        {
            return IntPtr.Zero;
        }

        if (!type.IsAssignableTo(typeof(IStub)))
        {
            return IntPtr.Zero;
        }

        var actualType = type;
        if (type.IsGenericType)
        {
            // TODO: Substitute
        }

        return ((IStub)Activator.CreateInstance(actualType)!).Handle;
    }

    public static delegate* unmanaged<IntPtr, IntPtr, ManagedValue*, bool> CallFunction;
    public static delegate* unmanaged<IntPtr, IntPtr, ManagedValue*, bool> GetProperty;
    public static delegate* unmanaged<IntPtr, IntPtr, ManagedValue*, bool> SetProperty;

    private static Dictionary<string, Type> _stubTypeMap = new();

}