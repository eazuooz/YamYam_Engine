#pragma once


namespace ya::math
{
#define PI 3.141592f

	struct Vector2
	{
		static Vector2 One;
		static Vector2 Zero;

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

		Vector2 operator+(Vector2 other)
		{
			return Vector2(x + other.x, y + other.y);
		}
		Vector2 operator-(Vector2 other)
		{
			return Vector2(x - other.x, y - other.y);
		}
		Vector2 operator/(float value)
		{
			return Vector2(x / value, y / value);
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

	Vector2 Rotate(Vector2 vector, float degree)
	{
		float radian = (degree / 180.f) * PI;
		vector.normalize();
		float x = cosf(radian) * vector.x - sinf(radian) * vector.y;
		float y = sinf(radian) * vector.x + cosf(radian) * vector.y;

		return Vector2(x, y);
	}

	float Dot(Vector2& v1, Vector2 v2)
	{
		return v1.x * v2.x + v1.y * v2.y;
	}

	float Cross(Vector2 v1, Vector2 v2)
	{
		return v1.x * v2.y - v1.y * v2.x;
	}
}