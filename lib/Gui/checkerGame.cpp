//Author: Aven Bross
//Date: 4-07-2014
//Graphical display for checkers matches

// OpenGL/GLUT includes - DO THESE FIRST
#include <cstdlib>       // Do this before GL/GLUT includes
using std::exit;

#ifndef __APPLE__
# include <GL/glut.h>    // Includes OpenGL headers as well
#else
# include <GLUT/glut.h>  // Apple puts glut.h in a different place
#endif

// Other includes
#include "lib381/bitmapprinter.h"
                         // For class BitmapPrinter
#include <iostream>
using std::cerr;
using std::endl;
#include <sstream>
using std::stringstream;

#include<math.h>

#include <vector>
using std::vector;

#include "../Board/checkerAI.h"


//Global variables
//Keyboard
const int ESCKEY = 27;         // ASCII value of Escape

//Window/viewport
const int startwinsize = 600;  //Start window width & height (pixels)
int winw = 1, winh = 1;        //Window width, height (pixels)
                               //(Initialize to avoid spurious errors)
double cam_xmin, cam_xmax, cam_ymin, cam_ymax;	//Viewport bounddaries in world (cam coords)

//Mouse
double cam_mousex, cam_mousey;	//Mouse pos in cam coords

//Timing
double savetime;	//Saves previous time for calculating elapsed time
double movetime;

int arr[4] = {32, 40, 10, 1};

//The neural nets to compete!
NeuralNet n1(arr, 4);
NeuralNet n2("../Generations/gen2076net0.bin");

const bool HUMAN_PLAYER = false;

//The checker board
CheckerBoard cb;

//The player selected move
CheckerBoard move;

//The selected square
int sel_x, sel_y;

//Stores previous boards for backtracking
vector<CheckerBoard> pBoards;

//Booleans for controlling the game
bool redTurn;
bool step=false;

//Converts from standard to compressed coordinates (my board representation)
int stdToComp(int row, int col){
	if(row%2==1){
		if(col%2==0) return -1;
		return (row+1)*8+(col-1)/2+1;
	}
	else{
		if(col%2==1) return -1;
		return (row+1)*8+col/2;
	}
}

//Converts from compressed (my board representation) to standard coordinates
int compToStd(int row, int col){
	if(row%2==0){
		if(col==0) return -1;
		return (row-1)*8+2*(col-1)+1;
	}
	else{
		if(col==4) return -1;
		return (row-1)*8+2*col;
	}
}

//Converts a CheckerBoard to a standardly organized vector of chars
vector<char> convertBoard(const CheckerBoard & board){
	vector<char> cBoard;
	for(int i=0; i<8; i++){
		for(int j=0; j<8; j++){
			cBoard.push_back('_');
		}
	}

	for (int i=1;i<HEIGHT-1;i++)
	{
		for(int j=0; j<WIDTH; j++){
			int coord = compToStd(i, j);	//Convert to standard coords
			if(coord!=-1)	//Coords off standard board
				cBoard[coord] = board.getPiece(i,j);
		}
	}
	return cBoard;
}

//Draws the difference between to char vector boards
void drawDiff(const vector<char> & b1, const vector<char> & b2){
	// draw grid
	double divx = (cam_xmax-cam_xmin)/8.;
	double divy = (cam_ymax-cam_ymin)/8.;
	double hx = divx/2;
	double hy = divy/2;
	for (int i=1;i<=8;i++)
	{
		for(int j=1; j<=8; j++){
		    double x = cam_xmin+i*divx-hx;
		    double y = cam_ymin+j*divy-hx;
		    int pos = (j-1)*8+(i-1);
		    if(b1[pos]=='_' && b2[pos]!='_'){
			if(redTurn) glColor3d(1., .5, .5);
			else glColor3d(.4,.4,.4);
		    }
		    else continue;

		    double divisor = 1;
		    hx/=divisor;
		    hy/=divisor;
		    //glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		    glBegin(GL_QUADS);
			glVertex2d(-hx+x, -hy+y);
			glVertex2d( hx+x, -hy+y);
			glVertex2d( hx+x,  hy+y);
			glVertex2d(-hx+x,  hy+y);
		    glEnd();
		    hx*=divisor;
		    hy*=divisor;
		}
	}
}

