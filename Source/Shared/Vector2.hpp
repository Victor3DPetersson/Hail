#pragma once
#include <assert.h>
#include <math.h>

template<class T>
class Vector2
{

public:

	union
	{			
		T x;
		T u;
	};

	union
	{
		T y;
		T V;
	};

	Vector2<T>()
	{
		x = static_cast<T>(0);
		y = static_cast<T>(0);
	}
	
	Vector2<T>(const T x, const T y) : x(x), y(y)
	{
	}

	Vector2<T>(const Vector2<T>& vec) : x(vec.x), y(vec.y)
	{
	}

	static const Vector2 Zero, UnitX, UnitY, One;

	T& operator()(const int index)
	{
		assert(index >= 0 && index < 2);
		switch (index)
		{
		case 0:
			return x;
		case 1:
			return y;
		default:
			break;
		}
	}

	Vector2<T>& operator=(const Vector2<T>& vec)
	{
		x = vec.x;
		y = vec.y;
		return *this;
	}

	~Vector2<T>() = default;

	inline T LengthSqr() const
	{
		return ((x * x) + (y * y));
	}

	inline T Length() const
	{
		return sqrt((x * x) + (y * y));
	}

	inline Vector2<T> GetNormalized() const
	{
		T length = sqrt((x * x) + (y * y));
		if (length == 0)
		{
			return Vector2<T>::Zero;
		}
		return *this / length;
	}

	inline void Normalize()
	{
		T length = sqrt((x * x) + (y * y));
		if (length == 0)
		{
			 *this = Vector2<T>::Zero;
		}
		else
		{
			*this /= length;
		}
	}

	inline T Dot(const Vector2<T>& vec) const
	{
		return ((this->x * vec.x) + (this->y * vec.y));
	}

	inline static Vector2<T> ComponentWiseLerp(const Vector2<T>& vec1, const Vector2<T>& vec2, const T t)
	{
		return { vec1.x + t * (vec2.x - vec1.x), vec1.y + t * (vec2.y - vec1.y) };
	}
	inline Vector2<T>& operator/=(const Vector2<T>& vec)
	{
		x /= vec.x;
		y /= vec.y;

		return *this;
	}

};

template<class T>Vector2<T> operator+(const Vector2<T>& vec1, const Vector2<T>& vec2)
{
	return Vector2<T>(vec1.x + vec2.x, vec1.y + vec2.y);
}
template<class T> Vector2<T> operator-(const Vector2<T>& vec1, const Vector2<T>& vec2)
{
	return Vector2<T>(vec1.x - vec2.x, vec1.y - vec2.y);
}
template<class T> Vector2<T> operator*(const Vector2<T>& vec, const T& scalar)
{
	return  Vector2<T>(vec.x * scalar, vec.y * scalar);
}
template<class T> Vector2<T> operator*(const T& scalar, const Vector2<T>& vec)
{
	return Vector2<T>(vec.x * scalar, vec.y * scalar);
}
template<class T> Vector2<T> operator/(const Vector2<T>& vec, const T& scalar)
{
	return Vector2<T>(vec.x / scalar, vec.y / scalar);
}
template<class T> void operator+=(Vector2<T>& vec1, const Vector2<T>& vec2)
{
	vec1.x += vec2.x;
	vec1.y += vec2.y;
}
template<class T> void operator-=(Vector2<T>& vec1, const Vector2<T>& vec2)
{
	vec1.x -= vec2.x;
	vec1.y -= vec2.y;
}
template<class T> void operator*=(Vector2<T>& vec1, const T& scalar)
{
	vec1.x *= scalar;
	vec1.y *= scalar;
}
template<class T> void operator/=(Vector2<T>& vec1, const T& scalar)
{
	vec1.x /= scalar;
	vec1.y /= scalar;
}
template<class T> bool operator==(Vector2<T>& vec1, const Vector2<T>& vec2)
{
	if (vec1.x == vec2.x && vec1.y == vec2.y)
	{
		return true;
	}
	return false;
}

using Vector2c = Vector2<char>;
using Vector2i = Vector2<int>;
using Vector2ui = Vector2<unsigned int>;
using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;

template<typename T> const Vector2<T> Vector2<T>::Zero(static_cast<T>(0), static_cast<T>(0));
template<typename T> const Vector2<T> Vector2<T>::UnitX(static_cast<T>(1), static_cast<T>(0));
template<typename T> const Vector2<T> Vector2<T>::UnitY(static_cast<T>(0), static_cast<T>(1));
template<typename T> const Vector2<T> Vector2<T>::One(static_cast<T>(1), static_cast<T>(1));