// globj.h
// Glenn G. Chappell
// 14 Oct 2013
//
// For CS 381 Fall 2013
// Header for OpenGL Object classes
// There is no associated source file
// Requires GLEW
//
// Before including this file, do not include any GLUT or OpenGL headers
// without including glew.h first.

#ifndef FILE_GLOBJ_H_INCLUDED
#define FILE_GLOBJ_H_INCLUDED

// OpenGL/GLUT includes - DO THESE FIRST
#include <cstdlib>       // Do this before GL/GLUT includes
using std::exit;
#ifndef __APPLE__
# include <GL/glew.h>
# include <GL/glut.h>    // Includes OpenGL headers as well
#else
# include <GLEW/glew.h>
# include <GLUT/glut.h>  // Apple puts glut.h in a different place
#endif
#ifdef _MSC_VER          // Tell MS-Visual Studio about GLEW lib
# pragma comment(lib, "glew32.lib")
#endif


// class GLObj
// Class template for creating classes managing specific kinds of OpenGL
// objects. Holds name of object. Handles generation, binding, deleting.
//
// USAGE
//
// - DO NOT USE THIS TEMPLATE DIRECTLY. Use a class made from this
//   template; see after class GLObj def.
//
// - glewInit() should be called before using member function bind or
//   unbind.
//
// - To manage an OpenGL object, create a (probably global) variable of
//   the appropriate type. For example, for a Vertex Buffer Object:
//
//       VBO vbo1;
//
// - When the OpenGL object is to be used, call member function bind.
//
//       vbo1.bind();
//
// OTHER MEMBER FUNCTIONS (RARELY NEEDED!)
//
// - (RARE) If you wish to switch back to client-side data, call static
//   member function unbind.
//
// - (RARE) To delete an object, call member function free. Note that
//   the destructor calls this, so calling free explicitly is typically
//   not necessary.
//
template<
    void (*GEN)(GLsizei,GLuint*),        // Generate function
    void (*BIND)(GLenum,GLuint),         // Bind function
    void (*DEL)(GLsizei,const GLuint*),  // Delete function
    GLenum TARGET>                       // Target to bind to
class GLObj {

// ***** GLObj: ctors, dctor, op= *****
public:

    // Default ctor
    GLObj()
        :_hasname(false)
    {}

    // Destructor
    // Deletes name if necessary.
    ~GLObj()
    { free(); }

// ***** GLObj: general public functions *****
public:

    // bind
    // Bind our name to our target. First generates name if necessary.
    // glewInit must be called before use.
    void bind()
    {
        if (!_hasname)
        {
            (*GEN)(1, &_name);
            _hasname = true;
        }
        bind_internal(_name);
    }

    // unbind
    // Unbind all objects to our target. Switch back to client-side
    // data.
    // glewInit must be called before use.
    static void unbind()
    {
        bind_internal(0);
    }

    // free
    // If we have a generated name, then delete it.
    void free()
    {
        if (_hasname)
        {
            (*DEL)(1, &_name);
            _hasname = false;
        }
    }

// ***** GLObj: Internal-use functions *****
private:

    // bind_internal
    // Binds given name to our target.
    static void bind_internal(GLuint theName)
    {
        (*BIND)(TARGET, theName);
    }

// ***** GLObj: data members *****
private:

    bool _hasname;  // Do we have a generated name?
    GLuint _name;   // Our name; valid if _hasname

};  // End class GLObj


// Make our own functions, to get around GLEW definition oddities.
inline
void glGenBuffers_GLOBJ_H_INTERNAL_(GLsizei a,GLuint* b)
{ glGenBuffers(a, b); }
inline
void glBindBuffer_GLOBJ_H_INTERNAL_(GLenum a,GLuint b)
{ glBindBuffer(a, b); }
inline
void glDeleteBuffers_GLOBJ_H_INTERNAL_(GLsizei a,const GLuint* b)
{ glDeleteBuffers(a, b); }
inline
void glGenTextures_GLOBJ_H_INTERNAL_(GLsizei a,GLuint* b)
{ glGenTextures(a, b); }
inline
void glBindTexture_GLOBJ_H_INTERNAL_(GLenum a,GLuint b)
{ glBindTexture(a, b); }
inline
void glDeleteTextures_GLOBJ_H_INTERNAL_(GLsizei a,const GLuint* b)
{ glDeleteTextures(a, b); }


// Classes for managing OpenGL objects

// VBO
// Type for Vertex Buffer Object. Binds to GL_ARRAY_BUFFER.
typedef GLObj<
    glGenBuffers_GLOBJ_H_INTERNAL_,
    glBindBuffer_GLOBJ_H_INTERNAL_,
    glDeleteBuffers_GLOBJ_H_INTERNAL_,
    GL_ARRAY_BUFFER> VBO;

// EBO
// Type for Element Buffer Object. Binds to GL_ELEMENT_ARRAY_BUFFER.
typedef GLObj<
    glGenBuffers_GLOBJ_H_INTERNAL_,
    glBindBuffer_GLOBJ_H_INTERNAL_,
    glDeleteBuffers_GLOBJ_H_INTERNAL_,
    GL_ELEMENT_ARRAY_BUFFER> EBO;

// Tex1D
// Type for 1-D Texture Object. Binds to GL_TEXTURE_1D.
typedef GLObj<
    glGenTextures_GLOBJ_H_INTERNAL_,
    glBindTexture_GLOBJ_H_INTERNAL_,
    glDeleteTextures_GLOBJ_H_INTERNAL_,
    GL_TEXTURE_1D> Tex1D;

// Tex2D
// Type for 2-D Texture Object. Binds to GL_TEXTURE_2D.
typedef GLObj<
    glGenTextures_GLOBJ_H_INTERNAL_,
    glBindTexture_GLOBJ_H_INTERNAL_,
    glDeleteTextures_GLOBJ_H_INTERNAL_,
    GL_TEXTURE_2D> Tex2D;

// Tex3D
// Type for 3-D Texture Object. Binds to GL_TEXTURE_3D.
typedef GLObj<
    glGenTextures_GLOBJ_H_INTERNAL_,
    glBindTexture_GLOBJ_H_INTERNAL_,
    glDeleteTextures_GLOBJ_H_INTERNAL_,
    GL_TEXTURE_3D> Tex3D;

// TexCube
// Type for Cube-Map Texture Object. Binds to GL_TEXTURE_CUBE_MAP.
typedef GLObj<
    glGenTextures_GLOBJ_H_INTERNAL_,
    glBindTexture_GLOBJ_H_INTERNAL_,
    glDeleteTextures_GLOBJ_H_INTERNAL_,
    GL_TEXTURE_CUBE_MAP> TexCube;


#endif //#ifndef FILE_GLOBJ_H_INCLUDED

