using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Apocalypse;

[Flags]
public enum AptpHeaderFlags
{
    None = 0,
    
    
}

public enum AptpSlotType
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

[Flags]
public enum AptpSlotFlags
{
    None = 0,

    Parameter = 1 << 10,
	
    ThisParameter = 1 << 11 | Parameter,
    OutParameter = 1 << 12 | Parameter,
    ReturnParameter = 1 << 13 | Parameter,

    All = Int32.MaxValue,
};

public struct AptpHeader
{
    public long Token;
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

    [FieldOffset(0)] [MarshalAs(UnmanagedType.I1)] public bool Boolean;
    
    [FieldOffset(0)] public IntPtr String;

    [FieldOffset(0)] public IntPtr Pointer;
};

public struct AptpSlot : IDisposable
{
    
    public static AptpSlot FromByte(byte content, AptpSlotFlags flags) => new(AptpSlotType.UInt8, flags, new() { UInt8 = content });
    public static AptpSlot FromSByte(sbyte content, AptpSlotFlags flags) => new(AptpSlotType.Int8, flags, new() { Int8 = content });
    public static AptpSlot FromUShort(ushort content, AptpSlotFlags flags) => new(AptpSlotType.UInt16, flags, new() { UInt16 = content });
    public static AptpSlot FromShort(short content, AptpSlotFlags flags) => new(AptpSlotType.Int16, flags, new() { Int16 = content });
    public static AptpSlot FromUInt(uint content, AptpSlotFlags flags) => new(AptpSlotType.UInt32, flags, new() { UInt32 = content });
    public static AptpSlot FromInt(int content, AptpSlotFlags flags) => new(AptpSlotType.Int32, flags, new() { Int32 = content });
    public static AptpSlot FromULong(ulong content, AptpSlotFlags flags) => new(AptpSlotType.UInt64, flags, new() { UInt64 = content });
    public static AptpSlot FromLong(long content, AptpSlotFlags flags) => new(AptpSlotType.Int64, flags, new() { Int64 = content });
    
    public static AptpSlot FromFloat(float content, AptpSlotFlags flags) => new(AptpSlotType.Float, flags, new() { Float = content });
    public static AptpSlot FromDouble(double content, AptpSlotFlags flags) => new(AptpSlotType.Double, flags, new() { Double = content });
    
    public static AptpSlot FromBool(bool content, AptpSlotFlags flags) => new(AptpSlotType.Boolean, flags, new() { Boolean = content });
    
    public static AptpSlot FromString(string content, AptpSlotFlags flags) => new(AptpSlotType.String, flags, new() { String = Marshal.StringToHGlobalUni(content) });

    public static AptpSlot FromStub(IStub content, AptpSlotFlags flags) => new(AptpSlotType.Pointer, flags, new() { Pointer = content.NativePtr });

    public void Dispose()
    {
        if (Type == AptpSlotType.String)
        {
            Marshal.FreeHGlobal(Content.String);
        }
    }
    
    private AptpSlot(AptpSlotType type, AptpSlotFlags flags, AptpSlotContent content)
    {
        Type = type;
        Flags = flags;
        Content = content;
    }

    public AptpSlotType Type;
    public AptpSlotFlags Flags;
    public AptpSlotContent Content;
};

public unsafe struct AptpMessage
{
    public AptpMessage(long token, int length, AptpSlot* slots)
    {
        Header.Token = token;
        Body.Length = length;
        Body.Slots = slots;
    }
    
    public AptpHeader Header;
    public AptpBody Body;
};

public unsafe class AptpEngine
{
    public static delegate* unmanaged<AptpMessage*, int> NativeProcess;
    
    public static int Send(AptpMessage* message)
    {
        return NativeProcess(message);
    }

    [UnmanagedCallersOnly]
    private static int Recv(AptpMessage* message)
    {
        // C++ calls C# function, only one case.
        long token = message->Header.Token;
        int len = message->Body.Length;
        AptpSlot* slots = message->Body.Slots;
        
        MethodInfo? method = ReflectionRegistry.GetMethodInfoByToken(token);
        if (method is null)
        {
            return 1;
        }

        if (method.IsStatic)
        {
            return 1; // Static not support.
        }

        if (method.IsGenericMethod)
        {
            return 1; // Generic not support.
        }

        if ((slots->Flags & AptpSlotFlags.ThisParameter) == 0)
        {
            return 1; // Signature mismatch.
        }

        AptpSlot* candidateReturnSlot = slots + len - 1;
        Type returnType = method.ReturnType;
        int returnNum = 1;
        if (returnType == typeof(void)) // No return value.
        {
            if ((candidateReturnSlot->Flags & AptpSlotFlags.ReturnParameter) != 0)
            {
                return 1; // Signature mismatch.
            }
            returnNum = 0;
        }
        else if (returnType.GetInterfaces().Contains(typeof(ITuple))) // Multiple return value.
        {
            return 1; // Currently not support.
        }
        else // Single return value.
        {
            if ((candidateReturnSlot->Flags & AptpSlotFlags.ReturnParameter) == 0)
            {
                return 1; // Signature mismatch.
            }
        }

        ParameterInfo[] parameterList = method.GetParameters();
        int paramNum = parameterList.Length;
        if (paramNum != len - returnNum - 1)
        {
            return 1; // Signature mismatch.
        }

        Object? @this = ObjectRegistry.Get(slots->Content.Pointer);
        object?[]? @params = paramNum > 0 ? new object?[paramNum] : null;
        
        for (int i = 0; i < paramNum; i++)
        {
            ParameterInfo parameter = parameterList[i];
            AptpSlot* slot = slots + i;

            Type parameterType = parameter.ParameterType;
            AptpSlotType slotType = slot->Type;
            AptpSlotFlags flags = slot->Flags;

            if ((flags & AptpSlotFlags.OutParameter) != 0)
            {
                // ...
            }
            
            // ...
            
            
        }

        // TODO: Convert message to object list.
        object? @return = method.Invoke(@this, @params);

        for (int i = 0; i < len; i++)
        {
            AptpSlot* slot = slots + i;

            if (i == len - 1 && (slot->Flags & AptpSlotFlags.ReturnParameter) != 0)
            {
                
            }
        }

        if (returnNum > 0)
        {
            // TODO: Copy values in return back to message.
        }

        return 0;
    }
};
