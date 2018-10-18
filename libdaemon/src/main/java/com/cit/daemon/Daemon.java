package com.cit.daemon;

public class Daemon
{
    private static boolean isLoaded;

    static
    {
        if (!isLoaded)
        {
            System.loadLibrary("daemon");
            isLoaded = true;
        }
    }

    public final static native void init(String url);
}
