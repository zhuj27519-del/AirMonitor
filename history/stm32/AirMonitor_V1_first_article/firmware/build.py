"""Portable build used by the CubeIDE external builder and command line."""
import os,pathlib,subprocess,sys,glob
root=pathlib.Path(__file__).resolve().parent
os.chdir(root)
matches=glob.glob(r'C:/ST/STM32CubeIDE_*/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin/arm-none-eabi-gcc.exe')
gcc=os.environ.get('ARM_GCC',matches[-1] if matches else 'arm-none-eabi-gcc')
binpath=str(pathlib.Path(gcc).parent)
def run(a):
    print(' '.join(map(str,a)),flush=True);subprocess.run(a,check=True)
pathlib.Path('build').mkdir(exist_ok=True)
flags=['-mcpu=cortex-m0plus','-mthumb','-std=c11','-Os','-g3','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-ffunction-sections','-fdata-sections','-DSTM32G0B1xx','-Ivendor','-Ivendor/CMSIS','-Isrc']
sources=['src/main.c','src/display.c','src/protocol.c','src/syscalls.c','vendor/system_stm32g0xx.c','vendor/startup_stm32g0b1xx.s']
objects=[]
for s in sources:
    o='build/'+pathlib.Path(s).stem+'.o';run([gcc,*flags,'-c',s,'-o',o]);objects.append(o)
run([gcc,*flags,*objects,'-TSTM32G0B1CBTX_FLASH.ld','--specs=nano.specs','--specs=nosys.specs','-Wl,--gc-sections,-Map=build/AirMonitor_V2.map','-Wl,--no-warn-rwx-segments','-o','build/AirMonitor_V2.elf'])
for fmt,ext in [('ihex','hex'),('binary','bin')]:run([str(pathlib.Path(binpath)/'arm-none-eabi-objcopy.exe'),'-O',fmt,'build/AirMonitor_V2.elf','build/AirMonitor_V2.'+ext])
run([str(pathlib.Path(binpath)/'arm-none-eabi-size.exe'),'build/AirMonitor_V2.elf'])
