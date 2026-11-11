@tool
extends EditorScript

const ARCHIVE_PATH := "res://StarForge_Asset_Pack.zip"
const ARCHIVE_PREFIX := "StarForge_Asset_Pack/assets/"
const DESTINATION := "res://assets"

func _run() -> void:
    var reader := ZIPReader.new()
    var error := reader.open(ARCHIVE_PATH)
    if error != OK:
        push_error("Could not open the Star Forge art archive.")
        return

    var written := 0
    for archive_path: String in reader.get_files():
        if not archive_path.begins_with(ARCHIVE_PREFIX):
            continue
        var relative_path := archive_path.trim_prefix(ARCHIVE_PREFIX)
        if relative_path.is_empty() or relative_path.ends_with("/"):
            continue

        var destination_path := DESTINATION.path_join(relative_path)
        var absolute_directory := ProjectSettings.globalize_path(destination_path.get_base_dir())
        DirAccess.make_dir_recursive_absolute(absolute_directory)

        var output := FileAccess.open(destination_path,FileAccess.WRITE)
        if output == null:
            push_warning("Could not extract asset: " + relative_path)
            continue
        output.store_buffer(reader.read_file(archive_path))
        written += 1

    reader.close()
    EditorInterface.get_resource_filesystem().scan()
    print("Star Forge: extracted %d art assets." % written)
