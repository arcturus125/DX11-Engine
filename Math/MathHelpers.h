//--------------------------------------------------------------------------------------
// Math convenience functions
//--------------------------------------------------------------------------------------

#ifndef _MATH_HELPERS_H_DEFINED_
#define _MATH_HELPERS_H_DEFINED_

#include <cmath>


// Surprisingly, pi is not *officially* defined anywhere in C++
const float PI = 3.14159265359f;



// Test if a float value is approximately 0
// Epsilon value is the range around zero that is considered equal to zero
const float EPSILON = 0.5e-6f; // For 32-bit floats, requires zero to 6 decimal places
inline bool IsZero(const float x)
{
    return std::abs(x) < EPSILON;
}


// 1 / Sqrt. Used often (e.g. normalising) and can be optimised, so it gets its own function
inline float InvSqrt(const float x)
{
    return 1.0f / std::sqrt(x);
}


// Pass an angle in degrees, returns the angle in radians
inline float ToRadians(float d)
{
    return  d * PI / 180.0f;
}

// Pass an angle in radians, returns the angle in degrees
inline float ToDegrees(float r)
{
    return  r * 180.0f / PI;
}


#endif // _MATH_HELPERS_H_DEFINED_
