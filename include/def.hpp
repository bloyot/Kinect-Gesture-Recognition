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
