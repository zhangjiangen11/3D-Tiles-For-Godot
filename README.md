
# Cesium Godot 3D Tiles Plugin

A Godot 4 extension that integrates Cesium 3D Tiles capabilities into Godot Engine through GDExtension.

![Godot Cesium Plugin](readme_resources/demo1.jpg)

## Prerequisites

- **C++ 20**
- **CMake 3.1+**
- **Python 3.8+** (with pip)
- **SCons** (`pip install scons`)
- **Visual Studio 2022** with C++ desktop development workload (If you plan on contributing or developing)
- **Git**
- **Godot 4.1+** (with GDExtension support)
- **Windows 10/11** (64-bit)

## Installation (as GDExtension)

### 1. Clone the Repository
```bash
git clone https://github.com/Battle-Road/3D-Tiles-For-Godot.git
cd 3D-Tiles-For-Godot
```

If you would like to build the binaries and you don't plan on modifying the underlying C++ code with any editor use the following command
`scons platform=windows arch=x64 compileTarget=extension`

Otherwise, to get a `compile_commands.json` file to use with an IDE or Text Editor, run
`scons platform=windows compiledb=true arch=x64 compileTarget=extension debug_symbols=true`

#### Build Flags Explained:

* `platform=windows`: Build for Windows (currently only supported platform)

* `compiledb=true`: Generate IDE-friendly compilation database (optional)

* `arch=x64`: 64-bit architecture, will be used as build architecture for all subsequent libraries 

* `compileTarget=extension`: Build as GDExtension

* `debug_symbols=yes`: Include debug information (use no for release)

**The build binaries will be at `godot3dtiles/bin/Godot3DTiles.windows.template_debug.x86_64.dll`**


## Double precision builds
For applications that require getting extremely close to the ellipsoid's surface we recommend building the extension for double precision, this **requires a custom build of Godot with precision=double enabled**

### Steps to build with doubles

* Add `precision=double` to the build flags on the plugin
* Download [Godot's source code on github](https://github.com/godotengine/godot)
* Build Godot with `scons platform=windows arch=x64 precision=double`
* Modify the `Godot3DTiles.gdextension` file to point to your double precision dll


## Setting Up a Project
### Create New Godot Project

* Use Godot 4.1 or newer

* Choose "Forward+" renderer in project settings

### Copy Plugin Files

* From the repository:

  * Copy `godot3dtiles/bin/` folder to your project root

  * Copy `godot3dtiles/addons/` folder to your project root

Your project structure should look like:

```
my_project/
├── addons/
│   └── cesium_godot/
│       ├── panels/
│       ├── scripts/
│       └── visuals/
├── bin/
└── project.godot
```

Alternatively, you can choose to create a new project using the `godot3dtiles/` folder as the project's root

### Enable the plugin

* Open your project in Godot

* Go to Project → Project Settings → Plugins

* Enable "cesium_godot"

## Getting Started

### Using the Cesium Ion Panel

![PanelDemo1](readme_resources/panel_default.png)

The Cesium Ion Panel provides essential tools for managing geospatial content and configuring plugin integration with Cesium services.

**Connect to Cesium ION**


* Select Connect to Cesium ION to connect the plugin to your Cesium account.

* Requires valid Cesium ION credentials

* Grants access to your private assets and token-protected resources

* Maintains session security through OAuth2 authorization flow.

### Setting up your first scene

**Create a new 3D scene**
![CreateScene](readme_resources/create_scene.png)

**Add a new tileset**
You can choose between the different pre-defined tilesets provided to you on the Cesium Ion Panel, or create a new blank tileset to customize it with your desired asset id and data source URL.

You should now see multiple nodes were added to your scene, depending on the option you picked.

*Example with pre-defined tileset Cesium World Terrain + Bing Maps Aerial Imagery*
![AddTileset](readme_resources/add_tileset.png)

**Add a dynamic camera**

Back to the Cesium Panel, you should now be able to click on the dynamic camera button and see it get added to your scene.

![DynamicCamera](readme_resources/dynamic_camera.png)

You should also see the tilesets and CesiumGeoreference have been added automatically to the camera's properties in the inspector.

![CameraProperties](readme_resources/camera_properties.png)

**Hit the Play button**
Tiles should start loading in now!

#### Note: This plugin is currently Windows-only. Linux/MacOS support is planned for future releases.

