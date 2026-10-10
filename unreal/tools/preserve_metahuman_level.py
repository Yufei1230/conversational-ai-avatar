"""Execute in the existing editor, before closing it or adding the receiver."""
import unreal

world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_path_name().startswith('/Temp/Untitled'), world.get_path_name()
destination = '/Game/MetaHumanKnownGood_Preserved'
assert not unreal.EditorAssetLibrary.does_asset_exist(destination), 'Refusing to overwrite backup'
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
targets = [a for a in actors if a.get_class().get_path_name().startswith('/Game/MetaHumans/NewMetaHumanCharacter/BP_NewMetaHumanCharacter.')]
assert len(targets) == 1, 'Expected exactly one working MetaHuman'
assert unreal.EditorLoadingAndSavingUtils.save_map(world, destination)
unreal.log('METAHUMAN_PRESERVED ' + destination + ' target=' + targets[0].get_path_name())
