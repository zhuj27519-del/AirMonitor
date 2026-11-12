"""Host tests execute the actual ARM parser machine code in Unicorn (no MCU I/O).
Usage: python tests/test_protocol.py [path-to-unicorn-package-parent]
Requires the built ELF/BIN and ARM nm; optional ARM_GCC selects toolchain.
"""
import pathlib,sys,glob,os,subprocess,struct,random,json
if len(sys.argv)>1:sys.path.insert(0,sys.argv[1])
from unicorn import Uc,UC_ARCH_ARM,UC_MODE_THUMB,UC_MODE_MCLASS
from unicorn.arm_const import UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_R3,UC_ARM_REG_SP,UC_ARM_REG_LR
root=pathlib.Path(__file__).resolve().parents[1]
gcc=os.environ.get('ARM_GCC',glob.glob('C:/ST/STM32CubeIDE_*/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin/arm-none-eabi-gcc.exe')[-1])
nm=pathlib.Path(gcc).with_name('arm-none-eabi-nm.exe')
symbols={p[2]:int(p[0],16) for l in subprocess.check_output([str(nm),str(root/'build/AirMonitor_V2.elf')],text=True).splitlines() if len(p:=l.split())==3 and p[1].lower()=='t'}
u=Uc(UC_ARCH_ARM,UC_MODE_THUMB|UC_MODE_MCLASS);u.mem_map(0x08000000,0x20000);u.mem_map(0x20000000,0x20000)
u.mem_write(0x08000000,(root/'build/AirMonitor_V2.bin').read_bytes())
buf,out,stream=0x20000000,0x20000100,0x20000200
checks=0
def call(name,*args):
    for r,v in zip([UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_R3],args):u.reg_write(r,v)
    u.reg_write(UC_ARM_REG_SP,0x2001fff0);u.reg_write(UC_ARM_REG_LR,0x0801fff1)
    u.emu_start(symbols[name]|1,0x0801fff0,count=100000)
    return u.reg_read(UC_ARM_REG_R0)
def check(ok,why):
    global checks
    assert ok,why
    checks+=1
def voc(tv=321,h=42,c=650,t=253,rh=555):
    p=bytearray(b'\x2c'+struct.pack('>HHHHH',tv,h,c,t,rh));p.append(-sum(p)&255);return p
def pm():
    p=bytearray(32);p[:4]=b'BM\x00\x1c';p[4:10]=struct.pack('>HHH',90,91,92);p[10:16]=struct.pack('>HHH',10,20,30);p[28]=1;p[30:]=struct.pack('>H',sum(p[:30]));return p
def parse(name,p):u.mem_write(buf,bytes(p));return call(name,buf,len(p),out)
p=voc();check(parse('voc_parse',p)==1,'valid VOC');check(struct.unpack('<HHHhh',u.mem_read(out,10))==(321,42,650,253,555),'VOC scaling/order')
check(parse('voc_parse',voc(t=0xfff5))==1,'negative temp accepted');check(struct.unpack('<HHHhh',u.mem_read(out,10))[3]==-10,'ones complement -1.0C')
for invalid in [voc(tv=5001),voc(rh=1001),voc(rh=0xfff5),voc(t=1251),voc(t=0xff00)]:check(parse('voc_parse',invalid)==0,'range rejected')
for length in [0,1,10,11,13]:check(parse('voc_parse',bytes(p[:length]) if length<12 else p+b'\x00')==0,'length rejected')
q=pm();check(parse('pm_parse',q)==1,'valid PMS');check(struct.unpack('<HHHBB',u.mem_read(out,8))==(10,20,30,1,0),'atmospheric fields')
for offset in range(12):
    bad=bytearray(p);bad[offset]^=1;check(parse('voc_parse',bad)==0,'VOC corrupt byte')
for offset in range(32):
    bad=bytearray(q);bad[offset]^=1;check(parse('pm_parse',bad)==0,'PMS corrupt byte')
bad=bytearray(q);bad[29]=1;bad[30:]=struct.pack('>H',sum(bad[:30]));check(parse('pm_parse',bad)==0,'PMS error flag')
for name,packet in [('voc_feed',p),('pm_feed',q)]:
    for cut in range(len(packet)):
        u.mem_write(stream,bytes(40));results=[]
        corrupt=bytearray(packet);corrupt[-1]^=0x80
        sequence=b'\x00\xff\x17'+packet[:cut]+corrupt+packet+packet
        for b in sequence:results.append(call(name,stream,b,out))
        check(sum(results)>=2,name+' recovers truncated/corrupt stream')
    u.mem_write(stream,bytes(40));rng=random.Random(42)
    for _ in range(1000):call(name,stream,rng.randrange(256),out)
    hits=sum(call(name,stream,b,out) for b in packet+packet)
    check(hits>=1,name+' recovers after random noise')
print(json.dumps({'passed':True,'assertions':checks,'method':'Actual compiled ARM C parser executed on host in Unicorn; no hardware emulation claimed'},indent=2))
