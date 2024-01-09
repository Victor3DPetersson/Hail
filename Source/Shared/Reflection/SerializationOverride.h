#pragma once

namespace Hail
{
    class InOutStream;

    class SerializeableObject
    {
    public:
        virtual ~SerializeableObject() {}
    };

    // use this class if you want your object to have its own serialize functionality
    class SerializeableObjectCustom
    {
    public:
        virtual void Serialize(InOutStream& outObject) = 0;
        virtual void Deserialize(InOutStream& inObject) = 0;
    };
}