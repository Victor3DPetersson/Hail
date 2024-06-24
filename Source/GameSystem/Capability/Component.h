#pragma once

struct ComponentOwner;

// "Component"
struct Component
{
    virtual ~Component() {}
    ComponentOwner* Owner = nullptr;
};

// have static array of Components?

