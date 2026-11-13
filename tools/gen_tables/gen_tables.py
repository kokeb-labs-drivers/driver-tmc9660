#!/usr/bin/env python3
"""Generate inc/tmc9660_params.hpp and inc/tmc9660_registers.hpp.

Sources (kept next to this script, extracted from the vendor manuals recorded in docs/datasheets.yml):
  params.json  Table 57 of the Parameter Mode Reference Manual (Rev. 0, 02/25): number, name, access, range,
               default, coded values and description of every axis parameter.
  regs.json    The register map of the Register Mode Reference Manual (Rev. 0, 04/25): blocks, offsets, fields,
               field types and reset values.
Corrections the manuals need (reset values of HALL_ERROR and T_VMAX, the duplicate _Y field names of
MCC_GDRV_STATUS_EN and MCC_GDRV_PROT_EN) are applied below, each in one visible line.

The output is passed through clang-format with _config/.clang-format (the version the gate uses, 20) and the
script checks that a second pass changes nothing, so the committed headers are formatter-stable.

  tools/gen_tables/gen_tables.py          rewrite the two headers
  tools/gen_tables/gen_tables.py --check  exit 1 when a committed header differs from the generator's output
"""
import json, os, re, subprocess, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
OUT = str(ROOT / 'inc') + '/'
STYLE = 'file:' + str(ROOT / '_config' / '.clang-format')
CHECK = '--check' in sys.argv[1:]
_written = {}

def check_version():
    out = subprocess.run([os.environ.get('CLANG_FORMAT', 'clang-format'), '--version'], capture_output=True,
                         text=True, check=True).stdout
    if not re.search(r'version 20\.', out):
        sys.exit('the gate formats with clang-format 20, this is: ' + out.strip())

def fmt(text, name):
    cmd = [os.environ.get('CLANG_FORMAT', 'clang-format'), '--style=' + STYLE, '--assume-filename=' + name]
    once = subprocess.run(cmd, input=text, capture_output=True, text=True, check=True).stdout
    twice = subprocess.run(cmd, input=once, capture_output=True, text=True, check=True).stdout
    if once != twice:
        sys.exit('clang-format is not idempotent on ' + name)
    for n, line in enumerate(once.split('\n'), 1):
        if len(line) > 100:
            sys.exit('%s:%d is longer than 100 columns after formatting' % (name, n))
    return once

def camel(s):
    # ENABLE_X -> EnableX, 2ND_PART -> V2ndPart: the style gate wants CamelCase type aliases.
    out = ''.join(w[:1].upper() + w[1:].lower() for w in re.split(r'_+', s) if w)
    return ('V' + out) if out[:1].isdigit() else out

def emit(name, text):
    _written[name] = fmt(text, name)

check_version()
p = json.load(open(HERE / 'params.json'))
r = json.load(open(HERE / 'regs.json'))
def ident(s):
    s=re.sub(r'[^A-Za-z0-9_]','_',s)
    return ('V_'+s) if s[0].isdigit() else s
# ---------------- params
L=[]
A=L.append
A('''// Generated from the TMC9660 Parameter Mode Reference Manual (Rev. 0, 02/25), Table 57. Do not edit by hand.
/**
 * @file tmc9660_params.hpp
 * @brief The axis parameters of parameter mode (SAP, GAP, STAP, AAP): number, range, default and access of
 * every entry of Table 57, and the value constants of the coded ones.
 * @copyright Copyright (c) 2026 The driver-tmc9660 authors. SPDX-License-Identifier: MIT
 */
#ifndef TMC9660_PARAMS_HPP
#define TMC9660_PARAMS_HPP

#include <array>
#include <cstddef>
#include <cstdint>

namespace tmc9660 {

/// Access class of a parameter (the R/W column of Table 57).
enum class Access : uint8_t {
  ReadOnly = 0,       ///< R: GAP only.
  ReadWrite = 1,      ///< RW: SAP and GAP, not stored by STAP.
  ReadWriteStored = 2, ///< RWE: SAP and GAP, STAP stores it.
  WriteOnly = 3,      ///< W: SAP only.
  ReadWriteClear = 4, ///< Flags: GAP; SAP with 1 clears the bit (RWC).
};

/// One row of Table 57.
struct ParamInfo {
  uint16_t id;            ///< The parameter number (the TMCL type field).
  Access access;          ///< Access class.
  bool coded;             ///< true: only the values in `kAllowedValues[allowed_first ...]` are valid.
  uint8_t allowed_count;  ///< Number of allowed values when `coded`.
  uint16_t allowed_first; ///< Index into `kAllowedValues`.
  int64_t min;            ///< Smallest value (a flag word: 0).
  int64_t max;            ///< Largest value (a flag word: 0xFFFFFFFF).
  int64_t def;            ///< Reset default.
};

/// The axis parameters, by name; the value is the number sent as the TMCL type.
enum class Param : uint16_t {''')
def cm(t,n=96):
    t=re.sub(r'\s+',' ',(t or '').strip()).replace('*/','* /').replace('@','')
    t=re.split(r'(?<=\w{4})\. ',t)[0].rstrip('.')
    return t[:n].rstrip()
