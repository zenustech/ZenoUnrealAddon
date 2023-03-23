# ZenoBridge

Plugin for Unreal Engine to allow UE user interacting with zeno.

## Setup

### Compile Zeno

To use this plugin, you have to source compile zeno with specific option. You can follow [Building Zeno extensions](https://github.com/zenustech/zeno/blob/master/docs/BUILD_EXT.md) to setup your environment.

Here is recommended options:
```bash
-DCMAKE_TOOLCHAIN_FILE=C:/Path/To/VcPkg
^
-DZENO_MULTIPROCESS=ON
^
-DZENO_SYSTEM_OPENVDB=OFF
^
-DZENO_SYSTEM_ALEMBIC=OFF
^
-DZENO_WITH_ZenoFX:BOOL=ON
^
-DZENOFX_ENABLE_OPENVDB:BOOL=ON
^
-DZENOFX_ENABLE_LBVH:BOOL=ON
^
-DZENO_WITH_zenvdb:BOOL=ON
^
-DZENO_WITH_FastFLIP:BOOL=ON
^
-DZENO_WITH_FEM:BOOL=ON
^
-DZENO_WITH_TOOL_FLIPtools:BOOL=ON
^
-DZENO_WITH_TOOL_cgmeshTools:BOOL=ON
^
-DZENO_WITH_TOOL_BulletTools:BOOL=ON
^
-DZENO_WITH_TOOL_HerculesTools:BOOL=ON
^
-DZENO_WITH_Rigid:BOOL=ON
^
-DZENO_WITH_cgmesh:BOOL=ON
^
-DZENO_WITH_oldzenbase:BOOL=ON
^
-DZENO_WITH_TreeSketch:BOOL=ON
^
-DZENO_WITH_Skinning:BOOL=ON
^
-DZENO_WITH_Euler:BOOL=ON
^
-DZENO_WITH_Functional:BOOL=ON
^
-DZENO_WITH_LSystem:BOOL=ON
^
-DZENO_WITH_mesher:BOOL=ON
^
-DZENO_WITH_Alembic:BOOL=ON
^
-DZENO_WITH_FBX:BOOL=ON
^
-DZENO_WITH_DemBones:BOOL=ON
^
-DZENO_WITH_CalcGeometryUV:BOOL=ON
^
-DZENO_WITH_SampleModel:BOOL=ON
^
-DZENO_WITH_MeshSubdiv:BOOL=ON
^
-DZENO_WITH_CUDA:BOOL=OFF
^
-DZENO_ENABLE_OPTIX:BOOL=ON
^
-DZENO_WITH_Audio:BOOL=ON
^
-DZENO_WITH_UnrealBridge:BOOL=ON
```

Anyway, you must to set `ZENO_WITH_UnrealBridge` on.
```bash
-DZENO_WITH_UnrealBridge:BOOL=ON
```

### Install this plugin

1. Cloning this repo using `git clone https://github.com/zenustech/ZenoUnrealAddon.git ZenoBridge`

2. Get into plugin folder `cd ZenoBridge`. Pulling includes from zeno repo `python USE-THIS-SCRIPT-TO-PULL-SUBMODULE.py`. BTW, aduit the script before you run it is a good behavior.

3. Copy entire plugin folder to Plugins folder in your project or engine. Then you can enable it in the editor, regenerate your proejct files and rerun!

