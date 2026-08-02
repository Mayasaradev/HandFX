#include <jni.h>
#include "engine/Application.hpp"

extern "C"
JNIEXPORT void JNICALL
Java_com_handfx_MainActivity_initEngine(
        JNIEnv*,
        jobject)
{
    HandFX::Application::init();
}
