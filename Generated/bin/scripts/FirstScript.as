#include "IncludeScript.as"

float gameTime = 0.0;

Array<Vec2> savedPositions;
Vec2 playerPosition = Vec2(0.5, 0.5);
float movementSpeed = 0.01;

int GetAMathematicalNumber(float gameTimeToInt)
{
    float constantValue = 1337.56789;
    //returnValue += 27;
    
    int returnValue = gameTimeToInt;
    returnValue += 27;
    returnValue /= 2;
    return returnValue;
}

void main()
{
    gameTime += 0.1;
    Vec2 direction = GetDirectionInput(PlayerMoveJoystickL, 0);
    direction.y = direction.y * -1.0;
    playerPosition += direction * movementSpeed;
    
    //direction.x += gameTime * 0.15;
    string inputOutputText = "Triggered input from AS and fetched a String! Number: ";
    //DrawLineNormalized(Vec2(0), Vec2(0.5));
    DrawLineNormalizedScreenAlligned(Vec2(0.5), (direction * 0.5) + 0.5);
    DrawCircleNormalized(playerPosition, 0.05);
    int numberFromAFunction = GetAMathematicalNumber(gameTime);

    inputOutputText += numberFromAFunction;
    inputOutputText += '!';

    if (GetButtonInput(eInputAction::PlayerAction1, 0) == 1)
    {
        savedPositions.Add(playerPosition);
        Print(inputOutputText);
    }

    for (uint i = 0; i < savedPositions.Count(); i++)
    {
        DrawCircleNormalized(savedPositions[i], 0.005);
    }

}