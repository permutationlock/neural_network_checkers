// rtt.h
// Glenn G. Chappell
// 15 Nov 2013
// 
// For CS 381 Fall 2013
// Header for class RTT: render-to-texture
// There is no associated source file
// Requires GLEW
//
// Before including this file, do not include any GLUT or OpenGL headers
// without including glew.h first.

#ifndef FILE_RTT_H_INCLUDED
#define FILE_RTT_H_INCLUDED

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

// Other includes
#include <string>        // For std::string,
#include <iostream>      // For std::cout, std::cin, std::endl
#include <cstdlib>       // For std::exit


// class RTT
// Render-to-texture
// Handles OpenGL framebuffer objects for render-to-texture. Also
//  handles associated renderbuffers (for depth buffering), mimmap
//  generation, and management of texture names. Works with both
//  standard 2-D textures and cube maps.
// Does not handle texture channel/unit activation.
//
// Usage Examples:
//
// To render a 2-D texture and send it to shaders over channel 0.
//
//   Globals:
//     RTT tex0;
//     const int TEX_WIDTH = 256, TEX_HEIGHT = 256;
//
//   In a function:
//     tex0.init(GL_TEXTURE_2D, TEX_WIDTH, TEX_HEIGHT);
//         // Above can also be passed as constructor parameters.
//
//     // Note: OpenGL must be initialized for the following
//     glActiveTexture(GL_TEXTURE0);
//     tex0.beginRender();
//     [ DRAW HERE; do glClear, but not glutSwapBuffers ]
//     tex0.endRender();
//     glTexParameteri(GL_TEXTURE_2D, ...
//
// To create a 2-D texture and render to it later:
//
//   Globals:
//     RTT tex0;
//     const int TEX_WIDTH = 256, TEX_HEIGHT = 256;
//
//   In a function:
//     tex0.init(GL_TEXTURE_2D, TEX_WIDTH, TEX_HEIGHT);
//
//     // Note: OpenGL must be initialized for the following
//     glActiveTexture(GL_TEXTURE0);
//     tex0.bind();
//     glTexParameteri(GL_TEXTURE_2D, ...
//
//   In another function:
//     tex0.beginRender();
//     [ DRAW HERE; do glClear, but not glutSwapBuffers ]
//     tex0.endRender();
//    
// To render a cube map:
//
//   Globals:
//     RTT tex0;
//     const int TEX_WIDTH = 256, TEX_HEIGHT = 256;
//
//   In a function:
//     tex0.init(GL_TEXTURE_CUBE_MAP, TEX_WIDTH, TEX_HEIGHT);
//
//     // Note: OpenGL must be initialized for the following
//     glActiveTexture(GL_TEXTURE0);
//     tex0.beginRender(GL_TEXTURE_CUBE_MAP_POSITIVE_X);
//     [ DRAW +X FACE HERE ]
//     tex0.endRender(false);  // "false": no mipmap generation yet
//     tex0.beginRender(GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
//     [ DRAW -X FACE HERE ]
//     tex0.endRender(false);
//     tex0.beginRender(GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
//     [ DRAW +Y FACE HERE ]
//     tex0.endRender(false);
//     tex0.beginRender(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
//     [ DRAW -Y FACE HERE ]
//     tex0.endRender(false);
//     tex0.beginRender(GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
//     [ DRAW +Z FACE HERE ]
//     tex0.endRender(false);
//     tex0.beginRender(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
//     [ DRAW -Z FACE HERE ]
//     tex0.endRender(true);   // "true": generate mipmaps now
//     glTexParameteri(GL_TEXTURE_2D, ...
//
class RTT {

// ***** RTT: Ctors, dctor, op= *****
public:

    // Default ctor
    // Leave in un-inited state
    // Member function init should be called later.
    RTT()
       :isinited_(false),
        iscubemap_(false),
        texnamegenerated_(false),
        buffersmade_(false),
        renderingnow_(false)
    {}

    // Init ctor
    // Save as default ctor, then call init
    RTT(GLenum textarget_in,
        GLsizei imgwidth_in,
        GLsizei imgheight_in)
       :isinited_(false),
        iscubemap_(false),
        texnamegenerated_(false),
        buffersmade_(false),
        renderingnow_(false)
    { init(textarget_in, imgwidth_in, imgheight_in); }

    // Dctor
    // Deletes OpenGL objects
    ~RTT()
    { deleteNames(); }

private:

    // Uncopyable class; no copy ctor, op=
    RTT(const RTT &);
    RTT & operator=(const RTT &);

// ***** RTT: Public functions callable before OpenGL init *****
public:

