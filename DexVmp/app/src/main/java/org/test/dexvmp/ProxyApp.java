package org.test.dexvmp;

import android.app.Application;

public class ProxyApp extends Application {

    static {
        System.loadLibrary("dexvmp");
    }

    public static native int interface1(Object[] args);
}
