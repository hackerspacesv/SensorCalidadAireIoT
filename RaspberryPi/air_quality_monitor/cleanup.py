#!/usr/bin/python
import os
import config as Config

if os.path.exists(Config.lockfile):
    os.remove(Config.lockfile)
