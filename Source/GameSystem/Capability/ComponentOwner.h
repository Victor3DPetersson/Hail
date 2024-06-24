#pragma once
#include "Component.h"

// "Entity / Actor"
struct ComponentOwner
{
    int Id = -1; // hash?

   template<class T>
    T* GetOrCreateData()
    {
       // go to class lookup and see if we have this data
       // otherwise create it
       Component* Data = nullptr;
        return Data;
    }
};
