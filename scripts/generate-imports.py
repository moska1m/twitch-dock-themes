"""Run from an x64 Native Tools Command Prompt for VS 2022."""
import argparse
import pathlib
import re
import subprocess

parser = argparse.ArgumentParser(description="Generate OBS x64 import libraries from an installed OBS.")
parser.add_argument("--obs-bin", required=True, type=pathlib.Path)
parser.add_argument("--output", required=True, type=pathlib.Path)
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
for name in ("obs", "obs-frontend-api"):
    dll = args.obs_bin / (name + ".dll")
    exports = subprocess.check_output(["dumpbin.exe", "/exports", str(dll)], text=True)
    names = re.findall(r"^\s+\d+\s+[0-9A-F]+\s+[0-9A-F]+\s+(\S+)", exports, re.M)
    if not names:
        raise SystemExit(f"No exports found: {dll}")
    definition = args.output / (name + ".def")
    definition.write_text("LIBRARY " + name + ".dll\nEXPORTS\n" + "\n".join(names) + "\n")
    subprocess.run(["lib.exe", "/nologo", "/machine:x64", "/def:" + str(definition),
                    "/out:" + str(args.output / (name + ".lib"))], check=True)
    print(f"{name}: {len(names)} exports")
