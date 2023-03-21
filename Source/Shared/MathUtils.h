#pragma once



namespace Math
{
	static constexpr float PIf = 3.1415926f;

	template <typename T>
	inline T Max(const T firstValue, const T secondValue)
	{
		if (frstValue < secondValue)
		{
			return secondValue;
		}
		return firstValue;
	}

	template <typename T>
	inline T Min(const T firstValue, const T secondValue)
	{
		if (firstValue < secondValue)
		{
			return firstValue;
		}
		return secondValue;
	}

	template <typename T>
	inline T Abs(const T value)
	{
		if (value < 0)
		{
			return value * (-1);
		}
		return value;
	}

	template <typename T>
	inline T Clamp(const T min, const T max, const T value)
	{
		if (min > max)
		{
			return false;
		}
		if (value > min && value < max)
		{
			return value;
		}
		if (value <= min)
		{
			return min;
		}
		return max;
	}

	template <typename T>
	inline T Lerp(const T x, const T y, const float t)
	{
		return x + t * (y - x);
	}

	template <typename T>
	inline void Swap(T& a, T& b)
	{
		T temp = a;
		a = b;
		b = a;
	}

	constexpr double DegToRad = PIf / 180.0;
	constexpr double RadToDeg = 180.0 / PIf;
	constexpr float DegToRadf = float(DegToRad);
	constexpr float RadToDegf = float(RadToDeg);
}

