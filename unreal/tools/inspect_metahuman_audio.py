"""Read-only UE 5.6 Windows audio probe; run in editor console before Play.

Samples for 120 seconds. Does not change audio, maps, or character assets.
Writes JSON evidence under the repository's ignored unreal/outputs directory.
The FApp exports are version-specific; unavailable symbols fail explicitly.
"""
import array
import ctypes
import json
import math
from pathlib import Path
import time
import wave
import unreal

previous_handle = getattr(unreal, '_metahuman_audio_probe_handle', None)
if previous_handle is not None:
    unreal.unregister_slate_post_tick_callback(previous_handle)
core = ctypes.WinDLL('UnrealEditor-Core.dll')
focus = getattr(core, '?HasFocus@FApp@@SA_NXZ')
focus.restype = ctypes.c_bool
unfocused = getattr(core, '?GetUnfocusedVolumeMultiplier@FApp@@SAMXZ')
unfocused.restype = ctypes.c_float
app_volume = ctypes.c_float.in_dll(core, '?VolumeMultiplier@FApp@@0MA')
destination = Path(__file__).resolve().parents[1] / 'outputs' / 'metahuman-audio-probe.json'
records = []
seen_wavs = set()
started = time.monotonic()
last_poll = 0
last_snapshot = None

def prop(obj, name):
    try:
        value = obj.get_editor_property(name)
        if isinstance(value, (bool, int, float, str)) or value is None:
            return value
        return str(value)
    except Exception:
        return 'unavailable'

def emit(record):
    record['elapsed'] = round(time.monotonic() - started, 3)
    records.append(record)
    unreal.log('METAHUMAN_AUDIO_PROBE ' + json.dumps(record))
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(json.dumps(records, indent=2), encoding='utf-8')

def tick(delta):
    global last_poll, last_snapshot
    now = time.monotonic()
    if now - started > 120:
        unreal.unregister_slate_post_tick_callback(handle)
        unreal._metahuman_audio_probe_handle = None
        emit({'event': 'probe_finished'})
        return
    if now - last_poll < 0.25:
        return
    last_poll = now
    snapshot = {'event': 'output_state', 'pid': __import__('os').getpid(),
                'focused': focus(), 'app_volume': app_volume.value,
                'unfocused_volume': unfocused(), 'components': []}
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    if world:
        for actor in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor):
            for source in actor.get_components_by_class(unreal.ACEAudioCurveSourceComponent):
                for audio in source.get_children_components(True):
                    if not isinstance(audio, unreal.AudioComponent):
                        continue
                    sound = audio.get_editor_property('sound')
                    entry = {'actor': actor.get_name(), 'component': audio.get_name(),
                             'source_volume': prop(source, 'volume'),
                             'playing': audio.is_playing(), 'state': str(audio.get_play_state()),
                             'virtualized': audio.is_virtualized(),
                             'location': [audio.get_world_location().x, audio.get_world_location().y,
                                          audio.get_world_location().z]}
                    for name in ('volume_multiplier', 'allow_spatialization', 'override_attenuation',
                                 'attenuation_settings', 'sound_class_override', 'concurrency_set',
                                 'is_ui_sound'):
                        entry[name] = prop(audio, name)
                    if sound:
                        for name in ('volume', 'sound_class_object', 'sound_submix_object',
                                     'enable_base_submix', 'concurrency_set'):
                            entry['sound_' + name] = prop(sound, name)
                    snapshot['components'].append(entry)
    # Changes only, including foreground/background transitions during playback.
    encoded = json.dumps(snapshot, sort_keys=True)
    if encoded != last_snapshot:
        last_snapshot = encoded
        emit(snapshot)
    for path in (Path(unreal.Paths.project_saved_dir()) / 'ACEConversation').glob('*.wav'):
        if str(path) in seen_wavs:
            continue
        try:
            with wave.open(str(path), 'rb') as wav:
                if wav.getsampwidth() != 2:
                    continue
                pcm = array.array('h', wav.readframes(wav.getnframes()))
                if not pcm:
                    continue
                rms = math.sqrt(sum(x*x for x in pcm)/len(pcm))/32768
                emit({'event': 'temporary_wav', 'file': path.name,
                      'rate': wav.getframerate(), 'channels': wav.getnchannels(),
                      'samples': len(pcm), 'peak': max(map(abs, pcm))/32768,
                      'rms': rms, 'rms_dbfs': 20*math.log10(max(rms, 1e-10))})
                seen_wavs.add(str(path))
        except (OSError, EOFError, wave.Error):
            pass  # Writer may still be finishing the file; retry on next tick.

handle = unreal.register_slate_post_tick_callback(tick)
unreal._metahuman_audio_probe_handle = handle
unreal.log('METAHUMAN_AUDIO_PROBE armed for 120 seconds; start Play and send a response.')
