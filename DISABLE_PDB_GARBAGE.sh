#!/usr/bin/env python3
# VS CEE PEE PEE GENERATES OVER 5 GB OF .pdb GARBAGE
# TO COMPILE A 200 MB EXECUTABLE!!!!!
# ??????????????
# ???????????????????????????????????????????

from pathlib import Path

out_dir = Path("out")
targets_path = out_dir / "Directory.Build.targets"
targets_content = """<Project>
  <ItemDefinitionGroup>
    <ClCompile>
      <DebugInformationFormat>None</DebugInformationFormat>
      <MultiProcessorCompilation>false</MultiProcessorCompilation>
      <!-- /MP20 already allows up to 20 compiler processes; /cgthreads4 would let each process spawn up to 4 backend threads, so that can fan out to roughly 80 compiler threads. -->
      <AdditionalOptions>%(AdditionalOptions) /MP20 /cgthreads4</AdditionalOptions>
    </ClCompile>
    <Link>
      <GenerateDebugInformation>false</GenerateDebugInformation>
      <AdditionalOptions>%(AdditionalOptions) /DEBUG:NONE</AdditionalOptions>
    </Link>
  </ItemDefinitionGroup>
</Project>
"""

if targets_path.exists():
    print(f"{targets_path} already exists, leaving it unchanged.")
else:
    out_dir.mkdir(exist_ok=True)
    targets_path.write_text(targets_content, encoding="utf-8", newline="\n")
    print(f"Created {targets_path}.")
