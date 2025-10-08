#!/usr/bin/env python3

import os
import argparse
import subprocess
import re
from pathlib import Path

# Replace with path to binary
CUTECHESS_CLI_BINARY_PATH = Path(os.environ.get('cutechess_cli_binary'))
VERSIONS_DIR = Path('versions')
BINARY_PATH = Path('build') / 'enigma'

# Find engine binaries matching version prefixes
def find_binary(version_prefix):
    matches = []
    if VERSIONS_DIR.exists():
        for file in VERSIONS_DIR.iterdir():
            if file.name.startswith(f'{version_prefix}_'):
                matches.append(file)

    if len(matches) == 0:
        print(f'Error: No binary found for version "{version_prefix}"')
        exit(1)
    elif len(matches) > 1:
        print(f'Error: Multiple binaries found for version "{version_prefix}"')
        for m in matches:
            print(f'  {m}')
        exit(1)

    return matches[0].resolve()

# Find the latest version in the versions directory
def find_latest_version():
    latest_version = -1
    latest_file = None

    if VERSIONS_DIR.exists():
        for file in VERSIONS_DIR.iterdir():
            match = re.match(r'v(\d+)_(.+)', file.name)
            if match:
                version_num = int(match.group(1))
                if version_num > latest_version:
                    latest_version = version_num
                    latest_file = file

    if latest_file is None:
        print('Error: No versions found in versions directory')
        exit(1)

    return latest_file.resolve()

# Use parser to extract command-line arguments
parser = argparse.ArgumentParser(
    description='Uses cutechess-cli to play two versions of the engine against each other'
)
parser.add_argument('engine_a_version', nargs='?', help='engine A version (e.g. v1) - optional')
parser.add_argument('engine_b_version', nargs='?', help='engine B version (e.g. v2) - optional')
parser.add_argument('-t', '--time', required=True, type=int, help='starting time for each side (in seconds)')
parser.add_argument('-i', '--increment', default=0, type=float, help='time added after each move (in seconds)')
parser.add_argument('-g', '--games', required=True, type=int, help='the number of games to play')
parser.add_argument('-d', '--debug', action='store_true', help='whether to run cutechess-cli in debug mode')
args = parser.parse_args()

# Determine which engines to use based on arguments
if args.engine_a_version is None and args.engine_b_version is None:
    # No args: current vs latest
    engine_a = BINARY_PATH.resolve()
    engine_b = find_latest_version()
    engine_a_name = 'current'
    engine_b_name = engine_b.name

elif args.engine_a_version is not None and args.engine_b_version is None:
    # One arg: current vs specified
    engine_a = BINARY_PATH.resolve()
    engine_b = find_binary(args.engine_a_version)
    engine_a_name = 'current'
    engine_b_name = engine_b.name

elif args.engine_a_version is not None and args.engine_b_version is not None:
    # Two args: version vs version
    engine_a = find_binary(args.engine_a_version)
    engine_b = find_binary(args.engine_b_version)
    engine_a_name = engine_a.name
    engine_b_name = engine_b.name

else:
    # This shouldn't happen with nargs='?' but handle it
    print('Error: Invalid argument combination')
    exit(1)

# Print command preview
print(f'\n{'='*60}')
print(f'Engine A: {engine_a_name}')
print(f'Engine B: {engine_b_name}')
print(f'Time Control: {args.time}+{args.increment}')
print(f'Games: {args.games}')
print(f'Debug: {'On' if args.debug else 'Off'}')
print(f'{'='*60}\n')

# Build cutechess-cli command
cmd = [
    str(CUTECHESS_CLI_BINARY_PATH),
    '-engine', f'cmd={engine_a}', f'name={engine_a_name}',
    '-engine', f'cmd={engine_b}', f'name={engine_b_name}',
    '-each', 'proto=uci', f'tc={args.time}+{args.increment}',
    '-games', str(args.games),
    '-repeat'
]

if args.debug:
    cmd.append("-debug")

# Print command and confirm run
print("Command:")
print(' '.join(cmd))
confirm = input('\nConfirm (y/N): ')
if confirm.lower() != 'y':
    print('Aborted')
    exit(0)

# Execute cutechess-cli
print('\n', flush=True)
subprocess.run(cmd, check=True)