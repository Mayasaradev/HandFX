#include "hand_tracker.h"
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model.h>
#include <android/log.h>
#include <cmath>

static std::unique_ptr<tflite::FlatBufferModel> model;
static std::unique_ptr<tflite::Interpreter> interpreter;
static float last_xy[2] = {0.5f,0.5f};

void hand_tracker_init(AAssetManager* mgr, const char* modelPath){
    AAsset* a = AAssetManager_open(mgr, modelPath, AASSET_MODE_BUFFER);
    if(!a){ __android_log_print(6,"HandFX","model not found"); return; }
    size_t len = AAsset_getLength(a);
    std::vector<char> buf(len);
    AAsset_read(a, buf.data(), len);
    AAsset_close(a);

    model = tflite::FlatBufferModel::BuildFromBuffer(buf.data(), len);
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder(*model, resolver)(&interpreter);
    interpreter->AllocateTensors();
    __android_log_print(4,"HandFX","TFLite init ok");
}

void hand_detect(uint8_t* y, uint8_t* uv, int w, int h, float* out_xy){
    if(!interpreter){
        // fallback dummy biar gak crash pas awal
        static float t=0; t+=0.02f;
        out_xy[0]=0.5f+0.3f*sinf(t); out_xy[1]=0.5f+0.3f*cosf(t);
        return;
    }
    // Resize Y 1280x720 -> 192x192 input model hand_landmarker_lite
    // (disederhanakan, nanti lu ganti pakai libyuv)
    float* input = interpreter->typed_input_tensor<float>(0);
    // isi input dari yData (dummy normalized)
    for(int i=0;i<192*192;i++) input[i] = y[(i*w/192 + (i/192)*w)%(w*h)] / 255.0f;

    interpreter->Invoke();
    float* out = interpreter->typed_output_tensor<float>(0); // [1,1,2] xy
    // low-pass biar gak geter
    last_xy[0] = last_xy[0]*0.7f + out[0]*0.3f;
    last_xy[1] = last_xy[1]*0.7f + out[1]*0.3f;
    out_xy[0]=last_xy[0]; out_xy[1]=last_xy[1];
}
