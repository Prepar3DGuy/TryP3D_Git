# TryP3D_Git

# Introduction

The purpose of this repository is to demonstrate some simple technics for interaction of simple C++ application (Direct2D in particular) with Prepar3D using SimConnect. I want to use this repository for answer on the forum https://www.fsdeveloper.com/forum/threads/2d-universal-panel.445554/ 

The main idea is that using information from ```add-on.xml``` file Prepar3D can start exe application and also pass command line arguments to it ([See Add-on Packages rules in SDK](http://www.prepar3d.com/SDKv4/sdk/add-ons/add-on_packages.html)). Receiving argument ```internal``` this example application doesn't show window but request SimConnect to add menu item using [SimConnect_MenuAddItem](http://www.prepar3d.com/SDKv4/sdk/simconnect_api/references/general_functions.html#SimConnect_MenuAddItem). When application receive client event registered with menu item it will show or hide window. When window is hiden there is no icon on taskbar. Application is running from the moment after Prepar3D start until it exit, you can find the process Universal2DPanel.exe in task manager. 

# Requirements
* [Prepar3D v4](https://prepar3d.com/) 
* [Prepar3D SDK](https://prepar3d.com/support/sdk/)
* Visual Studio 2015 (64bit C++ compiler)

# Build and installation

1. For portability this repository use User Environment Variables P3D and P3D_SDK. 

On my system they are:
```
P3D = D:\Program Files\Lockheed Martin\Prepar3D v4
P3D_SDK = D:\Program Files\Lockheed Martin\Prepar3D v4 SDK 4.0.28.21686
```

2. Succesfully build solution in Visual Studio.

3. For development purpose just ADD output directory as add-on to Prepar3D once using command (change Path value to your output directory)
```
"%P3D%\Prepar3D.exe" "-Configure: Category=Add-on Package, Operation=Add, Title=Universal2DPanel, Path=D:\P3D_Git\x64\Debug"
```

4. Start Prepar3D and use main menu Add-ons -> Universal 2DPanel to show/hide application window. 

5. If you want to debug application inside Visual Studio use menu Debug -> Attach to Process and select Universal2DPanel.exe, set breakpoints and so on.

6. If you already played with this just REMOVE add-on from Prepar3D using command (cmd or just inside Win+R window)
```
"%P3D%\Prepar3D.exe" "-Configure: Category=Add-on Package, Operation=Remove, Title=Universal2DPanel"
```

7. By the way you can just copy the output directory to the ```%USERPROFILE%\Documents\Prepar3D v4 Add-ons``` folder. 

8. If you doesn't want to se add-on technics or copy folders simply start exe file when Prepar3D is running.
