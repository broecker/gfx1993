#ifndef COLOuR_INCLUDED
#define COLOuR_INCLUDED

struct Colour
{
	float		r, g, b, a;

	inline Colour(float R=0.f, float G=0.f, float B=0.f, float A=1.f) : r(R), g(G), b(B), a(A)
	{
	}

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

	inline void set(const Colour& c)
	{
		r = c.r;
		g = c.g;
		b = c.b;
		a = c.a;
	}

	inline const Colour operator + (const Colour& rhs) const
	{
		return Colour(r+rhs.r, g+rhs.g, b+rhs.b, a+rhs.a);
	}

	inline const Colour operator - (const Colour& rhs) const
	{
		return Colour(r-rhs.r, g-rhs.g, b-rhs.b, a-rhs.a);
	}

	inline const Colour operator * (float s) const
	{
		return Colour(r*s, g*s, b*s, a*s);
	}

	inline Colour operator * (const Colour& other) const
	{
		return Colour(r*other.r, g*other.g, b*other.b, a*other.a);
	}

	static Colour lerp(const Colour& a, const Colour& b, float delta)
	{
		return a + (b-a) * delta;
	}
};


#endif