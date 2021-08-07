General
=======
These samples assume that a recent version of Orthanc is checked out in an
`orthanc` folder next to the `orthanc-stone` folder. Let's call the top folder
the `devroot` folder. This name does not matter and is not used anywhere.

Here's the directory layout that we suggest:

```
devroot/
 |
 +- orthanc/
 |
 +- orthanc-stone/
 |
 ...
```

 Orthanc can be retrieved with:
 ```
 hg clone https://hg.orthanc-server.com/orthanc
 ```

Furthermore, the samples usually assume that an Orthanc is running locally,
without authentication, on port 8042. The samples can easily be tweaked if 
your setup is different.

When Dicom resources are to be displayed, their IDs can be supplied in the 
various ways suitable for the platform (command-line arguments, URL parameters
or through the GUI)


This repo contains two sample projects:

SingleFrameViewer
-----------------

This sample application displays a single frame of a Dicom instance that can
be loaded from Orthanc, either by using the Orthanc REST API or through the 
Dicomweb server functionality of Orthanc.

RtViewer
--------

This sample application displays set of Radiotherapy data:
- a CT scan
- an RT-Dose
- an RT-Struct

The WebAssembly version displays 3 viewports with MPR data
while the SDL sample displays a single viewport.

 
WebAssembly samples
===================

