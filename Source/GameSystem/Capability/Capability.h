#pragma once
#include "ComponentOwner.h"

class CapabilityTickLinkedElement;

enum class ECapabilityTickGroup : uint8_t
{
    First = 0,
    Input,
    Gameplay,
    Last,
    
    Max
};

enum class ECapabilityTickSubGroup
{
    Pre = 0,
    Early,
    Normal,
    Late,
    Post
};

class Capability
{
public:
    ECapabilityTickGroup Group = ECapabilityTickGroup::Gameplay;
    ECapabilityTickSubGroup SubGroup = ECapabilityTickSubGroup::Normal;

    virtual ~Capability() {}
    virtual void Setup() {}
    virtual void LateSetup() {}
    
    virtual void OnOwnerDestroyed() {}
    
    virtual bool ShouldActivate() const { return false; }
    virtual bool ShouldDeactivate() const { return false; }
    
    virtual void TickActive(float DeltaTime) {}
    virtual void TickInactive(float DeltaTime) {}

    virtual void OnActivated() {}
    virtual void OnDeactivated() {}

    bool GetIsActive() const
    {
        return bActive;
    }

protected:
    float ActiveDuration = 0.0;
    float InactiveDuration = 0.0;
    ComponentOwner Owner;
    
private:
    float GetTimeDilation()
    {
        return 1.0;
    }
    
    friend  CapabilityTickLinkedElement;
    bool bActive = false;
    bool bDidLateSetup = false;
};
