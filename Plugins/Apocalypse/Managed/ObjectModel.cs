namespace Apocalypse;

/// <summary>
/// A stub object who handles a native apocalypse object.
/// </summary>
public interface IStub
{
    /// <summary>
    /// Pointer to the actual object.
    /// </summary>
    IntPtr NativeObject { get; }
}

/// <summary>
/// Mirrors FObjectKey.
/// </summary>
public struct ObjectKey
{
    public int Index;
    public int Serial;

    public override int GetHashCode()
    {
        return Index ^ Serial;
    }
}

public class Object : IStub
{
    public IntPtr NativeObject { get; } = IntPtr.Zero;
}

public class a : Object
{
    
}