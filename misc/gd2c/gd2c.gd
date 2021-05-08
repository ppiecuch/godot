extends SceneTree

# How to use:
# (required GDScriptBytecodeExporter to be compiled in)
#
# godot --no-window -s gd2c.gd -Din"." -Dout"."

func dump_directory(source_dir, dest_dir):
	var exporter = GDScriptBytecodeExporter.new()
	var dir = Directory.new()
	if dir.open(source_dir) == OK:
		dir.list_dir_begin(true, true)
		var name = dir.get_next()
		while name != "":
			var sd = source_dir
			if sd == '.':
				sd = ''
			else:
				sd = sd + '/'

			var dd = dest_dir
			if dd == '.':
				dd = ''
			else:
				dd = dd + '/'

			if dir.current_is_dir():
				dump_directory(sd + name, dd + name)
			else:
				if name.ends_with(".gd") and name != "gd2c.gd":
					print(sd + name + " to " + dd + name)
					exporter.export_bytecode_to_file(sd + name, dd + name + ".json")

			name = dir.get_next()

func _init():
	var exporter = GDScriptBytecodeExporter.new()

	var in_dir = ""
	var out_dir = ""

	var args = OS.get_cmdline_args()
	for arg in args:
		var din = arg.trim_prefix("-Din")
		if din != arg:
			in_dir = din

		var dout = arg.trim_prefix("-Dout")
		if dout != arg:
			out_dir = dout

	print("in_dir:  ", in_dir)
	print("out_dir: ", out_dir)

	if in_dir and out_dir:
		dump_directory(in_dir, out_dir)

	quit()



