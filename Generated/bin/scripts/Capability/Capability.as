
enum ECapabilityTickGroup
{
    First = 0,
    Input,
    Gameplay,
    Last,
    
    Max
};

enum ECapabilityTickSubGroup
{
    Pre = 0,
    Early,
    Normal,
    Late,
    Post
};

class HailCapability
{
    // Array<HashString> Tags;
    // Array<Object> Blockers;
    
    // Object Owner = nullptr;
    float ActiveDuration = 0.0;
    float InactiveDuration = 0.0;
    
    bool bActive = false;
    bool bDidLateSetup = false;
    bool bDidLateSetupHello = false;

    ECapabilityTickGroup Group = ECapabilityTickGroup::Gameplay;
    ECapabilityTickSubGroup SubGroup = ECapabilityTickSubGroup::Normal;

    void Setup() {}
    void LateSetup() {}
    
    void OnOwnerDestroyed() {}
    
    bool ShouldActivate() const { return false; }
    bool ShouldDeactivate() const { return false; }
    
    void TickActive(float DeltaTime) {}
    void TickInactive(float DeltaTime) {}

    void OnActivated() {}
    void OnDeactivated() {}

    bool GetIsActive() const
    {
        return bActive;
    }

    float GetTimeDilation() const
    {
        return 1.0;
    }

    // bool IsBlocked() const
    // {
    //     return Blockers.Num() > 0;
    // }
};
