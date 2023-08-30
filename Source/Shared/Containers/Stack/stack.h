#pragma once

template <typename T>
class Stack
{
public:
	Stack();
	T Pop();
	const T& Top() const;
	T& Top();
	inline int Size() const;
	void Push(const T& value);

private:
	GrowingArray<T> m_data;
};

template <typename T>
T Stack<T>::Pop()
{
	assert(m_data.Size() > 0 && "Trying to pop top element on empty stack!");
	T temp{};
	if (m_data.Size() > 0)
	{
		temp = Top();
		m_data.RemoveLast();
	}
	return temp;
}

template <typename T>
const T & Stack<T>::Top() const
{
	assert(m_data.Size() > 0 && "Trying to access top element on empty stack!");
	return m_data.GetLast();
}

template <typename T>
T& Stack<T>::Top()
{
	assert(m_data.Size() > 0 && "Trying to access top element on empty stack!");
	return m_data.GetLast();
}

template <typename T>
void Stack<T>::Push(const T& value)
{
	m_data.Add(value);
}

template <typename T>
Stack<T>::Stack()
{
	m_data.Init(10);
}

template <typename T>
int Stack<T>::Size() const
{
	return m_data.Size();
}
