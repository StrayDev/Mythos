#pragma once
#include <initializer_list>

namespace Mythos
{
	template <typename T, int N>
	class vector
	{
	public:
		vector();
		explicit vector(T val);
		vector(const std::initializer_list<T>& values);

		union
		{
			struct
			{
				T data[N];
			};
		};

		T magnitude() const;
		vector<T, N> normalize() const;
		T dot(const vector<T, N>& other) const;

		vector<T, N> operator+(const vector<T, N>& other) const;
		vector<T, N> operator-(const vector<T, N>& other) const;
		vector<T, N> operator*(T scalar) const;
		vector<T, N> operator/(T scalar) const;
	};

	// --

	template <typename T>
	class vector<T, 2>
	{
	public:
		vector(T tx, T ty) : x(tx), y(ty) {} 

		union
		{
			struct
			{
				T x, y;
			};
			T data[2];
		};

		static const vector<T, 2> up;
		static const vector<T, 2> down;
		static const vector<T, 2> left;
		static const vector<T, 2> right;
	};

	template <typename T>
	const vector<T, 2> vector<T, 2>::up(0, 1);

	template <typename T>
	const vector<T, 2> vector<T, 2>::down(0, -1);

	template <typename T>
	const vector<T, 2> vector<T, 2>::left(-1, 0);

	template <typename T>
	const vector<T, 2> vector<T, 2>::right(1, 0);

	// --

	template <typename T>
	class vector<T, 3>
	{
	public:
		vector(T tx, T ty, T tz) : x(tx), y(ty), z(tz) {}

		union
		{
			struct
			{
				T x, y, z;
			};
			T data[3];
		};

		static const vector<T, 3> up;
		static const vector<T, 3> down;
		static const vector<T, 3> left;
		static const vector<T, 3> right;
		static const vector<T, 3> forward;
		static const vector<T, 3> backward;
	};

	template <typename T>
	const vector<T, 3> vector<T, 3>::up(0, 1, 0);

	template <typename T>
	const vector<T, 3> vector<T, 3>::down(0, -1, 0);

	template <typename T>
	const vector<T, 3> vector<T, 3>::left(-1, 0, 0);

	template <typename T>
	const vector<T, 3> vector<T, 3>::right(1, 0, 0);

	template <typename T>
	const vector<T, 3> vector<T, 3>::forward(0, 0, 1);

	template <typename T>
	const vector<T, 3> vector<T, 3>::backward(0, 0, -1);

	// --

	template <typename T>
	class vector<T, 4>
	{
	public:
		vector(T tx, T ty, T tz, T tw) : x(tx), y(ty), z(tz), w(tw) {}

		union
		{
			struct
			{
				T x, y, z, w;
			};
			T data[4];
		};
	};

	// --

	template <typename T, int N>
	vector<T, N>::vector()
	{
		for (int i = 0; i < N; i++)
		{
			data[i] = 0;
		}
	}

	template <typename T, int N>
	vector<T, N>::vector(T val)
	{
		for (int i = 0; i < N; i++)
		{
			data[i] = val;
		}
	}

	template <typename T, int N>
	vector<T, N>::vector(const std::initializer_list<T>& values)
	{
		int i = 0;
		for (auto it = values.begin(); it != values.end() && i < N; ++it, ++i)
		{
			data[i] = *it;
		}
		for (; i < N; i++)
		{
			data[i] = 0;
		}
	}

	template <typename T, int N>
	T vector<T, N>::magnitude() const
	{
		T sum = 0;
		for (int i = 0; i < N; i++)
		{
			sum += data[i] * data[i];
		}
		return sqrt(sum);
	}

	template <typename T, int N>
	vector<T, N> vector<T, N>::normalize() const
	{
		T mag = magnitude();
		if (mag != 0)
		{
			return (*this) / mag;
		}
		return vector<T, N>();
	}

	template <typename T, int N>
	T vector<T, N>::dot(const vector<T, N>& other) const
	{
		T sum = 0;
		for (int i = 0; i < N; i++)
		{
			sum += data[i] * other.data[i];
		}
		return sum;
	}

	template <typename T, int N>
	vector<T, N> vector<T, N>::operator+(const vector<T, N>& other) const
	{
		vector<T, N> result;
		for (int i = 0; i < N; i++)
		{
			result.data[i] = data[i] + other.data[i];
		}
		return result;
	}

	template <typename T, int N>
	vector<T, N> vector<T, N>::operator-(const vector<T, N>& other) const
	{
		vector<T, N> result;
		for (int i = 0; i < N; i++)
		{
			result.data[i] = data[i] - other.data[i];
		}
		return result;
	}

	template <typename T, int N>
	vector<T, N> vector<T, N>::operator*(T scalar) const
	{
		vector<T, N> result;
		for (int i = 0; i < N; i++)
		{
			result.data[i] = data[i] * scalar;
		}
		return result;
	}

	template <typename T, int N>
	vector<T, N> vector<T, N>::operator/(T scalar) const
	{
		vector<T, N> result;
		for (int i = 0; i < N; i++)
		{
			result.data[i] = data[i] / scalar;
		}
		return result;
	}

}

using int2 = Mythos::vector<int, 2>;
using int3 = Mythos::vector<int, 3>;
using int4 = Mythos::vector<int, 4>;

using float2 = Mythos::vector<float, 2>;
using float3 = Mythos::vector<float, 3>;
using float4 = Mythos::vector<float, 4>;

using color3 = Mythos::vector<float, 3>;
using color4 = Mythos::vector<float, 4>;

