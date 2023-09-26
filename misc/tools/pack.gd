extends Node

var version = "0.1"

func _ready():
	var pck_file = args(0)
	var input_folder = args(1)
	var wd = args(2)
	print("***")
	print("*** Godot PCK v%s packer ready" % version)
	print("*** ---------------------------")
	print("***")
	print("Input:  %s" % input_folder)
	print("Output: %s" % pck_file)
	if input_folder.empty() or pck_file.empty():
		print("*** Missing input folder or output file.")
		return
	if not wd.empty():
		print("Working folder: %s" % wd)

	print("")
	var dir = Directory.new()
	if dir.open(".") == OK:
		print("Current folder: %s" % dir.get_current_dir())
	else:
		print("Current folder cannot read")
	pck_Packager(input_folder, pck_file)

func _add_dir_contents(dir:Directory, files:Array, directories:Array):
	dir.list_dir_begin(true, false)
	var file_name = dir.get_next()
	while (file_name != ""):
		var path = dir.get_current_dir() + "/" + file_name
		if dir.current_is_dir():
			printraw("#")
			var sub_dir = Directory.new()
			sub_dir.open(path)
			directories.append(path)
			_add_dir_contents(sub_dir, files, directories)
		else:
			printraw(".")
			files.append(path)

		file_name = dir.get_next()

	dir.list_dir_end()

func pck_Packager(path, pckfile):
	var files = []
	var dirs = []
	var packaging = PCKPacker.new()
	packaging.pck_start(pckfile, 0)
	var dir = Directory.new()
	if dir.open(path) == OK:
		var base_folder = dir.get_current_dir()+"/"
		print("Base folder: %s" % base_folder)
		_add_dir_contents(dir, files, dirs)
		print(" - scanning finished")
		print("Processing %d files .." % files.size())
		for i in range(0, files.size()):
			packaging.add_file("res://"+files[i].replace(base_folder, ""), files[i])
		packaging.flush(true)
	else:
		print("An error occurred when trying to access the path '%s'" % path)
