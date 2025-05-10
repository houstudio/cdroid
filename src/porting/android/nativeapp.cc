#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <memory>
#include <cdlog.h>
#include "android_native_app_glue.h"
namespace napp {

template <typename F>
auto ensure(F&& func){
    class _Ensure {
        F _func;
    public:
        _Ensure(F&& func)
            : _func(std::move(func)){}
        ~_Ensure() { _func(); }
    };
    return _Ensure(std::move(func));
};

struct NativeApp {
    struct android_app* _app;
    bool _initialized { false };

    EGLDisplay _egl_display { EGL_NO_DISPLAY };
    EGLConfig _egl_context { EGL_NO_CONTEXT };
    EGLSurface _egl_surface { EGL_NO_SURFACE };

    static void onAppCmd(struct android_app* app, int32_t cmd){
        auto* self = reinterpret_cast<NativeApp*>(app->userData);
        switch (cmd) {
        case APP_CMD_CONFIG_CHANGED:
            LOGI("density %d", AConfiguration_getDensity(app->config));
            AConfiguration_setDensity(app->config, ACONFIGURATION_DENSITY_ANY);
            break;
        case APP_CMD_SAVE_STATE:
            break;
        case APP_CMD_INIT_WINDOW:
            self->init();
            break;
        case APP_CMD_TERM_WINDOW:
            self->shutdown();
            break;
        }
    }

    static int32_t onInputEvent(struct android_app*, AInputEvent* input_event){
        return 0;//ImGui_ImplAndroid_HandleInputEvent(input_event);
    }

    void init(){
        if (_initialized) {
            return;
        }

        ANativeWindow_acquire(_app->window);

        _egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (_egl_display == EGL_NO_DISPLAY) {
            LOGE("eglGetDisplay failed");
            return;
        }

        if (eglInitialize(_egl_display, 0, 0) != EGL_TRUE) {
            LOGE("eglInitialize failed");
            return;
        }

        const EGLint egl_attributes[] = {
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 16,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
        };

        EGLint num_configs{0};
        if (eglChooseConfig(_egl_display, egl_attributes, nullptr, 0, &num_configs) != EGL_TRUE) {
            LOGE("eglChooseConfig failed");
            return;
        }

        if (num_configs == 0) {
            LOGE("eglChooseConfig returns no config");
            return;
        }

        EGLConfig egl_config{0};
        eglChooseConfig(_egl_display, egl_attributes, &egl_config, 1, &num_configs);

        EGLint egl_format{0};
        eglGetConfigAttrib(_egl_display, egl_config, EGL_NATIVE_VISUAL_ID, &egl_format);

        ANativeWindow_setBuffersGeometry(_app->window, 0, 0, egl_format);

        const EGLint egl_context_attrs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
        };

        _egl_context = eglCreateContext(_egl_display, egl_config, EGL_NO_CONTEXT, egl_context_attrs);
        if (_egl_context == EGL_NO_CONTEXT) {
            LOGE("eglCreateContext failed");
            return;
        }

        _egl_surface = eglCreateWindowSurface(_egl_display, egl_config, _app->window, nullptr);
        if (_egl_surface == EGL_NO_SURFACE) {
            LOGE("eglCreateWindowSurface failed");
            return;
        }

        if (eglMakeCurrent(_egl_display, _egl_surface, _egl_surface, _egl_context) != EGL_TRUE) {
            LOGE("eglMakeCurrent failed");
            return;
        }
#if 0
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();

        ImGui_ImplAndroid_Init(_app->window);
        ImGui_ImplOpenGL3_Init("#version 300 es");

        float scale = 1.0f;

        {
            auto& io = ImGui::GetIO();
            float window_width = ANativeWindow_getWidth(_app->window);
            float window_height = ANativeWindow_getHeight(_app->window);

            scale = window_width / AConfiguration_getScreenWidthDp(_app->config);
        }

        LOGI("scale %f", scale);

        const auto font_size = 22.0f * scale;

        if (std::filesystem::exists(ANDROID_DEFAULT_FONT)) {
            io.FontDefault = io.Fonts->AddFontFromFileTTF(ANDROID_DEFAULT_FONT, font_size);
        } else {
            ImFontConfig font_cfg{};
            font_cfg.SizePixels = font_size;
            io.Fonts->AddFontDefault(&font_cfg);
        }
        
        ImGui::GetStyle().ScaleAllSizes(scale);
#endif
        LOGI("initialized");

        _initialized = true;
    }

    void shutdown(){
        if (not _initialized)
            return;
#if 0
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplAndroid_Shutdown();
        ImGui::DestroyContext();
#endif
        if (_egl_display != EGL_NO_DISPLAY) {
            eglMakeCurrent(_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

            if (_egl_context != EGL_NO_CONTEXT) {
                eglDestroyContext(_egl_display, _egl_context);
                _egl_context = EGL_NO_CONTEXT;
            }

            if (_egl_surface != EGL_NO_SURFACE) {
                eglDestroySurface(_egl_display, _egl_surface);
                _egl_surface = EGL_NO_SURFACE;
            }

            eglTerminate(_egl_display);
            _egl_display = EGL_NO_DISPLAY;
        }
        
        ANativeWindow_release(_app->window);
        _initialized = false;
    }

    void tick(){
        if (not _initialized) {
            return;
        }
        
#if 0
        const ImVec4 clear_color{ 0.45f, 0.55f, 0.60f, 1.00f };
        auto& io = ImGui::GetIO();
        if (_egl_display == EGL_NO_DISPLAY) {
            return;
        }

        ImGui_ImplAndroid_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

#if 0
        ImGui::Begin("Hello", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::SetWindowPos({0.0f, 0.0f});
        ImGui::SetWindowSize(io.DisplaySize);
        ImGui::Text("This is some useful text.");
        ImGui::End();
#else
        ImGui::ShowDemoWindow();
#endif
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.0f, 0.0f,0.0f, .7f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        eglSwapBuffers(_egl_display, _egl_surface);
#endif
    }
};

} // namespace napp
void android_main(struct android_app* app){
#if 10
    using namespace napp;
    NativeApp napp {};
    napp._app = app;
    app->userData = &napp;
    app->onAppCmd = NativeApp::onAppCmd;
    app->onInputEvent = NativeApp::onInputEvent;

    while (true) {
        int ident;
        int events;
        struct android_poll_source* source;
        while ((ident = ALooper_pollOnce(napp._initialized ? 0 : -1, nullptr, &events, (void**)&source)) >= 0) {
            if (source != nullptr) {
                source->process(app, source);
            }

            if (app->destroyRequested != 0) {
                napp.shutdown();
            }
        }
        napp.tick();
    }
#endif
}
