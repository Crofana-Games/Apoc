using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

namespace Linker;

public class ApcDomain : AssemblyLoadContext
{
    public ApcDomain(string name) : base(name, true)
    {
        Handle = GCHandle.Alloc(this);
        //_resolver = new AssemblyDependencyResolver()
    }

    ~ApcDomain()
    {
        Handle.Free();
    }

    protected override Assembly? Load(AssemblyName assemblyName)
    {
        return base.Load(assemblyName);
    }

    public GCHandle Handle { get; }

    private AssemblyDependencyResolver _resolver;
    
}