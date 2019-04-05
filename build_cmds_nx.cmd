$NINTENDO_SDK_ROOT/Tools/CommandLineTools/MakeNso/MakeNso godot.nx.opt.aarch64.exe.nrs appimage/code/main
$NINTENDO_SDK_ROOT/Tools/CommandLineTools/MakeMeta/MakeMeta.exe --desc $NINTENDO_SDK_ROOT/Resources/SpecFiles/Application.desc --meta Application.aarch64.lp64.nmeta -o appimage/code/main.npdm
$NINTENDO_SDK_ROOT/Tools/CommandLineTools/AuthoringTool/AuthoringTool creatensp -o godot.nsp --desc $NINTENDO_SDK_ROOT/Resources/SpecFiles/Application.desc --meta Application.aarch64.lp64.nmeta --type Application --program appimage/code
#/d/Nintendo/godot_switch_2.0/NintendoSDK/Tools/CommandLineTools/AuthoringTool/AuthoringTool createnspd -o godot.nspd --meta Application.aarch64.lp64.nmeta --type Application --program appimage/code
