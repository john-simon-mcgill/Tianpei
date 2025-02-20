/*
* Copyright 1993-2014 NVIDIA Corporation.  All rights reserved.
*
* Please refer to the NVIDIA end user license agreement (EULA) associated
* with this source code for terms and conditions that govern your use of
* this software. Any use, reproduction, disclosure, or distribution of
* this software and related documentation outside the terms of the EULA
* is strictly prohibited.
*
*/

/*
    Bindless Texture/Surface

    This sample generates a few 2D textures and uses cudaTextureObjects to
    perform pseudo virtual texturing for display. One 2D texture stores
    references to other textures.
    Furthermore use of mip mapping is shown using both cudaTextureObjects
    and cudaSurfaceObjects.

    Look into the bindlessTexture_kernel.cu file for most relevant code.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include <vector>

#if defined (__APPLE__) || defined(MACOSX)
  #include <GLUT/glut.h>
  #ifndef glutCloseFunc
  #define glutCloseFunc glutWMCloseFunc
  #endif
#else
#include <GL/freeglut.h>
#endif

#include "bindlessTexture.h"

#include <helper_functions.h>
#include <cuda_gl_interop.h>
#include <helper_cuda_gl.h>


#define MAX_EPSILON_ERROR 5.0f
#define THRESHOLD         0.15f

const char *sSDKsample = "CUDA bindlessTexture";

const char *imageFilenames[] =
{
    "flower.ppm",
    "person.ppm",
    "sponge.ppm",
};

const cudaExtent atlasSize  = make_cudaExtent(4, 4, 0);
const dim3 windowSize(512, 512);
const dim3 windowBlockSize(16, 16, 1);
const dim3 windowGridSize(windowSize.x / windowBlockSize.x, windowSize.y / windowBlockSize.y);

float lod = 0.5;    // texture mip map level

GLuint pbo;         // OpenGL pixel buffer object
struct cudaGraphicsResource *cuda_pbo_resource = NULL; // CUDA Graphics Resource (to transfer PBO)

bool animate = true;

StopWatchInterface *timer = NULL;

uint *d_output = NULL;

// Auto-Verification Code
const int frameCheckNumber = 4;
int fpsCount = 0;  // FPS count for averaging
int fpsLimit = 1;  // FPS limit for sampling
int g_Index = 0;
unsigned int frameCount = 0;
unsigned int g_TotalErrors = 0;

int *pArgc = NULL;
char **pArgv = NULL;

extern "C" void initAtlasAndImages(const Image *images, size_t numImages, cudaExtent atlasSize);
extern "C" void deinitAtlasAndImages();
extern "C" void randomizeAtlas();
extern "C" void renderAtlasImage(dim3 gridSize, dim3 blockSize, uint *d_output, uint imageW, uint imageH, float lod);


void computeFPS()
{
    frameCount++;
    fpsCount++;

    if (fpsCount == fpsLimit)
    {
        char fps[256];
        float ifps = 1.f / (sdkGetAverageTimerValue(&timer) / 1000.f);
        sprintf(fps, "%s: %3.1f fps", sSDKsample, ifps);

        glutSetWindowTitle(fps);
        fpsCount = 0;

        fpsLimit = (int)MAX(1.0f, ifps);
        sdkResetTimer(&timer);
    }
}


// render image using CUDA
void render()
{
    // map PBO to get CUDA device pointer
    checkCudaErrors(cudaGraphicsMapResources(1, &cuda_pbo_resource, 0));
    size_t num_bytes;
    checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&d_output, &num_bytes, cuda_pbo_resource));

    // call CUDA kernel, writing results to PBO
    renderAtlasImage(windowGridSize, windowBlockSize, d_output, windowSize.x, windowSize.y, lod);

    getLastCudaError("render_kernel failed");

    checkCudaErrors(cudaGraphicsUnmapResources(1, &cuda_pbo_resource, 0));
}

// display results using OpenGL (called by GLUT)
void display()
{
    sdkStartTimer(&timer);

    render();

    // display results
    glClear(GL_COLOR_BUFFER_BIT);

    // draw image from PBO
    glDisable(GL_DEPTH_TEST);
    glRasterPos2i(0, 0);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
    glDrawPixels(windowSize.x, windowSize.y, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    glutSwapBuffers();
    glutReportErrors();

    sdkStopTimer(&timer);
    computeFPS();
}

void idle()
{
    if (animate)
    {
        lod += 0.02f;
        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27:
            exit(EXIT_SUCCESS);
            break;

        case '=':
        case '+':
            lod += 0.25f;
            break;

        case '-':
            lod -= 0.25f;
            break;

        case 'r':
            randomizeAtlas();
            break;

        case ' ':
            animate = !animate;
            lod = 0.0f;
            break;

        default:
            break;
    }

    glutPostRedisplay();
}

void reshape(int x, int y)
{
    glViewport(0, 0, x, y);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
}

// Global cleanup function
// Shared by both GL and non-GL code paths
void cleanup()
{
    sdkDeleteTimer(&timer);

    // unregister this buffer object from CUDA C
    if (cuda_pbo_resource)
    {
        cudaGraphicsUnregisterResource(cuda_pbo_resource);
        glDeleteBuffersARB(1, &pbo);
    }
}

void cleanup_all()
{
    cleanup();
    deinitAtlasAndImages();
}

void initGLBuffers()
{
    // create pixel buffer object
    glGenBuffersARB(1, &pbo);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, windowSize.x*windowSize.y*sizeof(GLubyte)*4, 0, GL_STREAM_DRAW_ARB);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    // register this buffer object with CUDA
    checkCudaErrors(cudaGraphicsGLRegisterBuffer(&cuda_pbo_resource, pbo, cudaGraphicsMapFlagsWriteDiscard));
}

// Load raw data from disk
uchar *loadRawFile(const char *filename, size_t size)
{
    FILE *fp = fopen(filename, "rb");

    if (!fp)
    {
        fprintf(stderr, "Error opening file '%s'\n", filename);
        return 0;
    }

    uchar *data = (uchar *) malloc(size);
    size_t read = fread(data, 1, size, fp);
    fclose(fp);

    printf("Read '%s', %lu bytes\n", filename, read);

    return data;
}

void initGL(int *argc, char **argv)
{
    // initialize GLUT callback functions
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(windowSize.x, windowSize.y);
    glutCreateWindow(sSDKsample);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    glewInit();

    if (!glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object"))
    {
        fprintf(stderr, "Required OpenGL extensions missing.");
        exit(EXIT_FAILURE);
    }
}

// General initialization call for CUDA Device
int chooseCudaDevice(int argc, char **argv, bool bUseOpenGL)
{
    int result = 0;

    if (bUseOpenGL)
    {
        result = findCudaGLDevice(argc, (const char **)argv);
    }
    else
    {
        result = findCudaDevice(argc, (const char **)argv);
    }

    return result;
}

void runAutoTest(const char *ref_file, char *exec_path)
{
    size_t windowBytes = windowSize.x * windowSize.y * sizeof(GLubyte)*4;

    checkCudaErrors(cudaMalloc((void **)&d_output, windowBytes));

    // render the volumeData
    renderAtlasImage(windowGridSize, windowBlockSize, d_output, windowSize.x, windowSize.y, lod);

    checkCudaErrors(cudaDeviceSynchronize());
    getLastCudaError("render_kernel failed");

    void *h_output = malloc(windowBytes);
    checkCudaErrors(cudaMemcpy(h_output, d_output, windowBytes, cudaMemcpyDeviceToHost));
    sdkDumpBin(h_output, windowBytes, "bindlessTexture.bin");

    bool bTestResult = sdkCompareBin2BinFloat("bindlessTexture.bin", sdkFindFilePath(ref_file, exec_path), windowSize.x*windowSize.y,
                                              MAX_EPSILON_ERROR, THRESHOLD, exec_path);

    checkCudaErrors(cudaFree(d_output));
    free(h_output);
    deinitAtlasAndImages();

    // cudaDeviceReset causes the driver to clean up all state. While
    // not mandatory in normal operation, it is good practice.  It is also
    // needed to ensure correct operation when the application is being
    // profiled. Calling cudaDeviceReset causes all profile data to be
    // flushed before the application exits
    cudaDeviceReset();
    sdkStopTimer(&timer);
    sdkDeleteTimer(&timer);

    exit(bTestResult ? EXIT_SUCCESS : EXIT_FAILURE);
}


void loadImageData(const char *exe_path)
{
    std::vector<Image> images;

    for (size_t i = 0; i < sizeof(imageFilenames)/sizeof(imageFilenames[0]); i++)
    {

        unsigned int imgWidth  = 0;
        unsigned int imgHeight = 0;
        uchar *imgData = NULL;
        const char *imgPath = 0;
        const char *imgFilename = imageFilenames[i];

        if (exe_path)
        {
            imgPath = sdkFindFilePath(imgFilename, exe_path);
        }

        if (imgPath == 0)
        {
            printf("Error finding image file '%s'\n", imgFilename);
            exit(EXIT_FAILURE);
        }

        sdkLoadPPM4(imgPath, (unsigned char **) &imgData, &imgWidth, &imgHeight);

        if (!imgData)
        {
            printf("Error opening file '%s'\n", imgPath);
            exit(EXIT_FAILURE);
        }

        printf("Loaded '%s', %d x %d pixels\n", imgPath, imgWidth, imgHeight);

        checkHost(imgWidth  > 1);
        checkHost(imgHeight > 1);


        Image img;
        img.size = make_cudaExtent(imgWidth,imgHeight,0);
        img.h_data = imgData;
        images.push_back(img);
    }

    initAtlasAndImages(&images[0],images.size(),atlasSize);
}


////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////
int
main(int argc, char **argv)
{
    sdkCreateTimer(&timer);

    pArgc = &argc;
    pArgv = argv;

    char *ref_file = NULL;

    printf("%s Starting...\n\n", sSDKsample);

    if (checkCmdLineFlag(argc, (const char **)argv, "file"))
    {
        fpsLimit = frameCheckNumber;
        getCmdLineArgumentString(argc, (const char **)argv, "file", &ref_file);
    }

    srand(15234);

    if (ref_file)
    {
        chooseCudaDevice(argc, argv, false);
    }
    else
    {
        // First initialize OpenGL context, so we can properly set the GL for CUDA.
        // This is necessary in order to achieve optimal performance with OpenGL/CUDA interop.
        initGL(&argc, argv);

        // use command-line specified CUDA device, otherwise use device with highest Gflops/s
        chooseCudaDevice(argc, argv, true);

        // OpenGL buffers
        initGLBuffers();
    }

    if (!checkCudaCapabilities(3,0))
    {
        cleanup();

        // cudaDeviceReset causes the driver to clean up all state. While
        // not mandatory in normal operation, it is good practice.  It is also
        // needed to ensure correct operation when the application is being
        // profiled. Calling cudaDeviceReset causes all profile data to be
        // flushed before the application exits
        cudaDeviceReset();
        exit(EXIT_SUCCESS);
    }

    loadImageData(argv[0]);

    if (ref_file)
    {
        runAutoTest(ref_file, argv[0]);
    }

    printf(
        "Press space to toggle animation\n"
        "Press '+' and '-' to change lod level\n"
        "Press 'r' to randomize virtual atlas\n");

#if defined (__APPLE__) || defined(MACOSX)
        atexit(cleanup_all);
#else
        glutCloseFunc(cleanup_all);
#endif

    glutMainLoop();

    // cudaDeviceReset causes the driver to clean up all state. While
    // not mandatory in normal operation, it is good practice.  It is also
    // needed to ensure correct operation when the application is being
    // profiled. Calling cudaDeviceReset causes all profile data to be
    // flushed before the application exits
    cudaDeviceReset();
    exit(EXIT_SUCCESS);
}
