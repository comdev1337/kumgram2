#!/usr/bin/env python3
# VS CEE PEE PEE GENERATES OVER 5 GB OF .pdb GARBAGE
# TO COMPILE A 200 MB EXECUTABLE!!!!!
# ??????????????
# ???????????????????????????????????????????

import os
import re

print("Disabling PDB generation and adjusting compiler memory/CPU usage in .vcxproj files...")

out_dir = 'out'

for root, dirs, files in os.walk(out_dir):
    for file in files:
        if file.endswith('.vcxproj'):
            filepath = os.path.join(root, file)
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()

            # Disables PDB
            content = re.sub(r'<DebugInformationFormat>[^<]*</DebugInformationFormat>', r'<DebugInformationFormat>None</DebugInformationFormat>', content)

            # Disables PDB for linker
            content = re.sub(r'<GenerateDebugInformation>[^<]*</GenerateDebugInformation>', r'<GenerateDebugInformation>false</GenerateDebugInformation>', content)

            # Clean up previous incorrect flags that caused linker warnings
            content = re.sub(r' /cgthreads[0-9]+', '', content)
            content = re.sub(r' /Zc:inline', '', content)

            # Find all <ClCompile> blocks and append compiler flags to their <AdditionalOptions>
            # to limit internal thread generation to 4 (preventing cl.exe from spawning too many threads)
            # and inline unreferenced code early.

            def replace_cl_compile(match):
                block = match.group(0)
                if '<AdditionalOptions>' in block:
                    return re.sub(r'<AdditionalOptions>(.*?)</AdditionalOptions>', r'<AdditionalOptions>\1 /cgthreads4 /Zc:inline</AdditionalOptions>', block)
                else:
                    return block.replace('</ClCompile>', '  <AdditionalOptions>%(AdditionalOptions) /cgthreads4 /Zc:inline</AdditionalOptions>\n    </ClCompile>')

            content = re.sub(r'<ClCompile>.*?</ClCompile>', replace_cl_compile, content, flags=re.DOTALL)

            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)

print("Done.")
