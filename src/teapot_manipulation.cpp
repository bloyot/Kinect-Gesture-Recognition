//---------------------------------------------------------------------------
// Author: Brendan Loyot
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "../include/hand_manipulation.hpp"
#include "../include/def.hpp"


using namespace xn;
//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------
XnRGB24Pixel* g_pTexMap = NULL;
unsigned int g_nTexMapX = 0;
unsigned int g_nTexMapY = 0;
XnBool g_bQuit = false;
unsigned int g_nViewState = DEFAULT_DISPLAY_MODE;
int gesture = 0;
float g_rotation_x = 0;
float g_rotation_y = 0;
float g_rotation_speed = 1.0f;
bool ready_state = false;
double global_hand_x = 0.0;
double global_hand_y = 0.0;


Context g_context;
Recorder recorder;
Player g_player;
ScriptNode g_scriptNode;

ImageGenerator g_image;
HandsGenerator g_hands;
GestureGenerator g_gesture;

ImageMetaData g_imageMD;
XnVSessionManager sessionManager;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// CALLBACKS
//---------------------------------------------------------------------------

void XN_CALLBACK_TYPE SessionStart(const XnPoint3D& pFocus, void* UserCxt)
{
    // Session started - do whatever
    std::cout << "Session Started" << std::endl;
}
void XN_CALLBACK_TYPE SessionEnd(void* UserCxt)
{
    // Session ended - do whatever
    std::cout << "Session Ended" << std::endl;
}

void XN_CALLBACK_TYPE Gesture_Recognized(xn::GestureGenerator& generator,
                   const XnChar* strGesture,
                   const XnPoint3D* pIDPosition,
                   const XnPoint3D* pEndPosition, void* pCookie)
{        
	printf("Gesture recognized: %s\n", strGesture);
	g_gesture.RemoveGesture(strGesture);
	g_hands.StartTracking(*pEndPosition);
        std::cout << strGesture << std::endl;
        if (!strcmp(strGesture, "Wave")) {
            gesture = 1;
            ready_state = true;
        }
        if (!strcmp(strGesture, "Click")) {
            gesture = 2;
            ready_state = false;
        }
        if (!strcmp(strGesture, "RaiseHand")) {gesture = 3;}
        std::cout << gesture << std::endl;
}

void XN_CALLBACK_TYPE Gesture_Process(xn::GestureGenerator& generator,
                const XnChar* strGesture,
                const XnPoint3D* pPosition,
                XnFloat fProgress,
                void* pCookie)
{}

void XN_CALLBACK_TYPE Hand_Create(xn::HandsGenerator& generator,
            XnUserID nId, const XnPoint3D* pPosition,
            XnFloat fTime, void* pCookie)
{
  printf("New Hand: %d @ (%f,%f,%f)\n", nId,
         pPosition->X, pPosition->Y, pPosition->Z);
}

void XN_CALLBACK_TYPE Hand_Update(xn::HandsGenerator& generator,
            XnUserID nId, const XnPoint3D* pPosition,
            XnFloat fTime, void* pCookie)
{
    printf("Position (%f,%f,%f)\n",
         pPosition->X, pPosition->Y, pPosition->Z);
    global_hand_x = pPosition->X;
    global_hand_y = pPosition->Y;
}

void XN_CALLBACK_TYPE Hand_Destroy(xn::HandsGenerator& generator,
             XnUserID nId, XnFloat fTime,
             void* pCookie)
{
  printf("Lost Hand: %d\n", nId);
    g_gesture.AddGesture("Click", NULL);
    g_gesture.AddGesture("RaiseHand", NULL);
    g_gesture.AddGesture("Wave", NULL);
    gesture = 0;
}
//---------------------------------------------------------------------------
// glut
//---------------------------------------------------------------------------

void CleanUpExit() {
    recorder.Release();
    g_player.Release();
    g_image.Release();
    g_scriptNode.Release();
    g_context.Release();
    g_hands.Release();
    g_gesture.Release();
    free(g_pTexMap);
    exit(1);
}

void glutIdle (void)
{
	// Display the frame
	glutPostRedisplay();
}

void glutDisplay(void)
{
	XnStatus rc = XN_STATUS_OK;

	// Read a new frame
	rc = g_context.WaitAnyUpdateAll();
	if (rc != XN_STATUS_OK)
	{
		printf("Read failed: %s\n", xnGetStatusString(rc));
		return;
	}
        // Clear Screen and Depth Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		     
	glLoadIdentity();
 
	// Define a viewing transformation
	gluLookAt( 0,0,7, 0,0,0, 0,1,0);					  
 
	// Push and pop the current matrix stack. 
	// This causes that translations and rotations on this matrix wont influence others.
 
	glPushMatrix();										
		glColor3f(0,1,1);
		glTranslatef(0,0,0);							
		glRotatef(g_rotation_x,0,1,0);
		glRotatef(g_rotation_y,1,0,0);
		glRotatef(0,0,1,0); // starts teapot sideways

		// Draw the teapot
	    glutSolidTeapot(1);
	glPopMatrix();	

       // print when the image is ready to be rotated
        if (ready_state) {
           std::string str = "ready to rotate image";
           glColor3f(1,0,0);
           glRasterPos3f(-3.0f, -2.0f, 0.0f);
           int len, i;
           for (i = 0; i < str.length(); i++) {
               glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[i]);
           }
        }									  
        if (ready_state) {
            if (global_hand_x > ROTATION_BUFFER) {g_rotation_x += g_rotation_speed;}
            if (global_hand_x < -ROTATION_BUFFER) {g_rotation_x -= g_rotation_speed;}
            if (global_hand_y > ROTATION_BUFFER) {g_rotation_y += g_rotation_speed;}
            if (global_hand_y < -ROTATION_BUFFER) {g_rotation_y -= g_rotation_speed;}
        }
        glutSwapBuffers();
}

