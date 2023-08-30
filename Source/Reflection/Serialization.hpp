#pragma once

#include "Utility\FilePath.hpp"

#include "Reflection.h"
#include "DebugMacros.h"

#include "Utility\InOutStream.h"
#include "Utility\StringUtility.h"

namespace Hail
{
    namespace Reflection
    {
        struct SerializationTypeHeader
        {
            String64 m_typeName;
            String64 m_fieldName;
            size_t m_typeLength = 0;
        };

        void WriteFieldHeader(InOutStream& outObject, SerializationTypeHeader header)
        {
            outObject.Write(&header, sizeof(SerializationTypeHeader), 1);
        }
        SerializationTypeHeader ReadFieldHeader(InOutStream& inObject)
        {
            SerializationTypeHeader headerToWriteToo;
            inObject.Read(&headerToWriteToo, sizeof(SerializationTypeHeader), 1);
            return headerToWriteToo;
        }


        template<typename T>
        void SerializeObject(T& outClass, Hail::FilePath pathToSerializeToo, String64 objectName, String64 objectType)
        {
            if (!pathToSerializeToo.CreateFileDirectory())
            {
                return;
            }
            if (pathToSerializeToo.IsFile())
            {
                pathToSerializeToo = pathToSerializeToo.Parent();
            }
            const Class* objectInfo = GetClass<T>();
            Hail::InOutStream inOutObject;
            Hail::FileObject objectToOpen = Hail::FileObject(objectName, objectType, pathToSerializeToo);

            inOutObject.OpenFile(pathToSerializeToo + objectToOpen, Hail::FILE_OPEN_TYPE::WRITE, true);

            inOutObject.Write(&objectInfo->numberOfFields, sizeof(uint16_t), 1);
            for (int i = 0; i < objectInfo->numberOfFields; i++)
            {
                const auto& field = objectInfo->fields[i];
                if (field.type == nullptr)
                {
                    Debug_PrintConsoleConstChar("Type not declared");
                    continue;
                }
                Debug_PrintConsoleString256(String256::Format("Size: %i Typename: %s Field offset: %i", field.type->size, field.type->name.Data(), field.offset));
                int8_t* source = reinterpret_cast<int8_t*>(&outClass) + field.offset;

                WriteFieldHeader(inOutObject, { field.type->name, field.name, field.type->size });
                inOutObject.Write(source, field.type->size, 1);

            }
            Debug_PrintConsoleString64(String64::Format("Number of fields: %i", objectInfo->numberOfFields));
        }

        template<typename T>
        T DeserializeObject(Hail::FilePath pathToSerializeToo, String64 objectName, String64 objectType) {

            if (pathToSerializeToo.IsFile())
            {
                pathToSerializeToo = pathToSerializeToo.Parent();
            }
            Hail::InOutStream inObject;
            Hail::FileObject objectToOpen = Hail::FileObject(objectName, objectType, pathToSerializeToo);

            inObject.OpenFile(pathToSerializeToo + objectToOpen, Hail::FILE_OPEN_TYPE::READ, true);
            uint16 numberOfFields = 0;
            inObject.Read(&numberOfFields, sizeof(uint16_t), 1);

            T result{};
            const Class* objectInfo = GetClass<T>();

            for (uint16_t i = 0; i < numberOfFields; i++)
            {
                SerializationTypeHeader header = ReadFieldHeader(inObject);
                Debug_PrintConsoleString64(header.m_typeName);
                Debug_PrintConsoleString64(header.m_fieldName);

                bool foundTypeInCurrentClass = false;
                //Search for field in code-struct, if object have been updated we only paste the value from the old file if it exists in current class
                for (uint16 j = 0; j < objectInfo->numberOfFields; j++)
                {
                    const Field& field = objectInfo->fields[j];
                    if (StringCompare(header.m_fieldName.Data(), field.name.Data()) && StringCompare(header.m_typeName.Data(), field.type->name.Data()))
                    {
                        int8_t* destination = reinterpret_cast<int8_t*>(&result) + field.offset;
                        inObject.Read(destination, field.type->size, 1);
                        foundTypeInCurrentClass = true;
                        break;
                    }
                }
                if (!foundTypeInCurrentClass)
                {
                    inObject.Seek(header.m_typeLength, 1);
                }
            }
            return result;
        }
    }
}

