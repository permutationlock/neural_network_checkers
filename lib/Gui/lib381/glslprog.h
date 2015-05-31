// glslprog.h
// Glenn G. Chappell
// 1 Oct 2013
// Based on code by Orion Lawlor
// Public Domain
//
// For CS 381 Fall 2013
// Header for shader-handling utilities
// There is no associated source file
// Requires GLEW, GLUT
//
// Before including this file, you may not include any GLUT or OpenGL
// headers without including glew.h first.

#ifndef FILE_GLSLPROG_H_INCLUDED
#define FILE_GLSLPROG_H_INCLUDED

// OpenGL/GLUT includes - DO THESE FIRST
#include <cstdlib>       // Do this before GL/GLUT includes
using std::exit;
#ifndef __APPLE__
# include <GL/glew.h>
# include <GL/glut.h>    // GLUT stuff, includes OpenGL headers as well
#else
# include <GLEW/glew.h>
# include <GLUT/glut.h>  // Apple puts glut.h in a different place
#endif

// Other includes
#include <cctype>        // For std::isspace
#include <fstream>       // For std::ifstream
#include <string>        // For std::string, std::getline
#include <iostream>      // For std::cout, std::cin, std::endl;
#include <cstdlib>       // For std::exit, std::size_t


// To create & use a shader program object:
//   o If your GLSL code is stored in text files.
//
//     GLhandleARB prog = makeProgramObjectFromFiles(VSHADER_FILENAME,
//                                                   FSHADER_FILENAME);
//     glUseProgramObjectARB(prog);
//
// -- OR --
//
//   o If your GLSL code is stored in strings (hard-coded in your C++
//      source?).
//
//     GLhandleARB prog = makeProgramObject(VSHADER_SOURCE_STRING,
//                                          FSHADER_SOURCE_STRING);
//     glUseProgramObjectARB(prog);
//
// Multiple shader program objects may be created by making multiple
//  calls to makeProgramObject or makeProgramObjectFromFiles, and
//  storing the return values in different GLhandleARB variables.

// You may also wish to use function getShaderFilenames to read
//  shader filenames from the command line or stdin.


// glslProgErrorExit
// Output given string to cout, followed by newline, then wait for
//  keypress, and do exit(1).
inline
void glslProgErrorExit(const std::string & msg)
{
    std::cout << msg << std::endl << std::endl;
    std::cout << "Press ENTER to quit ";
    while (std::cin.get() != '\n') ;
    std::exit(1);
}


// checkShaderOp
// Print message and exit if GLSL build error.
inline
void checkShaderOp(GLhandleARB obj,
                   GLenum errType,
                   const std::string & where)
{
    GLint compiled;
    glGetObjectParameterivARB(obj, errType, &compiled);
    if (!compiled)
    {
        const GLsizei LOGSIZE = 10000;
        GLcharARB errorLog[LOGSIZE];
        // We need to consider the possibility of buffer overflow here.
        //  Having considered it, we note that OpenGL knows how big our
        //  buffer is, and there is no possibility of overflow.
        GLsizei len = 0;
        glGetInfoLogARB(obj, LOGSIZE, &len, errorLog);
        glslProgErrorExit("ERROR - Could not build GLSL shader: "
                + where + "\n\nERROR LOG:\n" + errorLog);
    }
}


// makeShaderObject
// Create a vertex or fragment shader from given source code.
// Prints message and exits on GLSL compile error.
inline
GLhandleARB makeShaderObject(GLenum target,
                             const std::string & sourceCode)
{
    GLhandleARB h = glCreateShaderObjectARB(target);
    const GLcharARB * sourceCodeP = sourceCode.c_str();
    glShaderSourceARB(h, GLsizei(1), &sourceCodeP, NULL);
    glCompileShaderARB(h);
    checkShaderOp(h, GL_OBJECT_COMPILE_STATUS_ARB,
                  "compile\n" + sourceCode);
    return h;
}


