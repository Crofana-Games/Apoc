namespace Apocalypse.ObjectModel;

/// <summary>
/// An object who contains a list of apocalypse properties and can be retrieved via IProperty.
/// </summary>
public interface IPropertyContainer
{
    
}

/// <summary>
/// A stub object who handles a native apocalypse object.
/// </summary>
public interface IStub : IPropertyContainer
{
    /// <summary>
    /// Pointer to the actual object.
    /// </summary>
    IntPtr NativeObject { get; }
    
    /// <summary>
    /// Class of the object.
    /// </summary>
    IClass Class { get; }
}

/// <summary>
/// A byte buffer used to pass custom data between .Net and native code.
/// </summary>
public struct AptpBuffer : IPropertyContainer
{

}

/// <summary>
/// Apocalypse reflection information.
/// </summary>
public interface ITypeInfo : IStub
{
    /// <summary>
    /// Name of this object.
    /// </summary>
    public string Name { get; }
}

/// <summary>
/// Information about an apocalypse class.
/// </summary>
public interface IClass : ITypeInfo
{
    public Dictionary<string, IMember> Members { get; }
}

/// <summary>
/// Information about an apocalypse class member.
/// </summary>
public interface IMember : ITypeInfo
{
    
}

/// <summary>
/// Information about an apocalypse function.
/// </summary>
public interface IFunction : ITypeInfo
{
    public IFunctionParameter ReturnParameter { get; }
    public IList<IFunctionParameter> Parameters { get; }
    void Invoke(IStub? @this, object?[]? parameters);
}

/// <summary>
/// Information about an apocalypse property.
/// </summary>
public interface IProperty : ITypeInfo
{
    public IClass PropertyClass { get; }
    public object? this[IPropertyContainer? container] { get; set; }
    public IClass? InnerClass { get; }
    public IClass? OuterClass { get; }
}

/// <summary>
/// Information about an apocalypse function parameter.
/// </summary>
public interface IFunctionParameter : IProperty
{
    
}