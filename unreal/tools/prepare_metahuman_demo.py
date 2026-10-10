"""Run in MetaHumanTest after installing/building the conversation plugins."""
import unreal

backup = '/Game/MetaHumanKnownGood_Preserved'
demo = '/Game/MetaHumanConversationDemo'
exists = unreal.EditorAssetLibrary.does_asset_exist(demo)
world = unreal.EditorLoadingAndSavingUtils.load_map(demo if exists else backup)
assert world
# Commandlets do not restore the editor's loaded World Partition regions.
descriptors = unreal.WorldPartitionBlueprintLibrary.get_actor_descs()
unreal.WorldPartitionBlueprintLibrary.load_actors([d.guid for d in descriptors])
if not exists:
    assert unreal.EditorLoadingAndSavingUtils.save_map(world, demo)
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
receivers = [a for a in subsystem.get_all_level_actors() if isinstance(a, unreal.MetaHumanTtsReceiver)]
assert len(receivers) <= 1, 'Refusing to modify a map with multiple receivers'
targets = [a for a in subsystem.get_all_level_actors()
           if a.get_class().get_path_name().startswith('/Game/MetaHumans/NewMetaHumanCharacter/BP_NewMetaHumanCharacter.')]
assert len(targets) == 1
receiver = receivers[0] if receivers else subsystem.spawn_actor_from_class(unreal.MetaHumanTtsReceiver, targets[0].get_actor_location())
receiver.set_actor_label('MetaHuman TTS Observer')
receiver.set_editor_property('target_character', targets[0])
# Match the existing actor's streaming policy; hard references across different
# spatial-loading policies are invalid in a World Partition map.
receiver.set_editor_property('is_spatially_loaded', targets[0].get_editor_property('is_spatially_loaded'))
receiver.set_actor_location(targets[0].get_actor_location(), False, False)
assert unreal.EditorLoadingAndSavingUtils.save_map(world, demo)
unreal.log('METAHUMAN_DEMO_READY ' + demo + ' target=' + targets[0].get_path_name())
