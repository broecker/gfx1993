#ifndef COLOuR_INCLUDED
#define COLOuR_INCLUDED

#include <glm/glm.hpp>

struct Color
{
	float		r, g, b, a;

	inline Color(float R=0.f, float G=0.f, float B=0.f, float A=1.f) : r(R), g(G), b(B), a(A)
	{
	}

	inline Color(const glm::vec4& v) : r(v.x), g(v.y), b(v.z), a(v.w) {}

	inline void setBlack()
	{
		r = g = b = a = 0.f;
	}

	inline void set(float R, float G, float B, float A=1.f)
	{
		r = R;
		g = G;
		b = B;
		a = A;
	}

	inline void set(const Color& c)
	{
		r = c.r;
		g = c.g;
		b = c.b;
		a = c.a;
	}

	inline const Color operator + (const Color& rhs) const
	{
		return Color(r + rhs.r, g + rhs.g, b + rhs.b, a + rhs.a);
	}

	inline const Color operator - (const Color& rhs) const
	{
		return Color(r - rhs.r, g - rhs.g, b - rhs.b, a - rhs.a);
	}

	inline const Color operator * (float s) const
	{
		return Color(r * s, g * s, b * s, a * s);
	}

	inline Color operator * (const Color& other) const
	{
		return Color(r * other.r, g * other.g, b * other.b, a * other.a);
	}

	static Color lerp(const Color& a, const Color& b, float delta)
	{
		return a + (b-a) * delta;
	}
};


#endif