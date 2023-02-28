#!/usr/bin/env python

import os
import shutil

zeno_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), ".git/modules/Source/ZenoBridge/Public/zeno")

os.system("git submodule update --depth 1 --init")
os.system(f"git -C Source/ZenoBridge/Public/zeno config core.sparseCheckout true")

zeno_dir_repo = os.path.join(os.path.dirname(os.path.realpath(__file__)), "Source/ZenoBridge/Public/zeno")

if os.path.exists(zeno_dir_repo):
    shutil.rmtree(zeno_dir_repo)

info_dir = os.path.join(zeno_dir, "info")

if not os.path.exists(info_dir):
    os.makedirs(info_dir)

sparse_file = os.path.realpath(os.path.join(zeno_dir, "info/sparse-checkout"))
if not os.path.exists(sparse_file):
    with open(sparse_file, "w+") as s:
        s.write("projects/UnrealBridge/model\n")

os.system("git submodule update --depth 1 --init")

