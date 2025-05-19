# This file contains utility functions to build CesiumForGodot in SCons
import subprocess
import os
import fnmatch
import sys

from SCons.Script import Dir

ROOT_DIR_MODULE = "#modules/cesium_godot"

ROOT_DIR_EXT = "#cesium_godot"

BINDINGS_DIR = "#godot-cpp"

CESIUM_MODULE_DEF = "CESIUM_GD_MODULE"

CESIUM_EXT_DEF = "CESIUM_GD_EXT"

CESIUM_NATIVE_DIR_EXT = "#cesium_godot/native"

CESIUM_NATIVE_DIR_MODULE = "#modules/cesium_godot/native"

OS_WIN = "nt"

STATIC_TRIPLET = "x64-windows-static"

RELEASE_CONFIG = "Release"


def is_extension_target(argsDict) -> bool:
    return get_compile_target_definition(argsDict) == CESIUM_EXT_DEF


def generate_precision_symbols(argsDict, env):
    print("Generating double precision compile symbols")
    desiredPrecision = argsDict.get("precision")
    if (desiredPrecision == "double"):
        env.Append(CPPDEFINES=["REAL_T_IS_DOUBLE"])


def get_compile_target_definition(argsDict) -> str:
    # Get the format (default is extension)
    global currentRootDir
    compileTarget = argsDict.get("compileTarget", CESIUM_EXT_DEF)
    if (compileTarget == "module"):
        print("[CESIUM] - Compiling Cesium For Godot as an engine module...")
        currentRootDir = ROOT_DIR_MODULE
        return CESIUM_MODULE_DEF
    if (compileTarget == "" or compileTarget == "extension"):
        print("[CESIUM] - Compiling Cesium For Godot as a GDExtension")
        currentRootDir = ROOT_DIR_EXT
        return CESIUM_EXT_DEF

    print("[CESIUM] - Compile target not recognized, options are: module / extension")
    exit(1)


def clone_native_repo_if_needed():
    clone_repo_if_needed(ROOT_DIR_EXT + "/native", "Cesium Native",
                         "https://github.com/CesiumGS/cesium-native.git", "v0.46.0", "ae62bd8c6a7fbce08a541eecd86a313bfb906e15")


def clone_bindings_repo_if_needed():
    clone_repo_if_needed(BINDINGS_DIR, "Godot CPP Bindings", "https://github.com/godotengine/godot-cpp",
                         "4.1", "6388e26dd8a42071f65f764a3ef3d9523dda3d6e")


def clone_lite_html_if_needed():
    # clone_repo_if_needed(ROOT_DIR_EXT + "/third_party/lite-html", "Lite HTML",
    #                      "https://github.com/litehtml/litehtml.git", "v0.9", "6ca1ab0419e770e6d35a1ef690238773a1dafcee")
    pass


def clone_repo_if_needed(targetDir: str, name: str, repoUrl: str, branch: str, acceptedCommitSHA: str):
    print(f"Cloning {name} repo")
    repoDirectory = _scons_to_abs_path(targetDir)
    if (os.path.exists(repoDirectory)):
        return
    subprocess.run(["git", "clone", "-b", branch,
                   repoUrl, "--recursive", repoDirectory])

    prevDir: str = os.getcwd()
    os.chdir(repoDirectory)
    subprocess.run(["git", "reset", "--hard", acceptedCommitSHA])
    os.chdir(prevDir)


# Configure with CMake
def configure_native(argumentsDict):
    print("Configuring Cesium Native")
    isExt = is_extension_target(argumentsDict)
    repoDirectory = CESIUM_NATIVE_DIR_EXT if isExt else CESIUM_NATIVE_DIR_MODULE
    repoDirectory = _scons_to_abs_path(repoDirectory)
    os.chdir(repoDirectory)
    # Assume you already have the triplet (for now)
    triplet = "x64-windows-static"
    os.environ["VCPKG_TRIPLET"] = triplet
    # Run Cmake with the /MT flag on
    result = subprocess.run(
        ["cmake", "-DCESIUM_MSVC_STATIC_RUNTIME_ENABLED=ON", "DGIT_LFS_SKIP_SMUDGE=1", "-DVCPKG_TRIPLET=%s" % triplet, "."])

    # We pray this works haha
    if result.returncode != 0:
        errorMsg = "cmake return code: %s" % str(result.returncode)
        print('Error configuring Cesium native, please make sure you have CMake installed and up to date: ' + errorMsg)
        exit(1)
    print("Configuration completed without any errors!")


