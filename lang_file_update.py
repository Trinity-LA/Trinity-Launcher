import os
import subprocess
import sys
import shutil

def get_lupdate_command():
    pyside_lupdate = shutil.which("pyside6-lupdate")
    if pyside_lupdate:
            return pyside_lupdate
    
    commands = ['lupdate-qt6', 'lupdate', '/usr/lib/qt6/bin/lupdate']
    for cmd in commands:
        try:
            subprocess.run([cmd, '-version'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            return cmd
        except FileNotFoundError:
            continue
    return None

def update_translations():
    lupdate = get_lupdate_command()
    if not lupdate:
        print("Error: lupdate not found. Needed qt6-tools.")
        sys.exit(1)

    result = subprocess.run(
        ['git', 'ls-files', '*.ts'], 
        capture_output=True, 
        text=True, 
        check=True
    )
    ts_files = result.stdout.splitlines()

    project_root = "."

    for ts_file in ts_files:
        print(f"  → Updating: {ts_file}")
        cmd = [lupdate, "-recursive", project_root, "-ts", ts_file]
        
        proc = subprocess.run(cmd, capture_output=True, text=True)
        if proc.returncode != 0:
            print(f"    Error while update {ts_file}: {proc.stderr}")
        else:
            print(f"    Done")

    print("\nAll localization files done")

if __name__ == "__main__":
    update_translations()