#!/usr/bin/env python3

import os
import argparse
import subprocess
from pathlib import Path

# Replace with path to binary
CUTECHESS_CLI_BINARY_PATH = Path(os.environ.get('cutechess_cli_binary'))
VERSIONS_DIR = Path('versions')

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

# Use parser to extract command-line arguments
parser = argparse.ArgumentParser(
    description='Uses cutechess-cli to play two versions of the engine against each other'
)
parser.add_argument('engine_a_version', help='engine A version (e.g. v1)')
parser.add_argument('engine_b_version', help='engine B version (e.g. v2)')
parser.add_argument('-t', '--time', required=True, type=int, help='starting time for each side')
parser.add_argument('-i', '--increment', default=0, type=int, help='time added after each move')
parser.add_argument('-g', '--games', required=True, type=int, help='the number of games to play')
parser.add_argument('-d', '--debug', action='store_true', help='whether to run cutechess-cli in debug mode')
args = parser.parse_args()

engine_a = find_binary(args.engine_a_version)
engine_b = find_binary(args.engine_b_version)

# Print command preview
print(f'\n{'='*60}')
print(f'Engine A: {engine_a.name}')
print(f'Engine B: {engine_b.name}')
print(f'Time Control: {args.time}+{args.increment}')
print(f'Games: {args.games}')
print(f'Debug: {'On' if args.debug else 'Off'}')
print(f'{'='*60}\n')

# Build cutechess-cli command
cmd = [
    str(CUTECHESS_CLI_BINARY_PATH),
    '-engine', f'cmd={engine_a}', f'name={engine_a.name}',
    '-engine', f'cmd={engine_b}', f'name={engine_b.name}',
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