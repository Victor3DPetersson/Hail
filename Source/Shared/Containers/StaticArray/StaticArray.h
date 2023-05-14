 #pragma once
#include <cassert>

template <typename Type, uint32_t size>
class StaticArray
{
public:
	StaticArray();
	StaticArray(const std::initializer_list<Type>& initList);
	~StaticArray();

	inline const Type& operator[](const int& aIndex) const;
	inline Type& operator[](const int& aIndex);

	// Utility functions
	inline int Getsize();
	inline void DeleteAll();
	Type* Data() { return m_data; };

private:
	Type m_data[size];
	unsigned int m_size = size - 1;
};

template <typename Type, uint32_t size>
int StaticArray<Type, size>::Getsize()
{
	return m_size;
}

template <typename Type, uint32_t size>
StaticArray<Type, size>::StaticArray()
{
}

template <typename Type, uint32_t size>
StaticArray<Type, size>::StaticArray(const std::initializer_list<Type>& initList) :	m_size(size)
{
	int counter{ 0 };
	assert(initList.size() <= static_cast<unsigned int>(m_size) && "List initialization larger than size.");
	for (Type object : initList)
	{
		m_data[counter] = object;
		counter++;
	}
}

template <typename Type, uint32_t size>
StaticArray<Type, size>::~StaticArray()
{
}

template <typename Type, uint32_t size>
const Type& StaticArray<Type, size>::operator[](const int& index) const
{
	assert(index <= m_size && "Index is out of range");
	return m_data[index];
}

template <typename Type, uint32_t size>
Type& StaticArray<Type, size>::operator[](const int& index)
{
	assert(index <= m_size && "Index is out of range");
	return m_data[index];
}

template <typename Type, uint32_t size>
void StaticArray<Type, size>::DeleteAll()
{
	for (size_t iSlot = 0; iSlot <= static_cast<size_t>(m_size); ++iSlot)
	{
		delete m_data[iSlot];
		m_data[iSlot] = nullptr;
	}
}
