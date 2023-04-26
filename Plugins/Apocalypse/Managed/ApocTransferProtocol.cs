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

    Pointer,

    Num,
};

public enum AptpSlotSubtype : sbyte
{
    None,

    Object,
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
    uint Version;
};

public unsafe struct AptpBody
{
    int Length;
    AptpSlot* Slots;
};

[StructLayout(LayoutKind.Explicit)]
public struct AptpSlotContent
{
    [FieldOffset(0)] byte UInt8;
    [FieldOffset(0)] sbyte Int8;
    [FieldOffset(0)] ushort UInt16;
    [FieldOffset(0)] short Int16;
    [FieldOffset(0)] uint UInt32;
    [FieldOffset(0)] int Int32;
    [FieldOffset(0)] ulong UInt64;
    [FieldOffset(0)] long Int64;
    
    [FieldOffset(0)] float Float;
    [FieldOffset(0)] double Double;
    
    [FieldOffset(0)] IntPtr String;

    [FieldOffset(0)] IntPtr Pointer;
};

public struct AptpSlot
{
    AptpSlotType Type;
    AptpSlotSubtype Subtype;
    AptpSlotFlags Flags;
    AptpSlotContent Content;
};

public struct AptpMessage
{
    AptpHeader Header;
    AptpBody Body;
};

public unsafe class AptpEngine
{
    public static delegate* unmanaged<AptpMessage*, void> NativeProcess;
    
    public static void Send(AptpMessage* message)
    {
        NativeProcess(message);
    }

    [UnmanagedCallersOnly]
    public static void Recv(AptpMessage* message)
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
