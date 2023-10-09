#include <SDL.hpp>
#include <type_traits>
#include <cassert>

inline constexpr SDL::Point Clamp(const SDL::Point& value, const SDL::Point& min, const SDL::Point& max)
{
	assert(min.x <= max.x);
	assert(min.y <= max.y);

	return
	{
		value.x < min.x ? min.x : value.x > max.x ? max.x : value.x,
		value.x < min.y ? min.y : value.y > max.y ? max.y : value.y,
	};
}

inline constexpr SDL::FPoint Clamp(const SDL::FPoint& value, const SDL::FPoint& min, const SDL::FPoint& max)
{
	assert(min.x <= max.x);
	assert(min.y <= max.y);

	return
	{
		value.x < min.x ? min.x : value.x > max.x ? max.x : value.x,
		value.x < min.y ? min.y : value.y > max.y ? max.y : value.y,
	};
}

template<typename T>
inline constexpr T Lerp(double t, const T& min, const T& max)
{
	if constexpr (std::numeric_limits<T>::is_integer)
	{
		return (T)std::round((double)min + (double)(max - min) * t);
	}
	else
	{
		return min + (max - min) * t;
	}
}

template<typename T>
inline constexpr T Lerp(float t, const T& min, const T& max)
{
	if constexpr (std::numeric_limits<T>::is_integer)
	{
		return (T)std::round((float)min + (float)(max - min) * t);
	}
	else
	{
		return min + (max - min) * t;
	}
}

template<typename T>
inline constexpr T LerpClamped(double t, const T& min, const T& max)
{
	if constexpr (std::numeric_limits<T>::is_integer)
	{
		return (T)std::round((double)min + ((double)max - (double)min) * std::clamp(t, 0.0, 1.0));
	}
	else
	{
		return min + (max - min) * std::clamp(t, 0.0, 1.0);
	}
}

template<typename T>
inline constexpr T LerpClamped(float t, const T& min, const T& max)
{
	if constexpr (std::numeric_limits<T>::is_integer)
	{
		return (T)std::round((float)min + ((float)max - (float)min) * std::clamp(t, 0.f, 1.f));
	}
	else
	{
		return min + (max - min) * std::clamp(t, 0.f, 1.f);
	}
}

inline double InverseLerpClamped(SDL::FPoint value, const SDL::FPoint& min, const SDL::FPoint& max)
{
	const SDL::FPoint diff = max - min;
	const float len = diff.mag();

	return std::clamp(SDL::FPoint::dot((value - min) / len, diff / len), 0.f, 1.f);
}

inline double InverseLerp(const SDL::FPoint& value, const SDL::FPoint& min, const SDL::FPoint& max)
{
	const SDL::FPoint diff = max - min;
	const float len = diff.mag();

	return SDL::FPoint::dot((value - min) / len, diff / len);
}

inline constexpr double InverseLerpClamped(double value, double min, double max)
{
	return std::clamp((value - min) / (max - min), 0.0, 1.0);
}

inline constexpr double InverseLerp(double value, double min, double max)
{
	return (value - min) / (max - min);
}

inline constexpr float InverseLerpClamped(float value, float min, float max)
{
	return std::clamp((value - min) / (max - min), 0.f, 1.f);
}

inline constexpr float InverseLerp(float value, float min, float max)
{
	return (value - min) / (max - min);
}

template<typename T>
inline constexpr double InverseLerp(const T& value, const T& min, const T& max)
{
	return (value - min) / (double)(max - min);
}

template<typename T>
inline constexpr double InverseLerpClamped(const T& value, const T& min, const T& max)
{
	return std::clamp((value - min) / (double)(max - min), 0.0, 1.0);
}

template<typename T1, typename T2>
inline constexpr T2 MapRange(const T1& value, const T1& min_in, const T1& max_in, const T2& min_out, const T2& max_out)
{
	return Lerp(InverseLerp(value, min_in, max_in), min_out, max_out);
}

template<typename T1, typename T2>
inline constexpr T2 MapRangeClamped(const T1& value, const T1& min_in, const T1& max_in, const T2& min_out, const T2& max_out)
{
	return Lerp(InverseLerpClamped(value, min_in, max_in), min_out, max_out);
}