// grand.h
// VERSION 1.0
// Glenn G. Chappell
// 25 Sep 2013
//
// Public Domain
//
// Header for class GRand
// There is no associated source file
//
// Convenience wrapper around C++11 RNG.
// Intended for simple pseudorandom number generation, or as sample
//  code. Not for cryptographic use.

/*

EXAMPLE USAGE

GRand r;      // Seeded with unpredictable value
GRand r2(7);  // A separate pseudorandom number generator
              //  A fixed seed results in a predictable sequence
GRand r3;
r3.seed(2);   // Another way to seed

// Print 10 values, each in { 0, 1, 2, 3 }
for (int k = 0; k < 10; ++k)
    cout << r.i(4) << endl;

// Make a random decision
if (r.b())
   cout << "Yes!" << endl;
else
   cout << "No." << endl;

*/

#ifndef FILE_GRAND_H_INCLUDED
#define FILE_GRAND_H_INCLUDED

#include <random>
// For:
//  std::random_device
//  std::mt19937
//  std::uniform_int_distribution
//  std::uniform_real_distribution
//  std::bernoulli_distribution
#include <cassert>
// For:
//  assert

// Version number
// Guaranteed to increase with with each release
#define GRAND_PACKAGE_VERSION (10000)  // 1 00 00 means 1.0.0


// class GRand
// Good random number generation
// Convenience wrapper around C++11 RNG.
class GRand {

// ***** GRand: ctors, dctor, op= *****
public:

    // Default ctor
    // Seed RNG with unpredictable value.
    GRand()
        :_rdev()
    { seed(); }

    // Ctor from number
    // Seed RNG with given value.
    // Template so we can seed from anything without warnings.
    // Requirements on Types:
    //     SeedType must be convertible to std::mtt19937::result_type
    //      (guaranteed to be an unsigned integral type).
    template <typename SeedType>
    GRand(SeedType s)
        :_rdev()
    { seed(s); }

    // Copy ctor
    // New GRand object is now a separate sequence duplicating other.
    // So, for example, the next value generated by (*this) and other --
    // assuming no seeding in the meantime -- will be the same.
    // Note: We must write this, as std::random_device has no copy ctor.
    GRand(const GRand & other)
        :_rdev(),
         _rng(other._rng)
    {}

    // Copy assignment operator
    // Copies just like copy ctor.
    // Note: We must write this, as std::random_device has no copy op=.
    GRand & operator=(const GRand & rhs)
    {
        _rng = rhs._rng;
        return *this;
    }

    // Compiler-generated dctor used.

// ***** GRand: general public functions *****
public:

    // seed - no parameters
    // Seed RNG with unpredictable value.
    void seed()
    { _rng.seed(_rdev()); }

    // seed from number
    // Seed RNG with given value.
    // Template so we can seed from anything without warnings.
    // Requirements on Types:
    //     SeedType must be convertible to std::mtt19937::result_type
    //      (guaranteed to be an unsigned integral type).
    template <typename SeedType>
    void seed(SeedType s)
    { _rng.seed(std::mt19937::result_type(s)); }

    // i
    // Return random integer in range [0, n-1], or 0 if n <= 0. Range is
    //  [0, 1] if no parameter given.
    int i(int n=2)
    {
        if (n <= 0) return 0;
        return std::uniform_int_distribution<int>(0, n-1)(_rng);
    }

    // d
    // Return random double in range [0.0, d) if d > 0.0, in range
    //  (d, 0.0] if d < 0, or 0.0 if d == 0.0. Range is [0.0, 1.0) if no
    //  parameter given.
    double d(double d=1.0)
    {
        if (d > 0.0)
            return std::uniform_real_distribution<double>(0.0, d)(_rng);
        if (d < 0.0)
            return
                -std::uniform_real_distribution<double>(0.0, -d)(_rng);
        return 0.0;
    }

    // b
    // Return random bool value. Probability of true is given value, or
    //  0.5 if none given.
    bool b(double p=0.5)
    {
        if (p <= 0.0) return false;
        if (p >= 1.0) return true;
        return std::bernoulli_distribution(p)(_rng);
    }

// ***** GRand: data members *****
private:

    std::random_device _rdev;  // For unpredictable seed values
    std::mt19937 _rng;         // RNG; uses Mersenne Twister algorithm

};  // End class GRand

#endif //#ifndef FILE_GRAND_H_INCLUDED
