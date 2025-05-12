#include <errno.h>
#include <memory>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <cdlog.h>
#include <malloc.h>
#include <jni.h>
#include <android/sensor.h>
#include <android/asset_manager.h>
#include <android/choreographer.h>
#include <android/asset_manager_jni.h>
#include "android_native_app_glue.h"

/* Our saved state data.*/
struct saved_state {
    float angle; /* RGB 中的绿色值 */
    int32_t x;     /* X 坐标 */
    int32_t y;     /* Y 坐标 */
};

/*Shared state for our app.*/
struct engine {
    struct android_app *app;
    ASensorManager *sensorManager;/* sensor.h */
    const ASensor *accelerometerSensor;
    ASensorEventQueue *sensorEventQueue;
    AChoreographer* choreographer;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int animating;
    int32_t width;
    int32_t height;
    struct saved_state state;
};

/*Initialize an EGL context for the current display.*/
static int engine_init_display(struct engine *engine) {
    LOGI("engine_init_display");
    /* initialize OpenGL ES and EGL*/
    const EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                              EGL_BLUE_SIZE, 8,
                              EGL_GREEN_SIZE, 8,
                              EGL_RED_SIZE, 8,
                              EGL_NONE};

    EGLint w,h;
    EGLint major,minor;
    EGLint format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY); /* 得到系统默认的 */

    eglInitialize(display, &major, &minor); /* 获取主次版本号 - 不关心可设为 NULL 值或零(0) */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs); /* 系统中 Surface 的 EGL 配置 的总个数 */
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
    LOGI("Screen Size =%dx%d", w, h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;

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
    /* Just fill the screen with a color.*/
    //glClearColor(((float) engine->state.x) / engine->width, engine->state.angle,((float) engine->state.y) / engine->height, 1);
    glClear(GL_COLOR_BUFFER_BIT);
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
            eglDestroySurface(engine->display,engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

/*Process the next input event.*/
static int32_t engine_handle_input(struct android_app *app, AInputEvent *event) {
    struct engine *engine = (struct engine *) app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;
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

            /* Also stop animating.*/
            engine->animating = 0;
            engine_draw_frame(engine);

            break;
    }
}

// Choreographer 回调函数
void onVsyncCallback(long frameTimeNanos, void* data) {
    struct android_app *app =(struct android_app*)data;
    struct engine*eng=(struct engine*)app->userData;
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(eng->display, eng->surface);
    AChoreographer_postFrameCallback(eng->choreographer, onVsyncCallback, app);
}

void android_main(struct android_app *state) {
    LOGI("android_main,start Activity...");
    struct engine engine;

    //app_cdroid();//

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    /* Prepare to monitor accelerometer.*/
    engine.sensorManager = ASensorManager_getInstance();

    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);
    engine.choreographer = AChoreographer_getInstance();
    AChoreographer_postFrameCallback(engine.choreographer, onVsyncCallback, state);

    if (state->savedState != NULL) {
        /* We are starting with a previous saved state;restore from it.*/
        engine.state = *(struct saved_state *) state->savedState;
    }

    /* loop waiting for stuff to do.*/
    while (1) {
        int ident,events;
        struct android_poll_source *source;

        while ((ident = ALooper_pollOnce(engine.animating ? 0 : -1, NULL, &events, (void **) &source)) >= 0) {
            if (source != NULL) {/* Process this event.*/
                source->process(state, source);
            }

            /* If a sensor has data, process it now.*/
            if ((ident == LOOPER_ID_USER) &&(engine.accelerometerSensor != NULL)) {
                ASensorEvent event;
                while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &event, 1) > 0) {
                    LOGI("accelerometer: x=%f y=%f z=%f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
                }
            }

            /* Check if we are exiting.*/
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }
    }
}

