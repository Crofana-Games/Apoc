



namespace Kernel;

public unsafe partial class Object
{

    public static string StaticClassPath = "/Script/CoreUObject.Object";
    public static Class StaticClass
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
