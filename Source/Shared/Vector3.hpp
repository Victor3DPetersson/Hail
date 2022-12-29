#pragma once
#include "math.h"
#include <assert.h>

template<class T>
class Vector3
{

public:
	union
	{
		T x;
		T r;
	};

	union
	{
		T y;
		T g;
	};

	union
	{
		T z;
		T b;
	};

	Vector3<T>()
	{
		x = static_cast<T>(0);
		y = static_cast<T>(0);
		z = static_cast<T>(0);
	}

	Vector3<T>(const T& x, const T& y, const T& z) : x(x), y(y), z(z)
	{
	}
	Vector3<T>(const Vector3<T>& vec) : x(vec.x), y(vec.y), z(vec.z)
	{
	}

	T& operator()(const int index)
	{
		assert(index >= 0 && index < 3);
		switch (index)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			break;
		}
	}

	Vector3<T>& operator=(const Vector3<T>& vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
		return *this;
	}
	~Vector3<T>() = default;

	inline T LengthSqr() const
	{
		return (x * x) + (y * y) + (z * z);
	}

	inline T Length() const
	{
		return (T)sqrt((x * x) + (y * y) + (z * z));
	}

	inline Vector3<T> GetNormalized() const
	{
		T length = (T)sqrt((x * x) + (y * y) + (z * z));
		if (length == 0)
		{
			return Vector3<T>::Zero;
		}
		return *this / length;
	}

	inline void Normalize()
	{
		T length = (T)sqrt((x * x) + (y * y) + (z * z));
		if (length == 0)
		{
			*this = Vector3<T>::Zero;
		}
		else
		{
			*this /= length;
		}
	}

	inline T Dot(const Vector3<T>& aVector) const
	{
		return ((x * aVector.x) + (y * aVector.y) + (z * aVector.z));
	}

	inline Vector3<T> Cross(const Vector3<T>& aVector) const
	{
		Vector3<T> cross;
		cross.x = (y * aVector.z) - (z * aVector.y);
		cross.y = (z * aVector.x) - (x * aVector.z);
		cross.z = (x * aVector.y) - (y * aVector.x);
		return (cross);
	}

	static const Vector3	Zero,
		UnitX,
		UnitY,
		UnitZ,
		One;
};

template<class T>Vector3<T> operator+(const Vector3<T>& vec1, const Vector3<T>& vec2)
{
	return Vector3<T>( vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z );
}

template<class T> Vector3<T> operator-(const Vector3<T>& vec1, const Vector3<T>& vec2)
{
	return Vector3<T>( vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z );
}

template<class T> Vector3<T> operator*(const Vector3<T>& vec, const T& scalar)
{
	return Vector3<T>( vec.x * scalar, vec.y * scalar, vec.z * scalar );
}

template<class T> Vector3<T> operator*(const T& scalar, const Vector3<T>& vector)
{
	return Vector3<T>( vec.x * scalar, vec.y * scalar, vec.z * scalar );
}

template<class T> Vector3<T> operator/(const Vector3<T>& aVector, const T& scalar)
{
	return Vector3<T>( vec.x / scalar, vec.y / scalar, vec.z / scalar );
}

template<class T> void operator+=(Vector3<T>& vec1, const Vector3<T>& vec2)
{
	vec1.x += vec2.x;
	vec1.y += vec2.y;
	vec1.z += vec2.z;
}

template<class T> void operator-=(Vector3<T>& vec1, const Vector3<T>& vec2)
{
	vec1.x -= vec2.x;
	vec1.y -= vec2.y;
	vec1.z -= vec2.z;
}

template<class T> void operator*=(Vector3<T>& vec, const T& scalar)
{
	vec.x *= scalar;
	vec.y *= scalar;
	vec.z *= scalar;
}

template<class T> void operator/=(Vector3<T>& vec, const T& scalar)
{
	vec.x /= scalar;
	vec.y /= scalar;
	vec.z /= scalar;
}

template<class T> bool operator==(Vector3<T>& vec1, const Vector3<T>& vec2)
{
	if (vec1.x == vec2.x && vec1.y == vec2.y &&	vec1.z == vec2.z)
	{
		return true;
	}
	return false;
}

using Vector3c = Vector3<char>;
using Vector3i = Vector3<int>;
using Vector3ui = Vector3<unsigned int>;
using Vector3f = Vector3<float>;
using Vector3d = Vector3<double>;

template<typename T> const Vector3<T> Vector3<T>::Zero(0, 0, 0);
template<typename T> const Vector3<T> Vector3<T>::UnitX(1, 0, 0);
template<typename T> const Vector3<T> Vector3<T>::UnitY(0, 1, 0);
template<typename T> const Vector3<T> Vector3<T>::UnitZ(0, 0, 1);
template<typename T> const Vector3<T> Vector3<T>::One(1, 1, 1);
