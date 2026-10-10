"""Run in the demo editor to simulate without changing character or level assets."""
import unreal

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not level_editor.is_in_play_in_editor(), 'Stop Play/Simulate before running this setup script'
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_path_name().startswith('/Game/MetaHumanConversationDemo.')
descriptors = unreal.WorldPartitionBlueprintLibrary.get_actor_descs()
unreal.WorldPartitionBlueprintLibrary.load_actors([d.guid for d in descriptors])
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
receivers = [a for a in actors.get_all_level_actors() if isinstance(a, unreal.MetaHumanTtsReceiver)]
assert len(receivers) == 1
target = receivers[0].get_editor_property('target_character')
assert target
actors.set_selected_level_actors([target])
location = target.get_actor_location()
unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).set_level_viewport_camera_info(
    location + unreal.Vector(180, 0, 155), unreal.Rotator(pitch=0, yaw=180, roll=0))
unreal.log('METAHUMAN_VALIDATION starting simulation; target=' + target.get_path_name())
level_editor.editor_play_simulate()
