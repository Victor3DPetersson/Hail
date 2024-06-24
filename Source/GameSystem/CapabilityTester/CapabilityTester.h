#pragma once
#include "Capability/Component.h"
#include "Capability/Capability.h"
#include "CapabilityTickManager/CapabilityTickManager.h"
#include "../Engine/RenderCommands.h"

struct SpriteComponent : Component
{
    Hail::RenderCommand_Sprite Sprite;
    void Init();

    // static list lookup of Entity owners?
};

struct MovementComponent : Component
{
    float PositionX = 0.0f;
    float PositionY = 0.0f;

    float VelocityX = 0.0f;
    float VelocityY = 0.0f;

    float Speed = 1.0f;
    
    void Init();
};

class InputScreenMovementCapability : public Capability
{
    ~InputScreenMovementCapability()  {};
    void LateSetup() override;
    bool ShouldActivate() const override;
    bool ShouldDeactivate() const override;
    
    void TickActive(float DeltaTime) override;
    
    MovementComponent* MovementData = nullptr;
};

class ScreenMovementCapability : public Capability
{
    ~ScreenMovementCapability() {}
    void LateSetup() override;
    bool ShouldActivate() const override;
    bool ShouldDeactivate() const override;
    void TickActive(float DeltaTime) override;

    MovementComponent* MovementData = nullptr;
    SpriteComponent* SpriteData = nullptr;
    float SmallNumber = 0.001f;
};

// ------------------------------

class CapabilityTester
{
public:
    CapabilityTester();

    void Update(float DeltaTime);

private:
    void Init();

    bool bHasRunInit = false;
    CapabilityTickWorld* TickWorld = nullptr;
};