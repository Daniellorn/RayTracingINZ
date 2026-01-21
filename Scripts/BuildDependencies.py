import os
import subprocess
import multiprocessing
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ASSIMP_DIR = os.path.join(PROJECT_ROOT, "RayTracingINZ", "vendor", "Assimp")
BUILD_DIR = os.path.join(ASSIMP_DIR, "build")

def Run_Command(command, cwd = None):
    print(f"Command: {command}")
    process = subprocess.Popen(command, shell=True, cwd=cwd)
    process.communicate()

    if process.returncode != 0:
        print(f"Error during command execution in: {cwd if cwd else os.getcwd()}")
        sys.exit(1)

def Check_Submodule():
    print("Checking assimp submodule")
    cmakelists_path = os.path.join(ASSIMP_DIR, "CMakeLists.txt")

    if not os.path.exists(cmakelists_path):
        print("Files don't exist. Running initialization command")
        Run_Command("git submodule update --init --recursive", cwd=PROJECT_ROOT)
    else:
        print("Assimp files status: OK")


def Build_Assimp(build_type):
    print("Building assimp")

    cmake_flags = [
        f"-DCMAKE_BUILD_TYPE={build_type}",
        "-DASSIMP_BUILD_ZLIB=ON",
        "-DUSE_STATIC_CRT=ON",
        "-DBUILD_SHARED_LIBS=OFF",
        "-DASSIMP_BUILD_ASSIMP_TOOLS=OFF",
        "-DASSIMP_BUILD_TESTS=OFF"
    ]

    flags = " ".join(cmake_flags)
    Run_Command(f'cmake CMakeLists.txt -S "{ASSIMP_DIR}" -B "{BUILD_DIR}" {flags}')

    num_cores = multiprocessing.cpu_count()
    Run_Command(f'cmake --build "{BUILD_DIR}" --config {build_type} --parallel {num_cores}')


if __name__ == "__main__":
    os.chdir(PROJECT_ROOT)

    Check_Submodule()

    Build_Assimp("Debug")
    Build_Assimp("Release")

    print("Done")