//Draws possible moves
void drawMoves(const CheckerBoard & board){
	vector<char> cBoard = convertBoard(board);
	vector<CheckerBoard> moves = posMoves(board, redTurn);
	
	for(int i=0; i<moves.size(); i++){
		drawDiff(cBoard, convertBoard(moves[i]));
	}
}

//Draws pieces on the board
void drawPieces(const CheckerBoard & board){
	vector<char> cBoard = convertBoard(board);

	// draw grid
	double divx = (cam_xmax-cam_xmin)/8.;
	double divy = (cam_ymax-cam_ymin)/8.;
	double hx = divx/2;
	double hy = divy/2;
	for (int i=1;i<=8;i++)
	{
		for(int j=1; j<=8; j++){
		    double x = cam_xmin+i*divx-hx;
		    double y = cam_ymin+j*divy-hx;
		    bool isKing = false;
		    switch(cBoard[(j-1)*8+(i-1)]){
			case 'X':
			case '_': continue;
			case 'R': isKing=true;
			case 'r': glColor3d(1., 0., 0.);
				  break;
			case 'B': isKing=true;
			case 'b': glColor3d(0., 0., 0.);
				  break;
		    }
		    double divisor = 2;
		    if(isKing) divisor = 1.5;
		    hx/=divisor;
		    hy/=divisor;
		    //glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		    glBegin(GL_QUADS);
			glVertex2d(-hx+x, -hy+y);
			glVertex2d( hx+x, -hy+y);
			glVertex2d( hx+x,  hy+y);
			glVertex2d(-hx+x,  hy+y);
		    glEnd();
		    hx*=divisor;
		    hy*=divisor;
		}
	}
}

//Draw colors on the board
void drawColors(){
	// draw grid
	double divx = (cam_xmax-cam_xmin)/8.;
	double divy = (cam_ymax-cam_ymin)/8.;
	double hx = divx/2;
	double hy = divy/2;
	for (int i=1;i<=8;i++)
	{
		for(int j=1; j<=8; j++){
		    double x = cam_xmin+i*divx-hx;
		    double y = cam_ymin+j*divy-hx;
		    if((i+j)%2==0) glColor3d(0.7, 0.7, 0.7);
		    else glColor3d(0., 0., 0.);
		    //glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		    glBegin(GL_QUADS);
			glVertex2d(-hx+x, -hy+y);
			glVertex2d( hx+x, -hy+y);
			glVertex2d( hx+x,  hy+y);
			glVertex2d(-hx+x,  hy+y);
		    glEnd();;
		}
	}
}

//Draws a grid on the board (only used this for debugging really)
void drawGrid(){
	double divx = (cam_xmax-cam_xmin)/8.;
	double divy = (cam_ymax-cam_ymin)/8.;

	glColor3d(0.,0.,0.);
	glBegin(GL_LINES);
	for (int i=0;i<8;i++)
	{
	    glVertex2d(cam_xmin+i*divx,cam_ymin);
	    glVertex2d(cam_xmin+i*divx,cam_ymax);

	    glVertex2d(cam_xmin,cam_ymin+i*divy);
	    glVertex2d(cam_xmax,cam_ymin+i*divy);
	}
	glEnd();
}

//GLUT display function
void myDisplay()
{
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    drawColors();
    drawMoves(cb);
    drawPieces(cb);
    //drawGrid();

    /*
    // Draw documentation
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);  // Set up simple ortho projection
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0., double(winw), 0., double(winh));
    glColor3d(0., 0., 0.);        // Black text
    BitmapPrinter p(20., winh-20., 20.);
    p.print("Try clicking & dragging the square");
    p.print("Also try dragging it out of the window");
    p.print("(& compare this with the other program)");
    p.print("Esc      Quit");
    glPopMatrix();                // Restore prev projection
    glMatrixMode(GL_MODELVIEW);*/

    glutSwapBuffers();
}

