"""Apply the project background-audio policy to an already-running UE 5.6 editor.

The persistent setting is [Audio] UnfocusedVolumeMultiplier=1.0 in this
project's Config/DefaultEngine.ini. FApp caches it, so this optional command
avoids restarting an editor with an unsaved level. No assets are changed.
"""
import ctypes
from pathlib import Path
import unreal

assert Path(unreal.Paths.get_project_file_path()).stem == 'MetaHumanTest'
core = ctypes.WinDLL('UnrealEditor-Core.dll')
set_unfocused = getattr(core, '?SetUnfocusedVolumeMultiplier@FApp@@SAXM@Z')
set_unfocused.argtypes = [ctypes.c_float]
set_unfocused.restype = None
get_unfocused = getattr(core, '?GetUnfocusedVolumeMultiplier@FApp@@SAMXZ')
get_unfocused.restype = ctypes.c_float
previous = get_unfocused()
set_unfocused(1.0)
unreal.log('METAHUMAN_AUDIO Background volume multiplier: %g -> %g (engine FApp setting; no component/asset changes)' % (previous, get_unfocused()))
