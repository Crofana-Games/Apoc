using System.Reflection;

namespace Apocalypse;

/// <summary>
/// A stub object who handles a native object by some way.
/// </summary>
public interface IStub
{
    public IntPtr NativePtr { get; }
}

public static class ObjectRegistry
{
    public static Object? Get(IntPtr pointer)
    {
        _objectLookup.TryGetValue(pointer, out Object? obj);
        return obj;
    }

    private static Dictionary<IntPtr, Object> _objectLookup = new();
}

public static class ReflectionRegistry
{
    public static MethodInfo? GetMethodInfoByToken(long memberToken)
    {
        return null;
    }

    public static long GetNativeTokenByName(string name)
    {
        return 0;
    }
}