    // init
    // Initialize basic info
    // OpenGL not required to be init'ed before calling this.
    void init(GLenum textarget_in,
             GLsizei imgwidth_in,
             GLsizei imgheight_in)
    {
        if (renderingnow_)
        {
            errorExit("init",
                      "Called during texture render");
        }

        if (isinited_)
            deleteNames();

        textarget_ = textarget_in;
        imgwidth_= imgwidth_in;
        imgheight_ = imgheight_in;

        iscubemap_ = (textarget_in == GL_TEXTURE_CUBE_MAP);
        texnamegenerated_ = false;
        buffersmade_ = false;
        renderingnow_ = false;

        isinited_ = true;
    }

// ***** RTT: Public functions NOT callable before OpenGL init *****
public:

    // bind
    // Bind texture name to given target.
    // If necessary, generate names and make framebuffer objects.
    // Does error checking, exiting on any of the following:
    // - FBO extension not available
    // - RTT object not init'ed yet
    // - unsuccessful FBO creation
    //
    // OpenGL must be init'ed before calling this.
    void bind()
    {
        // Are we ready & able to do this?
        if (renderingnow_)
        {
            errorExit("bind",
                      "Called during texture render");
        }
        if (!isinited_)
        {
            errorExit(
                "bind",
                "First call init or pass params to ctor");
        }
        if (glBindFramebufferEXT == 0)
        {
            errorExit(
                "bind",
                "FBO extension not available\n"
                "    (Hardware/software too old? glewInit not called?");
        }

        // Do the bind, generating texture name if necessary
        if (!texnamegenerated_)
        {
            glGenTextures(1, &texname_);
            texnamegenerated_ = true;
        }
        glBindTexture(textarget_, texname_);

        // Make buffers, if necessary
        if (buffersmade_)
            return;

        if (iscubemap_)
        {
            for (int i = 0; i < 6; ++i)
                createBuffer(i, indexToFaceTarget(i));
        }
        else
        {
            createBuffer(0, textarget_);
        }

        // Check if successful framebuffer creation
        GLenum fbstat = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        if (fbstat != GL_FRAMEBUFFER_COMPLETE_EXT)
        {
            std::string errstr = fbStatusToString(fbstat);
            errorExit("bind", "Framebuffer error: " + errstr);
        }
        buffersmade_ = true;

        // Return to usual framebuffer
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }

    // beginRender
    // Start texture rendering.
    // Call this function, draw, and then call endRender.
    // Optional parameter is target for cube-map face.
    // OpenGL must be init'ed before calling this.
    void beginRender(GLenum target = 0)
    {
        if (renderingnow_)
        {
            errorExit("beginRender",
                      "Called twice");
        }
        bind();

        int index;
        if (iscubemap_)
        {
            index = faceTargetToIndex(target);
            if (index == -1)
                errorExit("beginRender",
                          "Must specify cube-map face target");
        }
        else
        {
            if (target != 0 && target != textarget_)
                errorExit("beginRender",
                          "Incorrect target specified");
            index = 0;
        }

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbnames_[index]);
        glPushAttrib(GL_VIEWPORT_BIT);
        glViewport(0, 0, imgwidth_, imgheight_);
        renderingnow_ = true;
    }

    // endRender
    // End texture rendering.
    // Call beginRender, draw, and then call this function.
    // Generates mipmaps unless false is passed for parameter.
    // OpenGL must be init'ed before calling this.
    void endRender(bool genMipmaps = true)
    {
        if (!renderingnow_)
            errorExit("endRender",
                      "endRender called without beginRender");

        glPopAttrib();  // Restore viewport
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                        // Switch rendering back to standard buffers
        renderingnow_ = false;

        if (genMipmaps)
            generateMipmaps();
    }

    // generateMipmaps
    // Generate mipmaps for the texture image(s).
    // This function is called automatically if endRender is called
    //  with no parameters.
    // OpenGL must be init'ed before calling this.
    void generateMipmaps()
    {
        if (renderingnow_)
        {
            errorExit("generateMipmaps",
                      "Called during texture render");
        }
        bind();
        glGenerateMipmapEXT(textarget_);
    }

