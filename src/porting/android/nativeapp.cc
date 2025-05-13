#include <errno.h>
#include <memory>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <cdlog.h>
#include <malloc.h>
#include <jni.h>
#include <android/sensor.h>
#include <android/asset_manager.h>
#include <android/choreographer.h>
#include <android/asset_manager_jni.h>
#include "android_native_app_glue.h"
//REF:https://github.com/dd-Dog/NativeActivity-Test/blob/master/src/main/cpp/main.c
struct saved_state {/* Our saved state data.*/
    float angle; /* RGB 中的绿色值 */
    int32_t x;   /* X 坐标 */
    int32_t y;   /* Y 坐标 */
};

/*Shared state for our app.*/
struct engine {
    struct android_app *app;
    AChoreographer *choreographer;
    ASensorManager *sensorManager;
    const ASensor *accelerometerSensor;
    ASensorEventQueue *sensorEventQueue;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    EGLint shaderProgram;
    struct saved_state state;
};
extern "C" int primarySurfaceTexture;/*defined in graph_egl*/
static GLuint createShaderProgram();
static void getScreenSize(struct android_app *state);
static void drawTexture(GLuint shaderProgram, GLuint textureId);
/*Initialize an EGL context for the current display.*/
static int engine_init_display(struct engine *engine) {
    LOGI("engine_init_display");
    /* initialize OpenGL ES and EGL*/
    const EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                              EGL_BLUE_SIZE, 8,
                              EGL_GREEN_SIZE, 8,
                              EGL_RED_SIZE, 8,
                              EGL_NONE};

    EGLint w , h;
    EGLint major , minor;
    EGLint format, numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, &major, &minor);
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);
    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);  /* 属性列表可以是空，使用默认值 */

    if (EGL_FALSE == eglMakeCurrent(display, surface, surface, context)) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height= h;
    engine->shaderProgram = createShaderProgram();

    glHint(GL_PROGRAM_BINARY_RETRIEVABLE_HINT,GL_FASTEST); /* 指定颜色和纹理坐标的插值质量 使用速度最快的模式 */
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    return 0;
}

/*Just the current frame in the display.*/
static void engine_draw_frame(struct engine *engine) {
    LOGI("engine_draw_frame");
    if (engine->display == NULL) {/* No display.*/
        return;
    }
    glClear(GL_COLOR_BUFFER_BIT);

    /*glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, primarySurfaceTexture);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f, 0.0f);
    glEnd();*/
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    eglSwapBuffers(engine->display,engine->surface);
}

/*Tear down the EGL context currently associated with the display.*/
static void engine_term_display(struct engine *engine) {
    LOGI("engine_term_display");
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

static int32_t engine_handle_input(struct android_app *app, AInputEvent *event) {
    struct engine *engine = (struct engine *) app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        AMotionEvent_getX(event, 0);
        AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}

/*Process the next main command.*/
static void engine_handle_cmd(struct android_app *app, int32_t cmd) {
    struct engine *engine = (struct engine *) app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            LOGI("engine_handle_cmd,cmd=APP_CMD_SAVE_STATE");
            /* The system has asked us to save our current state.Do so.*/
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state *) engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);

            break;

        case APP_CMD_INIT_WINDOW:
            LOGI("engine_handle_cmd,cmd=APP_CMD_INIT_WINDOW");
            /* The window is being shown, get it ready.*/
            if (engine->app->window != NULL) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }

            break;

        case APP_CMD_TERM_WINDOW:
            LOGI("engine_handle_cmd,cmd=APP_CMD_TERM_WINDOW");
            /* The window is being hidden or closed, clean it up.*/
            engine_term_display(engine);
            break;

        case APP_CMD_GAINED_FOCUS:
            LOGI("engine_handle_cmd,cmd=APP_CMD_GAINED_FOCUS");
            /* When our app gains focus, we start monitoring the accelerometer. */
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
                /* We'd like to get 60 events per second (in us).*/
                ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->accelerometerSensor, (1000L / 60) * 1000);
            }
            break;

        case APP_CMD_LOST_FOCUS:
            LOGI("engine_handle_cmd,cmd=APP_CMD_LOST_FOCUS");
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
            }
            engine_draw_frame(engine);
            break;
    }
}

static void onVsyncCallback(long frameTimeNanos, void* data) {
    struct android_app *app =(struct android_app*)data;
    struct engine*eng=(struct engine*)app->userData;
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    if(eng->shaderProgram!=0)
        drawTexture(eng->shaderProgram,primarySurfaceTexture);
    eglSwapBuffers(eng->display, eng->surface);
    AChoreographer_postFrameCallback(eng->choreographer, onVsyncCallback, app);
}

void android_main(struct android_app *state) {
    LOGI("android_main,start Activity...");
    struct engine engine;
    memset(&engine, 0, sizeof(engine));
    getScreenSize(state);
    app_dummy();//
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;

    engine.app = state;
    primarySurfaceTexture=12;
    engine.sensorManager = ASensorManager_getInstance();/* Prepare to monitor accelerometer.*/
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);
    engine.choreographer = AChoreographer_getInstance();
    AChoreographer_postFrameCallback(engine.choreographer, onVsyncCallback, state);

    if (state->savedState != NULL) {/*We are starting with a previous saved state;restore from it.*/
        engine.state = *(struct saved_state *) state->savedState;
    }

    while (1) {/* loop waiting for stuff to do.*/
        int ident,events;
        struct android_poll_source *source;

        while ((ident = ALooper_pollOnce( 0, NULL, &events, (void **) &source)) >= 0) {
            if (source != NULL) {/* Process this event.*/
                source->process(state, source);
            }

            if ((ident == LOOPER_ID_USER) && (engine.accelerometerSensor != NULL) ) {/* If a sensor has data, process it now.*/
                ASensorEvent event;
                while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &event, 1) > 0) {
                    LOGI("accelerometer: x=%f y=%f z=%f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
                }
            }

            if (state->destroyRequested != 0) {/*Check if we are exiting.*/
                engine_term_display(&engine);
                return;
            }
        }
    }
}