// makeProgramObject
// Create complete program object from these chunks of GLSL shader code.
// Prints message and exits on GLSL build error, or if GLSL unavailable.
// To USE this program object, do glUseProgramObjectARB(RETURN_VALUE);
//
// *****************************************************************
// ** THIS IS THE FUNCTION TO CALL IF YOUR GLSL CODE IS STORED IN **
// ** STRINGS (HARD-CODED IN YOUR C++ SOURCE?).                   **
// *****************************************************************
//
inline
GLhandleARB makeProgramObject(const std::string & vShaderSource,
                              const std::string & fShaderSource)
{
    if (glUseProgramObjectARB == 0)
    { // glew never set up, or OpenGL is too old.
        glslProgErrorExit(
            "ERROR - GLSL not available\n"
            "(Hardware/software too old? glewInit not called?)");
    }

    GLhandleARB p = glCreateProgramObjectARB();
    glAttachObjectARB(p,
        makeShaderObject(GL_VERTEX_SHADER_ARB, vShaderSource));
    glAttachObjectARB(p,
        makeShaderObject(GL_FRAGMENT_SHADER_ARB, fShaderSource));
    glLinkProgramARB(p);
    checkShaderOp(p, GL_OBJECT_LINK_STATUS_ARB, "link");
    return p;
}


// readFileIntoString
// Read entire contents of file with given name into given string.
// Returns true on success, false if read error.
inline
bool readFileIntoString(const std::string & fName,
                        std::string & contents)
{
    contents.clear();
    std::ifstream f(fName.c_str());
    if (!f)
        return false;
    while (true)
    {
        char c = char(f.get());
        if (!f)
            return f.eof();
        contents += c;
    }
}


// makeProgramObjectFromFiles
// Create a complete shader object from these GLSL files.
// Prints message and exits on file read error, GLSL build error,
//  or if GLSL unavailable.
// To USE this program object, do glUseProgramObjectARB(RETURN_VALUE);
//
// *****************************************************************
// ** THIS IS THE FUNCTION TO CALL IF YOUR GLSL CODE IS STORED IN **
// ** TEXT FILES.                                                 **
// *****************************************************************
//
inline
GLhandleARB makeProgramObjectFromFiles(
    const std::string & vFilename = "shader_v.glsl",
    const std::string & fFilename = "shader_f.glsl")
{
    std::string vShaderSource;
    if (!readFileIntoString(vFilename, vShaderSource))
    {
        glslProgErrorExit(
            "ERROR - Could not read vertex shader code file: "
          + vFilename);
    }

    std::string fShaderSource;
    if (!readFileIntoString(fFilename, fShaderSource))
    {
        glslProgErrorExit(
            "ERROR - Could not read fragment shader code file: "
          + fFilename);
    }

    return makeProgramObject(vShaderSource, fShaderSource);
}


// getShaderFilenames
// Gets base for shader source filenames from command line or cin, as
//  appropriate. Sets vFile and fFile to the appropriate filenames.
//  Command-line processing is optional. To do such processing, pass the
//  arguments received by function main; leave them off to disable it.
inline
void getShaderFilenames(std::string & vFilename,
                        std::string & fFilename,
                        int argc=0,
                        char ** argv=0)
{
    std::string fnameBase;  // Base for shader filenames

    // Try to get shader filename base from the command line?
    if (argc > 1)
    {
        fnameBase = argv[1];
    }

    // Iterate until we have a good shader filename base
    // Read one from stdin if we do not have one yet
    while (true)
    {
        // Elim leading space, trailing space & "_" in fnameBase
        while (!fnameBase.empty())
        {
            if (!std::isspace(fnameBase[0]))
                break;
            fnameBase = fnameBase.substr(1);
        }
        while (!fnameBase.empty())
        {
            std::size_t s = fnameBase.size();
            char c = fnameBase[s-1];
            if (!isspace(c) && c != '_')
                break;
            fnameBase.resize(s-1);
        }

        // Did we get a nonempty base name?
        if (!fnameBase.empty())
            break;

        // If not, then input one from stdin
        std::cout << std::endl;
        std::cout << "Enter the base for shader source filenames."
                  << std::endl;
        std::cout << std::endl;
        std::cout << "For example, if you enter `abc', then shaders "
             << "will be read from" << std::endl;
        std::cout << "`abc_v.glsl' and `abc_f.glsl'." << std::endl;
        std::cout << std::endl;
        std::cout << "===> ";  // Prompt
        getline(std::cin, fnameBase);
    }

    // We have a good shader filename base; set shader source filenames
    vFilename = fnameBase + "_v.glsl";
    fFilename = fnameBase + "_f.glsl";
}


#endif //#ifndef FILE_GLSLPROG_H_INCLUDED

