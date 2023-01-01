#pragma once

namespace CommonUtilities
{
	template <typename T>
	class Stack
	{
	public:
		Stack(); // konstruktor som skapar en tom stack
		~Stack();

		T Pop(); // Assertar om stacken är tom

		const T & Top() const; // Assertar om stacken är tom

		inline int Size() const;

		void Push(const T& aValue);

	private:
		GrowingArray<T, int> myData;
	};

	template <typename T>
	T Stack<T>::Pop()
	{
		assert(myData.Size() > 0 && "Trying to pop top element on empty stack!");
		T temp = Top();
		myData.RemoveCyclicAtIndex(myData.Size() - 1);
		return temp;
	}

	template <typename T>
	const T & Stack<T>::Top() const
	{
		assert(myData.Size() > 0 && "Trying to access top element on empty stack!");
		return myData.GetLast();
	}

	template <typename T>
	void Stack<T>::Push(const T& aValue)
	{
		myData.Add(aValue);
	}

	template <typename T>
	Stack<T>::Stack()
	{
		myData.Init(10);
	}

	template <typename T>
	Stack<T>::~Stack()
	{
	}

	template <typename T>
	int Stack<T>::Size() const
	{
		return myData.Size();
	}
}