for x in p: A(f'  /// {cm(x["desc"]) or "Axis parameter "+str(x["nr"])}.\n  {x["name"]} = {x["nr"]},')
A('};\n')
allowed=[]; rows=[]
acc={'R':'ReadOnly','RW':'ReadWrite','RWE':'ReadWriteStored','W':'WriteOnly','':'ReadWriteClear'}
for x in p:
    mn,mx,df=x['min'],x['max'],x['default']
    coded=x['kind']=='set'
    first=len(allowed); cnt=0
    if coded:
        allowed+=x['allowed']; cnt=len(x['allowed'])
    if x['kind'] is None: mn,mx,df=0,4294967295,0; a='ReadWriteClear'
    else: a=acc[x['rw']]
    if x['nr']==165: pass
    rows.append((x['nr'],a,coded,cnt,first,mn,mx,df))
A(f'/// Every value a coded parameter accepts, concatenated.\ninline constexpr std::array<int32_t, {len(allowed)}> kAllowedValues{{')
for i in range(0,len(allowed),16): A('    '+', '.join(str(v) for v in allowed[i:i+16])+',')
A('};\n')
A(f'/// Table 57, sorted by number.\ninline constexpr std::array<ParamInfo, {len(rows)}> kParams{{{{')
for n,a,c,cnt,f,mn,mx,df in rows:
    A(f'    {{.id = {n}, .access = Access::{a}, .coded = {"true" if c else "false"}, .allowed_count = {cnt}, '
      f'.allowed_first = {f}, .min = {mn}LL, .max = {mx}LL, .def = {df}LL}},')
A('}};\n')
A('/**\n * @brief The table row of an axis parameter.\n * @param id The parameter number.\n * @return The row, or `nullptr` when the manual lists no such parameter.\n */\n[[nodiscard]] constexpr const ParamInfo* FindParam(uint32_t id) noexcept {\n  std::size_t lo = 0U;\n  std::size_t hi = kParams.size();\n  while (lo < hi) {\n    const std::size_t mid = lo + ((hi - lo) / 2U);\n    if (kParams[mid].id < id) {\n      lo = mid + 1U;\n    } else {\n      hi = mid;\n    }\n  }\n  if ((lo < kParams.size()) && (kParams[lo].id == id)) {\n    return &kParams[lo];\n  }\n  return nullptr;\n}\n')
A('/// Value constants of the coded parameters (the enumerations of Table 57), one namespace per parameter.\nnamespace value {')
for x in p:
    es=[(n,v) for n,v in x['enums'] if re.fullmatch(r'-?\d+',n)]
    if not es: continue
    A(f'/// Values of {x["name"]}.\nnamespace {x["name"].lower()} {{')
    seen=set()
    for n,v in es:
        nm=ident(v.strip().replace(' ','_'))
        if nm in seen: continue
        seen.add(nm)
        A(f'/// {cm(v) or nm}.\ninline constexpr int32_t {nm} = {n};')
    A('}')
A('} // namespace value\n')
A('} // namespace tmc9660\n\n#endif // TMC9660_PARAMS_HPP')
emit('tmc9660_params.hpp', '\n'.join(L) + '\n')
# ---------------- registers
for x in r:
    for f in x['fields']:
        if f['name']=='HALL_ERROR' and x['name']=='MCC_STATUS_FLAGS': f['reset']='0x0'
        if f['name']=='T_VMAX' and x['name']=='MCC_RAMPER_TIME_CONFIG': f['reset']='0x0'
        if x['name'] in ('MCC_GDRV_STATUS_EN','MCC_GDRV_PROT_EN') and f['name'].endswith('_Y') :
            f['name']=f['name']+'2'
