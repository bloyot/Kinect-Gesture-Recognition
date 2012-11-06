//---------------------------------------------------------------------------
// Author: Brendan Loyot
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

#ifndef __DEF_HPP_INCLUDED__
#define __DEF_HPP_INCLUDED__

#define SAMPLE_XML_PATH "./Config.xml"
#define RECORDING_PATH "recording.oni"

#define GL_WIN_SIZE_X 800
#define GL_WIN_SIZE_Y 600
#define GL_WIN_POSITION_X 800
#define GL_WIN_POSITION_Y 100

#define DISPLAY_MODE_OVERLAY	1
#define DISPLAY_MODE_DEPTH		2
#define DISPLAY_MODE_IMAGE		3
#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_IMAGE
#define GESTURE_TO_USE "Click"
#define ROTATION_BUFFER 200

//temp
#define WIDTH 640
#define HEIGHT 480
#define TITLE "teapot_manipulation"
#define FIELD_OF_VIEW_ANGLE 45
#define Z_NEAR 1.0f
#define Z_FAR 500.0f

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
#define CHECK_RC(rc, what) \
	if (rc != XN_STATUS_OK)	\
	{ \
	printf("%s failed: %s\n", what, xnGetStatusString(rc));	\
	return rc; \
	}
#endif
