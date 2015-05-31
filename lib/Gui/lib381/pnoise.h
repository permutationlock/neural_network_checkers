// pnoise.h
// Glenn G. Chappell
// 6 Nov 2013
// Header for class PNoise:
//   Noise Functions for Procedural Texture
// There is no associated source file
// Based on work of K. Perlin

/*
Class PNoise includes member functions to generate noise of various
kinds, based on Ken Perlin's "Making Noise" presentation.

    http://www.noisemachine.com/talk1/

Pseudorandom numbers are generated using the GRand package. Seeding is
available in PNoise constructors and member function "seed".

Example code:

    PNoise p(6);          // Seed with 6
                          // Default ctor seeds with unpredictable value

    const int N = 128;    // Array dims; make this anything you want
    double data2d[N][N];

    const int FREQ = 10;  // Frequency for noise; also whatever you want

    // Put basic Perlin-ish noise into data array
    // Parameters are pointer to 1st item in array, x-dimension of
    //  array, y-dimension, x-frequency of noise, y-frequency.
    // This, and other functions in this package, create noise that
    //  wraps smoothly.
    p.pnoise2d(&data2d[0][0], N, N, FREQ, FREQ);

    // Put fake 1/f-noise ("pink noise") into data array
    // The normalize function scales values so that their mean is 0 and
    //  values lie in [-1,1].
    p.fnoise2d(&data2d[0][0], N, N, FREQ, FREQ);
    p.normalize2d(&data2d[0][0], N, N);

    // Put Perlin's "noise with cusps" into data array
    // pnoise_abs is filter function. You may pass a pointer to any
    //  function that takes a double and returns a double. pnoise values
    //  are passed through this function before being summed.
    p.filterfnoise2d(&data2d[0][0], N, N, FREQ, FREQ,
                     pnoise_abs);
    p.normalize2d(&data2d[0][0], N, N);

    // 3-D versions of the above
    double data3d[N][N][N];

    p.pnoise3d(&data3d[0][0][0], N, N, N, FREQ, FREQ, FREQ);

    p.fnoise3d(&data3d[0][0][0], N, N, N, FREQ, FREQ, FREQ);
    p.normalize3d(&data3d[0][0][0], N, N, N);

    p.filterfnoise3d(&data3d[0][0][0], N, N, N, FREQ, FREQ, FREQ,
                     pnoise_abs);
    p.normalize3d(&data3d[0][0][0], N, N, N);
*/

#ifndef FILE_PNOISE_H_INCLUDED
#define FILE_PNOISE_H_INCLUDED

#include "grand.h"  // For class GRand
#include <vector>
using std::vector;


// *********************************************************************
// Global Utility Functions
// *********************************************************************


// pnoise_abs
// Absolute-value function, in case anyone needs one.
// Avoids troubles with std::abs.
//
// This is primarily intended as a filter function for filterfnoise3d
// and filterfnoise2d.
inline
double pnoise_abs(
    double t)
{
    return (t >= 0) ? t : -t;
}


// pnoise_mod
// Given integers a, b, with b > 0, computes a mod b.
// For those who don't know what "mod" means (like the designers of many
// computer languages): a mod b is nonnegative when b is positive. So,
// for example, (-1) mod 3 = 2.
// (No, I don't feel bitter about this; why do you ask? -GGC-)
inline
int pnoise_mod(
    int a,
    int b)
{
    if (a >= 0) return a % b;
    else
    {
        int c = (-a) % b;
        return (c == 0) ? 0 : b-c;
    }
}


// *********************************************************************
// Class PNoise Definition
// *********************************************************************


// class PNoise
// Noise functions for procedural texture.
class PNoise {

// ***** PNoise: ctors & seeding *****
public:

     // Default ctor
     // Seed RNG with unpredictable value.
     PNoise()
         :_rng()
     {}

     // Ctor from int
     // Seed RNG with given value.
     PNoise(int s)
         :_rng(s)
     {}

     // seed (no params)
     // Seed RNG with unpredictable value.
     void seed()
     { _rng.seed(); }

