#pragma once
#include <cstring>
#include <assert.h>

#include <functional>

template<typename T, typename CountType = unsigned int>
class GrowingArray
{
public:
	GrowingArray();

	GrowingArray(CountType nrOfRecommendedItems, bool useSafeModeFlag = true);
	GrowingArray(const GrowingArray& growingArray);
	GrowingArray(GrowingArray&& growingArray);

	~GrowingArray();

	GrowingArray& operator=(const GrowingArray& growingArray);
	GrowingArray& operator=(GrowingArray&& growingArray);

	void Init(CountType nrOfRecomendedItems, bool useSafeModeFlag = true);
	void InitAndFill(CountType nrOfRecomendedItems, bool useSafeModeFlag = true);
	void ReInit(CountType nrOfRecomendedItems, bool useSafeModeFlag = true);
	void Fill();

	inline T &operator[](const CountType& index);
	inline const T &operator[](const CountType& index) const;

	inline void Add(const T & object);
	inline void Insert(CountType index, const T & object);
	inline void DeleteCyclic(const T & object);
	inline void DeleteCyclicAtIndex(CountType itemNumber);
	inline void RemoveCyclic(const T & object);
	inline void RemoveCyclicAtIndex(CountType itemNumber);
	inline void RemoveAtIndex(CountType itemNumber);
	inline CountType Find(const T & object);

	inline T & GetLast();
	inline const T & GetLast() const;

	static const CountType FoundNone = static_cast<CountType>(-1);

	inline void RemoveAll();
	inline void DeleteAll();
	inline void DeleteAllAndDeinit();

	void Optimize();
	__forceinline CountType Size() const;
	inline bool Empty() { return m_elementCount == 0; }
	inline void Resize(CountType newSize);

	inline bool IsInitialized() const;

	inline void CallFunctionOnAllMembers(std::function<void(T&)> function);

	inline void CallFunctionOnAllMembers(std::function<void(const T&)> function) const;
	inline T* Data() { return m_arrayPointer; }


private:
	inline void MirrorWithMoveSemantics(GrowingArray & growingArray);
	void DumpAll();
	void GrowArray();
	void GrowArray(const CountType growSize);

	T *m_arrayPointer;
	CountType m_elementCount;
	CountType m_sizeActual;

	bool m_useSafeMode;
	bool m_imInitialized;

};

template <typename T, typename CountType>
GrowingArray<typename T, typename CountType>::GrowingArray()
{
	m_arrayPointer = nullptr;
	m_sizeActual = 0;
	m_elementCount = 0;
	m_useSafeMode = true;
	m_imInitialized = false;
}

template <typename T, typename CountType>
GrowingArray<typename T, typename CountType>::~GrowingArray()
{
	//assert(m_imInitialized == true && "Growing Array is not initialized");

	DumpAll();
}

template <typename T, typename CountType>
GrowingArray<typename T, typename CountType>::GrowingArray(CountType nrOfRecommendedItems, bool useSafeModeFlag)
{
	m_arrayPointer = nullptr;
	m_sizeActual = 0;
	m_elementCount = 0;
	m_useSafeMode = true;
	m_imInitialized = false;
	Init(nrOfRecommendedItems, useSafeModeFlag);
}

template <typename T, typename CountType>
GrowingArray<typename T, typename CountType>::GrowingArray(const GrowingArray& growingArray)
{
	m_arrayPointer = nullptr;
	m_sizeActual = 0;
	m_elementCount = 0;
	m_useSafeMode = true;
	m_imInitialized = false;
	(*this) = growingArray;
}

