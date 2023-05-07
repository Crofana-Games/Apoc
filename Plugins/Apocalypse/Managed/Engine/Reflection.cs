

using System.Runtime.InteropServices;

namespace Engine;

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

internal unsafe static class Reflection
{
    public static delegate* unmanaged<IntPtr, IntPtr, ManagedValue*, bool> CallFunction;
    public static delegate* unmanaged<IntPtr, IntPtr, ManagedValue*, bool> GetProperty;
    public static delegate* unmanaged<IntPtr, IntPtr, ManagedValue*, bool> SetProperty;
}