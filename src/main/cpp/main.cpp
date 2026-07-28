#include <android_native_app_glue.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraCaptureSession.h>
#include <media/NdkImageReader.h>
#include <media/NdkImage.h>
#include <string>
#include <vector>

#include "hand_tracker.h"
#include "ui/imgui.h"
#include "ui/imgui_impl_android.h"
#include "ui/imgui_impl_opengl3.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "HandFX", __VA_ARGS__)

struct AppState {
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext context = EGL_NO_CONTEXT;
    ACameraManager* camManager = nullptr;
    ACameraDevice* camDevice = nullptr;
    ACameraCaptureSession* camSession = nullptr;
    AImageReader* reader = nullptr;
    GLuint neonProg = 0;
    GLuint texY = 0, texUV = 0;
    GLuint quadVBO = 0;
    float handPos[2] = {0.5f, 0.5f};
    bool isRecording = false;
    float glowStrength = 1.0f;
};

static std::string LoadFile(AAssetManager* mgr, const char* path){
    AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_BUFFER);
    if(!asset) return "";
    size_t len = AAsset_getLength(asset);
    std::string s(len, '\0');
    AAsset_read(asset, s.data(), len);
    AAsset_close(asset);
    return s;
}

static GLuint CompileShader(GLenum type, const std::string& src){
    GLuint sh = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(sh, 1, &c, nullptr);
    glCompileShader(sh);
    GLint ok; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if(!ok){ char log[512]; glGetShaderInfoLog(sh,512,nullptr,log); LOGI("Shader err: %s",log); }
    return sh;
}

static GLuint CreateProgram(const std::string& vs, const std::string& fs){
    GLuint p = glCreateProgram();
    GLuint v = CompileShader(GL_VERTEX_SHADER, vs);
    GLuint f = CompileShader(GL_FRAGMENT_SHADER, fs);
    glAttachShader(p,v); glAttachShader(p,f);
    glLinkProgram(p);
    glDeleteShader(v); glDeleteShader(f);
    return p;
}

void android_main(struct android_app* app){
    AppState state;

    // === 1. INIT EGL (sekali) ===
    state.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(state.display, nullptr, nullptr);
    EGLConfig cfg; EGLint ncfg;
    EGLint att[] = {EGL_RENDERABLE_TYPE,EGL_OPENGL_ES3_BIT,EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_BLUE_SIZE,8,EGL_GREEN_SIZE,8,EGL_RED_SIZE,8,EGL_NONE};
    eglChooseConfig(state.display, att, &cfg, 1, &ncfg);
    state.surface = eglCreateWindowSurface(state.display, cfg, app->window, nullptr);
    EGLint ctxAtt[] = {EGL_CONTEXT_CLIENT_VERSION,3,EGL_NONE};
    state.context = eglCreateContext(state.display, cfg, EGL_NO_CONTEXT, ctxAtt);
    eglMakeCurrent(state.display, state.surface, state.surface, state.context);

    // === 2. INIT SHADER NEON (sekali) ===
    AAssetManager* assetMgr = app->activity->assetManager;
    std::string vertSrc = R"(#version 300 es
        layout(location=0) in vec2 a_pos; out vec2 v_uv;
        void main(){ v_uv=a_pos*0.5+0.5; gl_Position=vec4(a_pos,0,1); })";
    std::string fragSrc = LoadFile(assetMgr, "shaders/neon.glsl");
    if(fragSrc.empty()) fragSrc = LoadFile(assetMgr, "neon.glsl");
    state.neonProg = CreateProgram(vertSrc, fragSrc);

    // Quad
    float quad[] = {-1,-1, 1,-1, -1,1, 1,1};
    glGenBuffers(1,&state.quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, state.quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glGenTextures(1,&state.texY); glGenTextures(1,&state.texUV);

    // === 3. INIT IMGUI (sekali) ===
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplAndroid_Init(app->window);
    ImGui_ImplOpenGL3_Init("#version 300 es");
    ImGui::StyleColorsDark();

    // === 4. INIT HAND TRACKER + KAMERA (sekali) ===
    hand_tracker_init(assetMgr, "hand_landmarker_lite.tflite");
    state.camManager = ACameraManager_create();
    AImageReader_new(1280,720,AIMAGE_FORMAT_YUV_420_888,4,&state.reader);

    // callback kamera disederhanakan, buka kamera belakang id 0
    ACameraManager_openCamera(state.camManager, "0",
        AImageReader_getWindow(state.reader)? nullptr : nullptr, &state.camDevice);

    LOGI("HandFX INIT DONE");

    // === 5. LOOP 60FPS ===
    while(!app->destroyRequested){
        ALooper_pollOnce(0,nullptr,nullptr,nullptr);

        // ambil frame kamera
        AImage* image = nullptr;
        if(AImageReader_acquireLatestImage(state.reader, &image) == AMEDIA_OK){
            uint8_t *yData, *uvData; int yLen, uvLen;
            AImage_getPlaneData(image,0,&yData,&yLen);
            AImage_getPlaneData(image,1,&uvData,&uvLen);
            int w,h; AImage_getWidth(image,&w); AImage_getHeight(image,&h);

            // upload Y dan UV jadi texture
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, state.texY);
            glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,w,h,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,yData);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

            // deteksi tangan
            hand_detect(yData, uvData, w, h, state.handPos);
            AImage_delete(image);
        }

        // ImGui Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(20,20));
        ImGui::Begin("HandFX", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
        if(!state.isRecording){
            if(ImGui::Button("REC START", ImVec2(150,50))){ state.isRecording=true; }
        } else {
            if(ImGui::Button("STOP", ImVec2(150,50))){ state.isRecording=false; }
            ImGui::Text("REC...");
        }
        ImGui::SliderFloat("Glow", &state.glowStrength, 0.0f, 3.0f);
        ImGui::End();

        // Render
        glViewport(0,0,1280,720);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(state.neonProg);
        glBindBuffer(GL_ARRAY_BUFFER, state.quadVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,0);
        glUniform2f(glGetUniformLocation(state.neonProg,"u_hand"), state.handPos[0], state.handPos[1]);
        glUniform1f(glGetUniformLocation(state.neonProg,"u_glow"), state.glowStrength);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, state.texY);
        glUniform1i(glGetUniformLocation(state.neonProg,"u_yTex"),0);
        glDrawArrays(GL_TRIANGLE_STRIP,0,4);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        eglSwapBuffers(state.display, state.surface);
    }

    // cleanup
    ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplAndroid_Shutdown(); ImGui::DestroyContext();
    eglDestroySurface(state.display, state.surface);
    eglDestroyContext(state.display, state.context);
}
