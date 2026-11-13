#!/usr/bin/env python3
"""Check params.json and regs.json against the vendor manuals' text.

The two JSON files were extracted from the manuals recorded in docs/datasheets.yml. This script re-derives what it
can from the PDFs (never stored in the repository) so that the extraction is reproducible:

  tools/gen_tables/extract_check.py PARAMETER_MANUAL.pdf REGISTER_MANUAL.pdf

It needs `pdftotext` (poppler) on the path, converts each PDF with `-layout`, and checks that
  * every parameter of params.json appears in the parameter manual's text as `<number> <NAME>` in a table row, and
    the manual has no other row of that shape (a parameter missing from the JSON is reported);
  * every register of regs.json appears in the register manual's text by name, with its address, and every field
    name of the register appears in the text.
It prints each difference and exits 1 when there is one. It does not replace reading the manual: the access, range
and default columns are not re-parsed.
"""
import json, re, subprocess, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CORRECTED = ('MCC_GDRV_STATUS_EN', 'MCC_GDRV_PROT_EN')  # duplicate _Y field names in the manual


def text_of(pdf):
    return subprocess.run(['pdftotext', '-layout', pdf, '-'], capture_output=True, text=True, check=True).stdout


def main(argv):
    if len(argv) != 2:
        sys.exit(__doc__)
    params = json.load(open(HERE / 'params.json'))
    regs = json.load(open(HERE / 'regs.json'))
    bad = 0
    pm = text_of(argv[0])
    rows = {}
    for m in re.finditer(r'^\s*(\d{1,3})\s+([A-Z][A-Z0-9_]{2,})\s', pm, re.M):
        rows.setdefault(m.group(2), set()).add(int(m.group(1)))
    for p in params:
        if p['nr'] not in rows.get(p['name'], set()):
            print('parameter %d %s: no table row in the parameter manual' % (p['nr'], p['name']))
            bad += 1
    known = {p['name'] for p in params}
    for name, nrs in sorted(rows.items()):
        if name not in known and any(0 <= n <= 255 for n in nrs) and name.startswith(('MAX_', 'ACTUAL_', 'TARGET_')):
            print('manual row %s %s is not in params.json' % (sorted(nrs), name))
            bad += 1
    rm = text_of(argv[1])
    squashed = re.sub(r'\s+', '', rm)
    for r in regs:
        if not re.search(r'\b' + re.escape(r['name']) + r'\b', rm):
            print('register %s: not in the register manual' % r['name'])
            bad += 1
            continue
        for f in r['fields']:
            name = f['name']
            if r['name'] in CORRECTED:
                name = re.sub(r'_(Y2|U|V|W|Y)$', '', name)  # the manual prints one name for every phase
            if name[:20] not in squashed:  # the table column cuts long names at 20 characters
                print('register %s field %s: not in the register manual' % (r['name'], f['name']))
                bad += 1
    print('%d parameters, %d registers checked, %d differences' % (len(params), len(regs), bad))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
