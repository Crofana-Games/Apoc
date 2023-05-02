using System.Reflection;

namespace Apocalypse;

public class Object : IStub
{
    public IntPtr NativePtr { get; init; }

    public MemberInfo? GetMemberInfo(IntPtr memberPtr)
    {
        return null;
    }
}