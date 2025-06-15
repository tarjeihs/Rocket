#pragma once

template <typename TValue>
struct TVector
{
    TVector()
        : X(0), Y(0), Z(0)
    {
    }

    TVector(TValue InX, TValue InY, TValue InZ)
        : X(InX), Y(InY), Z(InZ)
    {
    }

    TVector(const TVector& Other)
        : X(Other.X), Y(Other.Y), Z(Other.Z)
    {
    }

    inline TVector operator+(const TVector &Additive)
    {
        return TVector(X + Additive.X, Y + Additive.Y, Z + Additive.Z);
    }

    inline TVector operator-(const TVector &Subtractive)
    {
        return TVector(X - Subtractive.X, Y - Subtractive.Y, Z - Subtractive.Z);
    }

    inline TVector operator*(const TVector &Multiplicative)
    {
        return TVector(X * Multiplicative.X, Y * Multiplicative.Y, Z * Multiplicative.Z);
    }

    inline TVector operator/(const TVector &Divisor)
    {
        return TVector(X / Divisor.X, Y / Divisor.Y, Z / Divisor.Z);
    }

private:
    TValue X;
    TValue Y;
    TValue Z;
};