L=[];A=L.append
A('''// Generated from the TMC9660 Register Mode Reference Manual (Rev. 0, 04/25). Do not edit by hand.
/**
 * @file tmc9660_registers.hpp
 * @brief The registers of register mode: block, offset, reset value and access masks of every register of the
 * manual's register map, and the bit fields of each.
 * @copyright Copyright (c) 2026 The driver-tmc9660 authors. SPDX-License-Identifier: MIT
 */
#ifndef TMC9660_REGISTERS_HPP
#define TMC9660_REGISTERS_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "tmc9660_detail.hpp"

namespace tmc9660 {

/// Register blocks (Table 5 of the register manual).
enum class Block : uint8_t {
  Mcc = 0,       ///< Motion control core.
  Adc = 1,       ///< ADC configuration.
  SysCtrl = 2,   ///< System control.
  RamDebug = 31, ///< RAMDebug; offsets are subcommand and index, not register numbers.
};

/// Address of a register: its block and its offset in the block.
struct RegAddr {
  Block block;     ///< The register block.
  uint16_t offset; ///< The register number in the block.
};

/// One row of the register map.
struct RegInfo {
  RegAddr addr;            ///< Where the register is.
  uint32_t reset;          ///< Reset value (the fields' reset values combined).
  uint32_t readable_mask;  ///< Bits the manual defines as fields.
  uint32_t writable_mask;  ///< Bits of fields whose type is RW (including W1C).
  uint32_t w1c_mask;       ///< Bits that clear when 1 is written.
};

/// The registers by name.
namespace regs {''')
def off(x): return f'{x["addr"]:#05x}'
blk={0:'Mcc',1:'Adc',2:'SysCtrl'}
for x in r: A(f'/// {cm(x["desc"]) or x["name"]}.\ninline constexpr RegAddr {x["name"]}{{.block = Block::{blk[x["block"]]}, .offset = {off(x)}}};')
A('} // namespace regs\n')
rows=[]
for x in sorted(r,key=lambda x:(x['block'],x['addr'])):
    reset=rd=wr=w1=0
    for f in x['fields']:
        w=f['hi']-f['lo']+1; m=((1<<w)-1)<<f['lo']
        rd|=m
        t=f['type']
        if 'RW' in t: wr|=m
        if 'W1C' in t: w1|=m
        v=f['reset']; v=int(v,16) if v.startswith('0x') else int(v)
        reset|=(v<<f['lo'])&m
    rows.append((x,reset,rd,wr,w1))
A(f'/// The register map, sorted by block and offset.\ninline constexpr std::array<RegInfo, {len(rows)}> kRegisters{{{{')
for x,rs,rd,wr,w1 in rows:
    A(f'    {{.addr = regs::{x["name"]}, .reset = {rs:#010x}U, .readable_mask = {rd:#010x}U, '
      f'.writable_mask = {wr:#010x}U, .w1c_mask = {w1:#010x}U}},')
A('}};\n')
A('''/**
 * @brief The map row of a register.
 * @param addr The register.
 * @return The row, or `nullptr` when the manual lists none at that address.
 */
[[nodiscard]] constexpr const RegInfo* FindRegister(RegAddr addr) noexcept {
  for (const RegInfo& info : kRegisters) {
    if ((info.addr.block == addr.block) && (info.addr.offset == addr.offset)) {
      return &info;
    }
  }
  return nullptr;
}
''')
A('/// Bit fields, one lower-case namespace per register: `fields::mcc_pwm_config::EnableX`.\nnamespace fields {')
for x,*_ in rows:
    A(f'/// Fields of {x["name"]}.\nnamespace {x["name"].lower()} {{')
    seen=set()
    for f in x['fields']:
        w=f['hi']-f['lo']+1
        if w>=32: continue
        n=camel(f['name'])
        if n in seen: continue
        seen.add(n)
        A(f'/// Bits {f["hi"]}:{f["lo"]}, {f["type"]}: the manual\'s {f["name"]}.\nusing {n} = detail::Field<uint32_t, {f["lo"]}U, {w}U>;')
    A('}')
A('} // namespace fields\n\n} // namespace tmc9660\n\n#endif // TMC9660_REGISTERS_HPP')
emit('tmc9660_registers.hpp', '\n'.join(L) + '\n')
bad = 0
for name, text in _written.items():
    path = Path(OUT + name)
    if CHECK:
        if not path.exists() or path.read_text() != text:
            print('out of date: inc/' + name)
            bad = 1
    else:
        path.write_text(text)
sys.exit(bad)
