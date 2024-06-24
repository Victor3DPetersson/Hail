#include "IncludeScript.as"

void main()
{
    int64 i = 66775;
    string stringToFill;
    stringToFill = "Hello world from angelscript.";
    Print(stringToFill);

    PrintError(stringToFill + globalInt);
}