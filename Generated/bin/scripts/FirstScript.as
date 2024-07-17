#include "IncludeScript.as"

void main()
{
    Vec2 direction = GetDirectionInput(PlayerMoveJoystickL, 0);

    DrawLineNormalized(Vec2(0), Vec2(1));
    DrawLineNormalizedScreenAlligned(Vec2(0.5), (direction * 0.5) + 0.5);

    if (GetButtonInput(eInputAction::PlayerAction1, 0) == 1)
    {
        Print("Triggered input from AS!!!");
    }
}