Building the WebAssembly samples require the Emscripten SDK 
(https://emscripten.org/). This SDK goes far beyond the simple compilation to
the wasm (Web Assembly) bytecode and provides a comprehensive library that 
eases porting native C and C++ programs and libraries. The Emscripten SDK also
makes it easy to generate the companion Javascript files requires to use a 
wasm module in a web application.

Although Emscripten runs on all major platforms, Stone of Orthanc is developed
and tested with the Linux version of Emscripten.

Emscripten runs perfectly fine under the Windows Subsystem for Linux (that is
the environment used quite often by the Stone of Orthanc team)

**Important note:** The following examples **and the build scripts** will 
assume that you have installed the Emscripten SDK in `~/apps/emsdk`.

The following packages should get you going (a Debian-like distribution such 
as Debian or Ubuntu is assumed)

```
sudo apt-get update 
sudo apt-get install -y build-essential curl wget git python cmake pkg-config
sudo apt-get install -y mercurial unzip npm ninja-build p7zip-full gettext-base 
```

To build the Wasm samples, just launch the `build-wasm-samples.sh` script from
this folder.  Optionaly, you can pass the build type as an argument.
We suggest that you do *not* use the `Debug` configuration unless you really 
need it, for the additional checks that are made will lead to a very long 
build time and much slower execution (more severe than with a native non-wasm
target)

In order to run the sample, you may serve it with the ServeFolders plugin.
You can i.e: add such a section in your orthanc configuration file:

```
{
  "Plugins" : ["LibServeFolders.so],
  "ServeFolders" : {
    "/single-frame-viewer" : "..../out/install-stone-wasm-samples-RelWithDebInfo/SingleFrameViewer",
    "/rt-viewer": "..../out/install-stone-wasm-samples-RelWithDebInfo/RtViewer"
  }
}
```

You'll then be able to open the single-frame-viewer demo at `http://localhost:8042/single-frame-viewer/index.html` 

The rt-viewer demo at
`http://localhost:8044/rt-viewer/index.html?ctseries=a04ecf01-79b2fc33-58239f7e-ad9db983-28e81afa&rtdose=eac822ef-a395f94e-e8121fe0-8411fef8-1f7bffad&rtstruct=54460695-ba3885ee-ddf61ac0-f028e31d-a6e474d9`.  Note that you must provide 3 ids in the url:

- the CT series Orthanc ID
- the RT-Dose instance Orthanc ID
- the RT-Struct instance Orthanc ID


RtViewer
-----------------

This sample application displays three MPR views of a CT+RTDOSE+RTSTRUCT dataset, loaded from Orthanc. The Orthanc IDs of the dataset must be supplied as URL parameters like:

```
http://localhost:9979/stone-rtviewer/index.html?loglevel=info&ctseries=a04ecf01-79b2fc33-58239f7e-ad9db983-28e81afa&rtdose=830a69ff-8e4b5ee3-b7f966c8-bccc20fb-d322dceb&rtstruct=54460695-ba3885ee-ddf61ac0-f028e31d-a6e474d9
```

(notice the loglevel parameter that can be `warning`, `info` or `trace`, in increasing verbosity order)

This sample uses plain Javascript and requires the 
Emscripten toolchain and cmake, in addition to a few standard packages.

To build it, just launch the `build-wasm-RtViewer.sh` script from
this folder.  Optionaly, you can pass the build type as an argument.
We suggest that you do *not* use the `Debug` configuration unless you really 
need it, for the additional checks that are made will lead to a very long 
build time and much slower execution (more severe than with a native non-wasm
target)

In order to run the sample, you may serve it with the ServeFolders plugin.
You can i.e: add such a section in your orthanc configuration file:

```
{
  "Plugins" : ["LibServeFolders.so],
  "ServeFolders" : {
    "/rt-viewer" : "..../out/install-stone-wasm-RtViewer-RelWithDebInfo"
  }
}
```

You'll then be able to open the demo at `http://localhost:8042/rt-viewer/index.html`


RtViewerPlugin
---------------
This C++ plugin allows to extend the Orthanc Explorer to add a button labeled "Stone RT Viewer" 
in the series page. 

It also embeds and serves the RT Viewer files and is thus a standalone way of using this viewer.

Please note that building this plugin requires that the RtViewer be built inside the wasm-binaries 
folder of the repo.

This will automatically be the case if you use the `<REPO-ROOT>/OrthancStone/Samples/WebAssembly/docker-build.sh` script.

If you use the `build-wasm-samples.sh` script above, you will have the copy `RtViewer` **folder**
from `<REPO-ROOT>/out/install-stone-wasm-RtViewer-RelWithDebInfo` to `<REPO-ROOT>/wasm-binaries/`.

TL;DR: Build like this (assuming `~/orthanc-stone` is the repo ):

```
~/orthanc-stone/OrthancStone/Samples/WebAssembly/docker-build.sh
~/orthanc-stone/OrthancStone/Samples/RtViewerPlugin/docker-build.sh
```

Once this is done, the plugin can be found in:

```
~/orthanc-stone/wasm-binaries/share/orthanc/plugins/libRtViewerPlugin.so
```

Add this path to the `"Plugins"` section of your Orthanc configuration, start Orthanc, and you 
should now see a "Stone MPR RT Viewer" button in the Orthanc Explorer, at the *series* level.

Open it on a CT series, and the RTSTRUCT and RTDOSE series of the same study will be loaded in
the viewer.

Native samples
=================

### Windows build 

Here's how to build the SdlSimpleViewer example using Visual Studio 2019
(the shell is Powershell, but the legacy shell can also be used with some 
tweaks). This example is meant to be launched from the folder above 
orthanc-stone.

```
  # create the build folder and navigate to it
  $buildDir = "build-stone-sdlviewer-msvc16-x64"

  if (-not (Test-Path $buildDir)) {
    mkdir -p $buildDir | Out-Null
  }
  
  cd $buildDir
  
  # perform the configuration
  cmake -G "Visual Studio 16 2019" -A x64 `
    -DMSVC_MULTIPLE_PROCESSES=ON `
    -DALLOW_DOWNLOADS=ON `
    -DSTATIC_BUILD=ON `
    -DOPENSSL_NO_CAPIENG=ON `
    ../orthanc-stone/OrthancStone/Samples/Sdl
  
  $solutionPath = ls -filter *.sln
  Write-Host "Solution file(s) available at: $solutionPath"
```

The initial configuration step will be quite lengthy, for CMake needs to 
setup its internal cache based on your environment and build tools.

Subsequent runs will be several orders of magnitude faster!

One the solution (.sln) file is ready, you can open it using the Visual Studio
IDE and choose Build --> Build solution.

An alternative is to execute `cmake --build .` in the build folder created by
the script.

In order to run the sample, make sure you've an Orthanc server running i.e. on 
port 8042 and launch:

```
./SdlSimpleViewer --orthanc http://localhost:8042 --instance 7fc84013-abef174e-3354ca83-b9cdb2a4-f1a74368

./RtViewerSdl --orthanc http://localhost:8042 --ctseries a04ecf01-79b2fc33-58239f7e-ad9db983-28e81afa --rtdose 830a69ff-8e4b5ee3-b7f966c8-bccc20fb-d322dceb --rtstruct 54460695-ba3885ee-ddf61ac0-f028e31d-a6e474d9
```


RtViewer
---------------

### Windows build 

Here's how to build the SdlSimpleViewer example using Visual Studio 2019
(the shell is Powershell, but the legacy shell can also be used with some 
tweaks). This example is meant to be launched from the folder above 
orthanc-stone.

```
  $buildRootDir = "out"
  $buildDirName = "build-stone-sdl-RtViewer-msvc16-x64"
  $buildDir = Join-Path -Path $buildRootDir -ChildPath $buildDirName

  if (-not (Test-Path $buildDir)) {
    mkdir -p $buildDir | Out-Null
  }
  
  cd $buildDir
  
  cmake -G "Visual Studio 16 2019" -A x64 `
    -DMSVC_MULTIPLE_PROCESSES=ON `
    -DALLOW_DOWNLOADS=ON `
    -DSTATIC_BUILD=ON `
    -DOPENSSL_NO_CAPIENG=ON `
    ../../orthanc-stone/OrthancStone/Samples/Sdl/RtViewer
```

Executing `cmake --build .` in the build folder will launch the Microsoft 
builder on the solution.

Alternatively, you can open and build the solution using Visual Studio 2019.