def compile_native(argumentsDict):
    shouldBuildArg = argumentsDict.get("buildCesium", None)
    if shouldBuildArg is None:
        shouldBuildResponse = input(
            "Do you wanna build Cesium Native (Choose yes if it's the first install)? [y/n]")
        shouldBuildArg = shouldBuildResponse.capitalize()[0] == 'Y'
    else:
        shouldBuildArg = shouldBuildArg.upper() == "YES" or shouldBuildArg.upper() == "TRUE"

    if not shouldBuildArg:
        return

    print("Building Cesium Native, this might take a few minutes...")
    configure_native(argumentsDict)
    print("Compiling Cesium Native...")
    if os.name != OS_WIN:
        print("Compiling for platform %s is not yet supported!" %
              os.name, file=sys.stderr)

    # execute MSBuild
    buildConfig: str = RELEASE_CONFIG
    solutionName: str = "cesium-native.sln"
    msbuildPath: str = find_ms_build()
    if msbuildPath == '':
        print(
            "Could not find MSBuild.exe, make sure to have Visual Studio installed", file=sys.stderr)
        return
    releaseConfig = "/property:Configuration=%s" % buildConfig
    result = subprocess.run([msbuildPath, solutionName, releaseConfig])
    if result.returncode != 0:
        print("Error building Cesium Native: %s" % str(result.stderr))
    print("Cleaning definitions on generated files...")
    clean_cesium_definitions()
    print("Finished building Cesium Native!")


def clean_cesium_definitions():
    """
    This function modifies some of Cesium's header files to clean up
    definitions that conflict with the engine's
    """
    # Get the conflicting file (Material.h in our case)
    print("Cleaning native definitions")

    conflictFilePath: str = "%s/%s" % (CESIUM_NATIVE_DIR_EXT,
                                       "/CesiumGltf/generated/include/CesiumGltf")
    conflictFilePath = _scons_to_abs_path(conflictFilePath) + "/Material.h"
    # Load the file into memory

    # Read in the file
    fileData: str = ""
    with open(conflictFilePath, 'r') as file:
        fileData = file.read()

    # Replace the target string
    fileData = fileData.replace('#pragma once', '#pragma once\n#undef OPAQUE')

    # Write the file out again
    with open(conflictFilePath, 'w') as file:
        file.write(fileData)
    print("Finished cleaning native definitions")


def install_additional_libs():
    print("Installing additional libraries")
    vcpkgPath = find_ezvcpkg_path()
    executable = "%s/%s" % (vcpkgPath, "vcpkg.exe")
    subprocess.run([executable, "install", "curl:%s" % (STATIC_TRIPLET)])
    subprocess.run([executable, "install", "uriparser:%s" % (STATIC_TRIPLET)])
    subprocess.run([executable, "install", "ada-url:%s" % (STATIC_TRIPLET)])


def find_ms_build() -> str:
    print("Searching for MS Build")
    # Try to search for an msbuild executable in the system
    try:
        testCmd = subprocess.run(
            ["msbuild", "-version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        # Yay, we found it... (not gonna happen lol)
        if testCmd.returncode == 0:
            return 'msbuild'
    except:
        # More likely we'll need to search for another path
        vsPath = "C:\\Program Files\\Microsoft Visual Studio"
        found, path = find_in_dir_recursive(vsPath, '*MSBuild.exe')

        if found:
            # Access the next latest directory (latest VS version)
            return path

        # Try with a .NET path
        print(".NET path is not yet supported!", sys.stderr)
        return ''


def find_in_dir_recursive(path: str, pattern: str) -> (bool, str):
    """
    Use only when there might be a few directories left to search
    as this function is recursive
    """

    if not os.path.exists(path):
        return False, ''

    foundFiles: list[str] = os.listdir(path)

    if len(foundFiles) == 0:
        return False, ''

    for root, dirnames, filenames in os.walk(path):
        for filename in fnmatch.filter(filenames, pattern):
            return True, os.path.join(root, filename)

    return False, ''


def find_ezvcpkg_path() -> str:
    # Search the C drive
    assumedPath = "%s.ezvcpkg" % (os.path.abspath(os.sep))
    print(f"Searching vcpkg at: {assumedPath}")
    if (not os.path.exists(assumedPath)):
        from pathlib import Path
        assumedPath = (Path.home() / ".ezvcpkg").as_posix()
        print(f"Searching vcpkg at: {assumedPath}")
        if (not os.path.exists(assumedPath)):
            print(
                "EZVCPKG not found, please make sure that CesiumNative was compiled and configured properly!")
            return ""
        # Assume it is in /home (C:/Users/currUser)
    # Then find the latest version (use the last created folder)
    subDirs = [x for x in next(os.walk(assumedPath))[1]]
    subDirs.sort(reverse=True, key=lambda x: os.stat(
        "%s/%s" % (assumedPath, x)).st_ctime)
    latestDir = subDirs[0]
    assumedPath = "%s/%s" % (assumedPath, latestDir)
    print(f"Found ezvcpkg at {assumedPath}")
    return assumedPath


def clone_engine_repo_if_needed():
    pass


def _scons_to_abs_path(path: str) -> str:
    return Dir(path).get_abspath()


def get_root_dir() -> str:
    return currentRootDir


def get_root_dir_native() -> str:
    return currentRootDir + "/native"