template <typename T, typename CountType>
GrowingArray<typename T, typename CountType>::GrowingArray(GrowingArray&& growingArray)
{
	MirrorWithMoveSemantics(growingArray);
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::Init(CountType nrOfRecomendedItems, bool useSafeModeFlag)
{
	assert(m_imInitialized == false, "Growing Array should not be initialized twice");
	m_sizeActual = nrOfRecomendedItems;
	m_useSafeMode = useSafeModeFlag;
	m_imInitialized = true;

	m_arrayPointer = new T[m_sizeActual];
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::InitAndFill(CountType nrOfRecomendedItems, bool useSafeModeFlag)
{
	assert(m_imInitialized == false, "Growing Array should not be initialized twice");
	m_sizeActual = nrOfRecomendedItems;
	m_useSafeMode = useSafeModeFlag;
	m_imInitialized = true;
	m_arrayPointer = new T[m_sizeActual];
	m_elementCount = m_sizeActual;
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::ReInit(CountType nrOfRecomendedItems, bool useSafeModeFlag)
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	DumpAll();

	Init(nrOfRecomendedItems, useSafeModeFlag);
}

template<typename T, typename CountType>
inline void GrowingArray<T, CountType>::Fill()
{
	m_elementCount = m_sizeActual;
}

template <typename T, typename CountType>
GrowingArray<typename T, typename CountType> & GrowingArray<typename T, typename CountType>::operator=(const GrowingArray& growingArray)
{
	if (m_imInitialized != false)
	{
		DumpAll();
	}

	Init(growingArray.m_sizeActual, growingArray.m_useSafeMode);

	for (CountType iSlot = 0; iSlot < m_sizeActual; ++iSlot)
	{
		growingArray[iSlot] = growingArray.m_arrayPointer[iSlot];
	}

	m_elementCount = growingArray.m_elementCount;
	m_imInitialized = growingArray.m_imInitialized;
		
	return (*this);
}

template <typename T, typename CountType>
GrowingArray<typename T, typename CountType> & GrowingArray<typename T, typename CountType>::operator=(GrowingArray&& growingArray)
{
	MirrorWithMoveSemantics(growingArray);

	return (*this);
}

template <typename T, typename CountType>
T & GrowingArray<typename T, typename CountType>::operator[](const CountType& index)
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	assert(index < m_elementCount, "Index is out of range");
	assert(index >= 0, "Index can not be a negative number");

	return m_arrayPointer[index];
}

template <typename T, typename CountType>
const T & GrowingArray<typename T, typename CountType>::operator[](const CountType& index) const
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	assert(index < m_elementCount, "Index is out of range");
	assert(index >= 0, "Index can not be a negative number");

	return m_arrayPointer[index];
}
	
