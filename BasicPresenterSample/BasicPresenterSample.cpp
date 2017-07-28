//////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015 zSpace, Inc.  All Rights Reserved.
//  
//  File:       BasicPresenterSample.cpp
//  Content:    This is a sample C++/OpenGL-based Windows application showing
//              how to integrate zView presenter support into such a native
//              application.  This sample displays a simple spinning cube and
//              acts as a zView presenter application supporting both standard
//              and augmented reality modes.
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
//  Overview:
//
//  This sample demonstrates how to integrate zView presenter support in a
//  native Windows application written using C++ and OpenGL.  While this sample
//  specifically uses C++ and OpenGL, the overall method that this sample uses
//  to integrate zView presenter support can be adapted to work with
//  applications written using other programming languages (if they can call C
//  functions), 3D rendering APIs, UI toolkits, game engines, etc.
//
//  Note:  The comments in this sample focus primarily on how to integrate
//  zView presenter support.  They do not go into detail on how to set up a
//  basic native Windows application with OpenGL rendering support, how to do
//  basic rendering using OpenGL, or how to add basic zSpace support (i.e.
//  head-tracked stereoscopic rendering with zSpace stylus interaction using
//  the zSpace Core API).  If you are unfamiliar with native Windows
//  applications or OpenGL, it may be useful to read up on these topics before
//  reading through this sample.  If you are unfamiliar to how to add basic
//  zSpace support to an application, please see the samples from the zSpace
//  Core SDK.
//
//  At a high level, the code in this sample can be broken down into three
//  parts:  initialization code that is run at startup, a main loop that is run
//  continuously after initialization completes until the application exits,
//  and shutdown code that is run when the application exits.  The main loop
//  can be further broken up into event handling, update, and draw parts.  See
//  the following functions for details on the zView-specific portions of each
//  of these parts:
//
//  - Initialization:  initializeZview()
//  - Main loop:
//      - Event handling:  windowProc()
//      - Update:  updateZview()
//      - Draw:  drawZview()
//  - Shutdown:  shutdown()
//
//  Controls:
//
//  The following keyboard shortcuts can be used to control the zView presenter
//  functionality implemented by this sample.  For details on how these
//  keyboard shortcuts are implemented, see the WM_KEYDOWN case in the
//  windowProc() function defined below.
//
//  Basic zView controls:
//
//  'C' - Connect to the default zView viewer application, launching it if
//        necessary.
//
//  'E' - If currently connected to a zView viewer application, close the
//        connection and request that the viewer application exit once the
//        connection is closed.
//
//  'M' - If currently connected to a zView viewer application, switch to a
//        different zView mode.  This will cycle through the supported and
//        available modes for the active zView connection.
//
//  'P' - If currently connected to a zView viewer application, pause the
//        currently active zView mode if it is not already paused or resume it
//        if it is paused. 
//
//  zView video recording controls:
//
//  These controls only function if currently connected to a zView viewer
//  application that supports video recording.
//
//  SHIFT + 'Q' - Change the current video recording quality.  This will cycle
//                through the available video recording qualities.
//
//  SHIFT + 'R' - Start a video recording if no recording is currently active.
//                The recording will be made using the current video recording
//                quality.  Note:  A new video recording cannot be started if
//                there an active finished video recording.  A finished video
//                recording must be either saved or discarded before a new
//                video recording can be started.
//
//  SHIFT + 'P' - If a video is currently being recorded and recording is not
//                currently paused, then pause recording.  If a video is
//                currently being recorded and recording is paused, then resume
//                recording.
//
//  SHIFT + 'F' - If a video is currently being recorded, then finish the
//                recording.
//
//  SHIFT + 'S' - If there is an active finished video recording, save it.
//                To keep this sample's code simple, all video recording are
//                saved to the same file name in the directory containing the
//                sample executable being run.
//
//  SHIFT + 'D' - If there is an active finished video recording, discard it.
//
//  Augmented reality mode overlay controls:
//
//  These controls only function if currently connected to a zView viewer
//  application with augmented reality mode active.  These controls allow for
//  tweaking the positioning and scale of the augmented reality mode images
//  rendered by this presenter when the viewer overlays them on top of the
//  augmented reality mode webcam video stream.  Such tweaking may be used if
//  the viewer is not able to closely line up the presenter's augmented reality
//  mode images with the webcam video stream.
//
//  CTRL + 'A' - Move the augmented reality mode overlay to the left.
//
//  CTRL + 'D' - Move the augmented reality mode overlay to the right.
//
//  CTRL + 'S' - Move the augmented reality mode overlay down.
//
//  CTRL + 'W' - Move the augmented reality mode overlay up.
//
//  CTRL + 'Q' - Reset the horizontal offset of the augmented reality overlay
//               to 0.
//
//  CTRL + 'E' - Reset the vertical offset of the augmented reality overlay to
//               0.
//
//  CTRL + 'F' - Decrease the horizontal scale of the augmented reality mode
//               overlay.
//
//  CTRL + 'H' - Increase the horizontal scale of the augmented reality mode
//               overlay.
//
//  CTRL + 'G' - Decrease the vertical scale of the augmented reality mode
//               overlay.
//
//  CTRL + 'T' - Increase the vertical scale of the augmented reality mode
//               overlay.
//
//  CTRL + 'R' - Reset the horizontal scale of the augmented reality overlay to
//               1.
// 
//  CTRL + 'Y' - Reset the vertical scale of the augmented reality overlay to
//               1.
//
//  Augmented reality mode visualization controls:
//
//  These controls only function if currently connected to a zView viewer
//  application with augmented reality mode active.  These controls allow for
//  toggling various visualizations that are useful for seeing how augmented
//  reality mode works.
//
//  'B' - Toggle drawing of the scene background in augmented reality mode
//        images.  This allows you to see what is behind the viewport region of
//        an augmented reality mode render in the augmented reality mode webcam
//        video stream.
//
//  'V' - Toggle drawing of the augmented reality mode mask geometry.  This
//        allows you to visualize the mask geometry that is used to clip scene
//        contents that are behind the plane of the zSpace display and outside
//        the bounds of the presenter application's viewport.
// 
//////////////////////////////////////////////////////////////////////////

#include <zSpace.h>

#include <zView.h>

#include <math.h>
#include <time.h>
#include <Windows.h>

#include <GL/glew.h>

#include <sstream>
#include <string>
#include <atlstr.h>

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>


//////////////////////////////////////////////////////////////////////////
// Macros
//////////////////////////////////////////////////////////////////////////

#define CHECK_ZC_ERROR(error)                                               \
    if (error != ZC_ERROR_OK)                                               \
    {                                                                       \
        char errorString[256];                                              \
        zcGetErrorString(error, errorString, sizeof(errorString));          \
        MessageBox(                                                         \
            NULL,                                                           \
            errorString,                                                    \
            "ZCError",                                                      \
            MB_OK|MB_SETFOREGROUND|MB_TOPMOST);                             \
        return false;                                                       \
    }

#define CHECK_ZV_ERROR(error)                                               \
    if (error != ZV_ERROR_OK)                                               \
    {                                                                       \
        char errorString[256];                                              \
        zvGetErrorString(error, errorString, sizeof(errorString));          \
        MessageBox(                                                         \
            NULL,                                                           \
            errorString,                                                    \
            "ZVError",                                                      \
            MB_OK|MB_SETFOREGROUND|MB_TOPMOST);                             \
        return false;                                                       \
    }


//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////

static const GLfloat LIGHT_COLOR[]          = { 0.6f, 0.6f, 0.6f, 1.0f };
static const GLfloat LIGHT_POSITION[]       = { 0.2f, 0.2f, 0.6f, 1.0f };
static const GLfloat AMBIENT_COLOR[]        = { 0.2f, 0.2f, 0.2f, 1.0f };
static const GLfloat SPECULAR_COLOR[]       = { 0.3f, 0.3f, 0.3f, 1.0f };


static const float STYLUS_LENGTH            = 0.1f;  // Meters
static const float ROTATION_PER_SECOND      = 45.0f; // Degrees
static const float CUBE_HALF_SIZE           = 0.03f; // Meters
static const float PI                       = 3.1415926535897932384626433832795f;

static const char* WINDOW_NAME              = "Basic Presenter Sample";
static const char* WINDOW_CLASS_NAME        = "BasicPresenterWindowClass";
static const char* WINDOW_ICON_NAME         = "zSpace.ico";
static const char* CUBE_TEXTURE_NAME        = "zSpaceLogo.bmp";
static const char* VERTEX_SHADER_NAME       = "BasicPresenterSampleVertexShader.glsl";
static const char* FRAGMENT_SHADER_NAME     = "BasicPresenterSampleFragmentShader.glsl";
static const WORD  BMP_SIGNATURE            = 'MB';

static const char* ZVIEW_NODE_NAME                 = "Basic Presenter Sample";
static const char* ZVIEW_NODE_STATUS_NOT_CONNECTED = "Awaiting connection";
static const char* ZVIEW_NODE_STATUS_CONNECTED     = "Connected";

static const char* ZVIEW_VIDEO_RECORDING_SAVE_NAME =
    "BasicPresenterSampleVideoRecordingSave.mp4";

static const char* ZVIEW_AUGMENTED_REALITY_MODE_MASK_VERTEX_SHADER_NAME   =
    "BasicPresenterSampleAugmentedRealityModeMaskVertexShader.glsl";
static const char* ZVIEW_AUGMENTED_REALITY_MODE_MASK_FRAGMENT_SHADER_NAME =
    "BasicPresenterSampleAugmentedRealityModeMaskFragmentShader.glsl";
static const char* ZVIEW_AUGMENTED_REALITY_MODE_BACKGROUND_VERTEX_SHADER_NAME   =
    "BasicPresenterSampleAugmentedRealityModeBackgroundVertexShader.glsl";
static const char* ZVIEW_AUGMENTED_REALITY_MODE_BACKGROUND_FRAGMENT_SHADER_NAME =
    "BasicPresenterSampleAugmentedRealityModeBackgroundFragmentShader.glsl";

static const int ZVIEW_AUGMENTED_REALITY_MODE_NUM_MASK_QUADS = 8;
static const int ZVIEW_AUGMENTED_REALITY_MODE_NUM_MASK_VERTS =
    ZVIEW_AUGMENTED_REALITY_MODE_NUM_MASK_QUADS * 4;

static const float ZVIEW_AUGMENTED_REALITY_MODE_MASK_CUBE_SIDE_LENGTH_METERS =
    10.0f;


//////////////////////////////////////////////////////////////////////////
// Global Variables
//////////////////////////////////////////////////////////////////////////

ZCContext   g_zSpaceContext         = NULL;
ZCHandle    g_displayHandle         = NULL;
ZCHandle    g_bufferHandle          = NULL;
ZCHandle    g_viewportHandle        = NULL;
ZCHandle    g_frustumHandle         = NULL;
ZCHandle    g_stylusHandle          = NULL;

float       g_cameraAngle           = 0.0f;

clock_t     g_previousTime          = clock();
bool        g_isCameraOrbitEnabled  = true;

HGLRC       g_hRC                   = NULL; // Permanent Rendering Context
HDC         g_hDC                   = NULL; // Private GDI Device Context
HWND        g_hWnd                  = NULL; // Holds Our Window Handle
HINSTANCE   g_hInstance             = NULL; // Holds The Instance Of The Application

int         g_windowX               = 0;
int         g_windowY               = 0;
int         g_windowWidth           = 1024;
int         g_windowHeight          = 768;

char*       g_vertexShaderSource          = NULL;
int         g_vertexShaderSourceLength    = 0;
char*       g_fragmentShaderSource        = NULL;
int         g_fragmentShaderSourceLength  = 0;

GLuint      g_vertexShader;
GLuint      g_fragmentShader;
GLuint      g_shaderProgram;
GLuint      g_modelViewUniform;
GLuint      g_projectionUniform;

GLuint      g_specularColorUniform;
GLuint      g_ambientColorUniform;

// Texture Info
GLuint      g_cubeTextureID;

struct Light
{
    GLuint position;
    GLuint diffuse;
    GLuint specular;
};

Light       g_lightsUniform[1];
GLuint      g_numLightsUniform;

GLuint      g_renderStylusUniform;
/////--------------------------------------------------------------------------------------------
glm::mat4   g_viewMatrix;
glm::mat4   g_cameraTransform;
glm::mat4   g_invCameraTransform;
glm::mat4   g_stylusWorldPose;
/////--------------------------------------------------------------------------------------------
GLuint      g_cubeVertexArrayID = 0;
GLuint      g_cubeVertexArrayBufferIDs[3];

// zView

ZVContext    g_zViewContext                    = NULL;
ZVConnection g_zViewActiveConnection           = NULL;
ZVMode       g_zViewStandardMode               = NULL;
ZVMode       g_zViewAugmentedRealityMode       = NULL;
ZVMode       g_zViewLatestActiveConnectionMode = NULL;
int          g_zViewCurrentConnectionModeIndex = 0;

ZVVideoRecordingState g_zViewVideoRecordingLatestState =
    ZV_VIDEO_RECORDING_STATE_NOT_AVAILABLE;

// zView Standard Mode

ZSUInt16 g_zViewStandardModeImageWidth  = 0;
ZSUInt16 g_zViewStandardModeImageHeight = 0;

ZCHandle g_zViewStandardModeViewportHandle = NULL;
ZCHandle g_zViewStandardModeFrustumHandle  = NULL;

ZSUInt64 g_zViewStandardModeFrameNumber = 0;

GLuint g_zViewStandardModeGlFramebufferId       = 0;
GLuint g_zViewStandardModeColorGlTextureId      = 0;
GLuint g_zViewStandardModeDepthGlRenderbufferId = 0;

// zView Augmented Reality Mode

GLuint g_zViewAugmentedRealityModeMaskVertexShader           = 0;
GLuint g_zViewAugmentedRealityModeMaskFragmentShader         = 0;
GLuint g_zViewAugmentedRealityModeMaskShaderProgram          = 0;
GLuint g_zViewAugmentedRealityModeMaskTransformMatrixUniform = 0;

GLuint g_zViewAugmentedRealityModeBackgroundVertexShader           = 0;
GLuint g_zViewAugmentedRealityModeBackgroundFragmentShader         = 0;
GLuint g_zViewAugmentedRealityModeBackgroundShaderProgram          = 0;
GLuint g_zViewAugmentedRealityModeBackgroundColorUniform           = 0;

ZSUInt16 g_zViewAugmentedRealityModeImageWidth  = 0;
ZSUInt16 g_zViewAugmentedRealityModeImageHeight = 0;
////////////------------------------------------------------------------------------------------------------------
ZSMatrix4 g_zViewAugmentedRealityModeCameraPoseDisplaySpace;
glm::mat4 g_zViewAugmentedRealityModeCameraWorldSpaceViewTransform;
glm::mat4 g_zViewAugmentedRealityModeCameraProjection;
glm::mat4 g_zViewAugmentedRealityModeMaskTransform;
/////--------------------------------------------------------------------------------------------------------------
GLuint g_zViewAugmentedRealityModeMaskVertexArrayId       = 0;
GLuint g_zViewAugmentedRealityModeMaskVertexArrayBufferId = 0;

GLuint g_zViewAugmentedRealityModeBackgroundVertexArrayId       = 0;
GLuint g_zViewAugmentedRealityModeBackgroundVertexArrayBufferId = 0;

GLuint g_zViewAugmentedRealityModeMaskGlFramebufferId          = 0;
GLuint g_zViewAugmentedRealityModeGlFramebufferId              = 0;
GLuint g_zViewAugmentedRealityModeColorGlTextureId             = 0;
GLuint g_zViewAugmentedRealityModeDepthStencilGlRenderbufferId = 0;

bool g_zViewAugmentedRealityModeShouldDrawMask       = false;
bool g_zViewAugmentedRealityModeShouldDrawBackground = true;


//////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////

bool initialize();
void shutDown();

bool update();
void updateCamera();

void draw();
void drawSceneForEye(ZCEye eye);
void drawCube();
void drawStylus();

void setDrawBuffer(ZCEye eye);
bool computeViewMatrix(ZCEye eye);
bool setProjectionMatrix(ZCEye eye);

bool setupScene();

bool createRenderContext(HWND hWnd, HDC& hDC, HGLRC& hRC);
void destroyRenderContext(HWND hWnd, HDC& hDC, HGLRC& hRC);

bool createWindow(int x, int y, int width, int height, HWND& hWnd, HINSTANCE& hInstance);
void destroyWindow(HWND& hWnd, HINSTANCE& hInstance);
LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

bool loadTexture(const char* filename);
bool loadShaderSource(std::string filename, std::string& source);
bool compileAndLinkShaderProgram(
    const std::string& vertexShaderSource,
    std::string& fragmentShaderSource,
    GLuint& vertexShaderId,
    GLuint& fragmentShaderId,
    GLuint& shaderProgramId);
bool loadShaders();

std::string convertToString(int value);

// zView

bool initializeZview();

bool loadZviewAugmentedRealityModeShaders();

bool updateZview();
bool updateZviewConnection(ZVConnection connection);

bool processNewZviewConnection(ZVConnection connection);

bool processZviewVideoRecording(ZVConnection connection);
bool processZviewVideoRecordingKeyPress(WPARAM key);
bool processZviewAugmentedRealityModeOverlayControlKeyPress(WPARAM key);
bool switchZviewVideoRecordingQuality();
bool incrementZviewSettingClampedF32(
    ZVConnection connection,
    ZVSettingKey settingKey,
    ZSFloat increment,
    ZSFloat min,
    ZSFloat max);

bool handleZviewStandardModeImageResolutionChange();

bool updateZviewAugmentedRealityMode();

bool setUpZviewMode(ZVConnection connection);
bool tearDownZviewMode();

bool setUpZviewStandardMode(ZVConnection connection);
bool tearDownZviewStandardMode();

bool setUpZviewAugmentedRealityMode(ZVConnection connection);
bool tearDownZviewAugmentedRealityMode();

bool drawZview();

bool drawZviewStandardMode();

bool drawZviewAugmentedRealityMode();
bool drawZviewAugmentedRealityModeMask();
bool drawZviewAugmentedRealityModeBackground();

glm::mat4 computeZviewAugmentedRealityModeProjectionMatrix(
    float focalLength,
    float principalPointOffsetX,
    float principalPointOffsetY,
    float pixelAspectRatio,
    float axisSkew,
    float imageWidth,
    float imageHeight,
    float nearClip,
    float farClip);

bool switchZviewMode();

bool pauseResumeZviewMode();