//GLUT idle function
void myIdle()
{
    //Calculate Elapsed Time
    double currtime = glutGet(GLUT_ELAPSED_TIME) / 1000.;
    double elapsedtime = currtime - savetime;
    if (step==true){
	pBoards.push_back(cb);
	if(redTurn) cb = getMove(cb, n1, true);
	else cb = getMove(cb, n2, false);
	redTurn = !redTurn;
	step=false;
	glutPostRedisplay();
    }
    

    //Print OpenGL errors, if there are any (for debugging)
    static int error_count = 0;
    if (GLenum err = glGetError())
    {
        ++error_count;
        cerr << "OpenGL ERROR " << error_count << ": "
             << gluErrorString(err) << endl;
    }
}


//GLUT keyboard function
void myKeyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case ESCKEY:  // Esc: quit
        exit(0);
        break;
    case ' ':
	if(cb.isDone()) break;
	step=true;
	break;
    case 8:
	if(pBoards.empty()) break;
	cb = pBoards[pBoards.size()-1];
	pBoards.pop_back();
	redTurn=!redTurn;
	glutPostRedisplay();
	break;
    }
}


//GLUT reshape function
void myReshape(int w, int h)
{
    if(w<h) h=w;
    else w=h;	
    //Set viewport & save window dimensions in globals
    glViewport(0, 0, w, h);
    winw = w;
    winh = h;

    //Set up projection
    //Save max/min x/y coords in globals
    //Projection is orthographic. Aspect ratio is correct,
    //and region -1..1 in x, y always just fits in viewport.
    if (w > h)
    {
        cam_xmin = -double(w)/h;
        cam_xmax = double(w)/h;
        cam_ymin = -1.;
        cam_ymax = 1.;
    }
    else
    {
        cam_xmin = -1.;
        cam_xmax = 1.;
        cam_ymin = -double(h)/w;
        cam_ymax = double(h)/w;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(cam_xmin, cam_xmax, cam_ymin, cam_ymax);

    glMatrixMode(GL_MODELVIEW);  // Always go back to model/view mode
}


//Given mouse pos in GLUT format; computes mouse pos in cam coords,
//saves it in globals cam_mousex, cam_mousey.
//Uses globals winw, winh, cam_xmin, cam_xmax, cam_ymin, cam_ymax.
void saveMouse(int x, int y)
{
    double t;  //Intermediate value of lirping

    t = double(x) / winw;
    cam_mousex = cam_xmin + t * (cam_xmax - cam_xmin);

    t = double(y) / winh;
    cam_mousey = cam_ymax + t * (cam_ymin - cam_ymax);
}


//GLUT mouse function
void myMouse(int button, int state, int x, int y)
{
    //Find mouse pos in cam coords (saved in cam_mousex, cam_mousey)
    saveMouse(x, y);

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)  //Left mouse clicked
        {
		
        }
        else //Left mouse up
        {
	    
        }
    }
}

//Initialize GL states & global data
//Called by main after window creation
void init()
{
    //Mouse
    cam_mousex = 0.0;
    cam_mousey = 0.0;

    //Starts with red
    redTurn=true;
}


int main(int argc, char ** argv)
{
    //Initialize OpenGL/GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    //Make a window
    glutInitWindowSize(startwinsize, startwinsize);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Checker GUI");

    //Initialize GL states & register GLUT callbacks
    init();
    glutDisplayFunc(myDisplay);
    glutIdleFunc(myIdle);
    glutKeyboardFunc(myKeyboard);
    glutReshapeFunc(myReshape);
    glutMouseFunc(myMouse);

    //Main loop for stuff
    glutMainLoop();

    return 0;
}


