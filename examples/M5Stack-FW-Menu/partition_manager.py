Import('env')
import os.path
import sys

# add esp32partitiontool to path
sys.path.append(os.path.abspath( env.PioPlatform().get_package_dir("tool-esp32partitiontool") ))
# import module
from esp32partitiontool import *
# run module
load_pm(env)


