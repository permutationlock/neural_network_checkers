// tshapes.h
// Glenn G. Chappell
// 30 Oct 2013
//
// For CS 381 Fall 2012
// Header for 3-D Shapes with Texture Coordinates
// There is no associated source file

#ifndef FILE_TSHAPES_H_INCLUDED
#define FILE_TSHAPES_H_INCLUDED

// OpenGL/GLUT includes - DO THESE FIRST
#include <cstdlib>       // Do this before GL/GLUT includes
using std::exit;
#ifndef __APPLE__
# include <GL/glut.h>    // Includes OpenGL headers as well
#else
# include <GLUT/glut.h>  // Apple puts glut.h in a different place
#endif

// Other includes
#include <cmath>         // For std::cos, std::sin


// Functions to draw 3-D shapes
//
// All use OpenGL and draw using glBegin .. glEnd.
// All use the current transformation and color, leaving these
//  unchanged.
// All include normals and 2-D texture coordinates.
// Functions drawCube, drawSphere, and drawTorus are drop-in
//  replacements for glutSolidCube, glutSolidSphere, and glutSolidTorus,
//  respectively. However, the functions defined here include texture
//  coordinates.
// Functions drawSquare and drawCylinder do not correspond to any
//  GLUT functionality.


// drawCube
// Draw cube with given side length, centered at origin.
// Same as glutSolidCube, but with texture coords.
// If tanloc is passed, send tangent vector to GLSL attribute vec3 with
//  the given location.
inline
void drawCube(double size,
              GLint tanloc = -1)
{
    // For performance, we avoid matrix operations & function calls
    double h = size/2.0;  // Half of cube side
    double mh = -h;       // Minus h, to minimize computation

    glBegin(GL_QUADS);
        // +z face
        if (tanloc != -1)
            glVertexAttrib3dARB(tanloc, 1., 0., 0.);
        glNormal3d(0., 0., 1.);
        glTexCoord2d(0., 0.);
        glVertex3d(mh, mh, h);
        glTexCoord2d(1., 0.);
        glVertex3d(h, mh, h);
        glTexCoord2d(1., 1.);
        glVertex3d(h, h, h);
        glTexCoord2d(0., 1.);
        glVertex3d(mh, h, h);

        // -z face
        if (tanloc != -1)
            glVertexAttrib3dARB(tanloc, -1., 0., 0.);
        glNormal3d(0., 0., -1.);
        glTexCoord2d(0., 0.);
        glVertex3d(h, mh, mh);
        glTexCoord2d(1., 0.);
        glVertex3d(mh, mh, mh);
        glTexCoord2d(1., 1.);
        glVertex3d(mh, h, mh);
        glTexCoord2d(0., 1.);
        glVertex3d(h, h, mh);

        // +x face
        if (tanloc != -1)
            glVertexAttrib3dARB(tanloc, 0., 0., -1.);
        glNormal3d(1., 0., 0.);
        glTexCoord2d(0., 0.);
        glVertex3d(h, mh, h);
        glTexCoord2d(1., 0.);
        glVertex3d(h, mh, mh);
        glTexCoord2d(1., 1.);
        glVertex3d(h, h, mh);
        glTexCoord2d(0., 1.);
        glVertex3d(h, h, h);

        // -x face
        if (tanloc != -1)
            glVertexAttrib3dARB(tanloc, 0., 0., 1.);
        glNormal3d(-1., 0., 0.);
        glTexCoord2d(0., 0.);
        glVertex3d(mh, mh, mh);
        glTexCoord2d(1., 0.);
        glVertex3d(mh, mh, h);
        glTexCoord2d(1., 1.);
        glVertex3d(mh, h, h);
        glTexCoord2d(0., 1.);
        glVertex3d(mh, h, mh);

        // +y face
        if (tanloc != -1)
            glVertexAttrib3dARB(tanloc, 1., 0., 0.);
        glNormal3d(0., 1., 0.);
        glTexCoord2d(0., 0.);
        glVertex3d(mh, h, h);
        glTexCoord2d(1., 0.);
        glVertex3d(h, h, h);
        glTexCoord2d(1., 1.);
        glVertex3d(h, h, mh);
        glTexCoord2d(0., 1.);
        glVertex3d(mh, h, mh);

        // -y face
        if (tanloc != -1)
            glVertexAttrib3dARB(tanloc, 1., 0., 0.);
        glNormal3d(0., -1., 0.);
        glTexCoord2d(0., 0.);
        glVertex3d(mh, mh, mh);
        glTexCoord2d(1., 0.);
        glVertex3d(h, mh, mh);
        glTexCoord2d(1., 1.);
        glVertex3d(h, mh, h);
        glTexCoord2d(0., 1.);
        glVertex3d(mh, mh, h);
    glEnd();
}


// drawSphere
// Draw solid sphere.
// Same as glutSolidSphere, but with texture coords.
// If tanloc is passed, send tangent vector to GLSL attribute vec3 with
//  the given location.
inline
void drawSphere(double radius,
                int slices, int stacks,
                int tanloc = -1)
{
    const double pi = 3.1415926535898;
    for (int i1 = 0; i1 < stacks; ++i1)
    {
        double t1a = double(i1)/stacks;
        double a1a = t1a * pi;
        double t1b = double(i1+1)/stacks;
        double a1b = t1b * pi;
        glBegin(GL_QUAD_STRIP);
        for (int i2 = 0; i2 <= slices; ++i2)
        {
            double t2 = double(i2)/slices;
            double a2 = t2 * 2.0*pi;

            double nax = cos(a2)*sin(a1a);
            double nay = sin(a2)*sin(a1a);
            double naz = cos(a1a);
            double pax = radius * nax;
            double pay = radius * nay;
            double paz = radius * naz;

            glNormal3d(nax, nay, naz);
            if (tanloc != -1)
                glVertexAttrib3dARB(tanloc, cos(a2)*cos(a1a),
                                            sin(a2)*cos(a1a),
                                            -sin(a1a));
            glTexCoord2d(t1a, t2);
            glVertex3d(pax, pay, paz);

            double nbx = cos(a2)*sin(a1b);
            double nby = sin(a2)*sin(a1b);
            double nbz = cos(a1b);
            double pbx = radius * nbx;
            double pby = radius * nby;
            double pbz = radius * nbz;

            glNormal3d(nbx, nby, nbz);
            if (tanloc != -1)
                glVertexAttrib3dARB(tanloc, cos(a2)*cos(a1b),
                                            sin(a2)*cos(a1b),
                                            -sin(a1b));
            glTexCoord2d(t1b, t2);
            glVertex3d(pbx, pby, pbz);
        }
        glEnd();
    }
}