static GLuint createShaderProgram(){//const char* vertexShaderSource, const char* fragmentShaderSource) {
    const char* vertexShaderSource =
        "#version 300 es\n"
        "in vec4 a_position;\n"
        "in vec2 a_texCoord;\n"
        "out vec2 v_texCoord;\n"
        "void main() {\n"
        "    gl_Position = a_position;\n"
        "    v_texCoord = a_texCoord;\n"
        "}\n";

    const char* fragmentShaderSource =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec2 v_texCoord;\n"
        "uniform sampler2D u_texture;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    fragColor = texture(u_texture, v_texCoord);\n"
        "}\n";
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


static void drawTexture(GLuint shaderProgram, GLuint textureId) {
    const GLfloat vertices[] = {
        -1.0f, -1.0f,   1.0f, -1.0f,
         1.0f,  1.0f,   -1.0f,  1.0f
    };

    const GLfloat texCoords[] = {
        0.0f, 0.0f,  1.0f, 0.0f,
        1.0f, 1.0f,  0.0f, 1.0f
    };
    glUseProgram(shaderProgram);

    GLint positionHandle = glGetAttribLocation(shaderProgram, "a_position");
    GLint texCoordHandle = glGetAttribLocation(shaderProgram, "a_texCoord");
    GLint textureHandle = glGetUniformLocation(shaderProgram, "u_texture");

    glEnableVertexAttribArray(positionHandle);
    glVertexAttribPointer(positionHandle, 2, GL_FLOAT, GL_FALSE, 0, vertices);

    glEnableVertexAttribArray(texCoordHandle);
    glVertexAttribPointer(texCoordHandle, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(textureHandle, 0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(positionHandle);
    glDisableVertexAttribArray(texCoordHandle);
}

static void getScreenSize(struct android_app *state){
    JavaVM *vm = state->activity->vm;
    JNIEnv *env = state->activity->env;

    jobject activity = state->activity->clazz;
    jclass displayMetricsClass = env->FindClass("android/util/DisplayMetrics");
    if (displayMetricsClass == NULL) {
        LOGI("Failed to find DisplayMetrics class");
        return;
    }

    jobject displayMetrics = env->AllocObject(displayMetricsClass);
    if (displayMetrics == NULL) {
        LOGI("Failed to allocate DisplayMetrics object");
        env->DeleteLocalRef(displayMetricsClass);
        return;
    }

    jclass activityClass = env->GetObjectClass(activity);
    jmethodID getResourcesMethod = env->GetMethodID(activityClass, "getResources", "()Landroid/content/res/Resources;");
    if (getResourcesMethod == NULL) {
        LOGI("Failed to find getResources method");
        env->DeleteLocalRef(displayMetricsClass);
        env->DeleteLocalRef(displayMetrics);
        env->DeleteLocalRef(activityClass);
        return;
    }

    jobject resources = env->CallObjectMethod(activity, getResourcesMethod);
    if (resources == NULL) {
        LOGI("Failed to get Resources object");
        env->DeleteLocalRef( displayMetricsClass);
        env->DeleteLocalRef( displayMetrics);
        env->DeleteLocalRef( activityClass);
        return;
    }

    jclass resourcesClass = env->GetObjectClass(resources);
    jmethodID getDisplayMetricsMethod = env->GetMethodID(resourcesClass, "getDisplayMetrics", "()V");
    if (getDisplayMetricsMethod == NULL) {
        LOGI("Failed to find getDisplayMetrics method");
        env->DeleteLocalRef(displayMetricsClass);
        env->DeleteLocalRef(displayMetrics);
        env->DeleteLocalRef(activityClass);
        env->DeleteLocalRef(resources);
        env->DeleteLocalRef(resourcesClass);
        return;
    }

    env->CallVoidMethod(resources, getDisplayMetricsMethod, displayMetrics);

    jfieldID widthPixelsField = env->GetFieldID(displayMetricsClass, "widthPixels", "I");
    jfieldID heightPixelsField = env->GetFieldID(displayMetricsClass, "heightPixels", "I");
    if (widthPixelsField == NULL || heightPixelsField == NULL) {
        LOGI("Failed to find widthPixels or heightPixels field");
        env->DeleteLocalRef(displayMetricsClass);
        env->DeleteLocalRef(displayMetrics);
        env->DeleteLocalRef(activityClass);
        env->DeleteLocalRef(resources);
        env->DeleteLocalRef(resourcesClass);
        return;
    }

    const int widthPixelSize = env->GetIntField(displayMetrics, widthPixelsField);
    const int heightPixelSize= env->GetIntField(displayMetrics, heightPixelsField);
    char sizevalue[128];
    sprintf(sizevalue,"%dx%d",widthPixelSize,heightPixelSize);
    setenv("SCREEN_SIZE",sizevalue,1);
    LOGI("Screen Size=%dx%d", widthPixelSize,heightPixelSize);

    env->DeleteLocalRef(displayMetricsClass);
    env->DeleteLocalRef(displayMetrics);
    env->DeleteLocalRef(activityClass);
    env->DeleteLocalRef(resources);
    env->DeleteLocalRef(resourcesClass);
}
