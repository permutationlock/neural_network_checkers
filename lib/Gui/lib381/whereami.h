// whereami.h
// Glenn G. Chappell
// 2 Oct 2013
//
// For CS 381 Fall 2013
// Header for Camera-Position Computation
// There is no associated source file

#ifndef FILE_WHEREAMI_H_INCLUDED
#define FILE_WHEREAMI_H_INCLUDED

#include <vector>        // For std::vector


// whereAmI
// Given matrix cammat in OpenGL format, representing transformation
//  composed only of translations & rotations, returns 1st 3 components
//  of cammat^(-1)*<0,0,0,1>. That is, finds the point that cammat takes
//  to (the homogeneous form of) the origin. Thus, if cammat is the
//  camera-transformation matrix, then we return the camera position in
//  world coordinates.
template <typename Coord>  // Type of coordinate (probably GLdouble)
auto whereAmI(const Coord * cammat) -> std::vector<Coord>
{
    // Inverses of translation part & rotation part of cammat
    std::vector<Coord> transmatinv(16);
    std::vector<Coord> rotmatinv(16);

    // Begin by storing identity matrices in both transmatinv, rotmatinv
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            transmatinv[j*4+i] = (i == j ? 1 : 0);
            rotmatinv[j*4+i] = (i == j ? 1 : 0);
        }
    }

    // Make transmatinv
    transmatinv[3*4+0] = -cammat[3*4+0];
    transmatinv[3*4+1] = -cammat[3*4+1];
    transmatinv[3*4+2] = -cammat[3*4+2];

    // Make rotmatinv
    for(int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            rotmatinv[j*4+i] = cammat[i*4+j];  // Transpose
        }
    }

    // Our answer is the 1st 3 components of the right-most column of
    //  the inverse of cammat. And the inverse of cammat is
    //  rotmatinv * transmatinv. So do the portion of the matrix
    //  multiplication that we need.
    std::vector<Coord> result(3);
    for (int i = 0; i < 3; ++i)
    {
        int j = 3;
        // Compute i,j entry of cammat^(-1); store in result[i];
        double total = 0.;
        for (int k = 0; k < 4; ++k)
        {
            total += double(rotmatinv[k*4+i]*transmatinv[j*4+k]);
        }
        result[i] = Coord(total);
    }

    return result;
}


#endif //#ifndef FILE_WHEREAMI_H_INCLUDED

