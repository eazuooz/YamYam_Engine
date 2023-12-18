#pragma once
#include <cmath>


namespace ya::math
{
#define PI 3.141592f

	static float ConvertDegree(float radian) { return (radian * (180 / PI)); }

	struct Vector2
	{
		static Vector2 Rotate(Vector2 vector, float degree)
		{
			float radian = (degree / 180.f) * PI;
			vector.normalize();
			float x = cosf(radian) * vector.x - sinf(radian) * vector.y;
			float y = sinf(radian) * vector.x + cosf(radian) * vector.y;

			return Vector2(x, y);
		}

		static float Dot(Vector2& v1, Vector2& v2)
		{
			return v1.x * v2.x + v1.y * v2.y;
		}

		static float Cross(Vector2& v1, Vector2& v2)
		{
			return v1.x * v2.y - v1.y * v2.x;
		}

		static Vector2 One;
		static Vector2 Zero;
		static Vector2 Right;
		static Vector2 Left;
		static Vector2 Up;
		static Vector2 Down;

		float x;
		float y;

		Vector2()
			: x(0.0f)
			, y(0.0f)
		{

		}
		Vector2(float _x, float _y)
			: x(_x)
			, y(_y)
		{

		}

		Vector2 operator-()
		{
			return Vector2(-x, -y);
		}

		Vector2 operator+(Vector2 other)
		{
			return Vector2(x + other.x, y + other.y);
		}

		void operator+=(Vector2 other)
		{
			x += other.x;
			y += other.y;
		}

		Vector2 operator-(Vector2 other)
		{
			return Vector2(x - other.x, y - other.y);
		}
		Vector2 operator/(float value)
		{
			return Vector2(x / value, y / value);
		}

		Vector2 operator*(float value)
		{
			return Vector2(x * value, y * value);
		}

		Vector2 operator*(Vector2 v)
		{
			return Vector2(x * v.x, y * v.y);
		}

		bool operator==(Vector2 v)
		{
			return (x == v.x) && (y == v.y);
		}

		void clear()
		{
			x = 0.0f;
			y = 0.0f;
		}

		float length()
		{
			return sqrtf(x * x + y * y);
		}

		Vector2 normalize()
		{
			float len = length();
			x /= len;
			y /= len;

			return *this;
		}
	};


}