     // seed (int)
     // Seed RNG with given value.
     void seed(int s)
     { _rng.seed(s); }

     // Compiler-generated copy ctor, copy =, dctor used

// ***** PNoise: public functions for 3-D noise generation *****
public:

    // pnoise3d
    // Set given data to Perlin-ish noise.
    void pnoise3d(
        double * data,                    // the 3-D array
        int dimx, int dimy, int dimz,     // array dims
        int freqx, int freqy, int freqz,  // "frequencies"
        double scale=1.,                  // "amplitude"
        double shiftx=0., double shifty=0., double shiftz=0.)
                                          // phase shifts
    {
        clear3d(data, dimx, dimy, dimz);
        addpnoise3d(
            data,
            dimx, dimy, dimz,
            freqx, freqy, freqz,
            scale,
            shiftx, shifty, shiftz);
    }

    // fnoise3d
    // Set given data to pseudo-1/f-noise.
    void fnoise3d(
        double * data,                    // the 3-D array
        int dimx, int dimy, int dimz,     // array dims
        int freqx, int freqy, int freqz,  // "frequencies"
        double scaleratio=0.5,            // scale multiplied by this,
                                          //  when frequency is doubled
        double scale=1.,                  // "amplitude"
        double shiftx=0., double shifty=0., double shiftz=0.)
                                          // phase shifts
    {
        clear3d(data, dimx, dimy, dimz);
        addfnoise3d(
            data,
            dimx, dimy, dimz,
            freqx, freqy, freqz,
            scaleratio,
            scale,
            shiftx, shifty, shiftz);
    }

    // filterpnoise3d
    // Set given data to Perlin-ish noise, each value passed thru given
    // filter function.
    void filterpnoise3d(
        double * data,                    // the 3-D array
        int dimx, int dimy, int dimz,     // array dims
        int freqx, int freqy, int freqz,  // "frequencies"
        double (* filter)(double),        // filter func for pnoise vals
        double scale=1.,                  // "amplitude"
        double shiftx=0., double shifty=0., double shiftz=0.)
                                          // phase shifts
    {
        pnoise3d(
            data,
            dimx, dimy, dimz,
            freqx, freqy, freqz,
            scale,
            shiftx, shifty, shiftz);

        const int size = dimx * dimy * dimz;
        for (int i=0; i<size; ++i)
            data[i] = filter(data[i]);
    }

    // filterfnoise3d
    // Set given data to sum of Perlin-ish noise values, each Perlin-ish
    // noise value passed thru given filter function.
    void filterfnoise3d(
        double * data,                    // the 3-D array
        int dimx, int dimy, int dimz,     // array dims
        int freqx, int freqy, int freqz,  // "frequencies"
        double (* filter)(double),        // filter func for pnoise vals
        double scaleratio=0.5,            // scale multiplied by this,
                                          //  when frequency is doubled
        double scale=1.,                  // "amplitude"
        double shiftx=0., double shifty=0., double shiftz=0.)
                                          // phase shifts
    {
        const int size = dimx * dimy * dimz;
        vector<double> tempdata(size);

        clear3d(data, dimx, dimy, dimz);
        int m;      // multiplier for frequency
        double sf;  // scale factor (multiplier for scale)
        for (m=1, sf=scaleratio;
             freqx*m<=dimx || freqy*m<=dimy || freqz*m<=dimz;
             m*=2, sf*=scaleratio)
        {
            filterpnoise3d(
                &tempdata[0],
                dimx, dimy, dimz,
                m*freqx, m*freqy, m*freqz,
                filter,
                sf*scale,
                shiftx+double(dimx)/(freqx*m*2),
                    shifty+double(dimy)/(freqy*m*2),
                    shiftz+double(dimz)/(freqz*m*2));
            for (int i=0; i<size; ++i)
                data[i] += tempdata[i];
        }
    }

