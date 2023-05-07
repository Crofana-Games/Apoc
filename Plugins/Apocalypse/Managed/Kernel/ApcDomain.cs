using System.Runtime.InteropServices;
using System.Runtime.Loader;

namespace Kernel;

public class ApcDomain : AssemblyLoadContext
{
    public ApcDomain(string name) : base(name, true)
    {
        Handle = GCHandle.Alloc(this);
    }

    ~ApcDomain()
    {
        Handle.Free();
    }

    public GCHandle Handle { get; }
    
}