// drawTorus
// Draw solid torus.
// Same as glutSolidTorus, but with texture coords
// If tanloc is passed, send tangent vector to GLSL attribute vec3 with
//  the given location.
inline
void drawTorus(double smallrad,
               double bigrad,
               int smallsubdivs,
               int bigsubdivs,
               int tanloc = -1)
{
    const double pi = 3.1415926535898;
    for (int i1 = 0; i1 < bigsubdivs; ++i1)
    {
        double t1a = double(i1)/bigsubdivs;
        double a1a = t1a * 2.0*pi;
        double t1b = double(i1+1)/bigsubdivs;
        double a1b = t1b * 2.0*pi;
        glBegin(GL_QUAD_STRIP);
        for (int i2 = 0; i2 <= smallsubdivs; ++i2)
        {
            double t2 = double(i2)/smallsubdivs;
            double a2 = t2 * 2.0*pi;

            double nax = cos(a2)*cos(a1a);
            double nay = cos(a2)*sin(a1a);
            double naz = sin(a2);
            double pax = (bigrad + smallrad * cos(a2))*cos(a1a);
            double pay = (bigrad + smallrad * cos(a2))*sin(a1a);
            double paz = smallrad * sin(a2);
            glNormal3d(nax, nay, naz);
            if (tanloc != -1)
                glVertexAttrib3dARB(tanloc, -sin(a1a), cos(a1a), 0.);
            glTexCoord2d(t1a, t2);
            glVertex3d(pax, pay, paz);

            double nbx = cos(a2)*cos(a1b);
            double nby = cos(a2)*sin(a1b);
            double nbz = sin(a2);
            double pbx = (bigrad + smallrad * cos(a2))*cos(a1b);
            double pby = (bigrad + smallrad * cos(a2))*sin(a1b);
            double pbz = smallrad * sin(a2);
            glNormal3d(nbx, nby, nbz);
            if (tanloc != -1)
                glVertexAttrib3dARB(tanloc, -sin(a1b), cos(a1b), 0.);
            glTexCoord2d(t1b, t2);
            glVertex3d(pbx, pby, pbz);
        }
        glEnd();
    }
}


// drawSubdividedSquare
// Draw a square, centered at origin, vertices at x,y,z = +/- 1.
// Uses given number of subdivisions.
// If tanloc is passed, send tangent vector to GLSL attribute vec3 with
//  the given location.
inline
void drawSubdividedSquare(int subdivs,
                          int tanloc = -1)
{
    // Square in x,y-plane
    glNormal3d(0., 0., 1.);  // Normal for every vertex in square
    if (tanloc != -1)
        glVertexAttrib3dARB(tanloc, 1., 0., 0.);
    for (int k = 0; k < subdivs; ++k)
    {
        double s1 = double(k)/subdivs;
        double x1 = s1 * 2.0 - 1.0;
        double s2 = double(k+1)/subdivs;
        double x2 = s2 * 2.0 - 1.0;
        glBegin(GL_QUAD_STRIP);
            for(int i = 0; i <= subdivs; ++i)
            {
                double t = double(i)/subdivs;
                double y = t * 2.0 - 1.0;
                glTexCoord2d(s1, t);
                glVertex3d(x1, y, 0.);
                glTexCoord2d(s2, t);
                glVertex3d(x2, y, 0.);
            }
        glEnd();
    }
}


// drawCylinder
// Draw a cylinder, radius 1, length 2, centered at origin, around
//  x-axis.
// Uses given number of subdivisions.
// If tanloc is passed, send tangent vector to GLSL attribute vec3 with
//  the given location.
inline
void drawCylinder(int subdivs,
                  int tanloc = -1)
{
    const double pi = 3.1415926535898;
    const double halflen = 1.;  // Half of cylinder's length
    const double radius = 1.;   // Cylinder's radius

    if (tanloc != -1)
        glVertexAttrib3dARB(tanloc, 1., 0., 0.);
    for (int i = 0; i < subdivs; ++i)
    {
        double s1 = double(i)/subdivs;
        double x1 = -halflen + s1 * 2. * halflen;
        double s2 = double(i+1)/subdivs;
        double x2 = -halflen + s2 * 2. * halflen;
        glBegin(GL_QUAD_STRIP);
        for (int k = 0; k <= subdivs*4; ++k)
        {
            double t = double(k)/(subdivs*4);
            double a = t * 2.*pi;
            double ca = std::cos(a);
            double sa = std::sin(a);
            double y = radius * ca;
            double z = radius * sa;
            glNormal3d(0., ca, sa);
            glTexCoord2d(s2, t);
            glVertex3d(x2, y, z);
            glTexCoord2d(s1, t);
            glVertex3d(x1, y, z);
        }
        glEnd();
    }
}


#endif //#ifndef FILE_TSHAPES_H_INCLUDED