template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::Add(const T & object)
{
	assert(m_imInitialized == true, "Growing Array is not initialized");

	if (m_elementCount + 1 > (m_sizeActual))
	{
		GrowArray();
	}

	m_arrayPointer[m_elementCount] = object;
	++m_elementCount;
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::Insert(CountType index, const T & object)
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	assert(index <= m_elementCount, "Index is out of range");
	assert(index >= 0, "Index can not be a negative number");

	T tempObject = m_arrayPointer[m_elementCount - 1];

	if (index != m_elementCount)
	{
		for (CountType iData = m_elementCount-1; iData > index; --iData)
		{
			m_arrayPointer[iData] = m_arrayPointer[iData - 1];
		}
	}
	m_arrayPointer[index] = object;
	Add(tempObject);
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::DeleteCyclic(const T & object)
{
	assert(m_imInitialized == true, "Growing Array is not initialized");

	const CountType ItemSlot = Find(object);
	if (ItemSlot != FoundNone)
	{
		DeleteCyclicAtIndex(ItemSlot);
	}	
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::DeleteCyclicAtIndex(CountType itemNumber)
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	assert(itemNumber < m_elementCount, "Index is out of range");
	assert(itemNumber >= 0, "Index can not be a negative number");

	if (m_elementCount != 1)
	{
		if (itemNumber == m_elementCount - 1)
		{
			delete m_arrayPointer[itemNumber];
			m_arrayPointer[itemNumber] = nullptr;
		}
		else
		{
			delete m_arrayPointer[itemNumber];
			m_arrayPointer[itemNumber] = m_arrayPointer[m_elementCount - 1];

		}
		--m_elementCount;
	}
	else
	{
		delete m_arrayPointer[0];
		m_arrayPointer[0] = nullptr;
		RemoveAll();
	}
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::RemoveCyclic(const T & object)
{
	assert(m_imInitialized == true, "Growing Array is not initialized");

	const CountType ItemSlot = Find(object);
	if (ItemSlot != FoundNone)
	{
		RemoveCyclicAtIndex(ItemSlot);
	}
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::RemoveCyclicAtIndex(CountType itemNumber)
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	assert(itemNumber < m_elementCount,"Index is out of range");
	assert(itemNumber >= 0, "Index can not be a negative number");

	if (m_elementCount != 1)
	{
		m_arrayPointer[itemNumber] = m_arrayPointer[m_elementCount - 1];
		--m_elementCount;
	}
	else
	{
		RemoveAll();
	}
}

template <typename T, typename CountType>
void GrowingArray<T, CountType>::RemoveAtIndex(CountType itemNumber)
{
	--m_elementCount;
	for (unsigned int i = itemNumber; i < m_elementCount; ++i)
	{
		m_arrayPointer[i] = m_arrayPointer[i + 1];
	}
}

template <typename T, typename CountType>
CountType GrowingArray<typename T, typename CountType>::Find(const T & object)
{
	assert(m_imInitialized == true, "Growing Array is not initialized");

	for (CountType iSlot = 0; iSlot < m_elementCount; ++iSlot)
	{
		if (m_arrayPointer[iSlot] == object)
		{
			return iSlot;
		}
	}

	return FoundNone;

}

template <typename T, typename CountType>
T & GrowingArray<typename T, typename CountType>::GetLast()
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	assert(m_elementCount > 0, "Vector is empty");

	return m_arrayPointer[m_elementCount - 1];
}

template <typename T, typename CountType>
const T & GrowingArray<typename T, typename CountType>::GetLast() const
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	assert(m_elementCount > 0, "Vector is empty");
	return m_arrayPointer[m_elementCount - 1];
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::RemoveAll()
{
	m_elementCount = 0;
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::DeleteAll()
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	delete[] m_arrayPointer;
	m_arrayPointer = nullptr;
	//for (CountType iSlot = 0; iSlot < m_elementCount; ++iSlot)
	//{
 //		delete m_arrayPointer[iSlot];
	//	m_arrayPointer[iSlot] = nullptr;
	//}
	RemoveAll();
}
template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::DeleteAllAndDeinit()
{
	DeleteAll();
	*this = GrowingArray();
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::Optimize()
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	if (m_sizeActual > m_elementCount)
	{
		GrowArray(m_elementCount);
	}
}

template <typename T, typename CountType>
CountType GrowingArray<typename T, typename CountType>::Size() const
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	return m_elementCount;
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::Resize(CountType aNewSize)
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	while (aNewSize > m_sizeActual)
	{
		GrowArray();
	}

	m_elementCount = aNewSize;
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::MirrorWithMoveSemantics(GrowingArray & growingArray)
{
	if (m_imInitialized != false)
	{
		DumpAll();
	}

	if (growingArray.m_useSafeMode == true)
	{
		//Init(growingArray.m_sizeActual, growingArray.m_useSafeMode);

		m_sizeActual = growingArray.m_sizeActual;
		m_useSafeMode = growingArray.m_useSafeMode;

		m_arrayPointer = growingArray.m_arrayPointer;

		m_elementCount = growingArray.m_elementCount;
		m_imInitialized = growingArray.m_imInitialized;
	}
	else
	{
		memcpy(this, &growingArray, sizeof(GrowingArray));
	}

	growingArray.m_arrayPointer = nullptr;
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::GrowArray()
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	GrowArray(m_sizeActual * 2);
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::GrowArray(const CountType aGrowSize)
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	T* TempPointer = m_arrayPointer;
	const CountType TempCount = m_elementCount;

	m_imInitialized = false;
	Init(aGrowSize);

	if (m_useSafeMode == true)
	{
		for (CountType iSlot = 0; iSlot < TempCount; ++iSlot)
		{
			m_arrayPointer[iSlot] = TempPointer[iSlot];
		}
	}
	else
	{
		memcpy(m_arrayPointer, TempPointer, (sizeof(T) * TempCount));
	}

	delete[] TempPointer;
	TempPointer = nullptr;
}

template <typename T, typename CountType>
void GrowingArray<typename T, typename CountType>::DumpAll()
{
	if (m_imInitialized == true)
	{
		delete[] m_arrayPointer; 
		m_arrayPointer = nullptr;
		m_sizeActual = 0;
	}
	m_imInitialized = false;
	RemoveAll();
}

template <typename T, typename CountType>
bool GrowingArray<typename T, typename CountType>::IsInitialized() const
{
	return m_imInitialized;
}


/*
	TODO ADD explanation on how to use.
*/
template<typename T, typename CountType /*= unsigned short*/>
void GrowingArray<T, CountType>::CallFunctionOnAllMembers(std::function<void(T&)> aFunction)
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	for (CountType iElement = 0; iElement < m_elementCount; ++iElement)
	{
		aFunction((*this)[iElement]);
	}
}

template<typename T, typename CountType /*= unsigned short*/>
void GrowingArray<T, CountType>::CallFunctionOnAllMembers(std::function<void(const T&)> aFunction) const
{
	assert(m_imInitialized == true, "Growing Array is not initialized");
	for (CountType iElement = 0; iElement < m_elementCount; ++iElement)
	{
		aFunction((*this)[iElement]);
	}
}
