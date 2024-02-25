 #pragma once
#include <cassert>
#include <initializer_list>

template <typename Type, size_t size = static_cast<size_t>(128)>
class StaticArray
{
public:
	StaticArray() = default;

	StaticArray(const Type initialArrayValue);
	StaticArray(const std::initializer_list<Type>& initList);

	inline const Type& operator[](const int& index) const;
	inline Type& operator[](const int& index);

	// Utility functions
	inline size_t Getsize();
	Type* Data() { return m_data; };

private:
	Type m_data[size];
};

template <typename Type, size_t size>
size_t StaticArray<Type, size>::Getsize()
{
	return size;
}

template <typename Type, size_t size>
StaticArray<Type, size>::StaticArray(const Type initialArrayValue)
{
	for (size_t i = 0; i < size; i++)
	{
		m_data[i] = initialArrayValue;
	}
}

template <typename Type, size_t size>
StaticArray<Type, size>::StaticArray(const std::initializer_list<Type>& initList)
{
	int counter{ 0 };
	assert(initList.size() <= static_cast<unsigned int>(size) && "List initialization larger than size.");
	for (Type object : initList)
	{
		m_data[counter] = object;
		counter++;
	}
}

template <typename Type, size_t size>
const Type& StaticArray<Type, size>::operator[](const int& index) const
{
	assert(index <= size && "Index is out of range");
	return m_data[index];
}

template <typename Type, size_t size>
Type& StaticArray<Type, size>::operator[](const int& index)
{
	assert(index <= size && "Index is out of range");
	return m_data[index];
}