    // normalize3d
    // Given data, normalize it, appling v -> av+b to each value, with
    // a, b chosen so that the new values have mean 0 and are scaled to
    // be as large as possible, while all lying in [-1,1].
    void normalize3d(
        double * data,                 // the 3-D array
        int dimx, int dimy, int dimz)  // array dims
    {
        double max = data[0];
        double min = data[0];
        double sum = data[0];

        const int size = dimx * dimy * dimz;
        int i;
        for (i=1; i<size; ++i)
        {
            if (data[i] > max) max = data[i];
            if (data[i] < min) min = data[i];
            sum += data[i];
        }

        const double avg = sum/size;
        double amp = max-avg;
        if (avg-min > amp) amp = avg-min;
        if (amp == 0.) amp = 1.;

        for (i=0; i<size; ++i)
        {
            data[i] = (data[i]-avg)/amp;
        }
    }

// ***** PNoise: public functions for 2-D noise generation *****
public:

    // pnoise2d
    // 2-D version of pnoise3d.
    void pnoise2d(
        double * data,                    // the 2-D array
        int dimx, int dimy,               // array dims
        int freqx, int freqy,             // "frequencies"
        double scale=1.0,                 // "amplitude"
        double shiftx=0.0, double shifty=0.0)
                                          // phase shifts
    {
        pnoise3d(data,
                 dimx, dimy, 1,
                 freqx, freqy, 1,
                 scale,
                 shiftx, shifty, 0.);
    }

    // fnoise2d
    // 2-D version of fnoise3d.
    void fnoise2d(
        double * data,                    // the 2-D array
        int dimx, int dimy,               // array dims
        int freqx, int freqy,             // "frequencies"
        double scaleratio=0.5,            // scale multiplied by this,
                                          //  when frequency is doubled
        double scale=1.0,                 // "amplitude"
        double shiftx=0.0, double shifty=0.0)
                                          // phase shifts
    {
        fnoise3d(data,
                 dimx, dimy, 1,
                 freqx, freqy, 1,
                 scaleratio, scale,
                 shiftx, shifty, 0.);
    }

    // filterfnoise2d
    // 2-D version of filterfnoise3d.
    void filterfnoise2d(
        double * data,                    // the 2-D array
        int dimx, int dimy,               // array dims
        int freqx, int freqy,             // "frequencies"
        double (* filter)(double),        // filter func for pnoise vals
        double scaleratio=0.5,            // scale multiplied by this,
                                          //  when frequency is doubled
        double scale=1.0,                 // "amplitude"
        double shiftx=0.0, double shifty=0.0)
                                          // phase shifts
    {
        filterfnoise3d(data,
                       dimx, dimy, 1,
                       freqx, freqy, 1,
                       filter,
                       scaleratio, scale,
                       shiftx, shifty, 0.);
    }

    // normalize2d
    // 2-D version of normalize3d
    void normalize2d(
        double * data,                 // the 2-D array
        int dimx, int dimy)            // array dims
    {
        normalize3d(data, dimx, dimy, 1);
    }

// ***** PNoise: internal-use functions *****
private:

    // pnoise_fade
    // Returns Perlin's S-shaped fade curve.
    // Valid for t in [-1.,1.].
    double pnoise_fade(
        double t)
    {
        double u = (t >= 0) ? 1-t : 1+t;
        return u*u*u*(u*(u*6-15)+10);
    }