void glutKeyboard (unsigned char key, int /*x*/, int /*y*/)
{
	switch (key)
	{
		case 27:
                        CleanUpExit();                       
	}
}

void initialize () 
{
   // select projection matrix
    glMatrixMode(GL_PROJECTION);												
 
	// set the viewport
    glViewport(0, 0, WIDTH, HEIGHT);									
 
	// set matrix mode
    glMatrixMode(GL_PROJECTION);												
 
	// reset projection matrix
    glLoadIdentity();															
    GLfloat aspect = (GLfloat) WIDTH / HEIGHT;
 
	// set up a perspective projection matrix
	gluPerspective(FIELD_OF_VIEW_ANGLE, aspect, Z_NEAR, Z_FAR);		
 
	// specify which matrix is the current matrix
	glMatrixMode(GL_MODELVIEW);											
    glShadeModel( GL_SMOOTH );
 
	// specify the clear value for the depth buffer
	glClearDepth( 1.0f );														
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
 
	// specify implementation-specific hints
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );						
 
	GLfloat amb_light[] = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat diffuse[] = { 0.6, 0.6, 0.6, 1 };
    GLfloat specular[] = { 0.7, 0.7, 0.3, 1 };
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, amb_light );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
    glLightfv( GL_LIGHT0, GL_SPECULAR, specular );
    glEnable( GL_LIGHT0 );
    glEnable( GL_COLOR_MATERIAL );
    glShadeModel( GL_SMOOTH );
    glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );
    glDepthFunc( GL_LEQUAL );
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); 
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

int main(int argc, char* argv[])
{
        int nRetVal;
	XnStatus rc;
	EnumerationErrors errors;

        // get playback file if using 
        if (argc > 2 && strcmp(argv[2], "true") == 0) {
            rc = g_context.Init();

            rc = g_context.OpenFileRecording(RECORDING_PATH, g_player);
            CHECK_RC(rc, "Opening file");

            rc = g_player.SetRepeat(TRUE);
	    CHECK_RC(rc, "Turn repeat off");
        } else {
            // get context from xml
	    rc = g_context.InitFromXmlFile(SAMPLE_XML_PATH, g_scriptNode, &errors);
        }

        // error checking
	if (rc == XN_STATUS_NO_NODE_PRESENT)
	{
		XnChar strError[1024];
		errors.ToString(strError, 1024);
		printf("%s\n", strError);
		return (rc);
	}
	CHECK_RC(rc, "Context initialization");        

        // get hand and image generator from context, check errors
	rc = g_context.FindExistingNode(XN_NODE_TYPE_IMAGE, g_image);
	CHECK_RC(rc, "Get image generator");

        rc = g_context.FindExistingNode(XN_NODE_TYPE_HANDS, g_hands);
	CHECK_RC(rc, "Get hand generator");       
       
        rc = g_context.FindExistingNode(XN_NODE_TYPE_GESTURE, g_gesture);
        CHECK_RC(rc, "Get gesture generator");

        // create and register callbacks
        XnCallbackHandle h1, h2;
        g_gesture.RegisterGestureCallbacks(Gesture_Recognized,
                                              Gesture_Process,
                                              NULL, h1);
        CHECK_RC(rc, "Get register gesture callback");     
  
        g_hands.RegisterHandCallbacks(Hand_Create, Hand_Update,
                                           Hand_Destroy, NULL, h2);
        CHECK_RC(rc, "Get hand callback");

        // add gestures to the generator
        rc = g_gesture.AddGesture("Click", NULL);
        CHECK_RC(rc, " add click gesture");
        rc = g_gesture.AddGesture("RaiseHand", NULL);
        CHECK_RC(rc, "add raise gesture");
        rc = g_gesture.AddGesture("Wave", NULL);
        CHECK_RC(rc, "add wave gesture");

        
	g_image.GetMetaData(g_imageMD);

	// RGB is the only image format supported.
	if (g_imageMD.PixelFormat() != XN_PIXEL_FORMAT_RGB24)
	{
		printf("The device image format must be RGB24\n");
		return 1;
	}

        // if argument is set true, then record the session
        if (argc > 1 && strcmp(argv[1], "true") == 0) {
            std::cout << "recording to " << RECORDING_PATH << std::endl;
            // Create Recorder
            rc = recorder.Create(g_context);
            CHECK_RC(rc, "create recorder");

            // Init it
            rc = recorder.SetDestination(XN_RECORD_MEDIUM_FILE, RECORDING_PATH);
            CHECK_RC(rc, "init recorder");

            // Add nodes to recording
            rc = recorder.AddNodeToRecording(g_image);
            CHECK_RC(rc, "add image node");
            
            rc = recorder.AddNodeToRecording(g_hands);
            CHECK_RC(rc, "add hands node");
        }

        // initialize and run program
	glutInit(&argc, argv);                                      // GLUT initialization
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );  // Display Mode
	glutInitWindowSize(WIDTH, HEIGHT);	// set window size
        glutInitWindowPosition(GL_WIN_POSITION_X, GL_WIN_POSITION_Y);
	glutCreateWindow(TITLE);	        // create Window
	glutDisplayFunc(glutDisplay);		// register Display Function
	glutIdleFunc(glutDisplay);		// register Idle Function
        glutKeyboardFunc(glutKeyboard );	// register Keyboard Handler
	initialize();
	glutMainLoop();	

        CleanUpExit();
	return 0;
}

