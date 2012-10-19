//---------------------------------------------------------------------------
// Author: Brendan Loyot
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "../include/hand_manipulation.hpp"
#include "../include/def.hpp"
#include <fstream>

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

Context g_context;
Recorder recorder;
ScriptNode g_scriptNode;
ImageGenerator g_image;
ImageMetaData g_imageMD;
XnVSessionManager sessionManager;
HandsGenerator g_hands;
GestureGenerator g_gesture;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// CALLBACKS
//---------------------------------------------------------------------------
void XN_CALLBACK_TYPE Gesture_Recognized(xn::GestureGenerator& generator,
                   const XnChar* strGesture,
                   const XnPoint3D* pIDPosition,
                   const XnPoint3D* pEndPosition, void* pCookie)
{        
	printf("Gesture recognized: %s\n", strGesture);
	g_gesture.RemoveGesture(strGesture);
	g_hands.StartTracking(*pEndPosition);
        std::cout << strGesture << std::endl;
        if (!strcmp(strGesture, "Wave")) {gesture = 1;}
        if (!strcmp(strGesture, "Click")) {gesture = 2;}
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
        if (g_bQuit) {
	    CleanUpExit();
	}
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

	g_image.GetMetaData(g_imageMD);

	// Copied from SimpleViewer
	// Clear the OpenGL buffers
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup the OpenGL viewpoint
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);

	xnOSMemSet(g_pTexMap, 0, g_nTexMapX*g_nTexMapY*sizeof(XnRGB24Pixel));

	// draw image
	const XnRGB24Pixel* pImageRow = g_imageMD.RGB24Data();
	XnRGB24Pixel* pTexRow = g_pTexMap + g_imageMD.YOffset() * g_nTexMapX;

	for (XnUInt y = 0; y < g_imageMD.YRes(); ++y)
	    {
		const XnRGB24Pixel* pImage = pImageRow;
		XnRGB24Pixel* pTex = pTexRow + g_imageMD.XOffset();

		for (XnUInt x = 0; x < g_imageMD.XRes(); ++x, ++pImage, ++pTex)
		    {
			*pTex = *pImage;
		    }

		pImageRow += g_imageMD.XRes();
		pTexRow += g_nTexMapX;
	}

	// Create the OpenGL texture map
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_nTexMapX, g_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, g_pTexMap);

	// Display the OpenGL texture map
	glColor4f(1,1,1,1);

	glBegin(GL_QUADS);

	int nXRes = g_imageMD.FullXRes();
	int nYRes = g_imageMD.FullYRes();

	// upper left
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);
	// upper right
	glTexCoord2f((float)nXRes/(float)g_nTexMapX, 0);
	glVertex2f(GL_WIN_SIZE_X, 0);
	// bottom right
	glTexCoord2f((float)nXRes/(float)g_nTexMapX, (float)nYRes/(float)g_nTexMapY);
	glVertex2f(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	// bottom left
	glTexCoord2f(0, (float)nYRes/(float)g_nTexMapY);
	glVertex2f(0, GL_WIN_SIZE_Y);

	glEnd();

        // Draw a different color triangle based on the gesture found
        glDisable(GL_TEXTURE_2D);
        if (gesture == 1) {glColor3f(0.0, 0.0, 0.0);}
        if (gesture == 1) {glColor3f(1.0, 0.0, 0.0);}
        if (gesture == 2) {glColor3f(0.0, 1.0, 0.0);}
        if (gesture == 3) {glColor3f(0.0, 0.0, 1.0);}
        glBegin(GL_TRIANGLES);
		glVertex3f(50.0f, 25.0f, 0.0f);
		glVertex3f( 25.0f, 75.0f, 0.0);
		glVertex3f( 75.0f, 75.0f, 0.0);
	glEnd();
        glEnable(GL_TEXTURE_2D);
	// Swap the OpenGL display buffers
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

int main(int argc, char* argv[])
{
        int nRetVal;
	XnStatus rc;
	EnumerationErrors errors;

        // get context from xml
	rc = g_context.InitFromXmlFile(SAMPLE_XML_PATH, g_scriptNode, &errors);

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

        rc = g_gesture.AddGesture("Click", NULL);
        CHECK_RC(rc, " add click gesture");
        rc = g_gesture.AddGesture("RaiseHand", NULL);
        CHECK_RC(rc, "add raise gesture");
        rc = g_gesture.AddGesture("Wave", NULL);
        CHECK_RC(rc, "add wave gesture");

        // get metadata
	g_image.GetMetaData(g_imageMD);

	// RGB is the only image format supported.
	if (g_imageMD.PixelFormat() != XN_PIXEL_FORMAT_RGB24)
	{
		printf("The device image format must be RGB24\n");
		return 1;
	}

        // if argument is set true, then record the session
        if (argc > 1 && strcmp(argv[1], "record") == 0) {
            std::cout << "recording to " << RECORDING_PATH << std::endl;
            // Create Recorder
            rc = recorder.Create(g_context);
            CHECK_RC(rc, "create recorder");

            // Init it
            rc = recorder.SetDestination(XN_RECORD_MEDIUM_FILE, RECORDING_PATH);
            CHECK_RC(rc, "init recorder");

            // Add depth node to recording
            rc = recorder.AddNodeToRecording(g_image, XN_CODEC_16Z_EMB_TABLES);
            CHECK_RC(rc, "add depth node");
        }

	// Texture map init
	g_nTexMapX = (((unsigned short)(g_imageMD.FullXRes()-1) / 512) + 1) * 512;
	g_nTexMapY = (((unsigned short)(g_imageMD.FullYRes()-1) / 512) + 1) * 512;
	g_pTexMap = (XnRGB24Pixel*)malloc(g_nTexMapX * g_nTexMapY * sizeof(XnRGB24Pixel));

	// OpenGL init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
        glutInitWindowPosition(GL_WIN_POSITION_X, GL_WIN_POSITION_Y);
	glutCreateWindow ("Image");
	glutSetCursor(GLUT_CURSOR_NONE);

	glutKeyboardFunc(glutKeyboard);
	glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutIdle);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	// Per frame code is in glutDisplay
	glutMainLoop();

        CleanUpExit();
	return 0;
}

