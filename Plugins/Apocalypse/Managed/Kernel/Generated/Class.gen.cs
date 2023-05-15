


namespace Kernel;

public unsafe partial class Class : Object
{

    public new static string StaticClassPath = "/Script/CoreUObject.Class";
    public new static Class StaticClass
    {
        get
        {
            if (_staticClass is null)
            {
                _staticClass = FindObject<Class>(null, null, StaticClassPath);
            }
            return _staticClass!;
        }
    }


    private static Class? _staticClass;

}