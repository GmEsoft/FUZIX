#!/usr/bin/env python3
"""Check production USB descriptor bytes using the installed TinyUSB macros."""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile
here=Path(__file__).resolve().parent
platform=Path(os.environ.get('PLATFORM_DIR',here/'../..')).resolve()
sdk=Path(os.environ['PICO_SDK_PATH'])
cc=shlex.split(os.environ.get('CC','cc'))
with tempfile.TemporaryDirectory(prefix='pico-usb-descriptors-') as directory:
    for count in range(1,5):
        exe=str(Path(directory)/('descriptor-'+str(count)))
        subprocess.run(cc+['-std=c99','-fsanitize=address,undefined','-DCONFIG_H',
                       '-DCFG_TUSB_MCU=OPT_MCU_RP2040','-DNUM_DEV_TTY_USB='+str(count),
                       '-I'+str(sdk/'lib/tinyusb/src'),'-I'+str(platform),
                       str(here/'check.c'),str(platform/'usbdescriptors.c'),'-o',exe],check=True)
        subprocess.run([exe],check=True)
