#include <jni.h>
#include <string>

extern "C"
JNIEXPORT jstring JNICALL
Java_com_handfx_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject) {

    std::string text = "HandFX Native Engine Running";

    return env->NewStringUTF(text.c_str());
}
