namespace Apocalypse;


// Generated stub part.
public unsafe partial class ApocalypseCharacter : Object
{
    public bool Attack(int skillId)
    {
        const int len = 3;
        AptpSlot* slots = stackalloc AptpSlot[len]
        {
            AptpSlot.FromStub(this, AptpSlotFlags.ThisParameter),
            AptpSlot.FromInt(skillId, AptpSlotFlags.Parameter),
            AptpSlot.FromBool(false, AptpSlotFlags.ReturnParameter),
        };

        AptpMessage msg = new(1, len, slots);

        AptpEngine.Send(&msg);

        return msg.Body.Slots[len - 1].Content.Boolean;
    }

    public int GetHealth()
    {
        const int len = 2;
        AptpSlot* slots = stackalloc AptpSlot[len]
        {
            AptpSlot.FromStub(this, AptpSlotFlags.ThisParameter),
            AptpSlot.FromInt(0, AptpSlotFlags.ReturnParameter),
        };
            
        AptpMessage msg = new(ReflectionRegistry.GetNativeTokenByName(nameof(GetHealth)), len, slots);
            
        AptpEngine.Send(&msg);

        return msg.Body.Slots[len - 1].Content.Int32;
    }

    public void SetHealth(int newValue)
    {
        const int len = 2;
        AptpSlot* slots = stackalloc AptpSlot[len]
        {
            AptpSlot.FromStub(this, AptpSlotFlags.ThisParameter),
            AptpSlot.FromInt(newValue, AptpSlotFlags.Parameter),
        };

        AptpMessage msg = new(ReflectionRegistry.GetNativeTokenByName(nameof(SetHealth)), len, slots);

        AptpEngine.Send(&msg);
    }
    
    public int Health
    {
        get => GetHealth();
        set => SetHealth(value);
    }

    public int Atk
    {
        get
        {
            const int len = 2;
            AptpSlot* slots = stackalloc AptpSlot[len]
            {
                AptpSlot.FromStub(this, AptpSlotFlags.ThisParameter),
                AptpSlot.FromInt(0, AptpSlotFlags.ReturnParameter),
            };
            
            AptpMessage msg = new(ReflectionRegistry.GetNativeTokenByName(nameof(Atk)), len, slots);
            
            AptpEngine.Send(&msg);

            return msg.Body.Slots[len - 1].Content.Int32;
        }
        set
        {
            const int len = 2;
            AptpSlot* slots = stackalloc AptpSlot[len]
            {
                AptpSlot.FromStub(this, AptpSlotFlags.ThisParameter),
                AptpSlot.FromInt(value, AptpSlotFlags.Parameter),
            };

            AptpMessage msg = new(ReflectionRegistry.GetNativeTokenByName(nameof(Atk)), len, slots);

            AptpEngine.Send(&msg);
        }
    }
}



// User code part.
public partial class ApocalypseCharacter
{
    
}