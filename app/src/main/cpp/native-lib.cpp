#include <jni.h>

#include "engine/Application.hpp"

static Application app;

extern "C"
JNIEXPORT void JNICALL
Java_com_handfx_MainActivity_startEngine(
        JNIEnv* env,
        jobject obj
) {
    app.initialize();
}
