using System.Reflection;
using System.Runtime.InteropServices;

namespace Apocalypse;

public enum AptpSlotType : byte
{
    None,
	
    UInt8,
    Int8,
    UInt16,
    Int16,
    UInt32,
    Int32,
    UInt64,
    Int64,

    Float,
    Double,

    Boolean,

    String,
    
    Object,

    Pointer,

    Num,
};

public enum AptpSlotSubtype : sbyte
{
    None,

    Struct,
	
    Array,
    Map,
    Set,
	
    Delegate,
    MulticastDelegate,
};

[Flags]
public enum AptpSlotFlags : ulong
{
    None = 0,

    Parameter = 1 << 0,
	
    ConstParameter = 1 << 1 | Parameter,
    RefParameter = 1 << 2 | Parameter,
    OutParameter = 1 << 3 | Parameter,
    ReturnParameter = 1 << 4 | Parameter,

    All = UInt64.MaxValue,
};

public struct AptpHeader
{
    public uint Version;
    public IntPtr Member;
};

public unsafe struct AptpBody
{
    public int Length;
    public AptpSlot* Slots;
};

[StructLayout(LayoutKind.Explicit)]
public struct AptpSlotContent
{
    [FieldOffset(0)] public byte UInt8;
    [FieldOffset(0)] public sbyte Int8;
    [FieldOffset(0)] public ushort UInt16;
    [FieldOffset(0)] public short Int16;
    [FieldOffset(0)] public uint UInt32;
    [FieldOffset(0)] public int Int32;
    [FieldOffset(0)] public ulong UInt64;
    [FieldOffset(0)] public long Int64;
    
    [FieldOffset(0)] public float Float;
    [FieldOffset(0)] public double Double;
    
    [FieldOffset(0)] public IntPtr String;

    [FieldOffset(0)] public ObjectKey Object;

    [FieldOffset(0)] public IntPtr Pointer;
};

public struct AptpSlot
{
    public AptpSlotType Type;
    public AptpSlotSubtype Subtype;
    public AptpSlotFlags Flags;
    public AptpSlotContent Content;
};

public struct AptpMessage
{
    public AptpHeader Header;
    public AptpBody Body;
};

public unsafe class AptpEngine
{
    public static delegate* unmanaged<AptpMessage*, void> NativeProcess;
    
    public static void Send(AptpMessage* message)
    {
        NativeProcess(message);
    }

    [UnmanagedCallersOnly]
    private static void Recv(AptpMessage* message)
    {
        // C++ calls C# function, only one case.
        
        object? @this = null;
        object? @return = null;
        
        // TODO: Convert message to object list.
        object?[] @params = { };

        MethodInfo? method = null;
        if (method is not null)
        {
            @return = method.Invoke(@this, @params);
        }

        if (@return is not null)
        {
            // TODO: Copy values in return back to message.
        }
    }
};