bool closeZviewConnectionAndExitViewer();


//////////////////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE    hInstance,      // Instance
                   HINSTANCE    hPrevInstance,  // Previous Instance
                   LPSTR        lpCmdLine,      // Command Line Parameters
                   int          nCmdShow)       // Window Show State
{
    bool isInitialized = initialize();
    if (!isInitialized)
    {
        shutDown();
        return -1;
    }

    MSG msg;
    while (TRUE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Update the scene state.
        update();

        updateZview();

        // Draw the frame.
        draw();

        drawZview();
    }

    shutDown();
    return 0;
}


//////////////////////////////////////////////////////////////////////////
// Functions
//////////////////////////////////////////////////////////////////////////

bool initialize()
{
    ZCError error;

    // Initialize the zSpace SDK. This MUST be called before
    // calling any other zSpace API.
    error = zcInitialize(&g_zSpaceContext);
    CHECK_ZC_ERROR(error);

    // Create a stereo buffer to handle L/R detection.
    error = zcCreateStereoBuffer(
        g_zSpaceContext, ZC_RENDERER_QUAD_BUFFER_GL, 0, &g_bufferHandle);
    CHECK_ZC_ERROR(error);

    // Create a zSpace viewport object and grab its associated frustum. 
    // Note: The zSpace viewport is abstract and not an actual window/viewport
    // that is created and registered through the Windows OS. It manages
    // a zSpace stereo frustum, which is responsible for various stereoscopic 
    // 3D calculations such as calculating the view and projection matrices for 
    // each eye.
    error = zcCreateViewport(g_zSpaceContext, &g_viewportHandle);
    CHECK_ZC_ERROR(error);

    error = zcGetFrustum(g_viewportHandle, &g_frustumHandle); 
    CHECK_ZC_ERROR(error);

    // Grab a handle to the stylus target.
    error = zcGetTargetByType(
        g_zSpaceContext, ZC_TARGET_TYPE_PRIMARY, 0, &g_stylusHandle);
    CHECK_ZC_ERROR(error);

    // Find the zSpace display and set the window's position
    // to be the top left corner of the zSpace display.
    error = zcGetDisplayByType(
        g_zSpaceContext, ZC_DISPLAY_TYPE_ZSPACE, 0, &g_displayHandle);
    CHECK_ZC_ERROR(error);

    error = zcGetDisplayPosition(g_displayHandle, &g_windowX, &g_windowY);
    CHECK_ZC_ERROR(error);

    // Create the OpenGL application window.
    bool isWindowCreated = createWindow(
        g_windowX, g_windowY, g_windowWidth, g_windowHeight, g_hWnd, g_hInstance);
    if (!isWindowCreated)
        return false;

    // Create the OpenGL rendering context.
    bool isRenderContextCreated = createRenderContext(g_hWnd, g_hDC, g_hRC);
    if (!isRenderContextCreated)
        return false;

    // Set up the OpenGL scene.
    bool isSceneSetup = setupScene();
    if (!isSceneSetup)
        return false;

    // Initialize zView.
    bool isZviewInitialized = initializeZview();
    if (!isZviewInitialized)
    {
        return false;
    }

    // Show the application window.
    ShowWindow(g_hWnd, SW_SHOW);
    SetForegroundWindow(g_hWnd);
    SetFocus(g_hWnd);

    return true;
}


void shutDown()
{
    // Shut down and clean up the zView SDK.
    if (g_zViewContext != NULL)
    {
        zvShutDown(g_zViewContext);
        g_zViewContext = NULL;
    }

    // Shut down and cleanup the zSpace SDK.
    zcShutDown(g_zSpaceContext);

    // Destroy the OpenGL rendering context.
    destroyRenderContext(g_hWnd, g_hDC, g_hRC);

    // Destroy the OpenGL application window.
    destroyWindow(g_hWnd, g_hInstance);
}

void printLog(std::string s)
{
    std::string st = s + "\n";
    TCHAR name[256];
    _tcscpy_s(name, CA2T(st.c_str()));
    OutputDebugString(name);
}

bool update()
{
    ZCError error;

    // Update the camera.
    updateCamera();

    // Update the zSpace viewport position and size based
    // on the position and size of the application window.
    error = zcSetViewportPosition(g_viewportHandle, g_windowX, g_windowY);
    CHECK_ZC_ERROR(error);

    error = zcSetViewportSize(g_viewportHandle, g_windowWidth, g_windowHeight);
    CHECK_ZC_ERROR(error);

    // Update the OpenGL viewport size;
    glViewport(0, 0, g_windowWidth, g_windowHeight);

    // Update the zSpace SDK. This updates both tracking information
    // as well as the head poses for any frustums that have been created.
    error = zcUpdate(g_zSpaceContext);
    CHECK_ZC_ERROR(error);

    // Grab the stylus pose (position and orientation) in tracker space.
    ZCTrackerPose stylusPose;
    error = zcGetTargetPose(g_stylusHandle, &stylusPose);
    CHECK_ZC_ERROR(error);

    // Transform the stylus pose from tracker to camera space.
    error = zcTransformMatrix(
        g_viewportHandle, ZC_COORDINATE_SPACE_TRACKER, ZC_COORDINATE_SPACE_CAMERA, &stylusPose.matrix);
    CHECK_ZC_ERROR(error);


    glm::mat4 stylusPoseCamera = glm::make_mat4(stylusPose.matrix.f);
    g_stylusWorldPose = g_invCameraTransform * stylusPoseCamera;/////////////////////////////////////////////////////////////////////////////////////////////

    return true;
}


void updateCamera()
{
    // Calculate the camera's new position such that it orbits
    // the world's origin.
    GLfloat eyeX = 0.222f * sin(g_cameraAngle * PI / 180.0f);
    GLfloat eyeY = 0.345f;
    GLfloat eyeZ = 0.222f * cos(g_cameraAngle * PI / 180.0f);

    glm::vec3 eye = glm::vec3(eyeX, eyeY, eyeZ);
    glm::vec3 center = glm::vec3();
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    g_cameraTransform = glm::lookAt(eye, center, up);
    g_invCameraTransform = glm::inverse(g_cameraTransform);

    // Update the camera angle if camera orbit is enabled.
    clock_t currentTime = clock();

    if (g_isCameraOrbitEnabled)
    {
        float deltaTime = static_cast<float>(currentTime - g_previousTime) / CLOCKS_PER_SEC;
        g_cameraAngle  += ROTATION_PER_SECOND * deltaTime;
        g_cameraAngle   = fmodf(g_cameraAngle, 360.0f);
    }

    g_previousTime = currentTime;
}


void draw()
{
    // This must be called every frame on the rendering thread in order 
    // to handle the initial sync and any subsequent pending sync requests 
    // for left/right frame detection.
    ZCError error = zcBeginStereoBufferFrame(g_bufferHandle);

    // Set the application window's rendering context as the current rendering context.
    wglMakeCurrent(g_hDC, g_hRC);

    // Draw the scene for each eye.
    drawSceneForEye(ZC_EYE_LEFT);
    drawSceneForEye(ZC_EYE_RIGHT);

    // Flush the render buffers.
    SwapBuffers(g_hDC);
}


void drawSceneForEye(ZCEye eye)
{
    // Set the view and projection matrices for the specified eye.
    computeViewMatrix(eye);
    setProjectionMatrix(eye);

    // Set the render target based for the specified eye.
    setDrawBuffer(eye);

    // Clear the scene - color and depth buffers.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw the cube.
    drawCube();

    // Draw the stylus.
    drawStylus();
}

