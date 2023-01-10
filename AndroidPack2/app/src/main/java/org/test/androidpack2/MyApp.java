package org.test.androidpack2;

import android.app.Application;
import android.content.Context;

public class MyApp extends Application {
    static {
        System.loadLibrary("androidpack2");
    }

    @Override
    public native void onCreate();

    @Override
    protected native void attachBaseContext(Context base);
}