// ***** RTT: Internal-use functions *****
private:

    // errorExit
    // Given member function name and error string. Prints message and
    //  exits.
    static void errorExit(const std::string func,
                          const std::string msg)
    {
        std::cout << "RTT::" << func << ": " << msg << std::endl;
        std::cout << "Press ENTER to quit ";
        while (std::cin.get() != '\n') ;
        std::exit(1);
    }

    // indexToFaceTarget
    // Given index (0-5) returns corresponding OpenGL target for
    //  cube-map face. Returns (-1) if index not in range.
    static GLenum indexToFaceTarget(int index)
    {
        GLenum facetargets[6] = {
            GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };
        return (0 <= index && index < 6) ? facetargets[index] : -1;
    }

    // faceTargetToIndex
    // Given OpenGL target for cube-map face, returns index in range
    //  0-5. Returns (-1) if argument is not valid cube-map face target.
    static int faceTargetToIndex(GLenum faceTarget)
    {
        switch (faceTarget)
        {
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            return 0;
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            return 1;
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            return 2;
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            return 3;
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            return 4;
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            return 5;
        default:
            return -1;
        }
    }

    // fbStatusToString
    // Given GLenum representing framebuffer status, returns string form
    //  of status, or string indicating unknown error if not a valid
    //  status.
    static std::string fbStatusToString(GLenum fbstat)
    {
        std::string statstr;
        switch(fbstat)
        {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            statstr = "GL_FRAMEBUFFER_COMPLETE_EXT";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            statstr = "GL_FRAMEBUFFER_UNSUPPORTED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            statstr = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            statstr = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            statstr = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            statstr = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
            break;
        case GL_FRAMEBUFFER_BINDING_EXT:
            statstr = "GL_FRAMEBUFFER_BINDING";
            break;
        default:
            statstr = "Unknown framebuffer error";
            break;
        }
        return statstr;
    }

    // createBuffer
    // Create framebuffer object as color buffer, attaches renderbuffer
    //  object as depth buffer, and allocates memory for all mipmaps.
    //  Members imgwidth_, imgheight_ used as buffer dimensions.
    //  Generates FBO and RBO names and stores these in fbnames_[index],
    //  rbnames_[index], respectively.
    // Assumes texture name already generated and bound to teximgtarget.
    // Ends with GL_FRAMEBUFFER_EXT bound to framebuffer target. Does
    //  no error checking.
    void createBuffer(int index,
                      GLenum teximgtarget)
    {
        // Make Framebuffer Object (FBO)
        glGenFramebuffersEXT(1, &fbnames_[index]);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbnames_[index]);

        // Add renderbuffer to FBO, as depth buffer
        glGenRenderbuffersEXT(1, &rbnames_[index]);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rbnames_[index]);

        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
            GL_DEPTH_COMPONENT,
            imgwidth_, imgheight_);

        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
            GL_DEPTH_ATTACHMENT_EXT,
            GL_RENDERBUFFER_EXT, rbnames_[index]);
            // Why oh why do I have to pass both the renderbuffer target
            //  and the renderbuffer name here? I bound the name to the
            //  target, so just the target should be enough. Right?
            //  RIGHT?!?!  -GGC-

        // Make space for all mipmpaps with a dummy call to
        //  gluBuild2DMipmaps
        GLuint * dummyimage = new GLuint[imgwidth_ * imgheight_ * 4];
        gluBuild2DMipmaps(teximgtarget,
            GL_RGBA,
            imgwidth_, imgheight_,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            dummyimage);
        delete [] dummyimage;

        // Attach texture to FBO, as color buffer
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
            GL_COLOR_ATTACHMENT0_EXT,
            teximgtarget, texname_,
            0);  // Mipmap level
    }

    // deleteNames
    // Deletes all generated texture, FBO, and RBO names.
    // If currently rendering, ends render.
    void deleteNames()
    {
        // End the render if currently rendering.
        // We do not do an error exit here, because this function
        //  is called by the RTT destructor. Thus, for example, it
        //  might be called because an exception was thrown during
        //  a texture render.
        if (renderingnow_)
            endRender();

        // Delete texture name
        if (texnamegenerated_)
            glDeleteTextures(1, &texname_);
        
        // Delete buffer names
        if (buffersmade_)
        {
            int numbuffers = iscubemap_ ? 6 : 1;
            glDeleteFramebuffersEXT(numbuffers, fbnames_);
            glDeleteRenderbuffersEXT(numbuffers, rbnames_);
        }
    }

// ***** RTT: Data members *****
private:

    bool isinited_;          // True if basic info passed & stored
    bool iscubemap_;         // True if our texture is a cube map
    bool texnamegenerated_;  // True if texture name has been generated
    bool buffersmade_;       // True if framebuffer(s) & associated
                             //  renderbuffer(s) named & allocated
    bool renderingnow_;      // True if we are between beginRender,
                             //  endRender calls
    // The following logical implications always hold.
    //   iscubemap_ -> isinited_
    //   renderingnow_ -> buffersmade_ -> texnamegenerated_ -> isinited_

    // The following are only valid if isinited_ is true.
    GLenum textarget_;   // Target for the texture as a whole
    GLsizei imgwidth_;   // Width of texture image (pixels)
    GLsizei imgheight_;  // Height of texture image (pixels)

    // The following is only valid if texnamegenerated_ is true.
    GLuint texname_;     // Generated texture name

    // The following are only valid if buffersmade_ is true. In
    //  addition, if iscubemap_ is true, then all 6 entries are valid;
    //  otherwise, only the first entry of each array (fbnames_[0],
    //  rbnames_[0]) is valid.
    GLuint fbnames_[6];  // Generated framebuffer names
    GLuint rbnames_[6];  // Generated renderbuffer names

};  // End class RTT


#endif //#ifndef FILE_RTT_H_INCLUDED

