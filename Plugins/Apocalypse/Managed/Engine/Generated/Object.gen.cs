



namespace Engine;

public unsafe abstract partial class Object
{

    public static Class StaticClass
    {
        get
        {
            if (_staticClass is null)
            {
                _staticClass = FindObject<Class>(null, null, "/Script/CoreUObject.Object");
            }
            return _staticClass!;
        }
    }


    private static Class? _staticClass;

}
