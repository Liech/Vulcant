float unpackHalf(uint val16)
{
    uint sign = (val16 & 0x8000u) << 16u;
    uint exp  = (val16 & 0x7C00u) >> 10u;
    uint mant = (val16 & 0x03FFu);

    if (exp == 0u)
    {
        if (mant == 0u)
        {
            return asfloat(sign);
        }
        while ((mant & 0x0400u) == 0u)
        {
            mant <<= 1u;
            exp--;
        }
        exp++;
        mant &= ~0x0400u;
    }
    else if (exp == 31u)
    {
        exp = 255u;
    }
    else
    {
        exp = exp + (127u - 15u);
    }

    uint res = sign | (exp << 23u) | (mant << 13u);
    return asfloat(res);
}

uint packHalf(float val)
{
    uint f = asuint(val);
    uint sign = (f >> 16u) & 0x8000u;
    int exp  = int((f >> 23u) & 0xFFu) - 127;
    uint mant = f & 0x007FFFFFu;

    if (exp < -24)
    {
        return sign;
    }
    if (exp < -14)
    {
        mant |= 0x00800000u;
        uint shift = uint(-14 - exp);
        mant >>= shift;
        return sign | (mant >> 13u);
    }
    if (exp > 15)
    {
        return sign | 0x7C00u;
    }

    uint hexp = uint(exp + 15) << 10u;
    uint hmant = mant >> 13u;
    return sign | hexp | hmant;
}
