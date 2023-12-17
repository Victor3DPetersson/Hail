#pragma once
#include <cassert>
#include <cstring>

template <typename Type, unsigned int Capacity, bool UseSafeModeFlag = true, typename CountType = unsigned int>
class VectorOnStack
{
public:
	VectorOnStack() = default;
	VectorOnStack(const VectorOnStack& vectorOnStack);
	VectorOnStack(const std::initializer_list<Type>& initList);
	~VectorOnStack();

	VectorOnStack & operator=(const VectorOnStack & vectorOnStack);

	inline const Type& operator[](const CountType & index) const;
	inline Type& operator[](const CountType & index);

	inline void Add(const Type& object);
	inline void Insert(CountType index, Type& object);
	inline void RemoveCyclicAtIndex(CountType itemNumber);
	inline Type* Data() { return m_data; }
	inline void Clear();
	inline void DeleteAll();
	inline bool Empty() { return static_cast<bool>(m_end == 0); }
	Type Last() const;
	inline Type RemoveLast();
	void TransferSize(const VectorOnStack& otherArray);

	__forceinline CountType Size() const;

private:
	Type m_data[Capacity];
	unsigned int m_size = Capacity - 1;
	CountType m_end = 0;
};


template <typename Type, unsigned int Capacity, bool UseSafeModeFlag, typename CountType>
VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType>::VectorOnStack(const VectorOnStack& vectorOnStack)
{
	(*this) = vectorOnStack;
}

template <typename Type, unsigned int Capacity, bool UseSafeModeFlag, typename CountType>
VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType>::VectorOnStack(const std::initializer_list<Type>& initList) : 
	m_end(static_cast<CountType>(initList.size())),
	m_size(Capacity)
{
	int counter{ 0 };
	assert(initList.size() <= static_cast<unsigned int>(m_size) && "List initialization larger than size.");
	for (Type object : initList)
	{
		m_data[counter] = object;
		counter++;
	}
}

template <typename Type, unsigned int Capacity, bool UseSafeModeFlag, typename CountType>
VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType>::~VectorOnStack()
{
	m_size = 0;
	m_end = 0;
}

template <typename Type, unsigned int Capacity, bool UseSafeModeFlag, typename CountType>
VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType> &  VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType>::operator=(const VectorOnStack & vectorOnStack)
{
	if (UseSafeModeFlag == true)
	{
		for (size_t iSlot = 0; iSlot <= vectorOnStack.m_end; ++iSlot)
		{
			m_data[iSlot] = vectorOnStack.m_data[iSlot];
		}
		m_end = vectorOnStack.m_end;
	}
	else
	{
		memcpy(this, &vectorOnStack, sizeof(VectorOnStack));
	}
	return (*this);
}

	

template <typename Type, unsigned int Capacity, bool UseSafeModeFlag, typename CountType>
const Type& VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType>::operator[](const CountType& index) const
{
	assert(index < m_end && "Index is out of range");
	return m_data[index];
}

template <typename Type, unsigned int Capacity, bool UseSafeModeFlag, typename CountType>
Type& VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType>::operator[](const CountType& index)
{
	assert(index < m_end && "Index is out of range");
	return m_data[index];
}

template <typename Type, unsigned int Capacity, bool UseSafeModeFlag, typename CountType>
void VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType>::Add(const Type& object)
{
	assert(m_end <= (m_size) && "Vector is full");

	m_data[m_end] = object;
	m_end++;
}

template <typename Type, unsigned int Capacity, bool UseSafeModeFlag, typename CountType>
void VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType>::Insert(CountType index, Type& object)
{
	assert(m_end <= (m_size) && "Vector is full");
	assert(index <= m_end && "Index is out of range");

	if (index != m_end)
	{
		for (CountType iData = m_end; iData > index; --iData)
		{
			m_data[iData] = m_data[iData - 1];
		}
	}

	m_data[index] = object;
	++m_end;
}

template <typename Type, unsigned int Capacity, bool UseSafeModeFlag, typename CountType>
void VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType>::RemoveCyclicAtIndex(CountType itemNumber)
{
	assert(itemNumber < m_end && "Index is out of range");

	if (m_end != 1)
	{
		m_data[itemNumber] = m_data[m_end - 1];
		--m_end;
	}
	else
	{
		Clear();
	}
}

template <typename Type, unsigned int Capacity, bool UseSafeModeFlag, typename CountType>
void VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType>::DeleteAll()
{
	for (size_t iSlot = 0; iSlot < m_end; ++iSlot)
	{
		delete m_data[iSlot];
		m_data[iSlot] = nullptr;
	}
}

template <typename Type, unsigned int Capacity, bool UseSafeModeFlag, typename CountType>
void VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType>::Clear()
{
	m_end = 0;
}

template <typename Type, unsigned int Capacity, bool UseSafeModeFlag, typename CountType>
Type VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType>::Last() const
{
	//Add error here if end = 0;
	if (m_end != 0)
	{
		return m_data[m_end - 1];
	}
	return Type();
}

template <typename Type, unsigned int Capacity, bool UseSafeModeFlag, typename CountType>
Type VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType>::RemoveLast()
{
	if (m_end != 0)
	{
		return m_data[--m_end];
	}
	return {};
}

template <typename Type, unsigned int Capacity, bool UseSafeModeFlag, typename CountType>
CountType VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType>::Size() const
{
	return m_end;
}


template <typename Type, unsigned int Capacity, bool UseSafeModeFlag, typename CountType>
void VectorOnStack<Type, Capacity, UseSafeModeFlag, CountType>::TransferSize(const VectorOnStack& otherArray)
{
	m_end = otherArray.m_end;
}