#include "GameSystem_PCH.h"
#include "CapabilityTester.h"


/*
void SpriteCapabilityData::Init()
{
    Sprite.transform.SetPosition({ 0.5f, 0.5f });
	Sprite.materialInstanceID = 1;
}
*/

// ----------------------------------------

void InputScreenMovementCapability::LateSetup()
{
    //MovementData = Owner.GetOrCreateData(); //GetOrCreateData<MovementCapabilityData>();
    //MovementData->Speed = 0.005f;
}

bool InputScreenMovementCapability::ShouldActivate() const
{
    /*if (InactiveDuration < 0.5f)
        return false;*/
    return true;
}

bool InputScreenMovementCapability::ShouldDeactivate() const
{
    return false;
}

void InputScreenMovementCapability::TickActive(float DeltaTime)
{
    /*
    float VelocityX = 0.0;
    float VelocityY = 0.0;
    if (frameData.inputData.keyMap[m_inputMapping.A] == Hail::KEY_PRESSED)
	{
		direction += (glm::vec2{ -1.0, 0.0 } *g_spriteMovementSpeed);
		moved = true;
	}
	if (frameData.inputData.keyMap[m_inputMapping.D] == Hail::KEY_PRESSED)
	{
		direction += (glm::vec2{ 1.0, 0.0 } *g_spriteMovementSpeed);
		moved = true;
	}
	if (frameData.inputData.keyMap[m_inputMapping.W] == Hail::KEY_PRESSED)
	{
		direction += (glm::vec2{ 0.0, 1.0 } *g_spriteMovementSpeed);
		moved = true;
	}
	if (frameData.inputData.keyMap[m_inputMapping.S] == Hail::KEY_PRESSED)
	{
		direction += glm::vec2{ 0.0, -1.0 } *g_spriteMovementSpeed;
		moved = true;
	}
    */
}

// ----------------------------------------

void ScreenMovementCapability::LateSetup()
{
    //MovementData = Owner.GetOrCreateData(); //GetOrCreateData<MovementCapabilityData>();
    //SpriteData = Owner.GetOrCreateData(); //GetOrCreateData<SpriteData>();
}

bool ScreenMovementCapability::ShouldActivate() const
{
    if (MovementData == nullptr)
        return false;

    if (SpriteData == nullptr)
        return false;
    
    if (MovementData->VelocityX > SmallNumber || MovementData->VelocityX < -SmallNumber )
        return true;

    if (MovementData->VelocityY > SmallNumber || MovementData->VelocityY < -SmallNumber )
        return true;
    return false;
}

bool ScreenMovementCapability::ShouldDeactivate() const
{
    if (MovementData->VelocityX > SmallNumber || MovementData->VelocityX < -SmallNumber )
        return false;

    if (MovementData->VelocityY > SmallNumber || MovementData->VelocityY < -SmallNumber )
        return false;
    return true;
}

void ScreenMovementCapability::TickActive(float DeltaTime)
{
    glm::vec2 Position;
    glm::vec2 Velocity;
    Velocity.x = MovementData->VelocityX * DeltaTime;
    Velocity.y = MovementData->VelocityY * DeltaTime;
    Position.x = MovementData->PositionX;  
    Position.y = MovementData->PositionY;  
    SpriteData->Sprite.transform.SetPosition(Position + Velocity);
    SpriteData->Sprite.transform.LookAt(glm::normalize(Velocity));
}

// ----------------------------------------

CapabilityTester::CapabilityTester()
{
}

void CapabilityTester::Init()
{
    TickWorld = new CapabilityTickWorld();
    TickWorld->Init();

    InputScreenMovementCapability* InputMovementCapa = new InputScreenMovementCapability();
    TickWorld->RegisterCapability(*InputMovementCapa);
    ScreenMovementCapability* MovementCapa = new ScreenMovementCapability();
    TickWorld->RegisterCapability(*MovementCapa);
}

void CapabilityTester::Update(float DeltaTime)
{
    if (!bHasRunInit)
    {
        bHasRunInit = true;
        Init();
    }

    TickWorld->Tick(DeltaTime);
}

