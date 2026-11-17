import os
import sys
import argparse
import subprocess
from pathlib import Path

CUSTOM_DSC = Path("PlaygroundPkg/OvmfPkgX64Custom.dsc")
CUSTOM_FDF = Path("PlaygroundPkg/OvmfPkgX64Custom.fdf")

def get_playground_definitions():
    dsc_file = Path("PlaygroundPkg/PlaygroundPkg.dsc")
    if not dsc_file.exists():
        sys.exit(f"Error: {dsc_file} not found.")

    libs = []
    components = []
    current_section = None

    for line in dsc_file.read_text().splitlines():
        line = line.split("#")[0].strip()
        if not line:
            continue
        if line.startswith("[") and line.endswith("]"):
            current_section = line[1:-1].strip().lower()
            continue
        if current_section and current_section.startswith("libraryclasses") and "playgroundpkg/" in line.lower():
            libs.append(line)
        elif current_section and current_section.startswith("components") and "playgroundpkg/" in line.lower():
            components.append(line)

    return libs, components

def patch_ovmf():
    print("=== Patching OVMF X64 DSC/FDF files dynamically (edk2 remains unmodified) ===")
    libs, components = get_playground_definitions()

    dsc_path = Path("edk2/OvmfPkg/OvmfPkgX64.dsc")
    if not dsc_path.exists():
        sys.exit(f"Error: {dsc_path} not found.")

    dsc_content = dsc_path.read_text().replace(
        "FLASH_DEFINITION               = OvmfPkg/OvmfPkgX64.fdf",
        f"FLASH_DEFINITION               = {CUSTOM_FDF.as_posix()}"
    )

    inc_dir = Path("PlaygroundPkg/Include").resolve()
    include_flags = [
        "[BuildOptions]",
        f'  GCC:*_*_*_CC_FLAGS = -I"{inc_dir.as_posix()}" -Wno-unused-function',
        f'  MSFT:*_*_*_CC_FLAGS = /I "{inc_dir}"'
    ]

    dsc_content += "\n" + "\n".join(include_flags)
    if libs:
        dsc_content += "\n[LibraryClasses]\n  " + "\n  ".join(libs)
    if components:
        dsc_content += "\n[Components]\n  " + "\n  ".join(components)
    dsc_content += "\n"

    CUSTOM_DSC.write_text(dsc_content)
    print(f"Generated {CUSTOM_DSC}")

    fdf_path = Path("edk2/OvmfPkg/OvmfPkgX64.fdf")
    if not fdf_path.exists():
        sys.exit(f"Error: {fdf_path} not found.")

    driver = "INF  PlaygroundPkg/PlaygroundDxe/PlaygroundDxe.inf\n\n[FV.FVMAIN_COMPACT]"
    fdf_content = fdf_path.read_text().replace("[FV.FVMAIN_COMPACT]", driver)
    CUSTOM_FDF.write_text(fdf_content)
    print(f"Generated {CUSTOM_FDF}")

def run_build(toolchain, target, clean=False):
    workspace = Path.cwd().resolve()
    edk2_dir = workspace / "edk2"

    env = os.environ.copy()
    env["WORKSPACE"] = str(edk2_dir)
    env["PACKAGES_PATH"] = f"{edk2_dir};{workspace}" if os.name == "nt" else f"{edk2_dir}:{workspace}"

    is_win = os.name == "nt"
    shell = ["cmd.exe", "/c"] if is_win else ["bash", "-c"]
    setup = "call edksetup.bat" if is_win else ". ./edksetup.sh"
    dsc_rel = f"../{CUSTOM_DSC.as_posix()}" if is_win else CUSTOM_DSC.as_posix()

    if clean:
        print(f"=== Performing EDK2 Clean Build (X64 | {target} | {toolchain}) ===")
        cmd = f"{setup} && build clean -p {dsc_rel} -a X64 -t {toolchain} -b {target}"
        res = subprocess.run(shell + [cmd], cwd=edk2_dir if is_win else workspace, env=env)
        if res.returncode != 0:
            sys.exit(res.returncode)

    print(f"=== Launching EDK2 Build on {'Windows' if is_win else 'Linux'} (X64 | {target} | {toolchain}) ===")
    cmd = f"{setup} && build -p {dsc_rel} -a X64 -t {toolchain} -b {target} -D SECURE_BOOT_ENABLE=FALSE"
    print(f"Command: {cmd}")
    res = subprocess.run(shell + [cmd], cwd=edk2_dir if is_win else workspace, env=env)
    if res.returncode != 0:
        print("Build failed!")
        sys.exit(res.returncode)

    fv_dir = edk2_dir / "Build" / "OvmfX64" / f"{target}_{toolchain}" / "FV"
    print(f"Build artifacts are located in: {fv_dir}")
    print("=== Build completed successfully! ===")

def cleanup():
    print("=== Cleaning up temporary DSC/FDF files ===")
    for path in (CUSTOM_DSC, CUSTOM_FDF):
        if path.exists():
            path.unlink()
            print(f"Removed {path}")

def main():
    parser = argparse.ArgumentParser(description="Simplified EDK2 OVMF X64 Builder for PlaygroundPkg")
    parser.add_argument("--arch", default="X64", choices=["X64", "AARCH64"],
                        help="Platform Architecture (default: X64)")
    parser.add_argument("--toolchain", default="VS2026" if os.name == "nt" else "GCC",
                        help="EDK2 toolchain tag (default: VS2026 on Windows, GCC on Linux)")
    parser.add_argument("--target", default="DEBUG", choices=["DEBUG", "RELEASE"],
                        help="EDK2 build target (DEBUG, RELEASE)")
    parser.add_argument("-c", "--clean", action="store_true",
                        help="Perform a clean build")
    args = parser.parse_args()

    if args.arch.upper() != "X64":
        sys.exit(f"Error: Architecture '{args.arch}' is not supported. This builder is streamlined for X64 (OVMF).")

    try:
        patch_ovmf()
        run_build(args.toolchain, args.target, clean=args.clean)
    finally:
        cleanup()

if __name__ == "__main__":
    main()
