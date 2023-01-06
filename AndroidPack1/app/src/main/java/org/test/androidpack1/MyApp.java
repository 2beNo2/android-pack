package org.test.androidpack1;

import android.app.Application;
import android.content.Context;

public class MyApp extends Application {

    static {
        System.loadLibrary("androidpack1");
    }

    @Override
    public native void onCreate();

    @Override
    protected native void attachBaseContext(Context base);
}