void createVertexArrays()
{
    const float halfSize = CUBE_HALF_SIZE;
    float normalData[24*3] = {
        0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f
    };

    float textureData[24 * 2] = {
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,0.0f, 0.0f,0.0f, 1.0f,0.0f, 0.0f,
        1.0f, 0.0f,1.0f, 1.0f,0.0f, 1.0f,0.0f, 0.0f,1.0f, 0.0f,1.0f, 1.0f,
        0.0f, 1.0f,0.0f, 0.0f,1.0f, 0.0f,1.0f, 1.0f,1.0f, 1.0f,0.0f, 1.0f,
        0.0f, 0.0f,1.0f, 0.0f,0.0f, 1.0f,0.0f, 0.0f,1.0f, 0.0f,1.0f, 1.0f
    };

    float vertexData[24 * 3] = {
        -halfSize, -halfSize, -halfSize,-halfSize, halfSize, -halfSize,halfSize, halfSize, -halfSize,halfSize, -halfSize, -halfSize,
        -halfSize, halfSize, halfSize,-halfSize, -halfSize, halfSize,halfSize, -halfSize, halfSize,halfSize, halfSize, halfSize,
        -halfSize, halfSize, -halfSize,-halfSize, halfSize, halfSize,halfSize, halfSize, halfSize,halfSize, halfSize, -halfSize,
        -halfSize, -halfSize, halfSize,-halfSize, -halfSize, -halfSize,halfSize, -halfSize, -halfSize,halfSize, -halfSize, halfSize,
        halfSize, halfSize, -halfSize,halfSize, halfSize, halfSize,halfSize, -halfSize, halfSize,halfSize, -halfSize, -halfSize,
        -halfSize, halfSize, -halfSize,-halfSize, -halfSize, -halfSize,-halfSize, -halfSize, halfSize,-halfSize, halfSize, halfSize
    };

    glGenVertexArrays(1, &g_cubeVertexArrayID);

    glBindVertexArray(g_cubeVertexArrayID);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &g_cubeVertexArrayBufferIDs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, g_cubeVertexArrayBufferIDs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(g_cubeVertexArrayID);
    glEnableVertexAttribArray(1);
    glGenBuffers(1, &g_cubeVertexArrayBufferIDs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, g_cubeVertexArrayBufferIDs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normalData), normalData, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(g_cubeVertexArrayID);
    glEnableVertexAttribArray(2);
    glGenBuffers(1, &g_cubeVertexArrayBufferIDs[2]);
    glBindBuffer(GL_ARRAY_BUFFER, g_cubeVertexArrayBufferIDs[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureData), textureData, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

void drawCube()
{
    glUseProgram(g_shaderProgram);
    glUniform1i(g_renderStylusUniform, 0);
    glUniformMatrix4fv(
        g_modelViewUniform, 1, GL_FALSE, glm::value_ptr(g_viewMatrix));
    glBindTexture(GL_TEXTURE_2D, g_cubeTextureID);

    if (g_cubeVertexArrayID == 0)
    {
        createVertexArrays();
    }

    glBindVertexArray(g_cubeVertexArrayID);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, g_cubeVertexArrayBufferIDs[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, g_cubeVertexArrayBufferIDs[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, g_cubeVertexArrayBufferIDs[2]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);


    for (int i=3; i<8; i++)
    {
        glDisableVertexAttribArray(i);
    }

    glDrawArrays(GL_QUADS, 0, 24);
}


void drawStylus()
{
    glUniformMatrix4fv(
        g_modelViewUniform, 1, GL_FALSE, glm::value_ptr(g_viewMatrix*g_stylusWorldPose));
    glUniform1i(g_renderStylusUniform, 1);

    // Draw the line.
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, -STYLUS_LENGTH);
    glEnd();
}


void setDrawBuffer(ZCEye eye)
{
    // Select appropriate back buffer to render to based on the specified eye.
    switch (eye)
    {
        case ZC_EYE_LEFT:
            glDrawBuffer(GL_BACK_LEFT);
            break;
        case ZC_EYE_RIGHT:
            glDrawBuffer(GL_BACK_RIGHT);
            break;
        default:
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool computeViewMatrix(ZCEye eye)
{
    // Get the view matrix from the zSpace StereoFrustum for the specified eye.
    ZSMatrix4 viewMatrix;
    ZCError error = zcGetFrustumViewMatrix(g_frustumHandle, eye, &viewMatrix);
    CHECK_ZC_ERROR(error);

    glm::mat4 zcViewMatrix = glm::make_mat4(viewMatrix.f);
    g_viewMatrix = zcViewMatrix * g_cameraTransform;
    return true;
}


bool setProjectionMatrix(ZCEye eye)
{
    // Get the projection matrix from the zSpace StereoFrustum for a specified
    // eye.
    ZSMatrix4 projectionMatrix;
    ZCError error = zcGetFrustumProjectionMatrix(g_frustumHandle, eye, &projectionMatrix);
    CHECK_ZC_ERROR(error);

    glm::mat4 zcProjMatrix = glm::make_mat4(projectionMatrix.f);
    glUseProgram(g_shaderProgram);
    glUniformMatrix4fv(
        g_projectionUniform, 1, GL_FALSE, glm::value_ptr(zcProjMatrix));
    return true;
}


bool setupScene()
{
    if (!loadShaders())
    {
        return false;
    }

    glm::vec3 eye = glm::vec3();
    glm::vec3 origin = glm::vec3(0.0f, 0.345f, 0.222f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    g_cameraTransform = glm::lookAt(eye, origin, up);

    // Enable depth testing.
    glEnable(GL_DEPTH_TEST);

    // Set up textures.
    glGenTextures(1, &g_cubeTextureID);
    glBindTexture(GL_TEXTURE_2D, g_cubeTextureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    // Load texture from bmp file.
    std::string cubeTextureName = CUBE_TEXTURE_NAME;
    if (!loadTexture(cubeTextureName.c_str()))
    {
        // If load fails:
        // The current working directory is not the same as the
        // executable's directory, so use the executable's directory.
        char moduleFilename[256];
        GetModuleFileName(NULL, moduleFilename, sizeof(moduleFilename));
        cubeTextureName = std::string(moduleFilename) + std::string("/../") + cubeTextureName;
        loadTexture(cubeTextureName.c_str());
    }

    // Set up and enable texture mapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);

    return true;
}


bool createRenderContext(HWND hWnd, HDC& hDC, HGLRC& hRC)
{
    GLuint pixelFormat;  // Holds the results after searching for a match

    // Set the appropriate pixel format.
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),      // Size Of This Pixel Format Descriptor
        1,                                  // Version Number
        PFD_DRAW_TO_WINDOW |                // Format Must Support Window
            PFD_SUPPORT_OPENGL |            // Format Must Support OpenGL
            PFD_STEREO |                    // Format Must Support Quad-buffer Stereo
            PFD_DOUBLEBUFFER,               // Must Support Double Buffering
        PFD_TYPE_RGBA,                      // Request An RGBA Format
        24,                                 // 24-bit color depth
        0, 0, 0, 0, 0, 0,                   // Color Bits Ignored
        0,                                  // No Alpha Buffer
        0,                                  // Shift Bit Ignored
        0,                                  // No Accumulation Buffer
        0, 0, 0, 0,                         // Accumulation Bits Ignored
        32,                                 // 32-bit Z-Buffer (Depth Buffer)
        0,                                  // No Stencil Buffer
        0,                                  // No Auxiliary Buffer
        PFD_MAIN_PLANE,                     // Main Drawing Layer
        0,                                  // Reserved
        0, 0, 0                             // Layer Masks Ignored
    };

    // Did we get a Device Context?
    if (!(hDC = GetDC(hWnd)))
    {
        MessageBox(NULL, "Can't create an OpenGL DeviceContext.", "OpenGL Error", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return false;
    }

    // Did windows find a matching Pixel Format?
    if (!(pixelFormat = ChoosePixelFormat(hDC, &pfd)))
    {
        MessageBox(NULL, "Can't find a suitable PixelFormat.", "OpenGL Error", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return false;
    }

    // Are we able to set the Pixel Format?
    if (!SetPixelFormat(hDC, pixelFormat, &pfd))
    {
        MessageBox(NULL, "Can't set the PixelFormat.", "OpenGL Error", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return false;
    }

    // Are we able to get a Rendering Context?
    if (!(hRC = wglCreateContext(hDC)))
    {
        MessageBox(NULL, "Can't create an OpenGL RenderingContext.", "OpenGL Error", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return false;
    }

    // Try to activate the current Rendering Context.
    if (!wglMakeCurrent(hDC, hRC))
    {
        MessageBox(NULL, "Can't activate the OpenGL RenderingContext.", "OpenGL Error" , MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return false;
    }

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        MessageBox(NULL, "Can't Initialize glew.", "OpenGL Error", MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
        return false;
    }

    return true;
}


void destroyRenderContext(HWND hWnd, HDC& hDC, HGLRC& hRC)
{
    // Do we have a rendering context?
    if (hRC)
    {
        // Are we able to release the device and rendering contexts?
        if (!wglMakeCurrent(NULL, NULL))
            MessageBox(NULL, "Release Of DC And RC Failed.", "OpenGL Error", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

        // Are we able to delete the rendering context.
        if (!wglDeleteContext(hRC))
            MessageBox(NULL, "Release Rendering Context Failed.", "OpenGL Error", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

        hRC = NULL;
    }

    // Are we able to release the device context?
    if (hDC)
    {
        if (!ReleaseDC(hWnd, hDC))
            MessageBox(NULL, "Release Device Context Failed.", "OpenGL Error", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

        hDC = NULL;
    }
}


bool createWindow(int x, int y, int width, int height, HWND& hWnd, HINSTANCE& hInstance)
{
    WNDCLASS windowClass;  // Windows Class Structure

    // Grab an application instance for our window.
    hInstance = GetModuleHandle(NULL);

    ZeroMemory(&windowClass, sizeof(WNDCLASS));
    windowClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;   // Redraw On Move, And Own DC For Window
    windowClass.lpfnWndProc   = (WNDPROC)&windowProc;                 // WindowProc Handles Messages
    windowClass.cbClsExtra    = 0;                                    // No Extra Window Data
    windowClass.cbWndExtra    = 0;                                    // No Extra Window Data
    windowClass.hInstance     = hInstance;                            // Set The Instance
    windowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);          // Load The Arrow Pointer
    windowClass.hbrBackground = NULL;                                 // No Background Required For GL
    windowClass.lpszMenuName  = NULL;                                 // We Don't Want A Menu
    windowClass.lpszClassName = WINDOW_CLASS_NAME;                    // Set The Class Name
    windowClass.hIcon         = (HICON)LoadImage(NULL, WINDOW_ICON_NAME, IMAGE_ICON, 16, 16, LR_LOADFROMFILE);

    // Attempt to register the window class.
    if (!RegisterClass(&windowClass))
    {
        MessageBox(NULL, "Failed to register the Window Class.", "Win32 Error", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return false;
    }

    hWnd = CreateWindowEx(
        NULL,                   // Extended Style For The Window
        WINDOW_CLASS_NAME,      // Class Name
        WINDOW_NAME,            // Window Title
        WS_OVERLAPPEDWINDOW |
        WS_CLIPCHILDREN     |
        WS_CLIPSIBLINGS,
        x, y,                   // Window Position
        width,                  // Window Width
        height,                 // Window Height
        NULL,                   // No Parent Window
        NULL,                   // No Menu
        hInstance,              // Instance
        NULL);

    //SetWindowLong(hWnd, GWL_STYLE, 0);

    if (!hWnd)
    {
        MessageBox(NULL, "Window creation error.", "Win32 Error", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return false;
    }

    return true;
}


void destroyWindow(HWND& hWnd, HINSTANCE& hInstance)
{
    // Are we able to destroy the window?
    if (hWnd)
    {
        if (!DestroyWindow(hWnd))
            MessageBox(NULL, "Could Not Release hWnd.", "Win32 Error", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

        hWnd = NULL;
    }

    // Are we able to unregister the window class?
    if (hInstance)
    {
        if (!UnregisterClass(WINDOW_CLASS_NAME, hInstance))
            MessageBox(NULL, "Could Not Unregister Class.", "Win32 Error", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

        hInstance = NULL;
    }
}


LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CLOSE:
            {
                shutDown();
                return 0;
            }
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            }
        case WM_SIZE:
            {
                // Get the window size.
                RECT rect;
                if (GetClientRect(hWnd, &rect))
                {
                    g_windowWidth  = rect.right - rect.left;
                    g_windowHeight = rect.bottom - rect.top;
                }

                // Update and draw the frame.
                update();
                draw();

                // Force a redraw on the application window.
                InvalidateRect(hWnd, NULL, TRUE);
                return 0;
            }
        case WM_MOVE:
            {
                // Get the window size.
                RECT rect;
                if (GetClientRect(hWnd, &rect))
                {
                    g_windowWidth  = rect.right - rect.left;
                    g_windowHeight = rect.bottom - rect.top;
                }

                // Get the window position.
                g_windowX = (int)(short)LOWORD(lParam);
                g_windowY = (int)(short)HIWORD(lParam);

                // Update and draw the frame.
                update();
                draw();

                // Force a redraw on the application window.
                InvalidateRect(hWnd, NULL, TRUE);
                return 0;
            }
        case WM_KEYDOWN:
            {
                const bool isControlKeyPressed =
                    (GetKeyState(VK_CONTROL) & 0x8000) != 0;

                const bool isShiftKeyPressed =
                    (GetKeyState(VK_SHIFT) & 0x8000) != 0;

                switch (wParam)
                {
                    case VK_ESCAPE:
                        // If 'escape' was pressed, exit the application.
                        PostMessage(g_hWnd, WM_DESTROY, 0, 0);
                        break;

                    case VK_SPACE:
                        // If 'space' was pressed, toggle camera orbit.
                        g_isCameraOrbitEnabled = !g_isCameraOrbitEnabled;
                        break;

                    case 'A':
                        if (isControlKeyPressed)
                        {
                            processZviewAugmentedRealityModeOverlayControlKeyPress(wParam);
                        }
                        break;

                    case 'B':
                        // Toggle drawing the background in augmented reality
                        // mode images.
                        g_zViewAugmentedRealityModeShouldDrawBackground =
                            !g_zViewAugmentedRealityModeShouldDrawBackground;
                        break;

                    case 'C':
                        // Initiate a connection to the default zView viewer if
                        // no zView connection is currently active.
                        if (g_zViewActiveConnection == NULL)
                        {
                            printLog("Connecting to default viewer...");
                            zvConnectToDefaultViewer(g_zViewContext, NULL);
                        }
                        break;

                    case 'D':
                        if (isShiftKeyPressed)
                        {
                            processZviewVideoRecordingKeyPress(wParam);
                        }
                        else if (isControlKeyPressed)
                        {
                            processZviewAugmentedRealityModeOverlayControlKeyPress(wParam);
                        }
                        break;

                    case 'E':
                        if (isControlKeyPressed)
                        {
                            processZviewAugmentedRealityModeOverlayControlKeyPress(wParam);
                        }
                        else
                        {
                            // Close the active zView connection (if there is
                            // one) and request that the viewer application
                            // exit after the connection is closed.
                            closeZviewConnectionAndExitViewer();
                        }
                        break;

                    case 'F':
                        if (isShiftKeyPressed)
                        {
                            processZviewVideoRecordingKeyPress(wParam);
                        }
                        else if (isControlKeyPressed)
                        {
                            processZviewAugmentedRealityModeOverlayControlKeyPress(wParam);
                        }
                        break;

                    case 'G':
                        if (isControlKeyPressed)
                        {
                            processZviewAugmentedRealityModeOverlayControlKeyPress(wParam);
                        }
                        break;

                    case 'H':
                        if (isControlKeyPressed)
                        {
                            processZviewAugmentedRealityModeOverlayControlKeyPress(wParam);
                        }
                        break;

                    case 'M':
                        // Switch to a different zView mode is there is an
                        // active zView connection.
                        switchZviewMode();
                        break;

                    case 'P':
                        if (isShiftKeyPressed)
                        {
                            processZviewVideoRecordingKeyPress(wParam);
                        }
                        else
                        {
                            // Pause or resume the current zView mode if there
                            // is an active zView connection with an active
                            // mode.
                            pauseResumeZviewMode();
                        }
                        break;

                    case 'Q':
                        if (isShiftKeyPressed)
                        {
                            processZviewVideoRecordingKeyPress(wParam);
                        }
                        else if (isControlKeyPressed)
                        {
                            processZviewAugmentedRealityModeOverlayControlKeyPress(wParam);
                        }
                        break;

                    case 'R':
                        if (isShiftKeyPressed)
                        {
                            processZviewVideoRecordingKeyPress(wParam);
                        }
                        else if (isControlKeyPressed)
                        {
                            processZviewAugmentedRealityModeOverlayControlKeyPress(wParam);
                        }
                        break;

                    case 'S':
                        if (isShiftKeyPressed)
                        {
                            processZviewVideoRecordingKeyPress(wParam);
                        }
                        else if (isControlKeyPressed)
                        {
                            processZviewAugmentedRealityModeOverlayControlKeyPress(wParam);
                        }
                        break;

                    case 'T':
                        if (isControlKeyPressed)
                        {
                            processZviewAugmentedRealityModeOverlayControlKeyPress(wParam);
                        }
                        break;

                    case 'V':
                        // Toggle drawing the mask in augmented reality mode
                        // images.
                        g_zViewAugmentedRealityModeShouldDrawMask =
                            !g_zViewAugmentedRealityModeShouldDrawMask;
                        break;

                    case 'W':
                        if (isControlKeyPressed)
                        {
                            processZviewAugmentedRealityModeOverlayControlKeyPress(wParam);
                        }
                        break;

                    case 'Y':
                        if (isControlKeyPressed)
                        {
                            processZviewAugmentedRealityModeOverlayControlKeyPress(wParam);
                        }
                        break;
                }

                return 0;
            }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}


bool loadTexture(const char* filename)
{
    if (!filename)
        return false;

    bool             success    = false;
    BITMAPFILEHEADER fileHeader = {};
    FILE*            fileHandle = NULL;
    ZSInt8*          data       = NULL;
    fopen_s(&fileHandle, filename, "rb");

    if (!fileHandle)
        return false;

    // Read the bmp file header.
    size_t bytesRead = fread(&fileHeader, 1, sizeof(fileHeader), fileHandle);
    if (bytesRead == sizeof(BITMAPFILEHEADER))
    {
        // Header is read, now check the signature.
        // NOTE: We aren't checking for endianness here which 
        //       could cause this check to fail! Just an FYI, you 
        //       really don't see BMPs anywhere other than Windows - 
        //       which is usually little endian Intel... 
        if (fileHeader.bfType == BMP_SIGNATURE)
        {
            // read the rest of the header
            BITMAPINFOHEADER header = {};
            bytesRead = fread(&header, 1, sizeof(header), fileHandle);
            if (bytesRead == sizeof(BITMAPINFOHEADER))
            {
                // bail on images that aren't 24 bits deep or aren't in a single plane
                if (header.biBitCount == 24 && header.biPlanes == 1)
                {
                    // Bmp is 4-byte aligned, compute row bytes (width * r, g, b components)
                    ZSUInt32 rowBytes         = (header.biWidth * 3 + 3) & ~3;
                    ZSUInt32 headerSize       = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
                    ZSUInt32 expectedFileSize = headerSize + (rowBytes * header.biHeight);
                    if (fileHeader.bfSize == expectedFileSize)
                    {
                        ZSUInt32 bufferSize = fileHeader.bfSize - headerSize;

                        // Allocate buffer.
                        data = (ZSInt8*)malloc(bufferSize);
                        if (data)
                        {
                            // Read pixel data from bmp.
                            bytesRead = fread(data, 1, bufferSize, fileHandle);

                            // Create a texture from the pixel data.
                            if (bytesRead == bufferSize)
                            {
                                glTexImage2D(
                                    GL_TEXTURE_2D,
                                    0,
                                    4,
                                    header.biWidth,
                                    header.biHeight,
                                    0,
                                    GL_BGR_EXT,
                                    GL_UNSIGNED_BYTE,
                                    data);
                                success = true;
                            }
                        }
                    }
                }
            }
        }
    }

    // Done with file.
    fclose(fileHandle);

    // glTexImage2D creates a texture from data[] so we can free it now.
    if (data)
        free(data);

    return success;
}


bool loadShaderSource(std::string filename, std::string& source)
{
    errno_t err;
    FILE* file;
    err = fopen_s(&file, filename.c_str(), "r");
    if (err)
    {
        // If load fails:
        // The current working directory is not the same as the
        // executable's directory, so use the executable's directory.
        char moduleFilename[256];
        GetModuleFileName(NULL, moduleFilename, sizeof(moduleFilename));
        filename = std::string(moduleFilename) + std::string("/../") + filename;
        err = fopen_s(&file, filename.c_str(), "r");
    }

    if (!err)
    {
        fseek(file, 0, SEEK_END);
        long shaderSourceLength = ftell(file);
        rewind(file);

        std::vector<char> shaderSourceVec(shaderSourceLength);
        shaderSourceLength = (long)fread_s(
            &(shaderSourceVec[0]),
            shaderSourceVec.size(),
            sizeof(char),
            shaderSourceVec.size(),
            file);
        fclose(file);

        source.assign(
            shaderSourceVec.begin(),
            shaderSourceVec.begin() + shaderSourceLength);
    }
    else
    {
        return false;
    }

    return true;
}

bool compileAndLinkShaderProgram(
    const std::string& vertexShaderSource,
    std::string& fragmentShaderSource,
    GLuint& vertexShaderId,
    GLuint& fragmentShaderId,
    GLuint& shaderProgramId)
{
    GLuint newVertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSourceCStr = vertexShaderSource.c_str();
    const int vertexShaderSourceLength = (int)vertexShaderSource.size();
    glShaderSource(
        newVertexShaderId,
        1,
        &vertexShaderSourceCStr,
        &vertexShaderSourceLength);
    glCompileShader(newVertexShaderId);
    if (glGetError() != GL_NO_ERROR)
    {
        glDeleteShader(newVertexShaderId);
        return false;
    }

    GLuint newFragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str();
    const int fragmentShaderSourceLength = (int)fragmentShaderSource.size();
    glShaderSource(
        newFragmentShaderId,
        1,
        &fragmentShaderSourceCStr,
        &fragmentShaderSourceLength);
    glCompileShader(newFragmentShaderId);
    if (glGetError() != GL_NO_ERROR)
    {
        glDeleteShader(newVertexShaderId);
        glDeleteShader(newFragmentShaderId);
        return false;
    }

    GLuint newShaderProgramId = glCreateProgram();
    glAttachShader(newShaderProgramId, newVertexShaderId);
    glAttachShader(newShaderProgramId, newFragmentShaderId);
    glLinkProgram(newShaderProgramId);
    if (glGetError() != GL_NO_ERROR)
    {
        glDeleteShader(newVertexShaderId);
        glDeleteShader(newFragmentShaderId);
        glDeleteProgram(newShaderProgramId);
        return false;
    }

    vertexShaderId = newVertexShaderId;
    fragmentShaderId = newFragmentShaderId;
    shaderProgramId = newShaderProgramId;

    return true;
}


bool loadShaders()
{
    std::string vertexShaderSource;
    if (!loadShaderSource(VERTEX_SHADER_NAME, vertexShaderSource))
    {
        return false;
    }

    std::string fragmentShaderSource;
    if (!loadShaderSource(FRAGMENT_SHADER_NAME, fragmentShaderSource))
    {
        return false;
    }

    if (!compileAndLinkShaderProgram(
        vertexShaderSource,
        fragmentShaderSource,
        g_vertexShader,
        g_fragmentShader,
        g_shaderProgram))
    {
        return false;
    }

    glUseProgram(g_shaderProgram);
    if (glGetError() != GL_NO_ERROR)
    {
        return false;
    }

    g_projectionUniform = glGetUniformLocation(g_shaderProgram, "projectionMatrix");
    g_modelViewUniform = glGetUniformLocation(g_shaderProgram, "modelViewMatrix");

    g_numLightsUniform = glGetUniformLocation(g_shaderProgram, "numLights");
    g_renderStylusUniform = glGetUniformLocation(g_shaderProgram, "renderStylus");

    g_lightsUniform[0].position = glGetUniformLocation(g_shaderProgram, "lights[0].position");
    g_lightsUniform[0].diffuse = glGetUniformLocation(g_shaderProgram, "lights[0].diffuse");
    g_lightsUniform[0].specular = glGetUniformLocation(g_shaderProgram, "lights[0].specular");


    g_specularColorUniform = glGetUniformLocation(g_shaderProgram, "specularColor");
    g_ambientColorUniform = glGetUniformLocation(g_shaderProgram, "ambientColor");


    glUniform3fv(g_lightsUniform[0].position, 1, LIGHT_POSITION);
    glUniform3fv(g_lightsUniform[0].diffuse, 1, LIGHT_COLOR);
    glUniform3fv(g_ambientColorUniform, 1, AMBIENT_COLOR);
    glUniform3fv(g_specularColorUniform, 1, SPECULAR_COLOR);
    glUniform1i(g_numLightsUniform, 1);

    return true;
}


std::string convertToString(int value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}


bool initializeZview()
{
    ZVError error = ZV_ERROR_OK;

    // Initialize a zView API context corresponding to a presenter node.  This
    // node represents this sample application.
    error = zvInitialize(ZV_NODE_TYPE_PRESENTER, &g_zViewContext);
    CHECK_ZV_ERROR(error);

    // Set the name of this zView node.  This may be displayed in viewer
    // applications that are connecting or connected to this node.
    error = zvSetNodeName(g_zViewContext, ZVIEW_NODE_NAME);
    CHECK_ZV_ERROR(error);

    // Set the initial status of this zView node.  This may be displayed in
    // viewer applications that are connecting or connected to this node.  A
    // zView node's status string is purely for informational purposes and may
    // contain any information that a user might find useful when browsing a
    // list of presenter applications to connect to.  For example, the status
    // string might contain information on whether an application is able to
    // accept additional zView connections or information on what scene is
    // currently active in the application.
    //
    // In the case of this sample, the status is used to indicate whether this
    // zView node is connected to another node or not.
    error = zvSetNodeStatus(g_zViewContext, ZVIEW_NODE_STATUS_NOT_CONNECTED);
    CHECK_ZV_ERROR(error);

    // Specify the zView modes that this node supports.  This is done using the
    // following steps:
    //
    // 1.  Create mode specs representing each supported mode.
    //
    // 2.  Get a mode handle corresponding to each created mode spec.
    //
    // 3.  Create ZVSupportedMode struct instances for each mode handle.
    //
    // 4.  Create an array of the ZVSupportedMode instances and register it
    //     with the zView node.
    //
    // Notes:
    //
    // - zView modes can be thought of as belonging to mode families.  Each
    //   family can contain multiple modes that all work similarly, but differ
    //   in terms of certain details.
    // - Currently there are two mode families:  the standard mode family and
    //   the augmented reality mode family.
    //     - The standard mode family contains modes in which the viewer
    //       displays renders that contain some 2D version of what the
    //       presenter application is currently rendering on screen in
    //       stereoscopic 3D.
    //     - The augmented reality mode family contains modes in which the
    //       viewer displays renders of the presenter's 3D scene from the
    //       perspective of a webcam positioned near the zSpace display that
    //       the presenter is running on.  Augmented reality mode renders are
    //       composited with the webcam video stream to produce images that
    //       look as though the presenter's 3D scene exists in the real world
    //       space around the zSpace display.

    // Create the mode spec and get the mode handle for the standard mode
    // family mode supported by this sample.  In general, a presenter
    // application could support more than one mode in the standard mode
    // family, but this sample only supports one in order to simplify the code.
    {
        // Create a new, default initialized mode spec.
        ZVModeSpec modeSpec = NULL;
        error = zvCreateModeSpec(g_zViewContext, &modeSpec);
        CHECK_ZV_ERROR(error);

        // Set the attributes of the mode spec to the values for the standard
        // mode family mode that this sample supports.

        // Use version 0 of the mode.
        error = zvSetModeSpecAttributeU32(
            modeSpec, ZV_MODE_ATTRIBUTE_KEY_VERSION, 0);
        CHECK_ZV_ERROR(error);

        // Use a mode with no compositing.  This implies that the mode is in
        // the standard mode family.
        error = zvSetModeSpecAttributeU32(
            modeSpec,
            ZV_MODE_ATTRIBUTE_KEY_COMPOSITING_MODE,
            ZV_COMPOSITING_MODE_NONE);
        CHECK_ZV_ERROR(error);

        // The camera that the presenter (i.e. this application) will use to
        // generate the images for this mode will move according to head
        // tracking data on the local node side (since this the presenter
        // camera mode, the local node is the presenter node, which is the node
        // associated with this application).
        error = zvSetModeSpecAttributeU32(
            modeSpec,
            ZV_MODE_ATTRIBUTE_KEY_PRESENTER_CAMERA_MODE,
            ZV_CAMERA_MODE_LOCAL_HEAD_TRACKED);
        CHECK_ZV_ERROR(error);

        // The images generated for this mode will contain rows of pixels
        // ordered from the bottom of the image to the top of the image.
        error = zvSetModeSpecAttributeU32(
            modeSpec,
            ZV_MODE_ATTRIBUTE_KEY_IMAGE_ROW_ORDER,
            ZV_IMAGE_ROW_ORDER_BOTTOM_TO_TOP);
        CHECK_ZV_ERROR(error);

        // The color images generated for this mode will contain 4 channel
        // pixels with 8 bits per channel in RGBA order.
        error = zvSetModeSpecAttributeU32(
            modeSpec,
            ZV_MODE_ATTRIBUTE_KEY_COLOR_IMAGE_PIXEL_FORMAT,
            ZV_PIXEL_FORMAT_R8_G8_B8_A8);
        CHECK_ZV_ERROR(error);

        // Get the actual mode handle for the specified mode spec.
        //
        // Note:  This could fail with the ZV_ERROR_UNSUPPORTED_MODE error code
        // if the version of the zView runtime being used does not support a
        // mode corresponding to the specified mode spec.  Applications should
        // detect this case and disable support for any modes that the zView
        // runtime does not support in order to maintain forward/backward
        // compatibility.
        ZVMode mode = NULL;
        error = zvGetModeForSpec(modeSpec, &mode);
        CHECK_ZV_ERROR(error);

        // Store the mode handle for later use.  This will be used to both
        // build the array of supported modes and to detect when a zView
        // connection is currently using this mode.
        g_zViewStandardMode = mode;

        // Destroy the mode spec since it is no longer needed now that the mode
        // handle has been fetched.
        error = zvDestroyModeSpec(modeSpec);
        CHECK_ZV_ERROR(error);
    }

    // Create the mode spec and get the mode handle for the augmented reality
    // mode family mode supported by this sample.  In general, a presenter
    // application could support more than one mode in the augmented reality
    // mode family, but this sample only supports one in order to simplify the
    // code.
    {
        // Create a new, default initialized mode spec.
        ZVModeSpec modeSpec = NULL;
        error = zvCreateModeSpec(g_zViewContext, &modeSpec);
        CHECK_ZV_ERROR(error);

        // Set the attributes of the mode spec to the values for the standard
        // mode family mode that this sample supports.

        // Use version 0 of the mode.
        error = zvSetModeSpecAttributeU32(
            modeSpec, ZV_MODE_ATTRIBUTE_KEY_VERSION, 0);
        CHECK_ZV_ERROR(error);

        // Use a mode that will composite images generated by the presenter
        // with an augmented reality camera.  This implies that the mode is in
        // the augmented reality mode mode family.
        error = zvSetModeSpecAttributeU32(
            modeSpec,
            ZV_MODE_ATTRIBUTE_KEY_COMPOSITING_MODE,
            ZV_COMPOSITING_MODE_AUGMENTED_REALITY_CAMERA);
        CHECK_ZV_ERROR(error);

        // The camera that the presenter (i.e. this application) will use to
        // generate the images for this mode will be moveable by the remote
        // node (since this the presenter camera mode, the remote node is the
        // viewer node).
        error = zvSetModeSpecAttributeU32(
            modeSpec,
            ZV_MODE_ATTRIBUTE_KEY_PRESENTER_CAMERA_MODE,
            ZV_CAMERA_MODE_REMOTE_MOVABLE);
        CHECK_ZV_ERROR(error);

        // The images generated for this mode will contain rows of pixels
        // ordered from the bottom of the image to the top of the image.
        error = zvSetModeSpecAttributeU32(
            modeSpec,
            ZV_MODE_ATTRIBUTE_KEY_IMAGE_ROW_ORDER,
            ZV_IMAGE_ROW_ORDER_BOTTOM_TO_TOP);
        CHECK_ZV_ERROR(error);

        // The color images generated for this mode will contain 4 channel
        // pixels with 8 bits per channel in RGBA order.
        error = zvSetModeSpecAttributeU32(
            modeSpec,
            ZV_MODE_ATTRIBUTE_KEY_COLOR_IMAGE_PIXEL_FORMAT,
            ZV_PIXEL_FORMAT_R8_G8_B8_A8);
        CHECK_ZV_ERROR(error);

        // Get the actual mode handle for the specified mode spec.
        //
        // Note:  This could fail with the ZV_ERROR_UNSUPPORTED_MODE error code
        // if the version of the zView runtime being used does not support a
        // mode corresponding to the specified mode spec.  Applications should
        // detect this case and disable support for any modes that the zView
        // runtime does not support in order to maintain forward/backward
        // compatibility.
        ZVMode mode = NULL;
        error = zvGetModeForSpec(modeSpec, &mode);
        CHECK_ZV_ERROR(error);

        // Store the mode handle for later use.  This will be used to both
        // build the array of supported modes and to detect when a zView
        // connection is currently using this mode.
        g_zViewAugmentedRealityMode = mode;

        // Destroy the mode spec since it is no longer needed now that the mode
        // handle has been fetched.
        error = zvDestroyModeSpec(modeSpec);
        CHECK_ZV_ERROR(error);
    }

    // Build the array of supported modes.  Each element of the array specifies
    // the mode handle of a mode that is supported by this zView node along
    // with availability information indicates whether this node considers the
    // mode to be available or not.  Since this sample has no special
    // requirements for modes being available, all supported modes are marked
    // as being available (this will generally be the case for all modes
    // supported by any presenter node).
    const ZVSupportedMode supportedModes[] = {
        { g_zViewStandardMode, ZV_MODE_AVAILABILITY_AVAILABLE },
        { g_zViewAugmentedRealityMode, ZV_MODE_AVAILABILITY_AVAILABLE },
    };

    // Set the supported modes for this zView node.
    error = zvSetSupportedModes(g_zViewContext, supportedModes, 2);
    CHECK_ZV_ERROR(error);

    // Set the supported capabilities for this zView node.  This sample does
    // not support any special capabilities, so no supported capabilities are
    // specified.
    error = zvSetSupportedCapabilities(g_zViewContext, NULL, 0);
    CHECK_ZV_ERROR(error);

    // Start listening for zView connections.  After this point, viewer nodes
    // will be able to initiate zView connections to this node.
    error = zvStartListeningForConnections(g_zViewContext, "");
    CHECK_ZV_ERROR(error);

    // Preemptively load the shaders that are needed for rendering images for
    // augmented reality mode.
    if (!loadZviewAugmentedRealityModeShaders())
    {
        return false;
    }

    return true;
}


bool loadZviewAugmentedRealityModeShaders()
{
    // Mask shader program.

    {
        std::string maskVertexShaderSource;
        if (!loadShaderSource(
            ZVIEW_AUGMENTED_REALITY_MODE_MASK_VERTEX_SHADER_NAME,
            maskVertexShaderSource))
        {
            return false;
        }

        std::string maskFragmentShaderSource;
        if (!loadShaderSource(
            ZVIEW_AUGMENTED_REALITY_MODE_MASK_FRAGMENT_SHADER_NAME,
            maskFragmentShaderSource))
        {
            return false;
        }

        if (!compileAndLinkShaderProgram(
            maskVertexShaderSource,
            maskFragmentShaderSource,
            g_zViewAugmentedRealityModeMaskVertexShader,
            g_zViewAugmentedRealityModeMaskFragmentShader,
            g_zViewAugmentedRealityModeMaskShaderProgram))
        {
            return false;
        }

        glUseProgram(g_zViewAugmentedRealityModeMaskShaderProgram);
        if (glGetError() != GL_NO_ERROR)
        {
            return false;
        }

        g_zViewAugmentedRealityModeMaskTransformMatrixUniform =
            glGetUniformLocation(
                g_zViewAugmentedRealityModeMaskShaderProgram,
                "transformMatrix");
    }

    // Background shader program.

    {
        std::string backgroundVertexShaderSource;
        if (!loadShaderSource(
            ZVIEW_AUGMENTED_REALITY_MODE_BACKGROUND_VERTEX_SHADER_NAME,
            backgroundVertexShaderSource))
        {
            return false;
        }

        std::string backgroundFragmentShaderSource;
        if (!loadShaderSource(
            ZVIEW_AUGMENTED_REALITY_MODE_BACKGROUND_FRAGMENT_SHADER_NAME,
            backgroundFragmentShaderSource))
        {
            return false;
        }

        if (!compileAndLinkShaderProgram(
            backgroundVertexShaderSource,
            backgroundFragmentShaderSource,
            g_zViewAugmentedRealityModeBackgroundVertexShader,
            g_zViewAugmentedRealityModeBackgroundFragmentShader,
            g_zViewAugmentedRealityModeBackgroundShaderProgram))
        {
            return false;
        }

        glUseProgram(g_zViewAugmentedRealityModeBackgroundShaderProgram);
        if (glGetError() != GL_NO_ERROR)
        {
            return false;
        }

        g_zViewAugmentedRealityModeBackgroundColorUniform =
            glGetUniformLocation(
                g_zViewAugmentedRealityModeBackgroundShaderProgram,
                "color");
    }

    return true;
}


bool updateZview()
{
    // Only perform zView update operations if a zView context exists.
    if (g_zViewContext == NULL)
    {
        return true;
    }

    ZVError error = ZV_ERROR_OK;

    // Update the zView connection list snapshot to contain the latest list of
    // connections.
    error = zvUpdateConnectionList(g_zViewContext);
    CHECK_ZV_ERROR(error);

    // Loop over the connections in the zView connection list snapshot and
    // update each one.

    ZSInt32 numConnections = 0;
    error = zvGetNumConnections(g_zViewContext, &numConnections);
    CHECK_ZV_ERROR(error);

    for (int i = 0; i < numConnections; ++i)
    {
        ZVConnection curConnection = NULL;
        error = zvGetConnection(g_zViewContext, i, &curConnection);
        CHECK_ZV_ERROR(error);

        if (!updateZviewConnection(curConnection))
        {
            return false;
        }
    }

    return true;
}


bool updateZviewConnection(ZVConnection connection)
{
    ZVError error = ZV_ERROR_OK;

    // Update the snapshot of the connection's internal state to reflect the
    // latest changes to this connection.
    error = zvUpdateConnection(connection);
    CHECK_ZV_ERROR(error);

    // Get the connection's current state and then perform state-specific
    // update operations.

    ZVConnectionState connectionState = ZV_CONNECTION_STATE_ERROR;
    error = zvGetConnectionState(connection, &connectionState);
    CHECK_ZV_ERROR(error);

    switch (connectionState)
    {
    case ZV_CONNECTION_STATE_CONNECTION_INITIALIZATION:
        // The connection is still initializing so there is no work to be done.
        break;

    case ZV_CONNECTION_STATE_AWAITING_CONNECTION_ACCEPTANCE:
        // The connection is waiting to be accepted or rejected.  If there is
        // currently no active connection, accept this connection and make it
        // the active connection.  If there is already an active connection,
        // then reject this connection.
        if (!processNewZviewConnection(connection))
        {
            return false;
        }

        break;

    case ZV_CONNECTION_STATE_SWITCHING_MODES:
        // The connection is in the process of switching modes internally so
        // there is no work to be done.
        break;

    case ZV_CONNECTION_STATE_NO_MODE:
        // The connection is fully established, but there is no zView mode
        // currently active.  Tear down the state related to the previously
        // active zView mode, if there was one.
        if (!tearDownZviewMode())
        {
            return false;
        }

        break;

    case ZV_CONNECTION_STATE_MODE_SETUP:
        // Perform operations related to setting up the zView mode that is
        // currently becoming active for the connection.
        if (!setUpZviewMode(connection))
        {
            return false;
        }
        break;

    case ZV_CONNECTION_STATE_MODE_ACTIVE:
        // The connection's current mode is active (i.e. frames are being sent
        // between the presenter node and the viewer node).  This sample does
        // no work during the update part of the main loop when in this state.
        // Instead, all work is done during the draw part of the main loop (see
        // the drawZview() function for details).
        break;

    case ZV_CONNECTION_STATE_MODE_PAUSED:
        // The connection's current mode is paused so there is no work to be
        // done.
        break;

    case ZV_CONNECTION_STATE_MODE_RESUMING:
        // The connection is in the process of resuming the current mode
        // internally so there is no work to be done.
        break;

    case ZV_CONNECTION_STATE_PROCESSING_MODE_SETTINGS_CHANGE:
        // The connection is internally processing a change to a mode-specific
        // setting so there is no work to be done.
        break;

    case ZV_CONNECTION_STATE_CLOSED:
        // The connection has been closed.

        // If the connection is the current active connection, clear the
        // current active connection, tear down any state related to the latest
        // active mode, and set the node status to indicate that this zView
        // node is no longer connected to another node.
        if (connection == g_zViewActiveConnection)
        {
            g_zViewActiveConnection = NULL;

            if (!tearDownZviewMode())
            {
                return false;
            }

            error = zvSetNodeStatus(
                g_zViewContext, ZVIEW_NODE_STATUS_NOT_CONNECTED);
            CHECK_ZV_ERROR(error);
        }

        // Destroy the connection since it is closed and nothing else can be
        // done with it.  This cleans up internal resources associated with the
        // connection.
        error = zvDestroyConnection(connection);
        CHECK_ZV_ERROR(error);

        break;

    case ZV_CONNECTION_STATE_ERROR:
        // An error has occurred that has made the connection unusable.  The
        // connection is effectively closed and should be destroyed.

        // If the connection is the current active connection, clear the
        // current active connection, tear down any state related to the latest
        // active mode, and set the node status to indicate that this zView
        // node is no longer connected to another node.
        if (connection == g_zViewActiveConnection)
        {
            g_zViewActiveConnection = NULL;

            if (!tearDownZviewMode())
            {
                return false;
            }

            error = zvSetNodeStatus(
                g_zViewContext, ZVIEW_NODE_STATUS_NOT_CONNECTED);
            CHECK_ZV_ERROR(error);
        }

        // Destroy the connection since nothing else can be done with it.  This
        // cleans up internal resources associated with the connection.
        error = zvDestroyConnection(connection);
        CHECK_ZV_ERROR(error);

        break;
    }

    // Perform video-recording-related related operations for the connection. 
    if (!processZviewVideoRecording(connection))
    {
        return false;
    }

    return true;
}


bool processNewZviewConnection(ZVConnection connection)
{
    ZVError error = ZV_ERROR_OK;

    // If the specified connection is already the active connection, do nothing.
    if (connection == g_zViewActiveConnection)
    {
        return true;
    }

    // If there is no active zView connection, then accept the specified
    // connection and make it the active connection.
    if (g_zViewActiveConnection == NULL)
    {
        error = zvAcceptConnection(connection);
        CHECK_ZV_ERROR(error);

        g_zViewActiveConnection = connection;

        // Set this zView node's status to indicate that it is now connected to
        // another node.
        error = zvSetNodeStatus(g_zViewContext, ZVIEW_NODE_STATUS_CONNECTED);
        CHECK_ZV_ERROR(error);
    }
    // If there is already an active zView connection, then close the specified
    // connection, indicating that the connection was rejected using the close
    // reason.
    //
    // Note:  Presenter application may include support for having more than
    // one active zView connection at the same time.  This sample only supports
    // having one active zView connection at a time in order to simplify the
    // code.
    else
    {
        error = zvCloseConnection(
            connection,
            ZV_CONNECTION_CLOSE_ACTION_NONE,
            ZV_CONNECTION_CLOSE_REASON_CONNECTION_REJECTED,
            "Maximum number of active zView connections exceeded");
        CHECK_ZV_ERROR(error);
    }

    return true;
}


bool processZviewVideoRecording(ZVConnection connection)
{
    ZVError error = ZV_ERROR_OK;

    // Get the current video recording state for the specified connection and
    // then perform operations based on that state.

    ZVVideoRecordingState videoRecordingState = ZV_VIDEO_RECORDING_STATE_ERROR;
    error = zvGetVideoRecordingState(connection, &videoRecordingState);
    CHECK_ZV_ERROR(error);

    // If the video recording state has changed since the last known video
    // recording state, log a message indicating that a state transition has
    // occurred.
    if (videoRecordingState != g_zViewVideoRecordingLatestState)
    {
        printLog(
            "zView Video Recording State Transition:  " +
                convertToString(g_zViewVideoRecordingLatestState) + " => " +
                convertToString(videoRecordingState));
    }

    // If video recording is currently in the error state, log a message with
    // the error code and clear the error so that a new video recording can be
    // started, if desired.
    if (videoRecordingState == ZV_VIDEO_RECORDING_STATE_ERROR)
    {
        ZVError videoRecordingError = ZV_ERROR_OK;
        error = zvGetVideoRecordingError(connection, &videoRecordingError);
        CHECK_ZV_ERROR(error);

        printLog(
            "Video recording error occurred (error code = " +
                convertToString(videoRecordingError) + ")");

        error = zvClearVideoRecordingError(connection);
        CHECK_ZV_ERROR(error);
    }

    // Note:  Presenter nodes are not required to do any work for video
    // recording states other than the error state.  However, if a presenter
    // application contains a video recording GUI, it is recommended that the
    // application handle other video recording states in order to update its
    // GUI to reflect the video recording operations that are available in each
    // state.  For example, an application might enable a record buttom while
    // video recording is in the ZV_VIDEO_RECORDING_STATE_NOT_RECORDING state
    // and disable it in other states to indicate to the user that video
    // recordings can only be started when there is no active recording.

    // Remember the current video recording state as the last known state.
    g_zViewVideoRecordingLatestState = videoRecordingState;

    return true;
}


bool processZviewVideoRecordingKeyPress(WPARAM key)
{
    printLog("Processing video recording key press...");

    ZVError error = ZV_ERROR_OK;

    // Only process video recording key presses if there is an active zView
    // connection.
    if (g_zViewActiveConnection == NULL)
    {
        return true;
    }

    // Only process video recording key presses if video recording is not in
    // the "not available" state (i.e. only if video recording support is
    // available).

    ZVVideoRecordingState videoRecordingState =
        ZV_VIDEO_RECORDING_STATE_NOT_AVAILABLE;
    error = zvGetVideoRecordingState(
        g_zViewActiveConnection, &videoRecordingState);
    CHECK_ZV_ERROR(error);

    printLog(
        "Video recording state = " + convertToString(videoRecordingState));

    if (videoRecordingState == ZV_VIDEO_RECORDING_STATE_NOT_AVAILABLE)
    {
        return true;
    }

    // Verify that the shift key was being held down when the key press
    // occurred since all video recording keyboard shortcuts include the
    // shift key being held down.

    const bool isShiftKeyPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

    if (!isShiftKeyPressed)
    {
        return true;
    }

    switch (key)
    {
    // Switch to a different video recording quality, if no recording is
    // currently in progress.

    case 'Q':
        if (videoRecordingState == ZV_VIDEO_RECORDING_STATE_NOT_RECORDING)
        {
            if (!switchZviewVideoRecordingQuality())
            {
                return false;
            }
        }
        break;

    // Start a new recording if no recording is currently in progress.

    case 'R':
        if (videoRecordingState == ZV_VIDEO_RECORDING_STATE_NOT_RECORDING)
        {
            printLog("Starting video recording...");
            error = zvStartVideoRecording(g_zViewActiveConnection);
            CHECK_ZV_ERROR(error);
        }
        break;

    // Pause or resume the current recording, if there is one.

    case 'P':
        if (videoRecordingState == ZV_VIDEO_RECORDING_STATE_RECORDING)
        {
            printLog("Pausing video recording...");
            error = zvPauseVideoRecording(g_zViewActiveConnection);
            CHECK_ZV_ERROR(error);
        }
        else if (videoRecordingState == ZV_VIDEO_RECORDING_STATE_PAUSED)
        {
            printLog("Resuming video recording...");
            error = zvResumeVideoRecording(g_zViewActiveConnection);
            CHECK_ZV_ERROR(error);
        }
        break;

    // Finish the current recording, if there is one.

    case 'F':
        if ((videoRecordingState == ZV_VIDEO_RECORDING_STATE_RECORDING) ||
            (videoRecordingState == ZV_VIDEO_RECORDING_STATE_PAUSED))
        {
            printLog("Finishing video recording...");
            error = zvFinishVideoRecording(g_zViewActiveConnection);
            CHECK_ZV_ERROR(error);
        }
        break;

    // Save the current recording, if there is one and it is finished.

    case 'S':
        if (videoRecordingState == ZV_VIDEO_RECORDING_STATE_FINISHED)
        {
            printLog("Saving video recording...");

            // Compute the absolute path of the file to save the recording to.

            const DWORD MODULE_FILENAME_BUF_SIZE = 1024;

            char moduleFilename[MODULE_FILENAME_BUF_SIZE];
            const DWORD moduleFilenameSize = GetModuleFileName(
                NULL, moduleFilename, MODULE_FILENAME_BUF_SIZE);

            if (moduleFilenameSize == MODULE_FILENAME_BUF_SIZE)
            {
                printLog("Module filename too large");
                return false;
            }

            char fullModulePathName[MODULE_FILENAME_BUF_SIZE];
            LPSTR fullModulePathNameFilePart = NULL;
            const DWORD modulePathNameSize = GetFullPathName(
                moduleFilename,
                MODULE_FILENAME_BUF_SIZE,
                fullModulePathName,
                &fullModulePathNameFilePart);

            if (modulePathNameSize >= MODULE_FILENAME_BUF_SIZE)
            {
                printLog("Module path name too large");
                return false;
            }

            if (fullModulePathNameFilePart == NULL)
            {
                printLog("Module path name does not contain file part");
                return false;
            }

            std::string fullModuleDirPath(
                fullModulePathName, fullModulePathNameFilePart);

            std::string saveFilePath =
                fullModuleDirPath + ZVIEW_VIDEO_RECORDING_SAVE_NAME;

            // Actually save the recording.

            error = zvSaveVideoRecording(
                g_zViewActiveConnection, saveFilePath.c_str());
            CHECK_ZV_ERROR(error);
        }
        break;

    // Discard the current recording, if there is one and it is finished.

    case 'D':
        if (videoRecordingState == ZV_VIDEO_RECORDING_STATE_FINISHED)
        {
            printLog("Discarding video recording...");
            error = zvDiscardVideoRecording(g_zViewActiveConnection);
            CHECK_ZV_ERROR(error);
        }
        break;
    }

    return true;
}


bool processZviewAugmentedRealityModeOverlayControlKeyPress(WPARAM key)
{
    ZVError error = ZV_ERROR_OK;

    // Only process augmented reality mode overlay control key presses if there
    // is an active zView connection.
    if (g_zViewActiveConnection == NULL)
    {
        return true;
    }

    // Only process augmented reality mode overlay control key presses if there
    // is a mode active for the active zView connection and that mode is an
    // augmented reality mode family mode.

    ZVConnectionState connectionState = ZV_CONNECTION_STATE_ERROR;
    error = zvGetConnectionState(g_zViewActiveConnection, &connectionState);
    CHECK_ZV_ERROR(error);

    if (connectionState != ZV_CONNECTION_STATE_MODE_ACTIVE)
    {
        return true;
    }

    ZVMode mode = NULL;
    error = zvGetConnectionMode(g_zViewActiveConnection, &mode);
    CHECK_ZV_ERROR(error);

    if (mode != g_zViewAugmentedRealityMode)
    {
        return true;
    }

    // Verify that the control key was being held down when the key press
    // occurred since all augmented reality mode overlay control keyboard
    // shortcuts include the control key being held down.

    const bool isControlKeyPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

    if (!isControlKeyPressed)
    {
        return true;
    }

    switch (key)
    {
    // Decrement/increment the augmented reality mode overlay's X offset, in
    // pixels.

    case 'A':
        if (!incrementZviewSettingClampedF32(
            g_zViewActiveConnection,
            ZV_SETTING_KEY_OVERLAY_OFFSET_X,
            -1.0f,
            (float)-g_zViewAugmentedRealityModeImageWidth,
            (float)g_zViewAugmentedRealityModeImageWidth))
        {
            return false;
        }
        break;

    case 'D':
        if (!incrementZviewSettingClampedF32(
            g_zViewActiveConnection,
            ZV_SETTING_KEY_OVERLAY_OFFSET_X,
            1.0f,
            (float)-g_zViewAugmentedRealityModeImageWidth,
            (float)g_zViewAugmentedRealityModeImageWidth))
        {
            return false;
        }
        break;

    // Decrement/increment the augmented reality mode overlay's Y offset, in
    // pixels.

    case 'S':
        if (!incrementZviewSettingClampedF32(
            g_zViewActiveConnection,
            ZV_SETTING_KEY_OVERLAY_OFFSET_Y,
            -1.0f,
            (float)-g_zViewAugmentedRealityModeImageWidth,
            (float)g_zViewAugmentedRealityModeImageWidth))
        {
            return false;
        }
        break;

    case 'W':
        if (!incrementZviewSettingClampedF32(
            g_zViewActiveConnection,
            ZV_SETTING_KEY_OVERLAY_OFFSET_Y,
            1.0f,
            (float)-g_zViewAugmentedRealityModeImageWidth,
            (float)g_zViewAugmentedRealityModeImageWidth))
        {
            return false;
        }
        break;

    // Reset the augmented reality mode overlay's X offset.

    case 'Q':
        error = zvSetSettingF32(
            g_zViewActiveConnection, ZV_SETTING_KEY_OVERLAY_OFFSET_X, 0.0f);
        CHECK_ZV_ERROR(error);
        break;

    // Reset the augmented reality mode overlay's Y offset.

    case 'E':
        error = zvSetSettingF32(
            g_zViewActiveConnection, ZV_SETTING_KEY_OVERLAY_OFFSET_Y, 0.0f);
        CHECK_ZV_ERROR(error);
        break;

    // Decrement/increment the augmented reality mode overlay's X scale factor.

    case 'F':
        if (!incrementZviewSettingClampedF32(
            g_zViewActiveConnection,
            ZV_SETTING_KEY_OVERLAY_SCALE_X,
            -0.01f,
            0.01f,
            10.0f))
        {
            return false;
        }
        break;

    case 'H':
        if (!incrementZviewSettingClampedF32(
            g_zViewActiveConnection,
            ZV_SETTING_KEY_OVERLAY_SCALE_X,
            0.01f,
            0.01f,
            10.0f))
        {
            return false;
        }
        break;

    // Decrement/increment the augmented reality mode overlay's Y scale factor.

    case 'G':
        if (!incrementZviewSettingClampedF32(
            g_zViewActiveConnection,
            ZV_SETTING_KEY_OVERLAY_SCALE_Y,
            -0.01f,
            0.01f,
            10.0f))
        {
            return false;
        }
        break;

    case 'T':
        if (!incrementZviewSettingClampedF32(
            g_zViewActiveConnection,
            ZV_SETTING_KEY_OVERLAY_SCALE_Y,
            0.01f,
            0.01f,
            10.0f))
        {
            return false;
        }
        break;

    // Reset the augmented reality mode overlay's X scale factor.

    case 'R':
        error = zvSetSettingF32(
            g_zViewActiveConnection, ZV_SETTING_KEY_OVERLAY_SCALE_X, 1.0f);
        CHECK_ZV_ERROR(error);
        break;

    // Reset the augmented reality mode overlay's Y scale factor.

    case 'Y':
        error = zvSetSettingF32(
            g_zViewActiveConnection, ZV_SETTING_KEY_OVERLAY_SCALE_Y, 1.0f);
        CHECK_ZV_ERROR(error);
        break;
    }

    return true;
}


bool switchZviewVideoRecordingQuality()
{
    ZVError error = ZV_ERROR_OK;

    // Do nothing if there is no active zView connection.

    if (g_zViewActiveConnection == NULL)
    {
        return true;
    }

    // Do nothing if the active zView connection does not support video
    // recording.

    ZSBool isVideoRecordingCapabilitySupported = 0;
    error = zvDoesConnectionSupportCapability(
        g_zViewActiveConnection,
        ZV_CAPABILITY_VIDEO_RECORDING,
        &isVideoRecordingCapabilitySupported);
    CHECK_ZV_ERROR(error);

    if (!isVideoRecordingCapabilitySupported)
    {
        return true;
    }

    // Get the current video recording quality.
    ZSUInt32 curVideoRecordingQualityU32 = ZV_VIDEO_RECORDING_QUALITY_480P;
    error = zvGetSettingU32(
        g_zViewActiveConnection,
        ZV_SETTING_KEY_VIDEO_RECORDING_QUALITY,
        &curVideoRecordingQualityU32);
    CHECK_ZV_ERROR(error);

    // Switch to the next higher video recording quality or, if the current
    // video recording quality is the highest, switch to the lowest video
    // recording quality.  This cycles through the available video recording
    // qualities as the user requests video recording quality changes.

    ZVVideoRecordingQuality newVideoRecordingQuality =
        ZV_VIDEO_RECORDING_QUALITY_480P;

    switch (curVideoRecordingQualityU32)
    {
    case ZV_VIDEO_RECORDING_QUALITY_480P:
        printLog("Switching video recording quality from 480p to 720p...");
        newVideoRecordingQuality = ZV_VIDEO_RECORDING_QUALITY_720P;
        break;

    case ZV_VIDEO_RECORDING_QUALITY_720P:
        printLog("Switching video recording quality from 720p to 1080p...");
        newVideoRecordingQuality = ZV_VIDEO_RECORDING_QUALITY_1080P;
        break;

    case ZV_VIDEO_RECORDING_QUALITY_1080P:
        printLog("Switching video recording quality from 1080p to 480p...");
        newVideoRecordingQuality = ZV_VIDEO_RECORDING_QUALITY_480P;
        break;

    default:
        printLog(
            "Current video recording quality unknown; leaving video recording "
                "quality unchanged");
        return true;
    }

    error = zvSetSettingU32(
        g_zViewActiveConnection,
        ZV_SETTING_KEY_VIDEO_RECORDING_QUALITY,
        newVideoRecordingQuality);
    CHECK_ZV_ERROR(error);

    return true;
}


bool incrementZviewSettingClampedF32(
    ZVConnection connection,
    ZVSettingKey settingKey,
    ZSFloat increment,
    ZSFloat min,
    ZSFloat max)
{
    ZVError error = ZV_ERROR_OK;

    ZSFloat settingValue = 0.0f;
    error = zvGetSettingF32(connection, settingKey, &settingValue);
    CHECK_ZV_ERROR(error);

    settingValue += increment;

    if (settingValue < min)
    {
        settingValue = min;
    }
    else if (settingValue > max)
    {
        settingValue = max;
    }

    error = zvSetSettingF32(connection, settingKey, settingValue);
    CHECK_ZV_ERROR(error);

    return true;
}


bool handleZviewStandardModeImageResolutionChange()
{
    ZVError error = ZV_ERROR_OK;

    // Do nothing if there is no active zView connection or if the current
    // standard mode image resolution matches the current application viewport
    // resolution.

    if (g_zViewActiveConnection == NULL)
    {
        return true;
    }

    if ((g_zViewStandardModeImageWidth == g_windowWidth) &&
        (g_zViewStandardModeImageHeight == g_windowHeight))
    {
        return true;
    }

    // If the application viewport resolution has changed, set the standard
    // mode image resolution to match the new application viewport resolution.

    // Note:  If the active zView connection is currently in the
    // ZV_CONNECTION_STATE_MODE_ACTIVE state, then setting the image resolution
    // settings here will automatically trigger a transition to the
    // ZV_CONNECTION_STATE_MODE_SETUP state in the
    // ZV_MODE_SETUP_PHASE_COMPLETION mode setup phase.  This allows both the
    // presenter node and the viewer node to take into account the new image
    // resolution.

    error = zvBeginSettingsBatch(g_zViewActiveConnection);
    CHECK_ZV_ERROR(error);

    error = zvSetSettingU16(
        g_zViewActiveConnection,
        ZV_SETTING_KEY_IMAGE_WIDTH,
        g_windowWidth);
    CHECK_ZV_ERROR(error);

    error = zvSetSettingU16(
        g_zViewActiveConnection,
        ZV_SETTING_KEY_IMAGE_HEIGHT,
        g_windowHeight);
    CHECK_ZV_ERROR(error);

    error = zvEndSettingsBatch(g_zViewActiveConnection);
    CHECK_ZV_ERROR(error);

    g_zViewStandardModeImageWidth = g_windowWidth;
    g_zViewStandardModeImageHeight = g_windowHeight;

    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
bool updateZviewAugmentedRealityMode()
{
    ZCError zcError = ZC_ERROR_OK;

    // Update world space augmented reality mode camera pose based on latest
    // stereo viewport and main camera transforms.

    ZSMatrix4 cameraPose = g_zViewAugmentedRealityModeCameraPoseDisplaySpace;
    zcTransformMatrix(
        g_viewportHandle,
        ZC_COORDINATE_SPACE_DISPLAY,
        ZC_COORDINATE_SPACE_CAMERA,
        &cameraPose);

    glm::mat4 cameraPoseCameraSpace = glm::make_mat4(cameraPose.f);

    g_zViewAugmentedRealityModeCameraWorldSpaceViewTransform =
        glm::inverse(g_invCameraTransform * cameraPoseCameraSpace);

    // Compute the augmented reality mode mask transform and geometry.  The
    // augmented reality mode mask is intended to clip scene elements that are
    // behind the plane of the zSpace display and outside of the bounds of the
    // presenter application's viewport.  The mask geomtry is a cube with a
    // rectangular hole the size of the presenter application's viewport cut in
    // one face and the opposite face removed (see the comments below for a
    // diagram).  This geometry is positioned such that the viewport sized hole
    // is exactly where the presenter application viewport is located within
    // the presenter application's 3D scene.

    // Compute the viewport size and center position in the display coordinate
    // space.  The viewport size is used for creating the hole within one of
    // the faces of the mask geometry cube.  The center position is used to
    // compute the transform necessary to properly position the mask geometry.

    // Compute the coordinates of the presenter application's viewport in the
    // Windows virtual desktop coordinate space.
    const glm::vec2 viewportCenterVirtualDesktop(
        g_windowX + (g_windowWidth * 0.5f),
        g_windowY + (g_windowHeight * 0.5f));

    // Get the zSpace Core API display handle for the display containing the
    // presenter application's viewport.
    ZCHandle curDisplay = NULL;
    zcError = zcGetDisplay(
        g_zSpaceContext,
        (ZSInt32)viewportCenterVirtualDesktop.x,
        (ZSInt32)viewportCenterVirtualDesktop.y,
        &curDisplay);
    CHECK_ZC_ERROR(zcError);

    // Get the display's size in meters.
    glm::vec2 curDisplaySize;
    zcError = zcGetDisplaySize(
        curDisplay, &(curDisplaySize.x), &(curDisplaySize.y));
    CHECK_ZC_ERROR(zcError);

    // Get the position of the display in the Windows virtual desktop
    // coordinate space.

    ZSInt32 curDisplayPositionVirtualDesktopInt32[2] = { 0 };
    zcError = zcGetDisplayPosition(
        curDisplay,
        &(curDisplayPositionVirtualDesktopInt32[0]),
        &(curDisplayPositionVirtualDesktopInt32[1]));
    CHECK_ZC_ERROR(zcError);

    const glm::vec2 curDisplayPositionVirtualDesktop(
        curDisplayPositionVirtualDesktopInt32[0],
        curDisplayPositionVirtualDesktopInt32[1]);

    // Get the display's native resolution.

    ZSInt32 curDisplayResolutionInt32[2] = { 0 };
    zcError = zcGetDisplayNativeResolution(
        curDisplay,
        &(curDisplayResolutionInt32[0]),
        &(curDisplayResolutionInt32[1]));
    CHECK_ZC_ERROR(zcError);

    const glm::vec2 curDisplayResolution(
        curDisplayResolutionInt32[0], curDisplayResolutionInt32[1]);

    // Compute the position of the center of the display in the Windows virtual
    // desktop coordinate space.
    const glm::vec2 curDisplayCenterVirtualDesktop =
        curDisplayPositionVirtualDesktop + (curDisplayResolution * 0.5f);

    // Compute the conversion factor from the display's pixels to meters.
    const glm::vec2 curDisplayMetersPerPixel =
        curDisplaySize / curDisplayResolution;

    // Compute the presenter application's viewport size in meters.  This is
    // the size in the display coordinate space because in this coordinate
    // space one unit equals one meter.
    const glm::vec2 viewportSizeMeters(
        g_windowWidth * curDisplayMetersPerPixel.x,
        g_windowHeight * curDisplayMetersPerPixel.y);

    // Compute the position of the center of the presenter application's
    // viewport in the display coordinate space.
    const glm::vec3 viewportCenterDisplaySpace(
        (viewportCenterVirtualDesktop.x - curDisplayCenterVirtualDesktop.x) *
            curDisplayMetersPerPixel.x,
        (curDisplayCenterVirtualDesktop.y - viewportCenterVirtualDesktop.y) *
            curDisplayMetersPerPixel.y,
        0.0f);

    // Compute the augmented reality mode mask transform matrix.  This matrix
    // contains a transform from the non-portal-mode viewport coordinate space
    // (which is the coordinate space in which the mask geometry is defined;
    // see below) to the normalized device coordinate space.  To compute this,
    // the following transforms are combined:
    //     - Non-portal-mode viewport coordinate space to display coordinate
    //       space
    //     - Display coordinate space to camera coordinate space
    //     - Inverse camera transform going from camera coordinate space to
    //       world coordinate space
    //     - Augmented reality mode webcam view transform going from world
    //       coordinate space to the augmented reality mode webcam's local
    //       coordinate space
    //     - Augmented reality mode webcam projection matrix going from the
    //       augmented reality mode webcam's local coordinate space to
    //       normalized device coordinate space

    const glm::mat4 nonPortalModeViewportToDisplay = glm::translate(
        glm::mat4(), viewportCenterDisplaySpace);

    ZSMatrix4 displayToCameraZs = { 0 };
    zcError = zcGetCoordinateSpaceTransform(
        g_viewportHandle,
        ZC_COORDINATE_SPACE_DISPLAY,
        ZC_COORDINATE_SPACE_CAMERA,
        &displayToCameraZs);
    CHECK_ZC_ERROR(zcError);

    const glm::mat4 displayToCamera = glm::make_mat4(displayToCameraZs.f);

    g_zViewAugmentedRealityModeMaskTransform =
        // Augmented reality mode webcam projection matrix going from the
        // augmented reality mode webcam's local coordinate space to normalized
        // device coordinate space.
        g_zViewAugmentedRealityModeCameraProjection *
            // Augmented reality mode webcam view transform going from world
            // coordinate space to the augmented reality mode webcam's local
            // coordinate space.
            g_zViewAugmentedRealityModeCameraWorldSpaceViewTransform *
            // Inverse camera transform going from camera coordinate space to
            // world coordinate space.
            g_invCameraTransform *
            // Display coordinate space to camera coordinate space.
            displayToCamera *
            // Non-portal-mode viewport coordinate space to display coordinate
            // space.
            nonPortalModeViewportToDisplay;

    // Compute the augmented reality mode mask geometry.  The mask geomtry is a
    // cube with a rectangular hole the size of the presenter application's
    // viewport cut in one face and the opposite face removed.  It looks
    // roughly like what is shown in the following diagrams:
    //
    // Isometric view:
    //
    //                            (t)
    //                   -------------------
    //                  /\                  /\
    //                 /  \                /  \
    //                /    \              /    \ (r)
    //           (l) /      \            /      \
    //              /        \          /        \
    //             /          \ (b)    /          \
    //            ---------------------............
    //             \          /    /   \  (tr)    /
    //              \        /(tl)......\......../
    //               \      /    /  (v)  \/     /
    //                \    /    /        /\(br)/
    //                 \  /..............  \  /
    //                  \/    (bl)     /    \/
    //                   --------------------
    //
    // View looking straight at the cube face with the viewport hole in it
    // through the cube face that has been removed:
    //
    //             --------------------------------
    //            |\                              /|
    //            | \                            / |
    //            |  \           (t)            /  |
    //            |   \                        /   |
    //            |    \                      /    |
    //            |     \                    /     |
    //            |      --------------------      |
    //            |      |    |    (tr)     |      |
    //            |      |(tl)--------------|      |
    //            | (l)  |    |  (v)  |     |  (r) |
    //            |      |    |       | (br)|      |
    //            |      |-------------     |      |
    //            |      |    (bl)    |     |      |
    //            |      --------------------      |
    //            |     /                    \     |
    //            |    /                      \    |
    //            |   /                        \   |
    //            |  /           (b)            \  |
    //            | /                            \ |
    //            |/                              \|
    //             --------------------------------
    //
    // Diagram key:
    //
    // - (v):  The hole that is size of the presenter application's viewport.
    // - (tl):  Quad making up the top left portion of the cube face with the
    //          hole in it.
    // - (tr):  Quad making up the top right portion of the cube face with the
    //          hole in it.
    // - (bl):  Quad making up the bottom left portion of the cube face with
    //          the hole in it.
    // - (br):  Quad making up the bottom right portion of the cube face with
    //          the hole in it.
    // - (t):  Quad making up the top face of the cube.
    // - (b):  Quad making up the bottom face of the cube.
    // - (l):  Quad making up the left face of the cube.
    // - (r):  Quad making up the right face of the cube.
    //
    // Note that (v), (tl), (tr), (bl), and (br) all lie in the screen plane of
    // the zSpace display that the presenter application is running on.

    // Compute sizes and half sizes of the presenter application viewport and
    // the mask geometry cube faces.  These are used to compute the mask
    // geometry vertex positions.

    const glm::vec2 viewportHalfSizeMeters = viewportSizeMeters * 0.5f;

    const glm::vec2 maskCubeFaceSizeMeters(
        ZVIEW_AUGMENTED_REALITY_MODE_MASK_CUBE_SIDE_LENGTH_METERS);

    const glm::vec2 maskCubeFaceHalfSizeMeters = maskCubeFaceSizeMeters * 0.5f;

    // Compute the positions of all vertices that will be used to form the mask geometry.
    // 
    // Note:  The following positions are all in the non-portal-mode viewport
    // coordinate space.

    // Vertices of the corners of the presenter application's viewport.
    const glm::vec3 viewportCornerTL(
        -viewportHalfSizeMeters.x, viewportHalfSizeMeters.y, 0.0f);
    const glm::vec3 viewportCornerTR(
        viewportHalfSizeMeters.x, viewportHalfSizeMeters.y, 0.0f);
    const glm::vec3 viewportCornerBL(
        -viewportHalfSizeMeters.x, -viewportHalfSizeMeters.y, 0.0f);
    const glm::vec3 viewportCornerBR(
        viewportHalfSizeMeters.x, -viewportHalfSizeMeters.y, 0.0f);

    // Vertices that split the sides of the mask cube face that lies in the
    // screen plane.  These are used to form the four quads that surround the
    // viewport hole.
    const glm::vec3 screenPlaneMaskCubeFaceSplitT(
        -viewportHalfSizeMeters.x, maskCubeFaceHalfSizeMeters.y, 0.0f);
    const glm::vec3 screenPlaneMaskCubeFaceSplitB(
        viewportHalfSizeMeters.x, -maskCubeFaceHalfSizeMeters.y, 0.0f);
    const glm::vec3 screenPlaneMaskCubeFaceSplitL(
        -maskCubeFaceHalfSizeMeters.x, -viewportHalfSizeMeters.y, 0.0f);
    const glm::vec3 screenPlaneMaskCubeFaceSplitR(
        maskCubeFaceHalfSizeMeters.x, viewportHalfSizeMeters.y, 0.0f);

    // Vertices of the corners of the mask cube face that lies in the screen
    // plane.
    const glm::vec3 screenPlaneMaskCubeFaceTL(
        -maskCubeFaceHalfSizeMeters.x, maskCubeFaceHalfSizeMeters.y, 0.0f);
    const glm::vec3 screenPlaneMaskCubeFaceTR(
        maskCubeFaceHalfSizeMeters.x, maskCubeFaceHalfSizeMeters.y, 0.0f);
    const glm::vec3 screenPlaneMaskCubeFaceBL(
        -maskCubeFaceHalfSizeMeters.x, -maskCubeFaceHalfSizeMeters.y, 0.0f);
    const glm::vec3 screenPlaneMaskCubeFaceBR(
        maskCubeFaceHalfSizeMeters.x, -maskCubeFaceHalfSizeMeters.y, 0.0f);

    // Vertices of the corners of the mask cube face that is removed.
    const glm::vec3 backMaskCubeFaceTL(
        -maskCubeFaceHalfSizeMeters.x,
        maskCubeFaceHalfSizeMeters.y,
        ZVIEW_AUGMENTED_REALITY_MODE_MASK_CUBE_SIDE_LENGTH_METERS);
    const glm::vec3 backMaskCubeFaceTR(
        maskCubeFaceHalfSizeMeters.x,
        maskCubeFaceHalfSizeMeters.y,
        ZVIEW_AUGMENTED_REALITY_MODE_MASK_CUBE_SIDE_LENGTH_METERS);
    const glm::vec3 backMaskCubeFaceBL(
        -maskCubeFaceHalfSizeMeters.x,
        -maskCubeFaceHalfSizeMeters.y,
        ZVIEW_AUGMENTED_REALITY_MODE_MASK_CUBE_SIDE_LENGTH_METERS);
    const glm::vec3 backMaskCubeFaceBR(
        maskCubeFaceHalfSizeMeters.x,
        -maskCubeFaceHalfSizeMeters.y,
        ZVIEW_AUGMENTED_REALITY_MODE_MASK_CUBE_SIDE_LENGTH_METERS);

    // Build an array of the vertex positions for all of the quads that make up
    // the mask geometry.

    const glm::vec3
        maskGeometryVerts[ZVIEW_AUGMENTED_REALITY_MODE_NUM_MASK_VERTS] = {
            // Screen-plane mask cube face top-left quad.
            screenPlaneMaskCubeFaceTL,
            screenPlaneMaskCubeFaceSplitL,
            viewportCornerBL,
            screenPlaneMaskCubeFaceSplitT,

            // Screen-plane mask cube face top-right quad.
            screenPlaneMaskCubeFaceSplitT,
            viewportCornerTL,
            screenPlaneMaskCubeFaceSplitR,
            screenPlaneMaskCubeFaceTR,

            // Screen-plane mask cube face bottom-left quad.
            screenPlaneMaskCubeFaceSplitL,
            screenPlaneMaskCubeFaceBL,
            screenPlaneMaskCubeFaceSplitB,
            viewportCornerBR,

            // Screen-plane mask cube face bottom-right quad.
            viewportCornerTR,
            screenPlaneMaskCubeFaceSplitB,
            screenPlaneMaskCubeFaceBR,
            screenPlaneMaskCubeFaceSplitR,

            // Top cube face quad.
            backMaskCubeFaceTL,
            screenPlaneMaskCubeFaceTL,
            screenPlaneMaskCubeFaceTR,
            backMaskCubeFaceTR,

            // Bottom cube face quad.
            backMaskCubeFaceBR,
            screenPlaneMaskCubeFaceBR,
            screenPlaneMaskCubeFaceBL,
            backMaskCubeFaceBL,

            // Left cube face quad.
            backMaskCubeFaceBL,
            screenPlaneMaskCubeFaceBL,
            screenPlaneMaskCubeFaceTL,
            backMaskCubeFaceTL,

            // Right cube face quad.
            backMaskCubeFaceTR,
            screenPlaneMaskCubeFaceTR,
            screenPlaneMaskCubeFaceBR,
            backMaskCubeFaceBR,
        };

    // Flatten the array of quad vertex positions so that it is in a form that
    // can be passed to OpenGL.
    float maskGeometryVertsFlat[ZVIEW_AUGMENTED_REALITY_MODE_NUM_MASK_VERTS * 3];
    for (int i = 0; i < ZVIEW_AUGMENTED_REALITY_MODE_NUM_MASK_VERTS; ++i)
    {
        const int curFlatArrayStartIndex = i * 3;

        const glm::vec3& curVert = maskGeometryVerts[i];

        maskGeometryVertsFlat[curFlatArrayStartIndex] = curVert.x;
        maskGeometryVertsFlat[curFlatArrayStartIndex + 1] = curVert.y;
        maskGeometryVertsFlat[curFlatArrayStartIndex + 2] = curVert.z;
    }

    // Copy the mask vertex position data into the appropriate OpenGL buffer
    // object.

    glBindBuffer(
        GL_ARRAY_BUFFER, g_zViewAugmentedRealityModeMaskVertexArrayBufferId);

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        ZVIEW_AUGMENTED_REALITY_MODE_NUM_MASK_VERTS * 3 * sizeof(float),
        maskGeometryVertsFlat);

    return true;
}


bool setUpZviewMode(ZVConnection connection)
{
    ZVError error = ZV_ERROR_OK;

    // Get the specified connection's current mode so that mode-specific setup
    // operations can be performed.
    ZVMode mode = NULL;
    error = zvGetConnectionMode(connection, &mode);
    CHECK_ZV_ERROR(error);

    // If the current mode does not match the latest active mode, then a mode
    // switch has occurred.  In this case, tear down the previously active mode
    // and remember the new mode as the latest active mode.
    if (mode != g_zViewLatestActiveConnectionMode)
    {
        tearDownZviewMode();

        g_zViewLatestActiveConnectionMode = mode;
    }

    // Get the specified connection's current mode setup phase so that setup
    // operations for that phase can be performed.
    ZVModeSetupPhase modeSetupPhase;
    ZSBool isAwaitingCompletion;
    error = zvGetConnectionModeSetupPhase(
        connection, &modeSetupPhase, &isAwaitingCompletion);
    CHECK_ZV_ERROR(error);

    // If the current mode setup phase is awaiting completion, then this node
    // has already finished performing its operations for this phase and marked
    // the phase as complete but the remote node has not.  Do nothing while
    // waiting for the remote node to complete the phase.
    if (isAwaitingCompletion)
    {
        return true;
    }

    // Perform setup operations based on the current mode and mode setup phase.

    switch (modeSetupPhase)
    {
    // Perform initialization setup phase operations.

    case ZV_MODE_SETUP_PHASE_INITIALIZATION:
        if (mode == g_zViewStandardMode)
        {
            // For modes in the standard mode family, the presenter node must
            // set the resolution of the images that it is going to send to the
            // viewer once the mode is active.  This must be done during the
            // initialization setup phase so that the viewer can take the image
            // resolution into account during the completion setup phase.
            //
            // In general, the resolution of the images sent to the viewer
            // should match the resolution of the viewport where the presenter
            // application is rendering its zSpace-enabled 3D content.

            // Begin a settings batch so that the image width and height can be
            // set and sent to the viewer as a single atomic unit.  This way
            // the viewer will always see the width and height change at the
            // same time.  If this is not done, the viewer might see the width
            // change in one frame and then the height change in a subsequent
            // frame.
            error = zvBeginSettingsBatch(connection);
            CHECK_ZV_ERROR(error);

            // Set the image width and height to match the width and height of
            // the presenter application's window, which is entirely filled by
            // the application's 3D rendering viewport.

            error = zvSetSettingU16(
                connection,
                ZV_SETTING_KEY_IMAGE_WIDTH,
                g_windowWidth);
            CHECK_ZV_ERROR(error);

            error = zvSetSettingU16(
                connection,
                ZV_SETTING_KEY_IMAGE_HEIGHT,
                g_windowHeight);
            CHECK_ZV_ERROR(error);

            // End the settings batch.  This sends all settings changes in the
            // batch to the viewer.
            error = zvEndSettingsBatch(connection);
            CHECK_ZV_ERROR(error);

            // Remember the image width and height for use during rendering.
            g_zViewStandardModeImageWidth = g_windowWidth;
            g_zViewStandardModeImageHeight = g_windowHeight;

            // Mark the initialization setup phase as complete now that all
            // operations that need to be performed during this phase have been
            // completed.
            error = zvCompleteModeSetupPhase(
                connection, ZV_MODE_SETUP_PHASE_INITIALIZATION);
            CHECK_ZV_ERROR(error);
        }
        else if (mode == g_zViewAugmentedRealityMode)
        {
            // For modes in the standard mode family, the presenter node is not
            // required to perform any operations during the initialization
            // setup phase.  In general, presenter applications may begin
            // performing their own setup operations to prepare for rendering
            // standard mode images during this phase.  This might include
            // loading or allocating resources related to rendering standard
            // mode images.
            //
            // Because this sample uses very basic rendering code, it does not
            // perform any setup operations during the initialization setup
            // phase.

            // Since there are no setup operations to perform during this setup
            // phase, immediately mark the initialization setup phase as
            // complete.
            error = zvCompleteModeSetupPhase(
                connection, ZV_MODE_SETUP_PHASE_INITIALIZATION);
            CHECK_ZV_ERROR(error);
        }
        break;

    // Perform completion setup phase operations.

    case ZV_MODE_SETUP_PHASE_COMPLETION:
        if (mode == g_zViewStandardMode)
        {
            // Prepare for rendering standard mode images.
            if (!setUpZviewStandardMode(connection))
            {
                return false;
            }

            // Mark the completion setup phase as complete now that all
            // operations that need to be performed during this phase have been
            // completed.
            error = zvCompleteModeSetupPhase(
                connection, ZV_MODE_SETUP_PHASE_COMPLETION);
            CHECK_ZV_ERROR(error);
        }
        else if (mode == g_zViewAugmentedRealityMode)
        {
            // Prepare for rendering augmented reality mode images.
            if (!setUpZviewAugmentedRealityMode(connection))
            {
                return false;
            }

            // Mark the completion setup phase as complete now that all
            // operations that need to be performed during this phase have been
            // completed.
            error = zvCompleteModeSetupPhase(
                connection, ZV_MODE_SETUP_PHASE_COMPLETION);
            CHECK_ZV_ERROR(error);
        }
        break;
    }

    return true;
}


bool tearDownZviewMode()
{
    // Tear down the state related to the latest active zView mode if there is
    // one.

    if (g_zViewLatestActiveConnectionMode == g_zViewStandardMode)
    {
        if (!tearDownZviewStandardMode())
        {
            return false;
        }
    }
    else if (g_zViewLatestActiveConnectionMode == g_zViewAugmentedRealityMode)
    {
        if (!tearDownZviewAugmentedRealityMode())
        {
            return false;
        }
    }

    g_zViewLatestActiveConnectionMode = NULL;

    return true;
}


bool setUpZviewStandardMode(ZVConnection connection)
{
    ZCError zcError = ZC_ERROR_OK;
    ZVError zvError = ZV_ERROR_OK;

    // Tear down any previously existing standard mode state, if it exists.
    if (!tearDownZviewStandardMode())
    {
        return false;
    }

    // Set up a zSpace Core API viewport and frustum to use for rendering
    // standard mode images.  These allow head tracking data to be queried.
    // This is necessary because the stanard mode family mode that this sample
    // supports uses a head tracked camera mode.

    zcError = zcCreateViewport(
        g_zSpaceContext, &g_zViewStandardModeViewportHandle);
    CHECK_ZC_ERROR(zcError);

    zcError = zcGetFrustum(
        g_zViewStandardModeViewportHandle, &g_zViewStandardModeFrustumHandle); 
    CHECK_ZC_ERROR(zcError);

    zcError = zcSetViewportPosition(
        g_zViewStandardModeViewportHandle, g_windowX, g_windowY);
    CHECK_ZC_ERROR(zcError);

    zcError = zcSetViewportSize(
        g_zViewStandardModeViewportHandle,
        g_zViewStandardModeImageWidth,
        g_zViewStandardModeImageHeight);
    CHECK_ZC_ERROR(zcError);

    // Set up the OpenGL objects to allow standard mode images to be rendered
    // in the background.

    // Color buffer texture.

    glGenTextures(1, &g_zViewStandardModeColorGlTextureId);

    glBindTexture(GL_TEXTURE_2D, g_zViewStandardModeColorGlTextureId);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        g_zViewStandardModeImageWidth,
        g_zViewStandardModeImageHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Depth buffer.

    glGenRenderbuffers(1, &g_zViewStandardModeDepthGlRenderbufferId);

    glBindRenderbuffer(
        GL_RENDERBUFFER, g_zViewStandardModeDepthGlRenderbufferId);

    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH_COMPONENT,
        g_zViewStandardModeImageWidth,
        g_zViewStandardModeImageHeight);

    // Framebuffer.

    glGenFramebuffers(1, &g_zViewStandardModeGlFramebufferId);

    glBindFramebuffer(GL_FRAMEBUFFER, g_zViewStandardModeGlFramebufferId);

    glFramebufferTexture(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        g_zViewStandardModeColorGlTextureId,
        0);

    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER,
        g_zViewStandardModeDepthGlRenderbufferId);

    const GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    // Re-enable screen rendering.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        return false;
    }

    // Initialize the frame number for standard mode images.  This will be
    // incremented for every standard mode image that is rendered and then sent
    // to the viewer along with the image.
    g_zViewStandardModeFrameNumber = 0;

    return true;
}


bool tearDownZviewStandardMode()
{
    // Tear down the state related to the zView standard mode implementation.
    // This consists of releasing various zSpace Core API handles and OpenGL
    // objects that have been allocated for use in standard mode rendering.

    ZCError zcError = ZC_ERROR_OK;

    if (g_zViewStandardModeViewportHandle != NULL)
    {
        zcError = zcDestroyViewport(g_zViewStandardModeViewportHandle);
        CHECK_ZC_ERROR(zcError);

        g_zViewStandardModeViewportHandle = NULL;
        g_zViewStandardModeFrustumHandle = NULL;
    }

    if (g_zViewStandardModeGlFramebufferId != 0)
    {
        glDeleteFramebuffers(1, &g_zViewStandardModeGlFramebufferId);
        g_zViewStandardModeGlFramebufferId = 0;
    }

    if (g_zViewStandardModeColorGlTextureId != 0)
    {
        glDeleteTextures(1, &g_zViewStandardModeColorGlTextureId);
        g_zViewStandardModeColorGlTextureId = 0;
    }

    if (g_zViewStandardModeDepthGlRenderbufferId != 0)
    {
        glDeleteRenderbuffers(1, &g_zViewStandardModeDepthGlRenderbufferId);
        g_zViewStandardModeDepthGlRenderbufferId = 0;
    }

    return true;
}


bool setUpZviewAugmentedRealityMode(ZVConnection connection)
{
    ZCError zcError = ZC_ERROR_OK;
    ZVError zvError = ZV_ERROR_OK;

    // Tear down any previously existing standard mode state, if it exists.
    if (!tearDownZviewAugmentedRealityMode())
    {
        return false;
    }

    // Query the resolution of the augmented reality mode images to render.
    // This resolution was set by the viewer node during the initialization
    // mode setup phase.

    zvError = zvGetSettingU16(
        connection,
        ZV_SETTING_KEY_IMAGE_WIDTH,
        &g_zViewAugmentedRealityModeImageWidth);
    CHECK_ZV_ERROR(zvError);

    zvError = zvGetSettingU16(
        connection,
        ZV_SETTING_KEY_IMAGE_HEIGHT,
        &g_zViewAugmentedRealityModeImageHeight);
    CHECK_ZV_ERROR(zvError);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create an OpenGL vertex array and associated buffer for the mask
    // geometry used for augmented reality mode renders.  The mask geometry is
    // used to clip any scene geometry that is positioned behind the plane of
    // the zSpace display and outside of the bounds of the presenter
    // application's viewport.  Performing this clipping allows the viewer to
    // achieve the correct augmented reality effect when the augmented reality
    // mode images from the presenter are composited with the augmented reality
    // mode webcam video stream.

    glGenVertexArrays(1, &g_zViewAugmentedRealityModeMaskVertexArrayId);

    glGenBuffers(1, &g_zViewAugmentedRealityModeMaskVertexArrayBufferId);

    glBindBuffer(
        GL_ARRAY_BUFFER, g_zViewAugmentedRealityModeMaskVertexArrayBufferId);

    glBufferData(
        GL_ARRAY_BUFFER,
        ZVIEW_AUGMENTED_REALITY_MODE_NUM_MASK_VERTS * 3 * sizeof(float),
        NULL,
        GL_DYNAMIC_DRAW);

    // Create an OpenGL vertex array and associated buffer for the background
    // quad geometry.  This is additional geometry that is rendered in
    // augmented reality mode to fill in the background color of the augmented
    // reality mode images.  Using an additional quad for this is necessary
    // because the glClear() function cannot be used to clear a subregion of
    // the color buffer.  See the augmented reality mode drawing code for more
    // details on this.

    glGenVertexArrays(1, &g_zViewAugmentedRealityModeBackgroundVertexArrayId);

    glGenBuffers(1, &g_zViewAugmentedRealityModeBackgroundVertexArrayBufferId);

    glBindBuffer(
        GL_ARRAY_BUFFER,
        g_zViewAugmentedRealityModeBackgroundVertexArrayBufferId);

    float backgroundGeometryVerts[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
    };

    glBufferData(
        GL_ARRAY_BUFFER,
        4 * 3 * sizeof(float),
        backgroundGeometryVerts,
        GL_STATIC_DRAW);

    // Set up the OpenGL objects to allow augmented reality mode images to be
    // rendered in the background.

    // Color buffer texture.

    glGenTextures(1, &g_zViewAugmentedRealityModeColorGlTextureId);

    glBindTexture(GL_TEXTURE_2D, g_zViewAugmentedRealityModeColorGlTextureId);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        g_zViewAugmentedRealityModeImageWidth,
        g_zViewAugmentedRealityModeImageHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Depth/stencil buffer.

    glGenRenderbuffers(
        1, &g_zViewAugmentedRealityModeDepthStencilGlRenderbufferId);

    glBindRenderbuffer(
        GL_RENDERBUFFER,
        g_zViewAugmentedRealityModeDepthStencilGlRenderbufferId);

    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH24_STENCIL8,
        g_zViewAugmentedRealityModeImageWidth,
        g_zViewAugmentedRealityModeImageHeight);

    // Mask Framebuffer.
    //
    // This frame buffer is used for drawing the mask geometry into a
    // depth/stencil buffer that can later be used to perform depth and
    // stencil testing while rendering the augmented relaity mode color image.

    glGenFramebuffers(1, &g_zViewAugmentedRealityModeMaskGlFramebufferId);

    glBindFramebuffer(
        GL_FRAMEBUFFER, g_zViewAugmentedRealityModeMaskGlFramebufferId);

    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER,
        g_zViewAugmentedRealityModeDepthStencilGlRenderbufferId);

    const GLenum maskFramebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    // Re-enable screen rendering.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (maskFramebufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        return false;
    }

    // Framebuffer.

    glGenFramebuffers(1, &g_zViewAugmentedRealityModeGlFramebufferId);

    glBindFramebuffer(
        GL_FRAMEBUFFER, g_zViewAugmentedRealityModeGlFramebufferId);

    glFramebufferTexture(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        g_zViewAugmentedRealityModeColorGlTextureId,
        0);

    // Use the same depth/stencil renderbuffer that is used by the mask
    // framebuffer.
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, 
        GL_DEPTH_STENCIL_ATTACHMENT, 
        GL_RENDERBUFFER, 
        g_zViewAugmentedRealityModeDepthStencilGlRenderbufferId);

    const GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    // Re-enable screen rendering.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        return false;
    }

    return true;
}


bool tearDownZviewAugmentedRealityMode()
{
    // Tear down the state related to the zView augmented reality mode
    // implementation.  This consists of releasing various OpenGL objects that
    // have been allocated for use in augmented reality mode rendering.

    ZVError zvError = ZV_ERROR_OK;

    if (g_zViewAugmentedRealityModeMaskVertexArrayId != 0)
    {
        glDeleteVertexArrays(1, &g_zViewAugmentedRealityModeMaskVertexArrayId);
        g_zViewAugmentedRealityModeMaskVertexArrayId = 0;
    }

    if (g_zViewAugmentedRealityModeMaskVertexArrayBufferId != 0)
    {
        glDeleteBuffers(1, &g_zViewAugmentedRealityModeMaskVertexArrayBufferId);
        g_zViewAugmentedRealityModeMaskVertexArrayBufferId = 0;
    }

    if (g_zViewAugmentedRealityModeBackgroundVertexArrayId != 0)
    {
        glDeleteVertexArrays(
            1, &g_zViewAugmentedRealityModeBackgroundVertexArrayId);
        g_zViewAugmentedRealityModeBackgroundVertexArrayId = 0;
    }

    if (g_zViewAugmentedRealityModeBackgroundVertexArrayBufferId != 0)
    {
        glDeleteBuffers(
            1, &g_zViewAugmentedRealityModeBackgroundVertexArrayBufferId);
        g_zViewAugmentedRealityModeBackgroundVertexArrayBufferId = 0;
    }

    if (g_zViewAugmentedRealityModeMaskGlFramebufferId != 0)
    {
        glDeleteFramebuffers(
            1, &g_zViewAugmentedRealityModeMaskGlFramebufferId);
        g_zViewAugmentedRealityModeMaskGlFramebufferId = 0;
    }

    if (g_zViewAugmentedRealityModeGlFramebufferId != 0)
    {
        glDeleteFramebuffers(
            1, &g_zViewAugmentedRealityModeGlFramebufferId);
        g_zViewAugmentedRealityModeGlFramebufferId = 0;
    }

    if (g_zViewAugmentedRealityModeColorGlTextureId != 0)
    {
        glDeleteTextures(1, &g_zViewAugmentedRealityModeColorGlTextureId);
        g_zViewAugmentedRealityModeColorGlTextureId = 0;
    }

    if (g_zViewAugmentedRealityModeDepthStencilGlRenderbufferId != 0)
    {
        glDeleteRenderbuffers(
            1, &g_zViewAugmentedRealityModeDepthStencilGlRenderbufferId);
        g_zViewAugmentedRealityModeDepthStencilGlRenderbufferId = 0;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool drawZview()
{
    // Only perform zView drawing if there is a zView context and an active
    // zView connection.

    if (g_zViewContext == NULL)
    {
        return true;
    }

    if (g_zViewActiveConnection == NULL)
    {
        return true;
    }

    ZVError error = ZV_ERROR_OK;

    // Only perform zView drawing if the active zView connection has a current
    // active mode.

    ZVConnectionState connectionState = ZV_CONNECTION_STATE_ERROR;
    error = zvGetConnectionState(g_zViewActiveConnection, &connectionState);
    CHECK_ZV_ERROR(error);

    if (connectionState != ZV_CONNECTION_STATE_MODE_ACTIVE)
    {
        return true;
    }

    // Get the active zView connection's current mode and then perform
    // mode-specific drawing operations.

    ZVMode mode = NULL;
    error = zvGetConnectionMode(g_zViewActiveConnection, &mode);
    CHECK_ZV_ERROR(error);

    // Perform standard mode drawing operations.
    if (mode == g_zViewStandardMode)
    {
        // Get the next zView frame that is available for sending.  This frame
        // will be filled with frame data and the color image for the current
        // frame and then sent to the viewer.
        ZVFrame frame = NULL;
        error = zvGetNextFrameToSend(
            g_zViewActiveConnection, ZV_STREAM_IMAGE, &frame);
        CHECK_ZV_ERROR(error);

        // If there are no available frames, skip drawing until the next main
        // loop iteration.  In this case, the viewer is not processing frames
        // as quickly as the presenter is sending them, so the presenter needs
        // to drop frames until the viewer catches up.
        if (frame == NULL)
        {
            return true;
        }

        // Set the current frame number in the frame data.  In standard mode
        // family modes, the frame number exists primarily for informational
        // purposes.
        error = zvSetFrameDataU64(
            frame,
            ZV_FRAME_DATA_KEY_FRAME_NUMBER,
            g_zViewStandardModeFrameNumber);
        CHECK_ZV_ERROR(error);

        // Actually draw the standard mode color image for the current frame.
        if (!drawZviewStandardMode())
        {
            return false;
        }

        // Get a pointer to the color image buffer within the current frame.
        ZSUInt8* imageBuffer = NULL;
        error = zvGetFrameBuffer(
            frame, ZV_FRAME_BUFFER_KEY_IMAGE_COLOR_0, &imageBuffer);
        CHECK_ZV_ERROR(error);

        // Copy the standard mode color image that was just drawn above into
        // the current frame's color image buffer.
        glBindTexture(GL_TEXTURE_2D, g_zViewStandardModeColorGlTextureId);
        glGetTexImage(
            GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageBuffer);

        // Send the current frame to the viewer.
        error = zvSendFrame(frame);
        CHECK_ZV_ERROR(error);

        // Increment the current frame number so that the next frame will have
        // the next frame number.
        ++g_zViewStandardModeFrameNumber;

        // Detect changes in this application's viewport resolution and apply
        // them to the standard mode image resolution.  This is technically an
        // update operation, but it is performed here so that it does not
        // impact drawing for the current main loop iteration. 
        if (!handleZviewStandardModeImageResolutionChange())
        {
            return false;
        }
    }
    // Perform augmented reality mode drawing operations.
    else if (mode == g_zViewAugmentedRealityMode)
    {
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // In augmented reality mode family modes, the viewer sends frames to
        // the presenter containing camera pose, and camera intrinsics data.
        // Whenever the presenter receives one of these frames, it then draws
        // an augmented reality mode image using the camera pose and intrinsics
        // and then sends it back to the viewer.

        // Receive the next frame from the viewer.
        ZVFrame receivedFrame = NULL;
        error = zvReceiveFrame(
            g_zViewActiveConnection, ZV_STREAM_IMAGE, &receivedFrame);
        CHECK_ZV_ERROR(error);

        // If there is no frame available to be received from the viewer, skip
        // drawing until the next main loop iteration.  In this case, the
        // viewer is not sending frames as quickly as the presenter is
        // processing them, so the presenter needs to drop frames until the
        // viewer catches up.
        if (receivedFrame == NULL)
        {
            return true;
        }

        // Get the frame number for the frame received from the viewer.  After
        // the presenter is done drawing the augmented reality mode image, it
        // will send the image to the viewer using this frame number.  This
        // allows the viewer to match up the augmented reality mode image with
        // the appropriate augmented reality mode webcam video frame.
        ZSUInt64 receivedFrameNumber = 0;
        error = zvGetFrameDataU64(
            receivedFrame,
            ZV_FRAME_DATA_KEY_FRAME_NUMBER,
            &receivedFrameNumber);
        CHECK_ZV_ERROR(error);
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Get the camera pose for the frame received from the viewer.  This is
        // a 4x4 matrix encoding the position and orientation of the augmented
        // reality mode webcam in the display coordinate space.
        error = zvGetFrameDataM4(
            receivedFrame,
            ZV_FRAME_DATA_KEY_CAMERA_POSE,
            &g_zViewAugmentedRealityModeCameraPoseDisplaySpace);
        CHECK_ZV_ERROR(error);
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Get various camera intrinsics for the frame received from the
        // viewer.  These intrinsics describe how the augmented reality mode
        // webcam projects images onto its image sensor.  This information can
        // be used to compute the projection matrix needed to draw augmented
        // reality mode images that will line up with the images captured by
        // the augmented reality mode webcam.

        ZSFloat cameraFocalLength = 0.0f;
        error = zvGetFrameDataF32(
            receivedFrame,
            ZV_FRAME_DATA_KEY_CAMERA_FOCAL_LENGTH,
            &cameraFocalLength);
        CHECK_ZV_ERROR(error);

        ZSFloat cameraPrincipalPointOffsetX = 0.0f;
        error = zvGetFrameDataF32(
            receivedFrame,
            ZV_FRAME_DATA_KEY_CAMERA_PRINCIPAL_POINT_OFFSET_X,
            &cameraPrincipalPointOffsetX);
        CHECK_ZV_ERROR(error);

        ZSFloat cameraPrincipalPointOffsetY = 0.0f;
        error = zvGetFrameDataF32(
            receivedFrame,
            ZV_FRAME_DATA_KEY_CAMERA_PRINCIPAL_POINT_OFFSET_Y,
            &cameraPrincipalPointOffsetY);
        CHECK_ZV_ERROR(error);

        ZSFloat cameraPixelAspectRatio = 0.0f;
        error = zvGetFrameDataF32(
            receivedFrame,
            ZV_FRAME_DATA_KEY_CAMERA_PIXEL_ASPECT_RATIO,
            &cameraPixelAspectRatio);
        CHECK_ZV_ERROR(error);

        ZSFloat cameraAxisSkew = 0.0f;
        error = zvGetFrameDataF32(
            receivedFrame,
            ZV_FRAME_DATA_KEY_CAMERA_AXIS_SKEW,
            &cameraAxisSkew);
        CHECK_ZV_ERROR(error);

        // Now that all necessary frame data in the frame received from the
        // viewer has been queried, release the frame so that its resources can
        // be freed up or possibly be reused in the future.
        error = zvReleaseReceivedFrame(receivedFrame);
        CHECK_ZV_ERROR(error);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Compute the projection matrix to use for drawing augmented reality
        // mode images.
        g_zViewAugmentedRealityModeCameraProjection =
            computeZviewAugmentedRealityModeProjectionMatrix(
                cameraFocalLength,
                cameraPrincipalPointOffsetX,
                cameraPrincipalPointOffsetY,
                cameraPixelAspectRatio,
                cameraAxisSkew,
                g_zViewAugmentedRealityModeImageWidth,
                g_zViewAugmentedRealityModeImageHeight,
                0.1f,
                100.0f);

        // Update various augmented reality mode values now that the latest
        // camera pose and projection are available.
        updateZviewAugmentedRealityMode();

        // Send a frame to the viewer with a augmented reality mode color image
        // rendered using the latest camera pose and projection.

        // Get the next zView frame that is available for sending.  This frame
        // will be filled with frame data and the color image for the current
        // frame and then sent to the viewer.
        ZVFrame frame = NULL;
        error = zvGetNextFrameToSend(
            g_zViewActiveConnection, ZV_STREAM_IMAGE, &frame);
        CHECK_ZV_ERROR(error);

        // If there are no available frames, skip drawing until the next main
        // loop iteration.  In this case, the viewer is not processing frames
        // as quickly as the presenter is sending them, so the presenter needs
        // to drop frames until the viewer catches up.
        if (frame == NULL)
        {
            return true;
        }

        // Set the frame number in the frame data to the frame number that was
        // in the latest frame received from the viewer.  This will allow the
        // viewer to match up the augmented reality mode image in this frame
        // with the appropriate augmented reality mode webcam video frame.

        error = zvSetFrameDataU64(
            frame,
            ZV_FRAME_DATA_KEY_FRAME_NUMBER,
            receivedFrameNumber);
        CHECK_ZV_ERROR(error);

        // Actually draw the augmented reality mode color image for the current
        // frame.
        if (!drawZviewAugmentedRealityMode())
        {
            return false;
        }

        // Get a pointer to the color image buffer within the current frame.
        ZSUInt8* imageBuffer = NULL;
        error = zvGetFrameBuffer(
            frame, ZV_FRAME_BUFFER_KEY_IMAGE_COLOR_0, &imageBuffer);
        CHECK_ZV_ERROR(error);

        // Copy the augmented reality mode color image that was just drawn
        // above into the current frame's color image buffer.
        glBindTexture(
            GL_TEXTURE_2D, g_zViewAugmentedRealityModeColorGlTextureId);
        glGetTexImage(
            GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageBuffer);

        // Send the current frame to the viewer.
        error = zvSendFrame(frame);
        CHECK_ZV_ERROR(error);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool drawZviewStandardMode()
{
    ZCError zcError = ZC_ERROR_OK;

    // The standard mode family mode supported by this sample uses the local
    // head tracked presenter camera mode.  This means that the view and
    // projection matrices to use for rendering standard mode images should
    // be queried from a zSpace Core API stereo frustum.  In particular, the
    // matrices for the center eye are used in order to produce images that are
    // in between the images seen by the user's left and right eyes on screen.

    // Update the position of the zSpace Core API viewport associated with the
    // stereo frustum used for standard mode rendering so that it matches the
    // current presenter application viewport position.  This keeps viewport
    // position used for standard mode rendering in sync with the position used
    // for rendering on screen.
    zcError = zcSetViewportPosition(
        g_zViewStandardModeViewportHandle, g_windowX, g_windowY);
    CHECK_ZC_ERROR(zcError);

    // Get the view matrix from the zSpace Core API stereo frustum for the
    // center eye.
    ZSMatrix4 viewMatrix;
    zcError = zcGetFrustumViewMatrix(
        g_zViewStandardModeFrustumHandle, ZC_EYE_CENTER, &viewMatrix);
    CHECK_ZC_ERROR(zcError);

    // Combine the view matrix from the stereo frustum with the current camera
    // transform to get the final view matrix to use.  Incorporating the camera
    // transform causes the camera to orbit around the scene in the same way
    // that it does on screen.
    //
    // Note:  The view matrix is set to the g_viewMatrix global variable.  This
    // will cause it to be used automatically when the scene rendering code is
    // called below.
    glm::mat4 zcViewMatrix = glm::make_mat4(viewMatrix.f);
    g_viewMatrix = zcViewMatrix * g_cameraTransform;

    // Get the projection matrix from the zSpace Core API stereo frustum for
    // the center eye.
    ZSMatrix4 projectionMatrix;
    zcError = zcGetFrustumProjectionMatrix(
        g_zViewStandardModeFrustumHandle, ZC_EYE_CENTER, &projectionMatrix);
    CHECK_ZC_ERROR(zcError);

    // Set the projection matrix to the appropriate shader uniform variable for
    // the shader used to render the scene.
    glm::mat4 zcProjMatrix = glm::make_mat4(projectionMatrix.f);
    glUseProgram(g_shaderProgram);
    glUniformMatrix4fv(g_projectionUniform, 1, GL_FALSE, glm::value_ptr(zcProjMatrix));

    // Configure OpenGL to use the standard mode framebuffer for rendering.

    glBindFramebuffer(GL_FRAMEBUFFER, g_zViewStandardModeGlFramebufferId);

    const GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    // Set the OpenGL viewport to match the standard mode image size.
    glViewport(0, 0, g_zViewStandardModeImageWidth, g_zViewStandardModeImageHeight);

    // Clear the color and depth buffers in the standard mode frame buffer.
    glClearColor(0.0f, 1.0f, 0.0f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw the scene using the same code that is used to draw the scene on
    // screen.

    drawCube();

    drawStylus();

    glFinish();

    // Re-enable screen rendering.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool drawZviewAugmentedRealityMode()
{
    // Augmented reality mode images are drawn using a two pass approach.  In
    // the first pass, the augmented reality mode mask geometry is drawn in a
    // depth/stencil 模板buffer.  In the second pass, the scene is drawn into a
    // color buffer using the depth/stencil buffer from the first pass to clip
    // scene elements that are behind the screen plane and outside the bounds
    // of the application viewport.

    // Pass 1:  Draw the augmented reality mode mask into the depth/stencil
    // buffer.

    glBindFramebuffer(
        GL_FRAMEBUFFER, g_zViewAugmentedRealityModeMaskGlFramebufferId);

    const GLenum pass0DrawBuffers[] = { GL_NONE };
    glDrawBuffers(1, pass0DrawBuffers);

    // Set the OpenGL viewport to match the augmented reality mode image size.
    glViewport(
        0,
        0,
        g_zViewAugmentedRealityModeImageWidth,
        g_zViewAugmentedRealityModeImageHeight);

    // Enable writing to the stencil buffer.
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF);

    // Write 0 into the stencil buffer when it is cleared.
    glClearStencil(0);

    // Clear the depth and stencil buffers only.
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Write 1 into the stencil buffer wherever the mask is drawn.
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Draw the augmented reality mode mask.
    if (!drawZviewAugmentedRealityModeMask())
    {
        return false;
    }

    // Pass 2:  Draw the scene using the depth/stencil buffer from pass 1.
    //
    // This sample draws the scene using two sub-passes.  In the first sub-pass
    // (pass 2a), the background of the scene is drawn using the stencil buffer
    // from pass 1 to clip it.  This causes the background to only appear
    // within the bounds of the presenter application's viewport (as seen from
    // the perspective of the augmented reality mode webcam).  In the second
    // sub-pass (pass 2b), the remainder of the scene (the cube and the stylus)
    // is drawn using the depth buffer from pass 1.  This causes elements of
    // the scene that are behind the mask (i.e. behind the plane of the zSpace
    // display's screen and outside the bounds of the presenter application's
    // viewport) to be clipped.
    //
    // This approach should work for more complex scenes as long as it is safe
    // for the background elements in the scene to be drawn completely before
    // the remaining scene elements.  If the background elements and the
    // remaining scene elements need to be drawn in an interleaved order (e.g.
    // if the background elements contain transparent objects that may be
    // positioned in front of non-background elements), then a different
    // approach is needed.  In this case, one option is to modify the approach
    // used by this sample so that the first sub-pass (pass 2a) renders all
    // scene elements (background and non-background) and the second sub-pass
    // (pass 2b) still renders only non-background scene elements, but uses the
    // stencil buffer from pass 1 to clip anthing that lies within the bounds
    // of the presenter application's viewport.  Doing this allows background
    // and non-background scene elements to be drawn in the correct order
    // during the first sub-pass (pass 2a) and prevents the second sub-pass
    // (pass 2b) from modifying the pixel drawn during the first sub-pass.

    // Configure OpenGL to use the augmented reality mode framebuffer for
    // rendering.  This framebuffer uses the same depth/stencil buffer as the
    // framebuffer used for pass 1.

    glBindFramebuffer(
        GL_FRAMEBUFFER, g_zViewAugmentedRealityModeGlFramebufferId);

    const GLenum pass1DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, pass1DrawBuffers);

    // Set the OpenGL viewport to match the augmented reality mode image size.
    glViewport(
        0,
        0,
        g_zViewAugmentedRealityModeImageWidth,
        g_zViewAugmentedRealityModeImageHeight);

    // Clear the color buffer but not the depth buffer or stencil buffer (in
    // order to reuse the depth and stencil information from pass 1).  Ensure
    // that the alpha channel is cleared to 0.0f so that the resulting image
    // can be composited on top of the augmentd reality mode webcam video
    // stream by the viewer.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Pass 2a:  Draw the background into the scene wherever the stencil buffer
    // is less than 1 (i.e. only where the mask was not drawn, which is the
    // region that is within the bounds of the presenter application's
    // viewport).
    //
    // In this sample, this is done by simply drawing a full frame quad with
    // the background color.  The depth test and writing to the depth buffer
    // are turned off so that the depth of the quad does not matter (this
    // effectively places the quad behind everything else in the scene).  More
    // advanced scenes with actual background elements will want to leave the
    // depth test and writing to the depth buffer turned on during this
    // sub-pass.

    glEnable(GL_STENCIL_TEST);

    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0x00);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // Optionally do not draw background for debugging/visualization purposes.
    if (g_zViewAugmentedRealityModeShouldDrawBackground)
    {
        if (!drawZviewAugmentedRealityModeBackground())
        {
            return false;
        }
    }

    // Disable the stencil test so that it does not affect the next sub-pass.

    glDisable(GL_STENCIL_TEST);

    // Re-enable the depth test and writing to the depth buffer for the next
    // sub-pass.

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    // Pass 2b:  Draw the rest of scene using the depth buffer from pass 1, but
    // not the stencil buffer.  This allows the rest of the scene elements may
    // be drawn on top of the mask if they are closer to the camera than the
    // mask is (i.e. if they are not behind the plane of the zSpace display's
    // screen).

    // Set the view and projection matrices to use the augmented reality mode
    // camera.

    g_viewMatrix = g_zViewAugmentedRealityModeCameraWorldSpaceViewTransform;

    glUseProgram(g_shaderProgram);
    glUniformMatrix4fv(
        g_projectionUniform,
        1,
        GL_FALSE,
        glm::value_ptr(g_zViewAugmentedRealityModeCameraProjection));

    // Optionally draw the augmented reality mode mask for
    // debugging/visualization purposes.
    if (g_zViewAugmentedRealityModeShouldDrawMask)
    {
        // Use the "less than or equal to" depth comparison when drawing the
        // mask so that the mask's depth from pass 1 does not cull the mask
        // now.
        glDepthFunc(GL_LEQUAL);

        if (!drawZviewAugmentedRealityModeMask())
        {
            return false;
        }

        glDepthFunc(GL_LESS);
    }

    // Draw the scene using the same code that is used to draw the scene on
    // screen.

    drawCube();

    drawStylus();

    glFinish();

    // Re-enable screen rendering.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}


bool drawZviewAugmentedRealityModeMask()
{
    glUseProgram(g_zViewAugmentedRealityModeMaskShaderProgram);
    glUniformMatrix4fv(
        g_zViewAugmentedRealityModeMaskTransformMatrixUniform,
        1,
        GL_FALSE,
        glm::value_ptr(g_zViewAugmentedRealityModeMaskTransform));

    glBindVertexArray(g_zViewAugmentedRealityModeMaskVertexArrayId);

    glEnableVertexAttribArray(0);
    glBindBuffer(
        GL_ARRAY_BUFFER, g_zViewAugmentedRealityModeMaskVertexArrayBufferId);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    for (int i=1; i<8; i++)
    {
        glDisableVertexAttribArray(i);
    }

    glDrawArrays(GL_QUADS, 0, ZVIEW_AUGMENTED_REALITY_MODE_NUM_MASK_VERTS);

    return true;
}


bool drawZviewAugmentedRealityModeBackground()
{
    glUseProgram(g_zViewAugmentedRealityModeBackgroundShaderProgram);
    glUniform4fv(
        g_zViewAugmentedRealityModeBackgroundColorUniform,
        1,
        glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)));

    glBindVertexArray(g_zViewAugmentedRealityModeBackgroundVertexArrayId);

    glEnableVertexAttribArray(0);
    glBindBuffer(
        GL_ARRAY_BUFFER,
        g_zViewAugmentedRealityModeBackgroundVertexArrayBufferId);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    for (int i=1; i<8; i++)
    {
        glDisableVertexAttribArray(i);
    }

    glDrawArrays(GL_QUADS, 0, 4);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////计算坐标变换
glm::mat4 computeZviewAugmentedRealityModeProjectionMatrix(
    float focalLength,
    float principalPointOffsetX,
    float principalPointOffsetY,
    float pixelAspectRatio,
    float axisSkew,
    float imageWidth,
    float imageHeight,
    float nearClip,
    float farClip)
{
    // The projection matrix to use for rendering augmented reality mode images
    // is computed by combining two other transform matrices:  a pure
    // perspective projection matrix and an OpenGL-style orthographic
    // projection matrix.  The perspective projection matrix takes into account
    // the camera intrinsics of the augmented reality mode webcam and the
    // orthographic projection matrix transforms perspective projected
    // coordinates into OpenGL's normalized device coordinate space.
    //
    // For more details on how this works, see the following article:
    //
    // http://ksimek.github.io/2013/06/03/calibrated_cameras_in_opengl/

    // Compute the perspective projection matrix using the camera intrinsics.

    // Assign/compute various intermediate values.  Short variable names are
    // used to make it easier to see where these values are used in building
    // the perspective projection matrix below.

    // X focal length.
    const float fX = focalLength;
    // Y focal length.  Can be different than the X focal length if the pixel
    // aspect ratio is not 1 (i.e. if the camera's pixels are not square).
    const float fY = focalLength * pixelAspectRatio;

    // Principal point offset.
    const float cX = principalPointOffsetX;
    const float cY = principalPointOffsetY;

    // Axis skew.
    const float s = axisSkew;

    // Projected depth coefficients.
    const float a = nearClip + farClip;
    const float b = nearClip * farClip;

    glm::mat4 cameraPerspectiveProjectionMatrix(
        fX,   0.0f, 0.0f, 0.0f,
        // Negate this column to take into account the fact that the image Y
        // axis is pointing down, which is the opposite of the OpenGL camera Y
        // axis.
        -s,    -fY, 0.0f, 0.0f,
        // Negate this column to take into account the fact that the OpenGL
        // camera looks down the negative Z axis, which is the opposite of
        // convention used in typical camera intrinsics matrices (where the
        // camera looks down the positive Z axis).
        -cX,  -cY,  a,    -1.0f,
        0.0f, 0.0f, b,    0.0f);

    // Compute the orthographic projection matrix.

    glm::mat4 ndcConversionMatrix = glm::ortho(
        0.0f, imageWidth, imageHeight, 0.0f, nearClip, farClip);

    // Combine the perspective projection matrix and the orthographic
    // projection matrix to get the final augmented reality mode projection
    // matrix.

    glm::mat4 arProjectionMatrix =
        ndcConversionMatrix * cameraPerspectiveProjectionMatrix;
	// normalized device coordinate space
    return arProjectionMatrix;
}


bool switchZviewMode()
{
    ZVError zvError = ZV_ERROR_OK;

    // Do nothing if there is no active zView connection.
    if (g_zViewActiveConnection == NULL)
    {
        return true;
    }

    // Get the current active connection state.
    ZVConnectionState state = ZV_CONNECTION_STATE_ERROR;
    zvError = zvGetConnectionState(g_zViewActiveConnection, &state);
    CHECK_ZV_ERROR(zvError);

    // Do nothing if the active connection is not in a state where it makes
    // sense to perform a mode switch.
    if ((state != ZV_CONNECTION_STATE_NO_MODE) &&
        (state != ZV_CONNECTION_STATE_MODE_ACTIVE))
    {
        return true;
    }

    // Loop through the modes supported by the active connection, starting at
    // the index after the index used during the last mode switch, until an
    // available mode is found and then switch to that mode.  This causes a
    // switch to the next available mode in the connection's list of supported
    // modes.
    //
    // If there are no available modes (or no supported modes), then no mode
    // switch is performed.

    ZSInt32 numSupportedModes = 0;
    zvError = zvGetNumConnectionSupportedModes(
        g_zViewActiveConnection, &numSupportedModes);
    CHECK_ZV_ERROR(zvError);

    ZSInt32 numModesTried = 0;

    // If all supported modes are visited before an available mode is found,
    // then there are no available modes.
    while (numModesTried < numSupportedModes)
    {
        ++g_zViewCurrentConnectionModeIndex;
        // If the index goes beyond the end of the list of supported modes,
        // then loop it back around to the beginning of the list.
        g_zViewCurrentConnectionModeIndex %= numSupportedModes;

        ZVSupportedMode curSupportedMode;
        zvError = zvGetConnectionSupportedMode(
            g_zViewActiveConnection,
            g_zViewCurrentConnectionModeIndex,
            &curSupportedMode);
        CHECK_ZV_ERROR(zvError);

        if (curSupportedMode.availability == ZV_MODE_AVAILABILITY_AVAILABLE)
        {
            printLog("Switching zView modes...");

            zvError = zvSetConnectionMode(
                g_zViewActiveConnection, curSupportedMode.mode);
            CHECK_ZV_ERROR(zvError);

            break;
        }

        ++numModesTried;
    }

    return true;
}


bool pauseResumeZviewMode()
{
    ZVError zvError = ZV_ERROR_OK;

    // Do nothing if there is no active zView connection.
    if (g_zViewActiveConnection == NULL)
    {
        return true;
    }

    // Get the current active connection state.
    ZVConnectionState state = ZV_CONNECTION_STATE_ERROR;
    zvError = zvGetConnectionState(g_zViewActiveConnection, &state);
    CHECK_ZV_ERROR(zvError);

    // If a mode is currently active, pause it.
    if (state == ZV_CONNECTION_STATE_MODE_ACTIVE)
    {
        zvError = zvPauseMode(g_zViewActiveConnection);
        CHECK_ZV_ERROR(zvError);
    }
    // If a mode is currently paused, resume it.
    else if (state == ZV_CONNECTION_STATE_MODE_PAUSED)
    {
        zvError = zvResumeMode(g_zViewActiveConnection);
        CHECK_ZV_ERROR(zvError);
    }

    return true;
}


bool closeZviewConnectionAndExitViewer()
{
    ZVError zvError = ZV_ERROR_OK;

    // Do nothing if there is no active zView connection.
    if (g_zViewActiveConnection == NULL)
    {
        return true;
    }

    zvError = zvCloseConnection(
        g_zViewActiveConnection,
        ZV_CONNECTION_CLOSE_ACTION_EXIT_APPLICATION,
        ZV_CONNECTION_CLOSE_REASON_USER_REQUESTED,
        "User requested connection to close and viewer to exit");
    CHECK_ZV_ERROR(zvError);

    return true;
}

