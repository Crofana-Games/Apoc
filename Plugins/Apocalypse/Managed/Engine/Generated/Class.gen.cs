


namespace Engine;

public unsafe partial class Class : Object
{

    public new static Class StaticClass
    {
        get
        {
            if (_staticClass is null)
            {
                _staticClass = FindObject<Class>(null, null, "/Script/CoreUObject.Class");
            }
            return _staticClass!;
        }
    }


    private static Class? _staticClass;

}