    // addpnoise3d
    // Add Perlin-ish noise to given data.
    void addpnoise3d(
        double * data,                    // the 3-D array
        int dimx, int dimy, int dimz,     // array dims
        int freqx, int freqy, int freqz,  // "frequencies"
        double scale, //=1.0              // "amplitude"
        double shiftx, double shifty, double shiftz)  //=0.0
                                          // phase shifts
    {
        double wlenx = double(dimx) / freqx;  // "wavelengths"
        double wleny = double(dimy) / freqy;
        double wlenz = double(dimz) / freqz;

        vector<int> xwhichs, ywhichs, zwhichs;
        for (int wx = 0; wx < freqx; ++wx)
        {
            double centerx = wlenx * wx + shiftx;
            int lo = int(centerx - wlenx + 1);
            int hi = int(centerx + wlenx);
            if (hi >= lo)
                xwhichs.push_back(wx);
        }
        for (int wy = 0; wy < freqy; ++wy)
        {
            double centery = wleny * wy + shifty;
            int lo = int(centery - wleny + 1);
            int hi = int(centery + wleny);
            if (hi >= lo)
                ywhichs.push_back(wy);
        }
        for (int wz = 0; wz < freqz; ++wz)
        {
            double centerz = wlenz * wz + shiftz;
            int lo = int(centerz - wlenz + 1);
            int hi = int(centerz + wlenz);
            if (hi >= lo)
                zwhichs.push_back(wz);
        }

        typedef vector<int>::const_iterator Vici;
        for (Vici xit = xwhichs.begin(); xit != xwhichs.end(); ++xit) {
        double centerx = wlenx * (*xit) + shiftx;
        for (Vici yit = ywhichs.begin(); yit != ywhichs.end(); ++yit) {
        double centery = wleny * (*yit) + shifty;
        for (Vici zit = zwhichs.begin(); zit != zwhichs.end(); ++zit) {
        double centerz = wlenz * (*zit) + shiftz;

            int vx = 0., vy = 0., vz = 0.;

            // Choose random vector, len = sqrt(2)
            int r = _rng.i(12);
            if (r < 8)
                vx = (r & 1) ? -1. : 1;
            else
                vy = (r & 1) ? -1. : 1;
            if (r < 4)
                vy = (r & 2) ? -1. : 1;
            else
                vz = (r & 2) ? -1. : 1;

            for (int ix = int(centerx-wlenx+1);
                 ix <= int(centerx + wlenx);
                 ++ix)
            {
                double tx = (ix - centerx) / wlenx;
                double fadex = pnoise_fade(tx);
                int coordx = pnoise_mod(ix, dimx);
                for (int iy = int(centery-wleny+1);
                     iy <= int(centery + wleny);
                     ++iy)
                {
                    double ty = (iy - centery) / wleny;
                    double fadey = pnoise_fade(ty);
                    int coordy = pnoise_mod(iy, dimy);
                    for (int iz = int(centerz-wlenz+1);
                         iz <= int(centerz + wlenz);
                         ++iz)
                    {
                        double tz = (iz - centerz) / wlenz;
                        double fadez = pnoise_fade(tz);
                        int coordz = pnoise_mod(iz, dimz);

                        double dot = tx*vx + ty*vy + tz*vz;
                        double fade = fadex * fadey * fadez;
                        data[coordx*dimy*dimz + coordy*dimz + coordz]
                            += dot * fade * scale;
                    }
                }
            } // End ix/y/z for loops

        }}} // End x/y/zwhichs for loops
    }

    // addfnoise3d
    // Add pseudo-1/f-noise to given data.
    void addfnoise3d(
        double * data,                    // the 3-D array
        int dimx, int dimy, int dimz,     // array dims
        int freqx, int freqy, int freqz,  // "frequencies"
        double scaleratio, //=0.5         // scale multiplied by this,
                                          //  when frequency is doubled
        double scale, //=1.0              // "amplitude"
        double shiftx, double shifty, double shiftz) //=0.0
                                          // phase shifts
    {
        int m;      // multiplier for frequency
        double sf;  // scale factor (multiplier for scale)
        for (m=1, sf=scaleratio;
             freqx*m<=dimx || freqy*m<=dimy || freqz*m<=dimz;
             m*=2, sf*=scaleratio)
        {
            addpnoise3d(
                data,
                dimx, dimy, dimz,
                m*freqx, m*freqy, m*freqz,
                sf*scale,
                shiftx+double(dimx)/(freqx*m*2),
                    shifty+double(dimy)/(freqy*m*2),
                    shiftz+double(dimz)/(freqz*m*2));
        }
    }

    // clear3d
    // Set given data to 0.
    void clear3d(
        double * data,                // the 3-D array
        int dimx, int dimy, int dimz) // array dims
    {
        const int size = dimx * dimy * dimz;
        for (int i=0; i<size; ++i)
             data[i] = 0.;
    }

// ***** PNoise: data members *****
private:

     GRand _rng;  // Our random-number generator

};  // End class PNoise


#endif //#ifndef FILE_PNOISE_H_INCLUDED

