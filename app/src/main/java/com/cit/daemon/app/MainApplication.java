package com.cit.daemon.app;

import android.app.Application;

import com.cit.daemon.Daemon;

public class MainApplication extends Application
{
    @Override
    public void onCreate()
    {
        super.onCreate();
        Daemon.init("https://github.com/chengit163");
    }
}
