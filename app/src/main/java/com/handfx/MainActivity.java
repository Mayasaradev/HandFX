package com.handfx;

import android.app.Activity;
import android.os.Bundle;

public class MainActivity extends Activity {

    static {
        System.loadLibrary("handfx");
    }

    public native void startEngine();

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);

        startEngine();
    }
}
