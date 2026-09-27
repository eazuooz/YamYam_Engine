from pathlib import Path
import json, re, sys

HERE = Path(__file__).resolve().parent
inventory = json.loads((HERE/'inventory.json').read_text(encoding='utf-8'))
sys.stdout.reconfigure(encoding='utf-8')
for i in range(int(sys.argv[1]), min(int(sys.argv[2]),len(inventory))):
    item = inventory[i]
    p = json.loads((HERE/'original'/f"{item['id']}.json").read_text(encoding='utf-8'))
    s = p['text'].split('<content>\n',1)[-1].split('\n</content>',1)[0]
    blocks = list(re.finditer(r'(?m)^([\t ]*)```([^\n]*)\n([\s\S]*?)^\1```',s))
    print(f"\nPAGE {i} {item['id']} {p['title']} ({len(s)} chars, {len(blocks)} code blocks)")
    if len(sys.argv)>3 and sys.argv[3]=='code':
        for n,b in enumerate(blocks):
            print(f'\nCODE {n} [{b[2]}]\n{b[3]}')
    else:
        for n,b in reversed(list(enumerate(blocks))):
            lines=b[3].strip().splitlines()
            preview=' / '.join(x.strip() for x in lines[:3])
            s=s[:b.start()]+f'[CODE {n}: {b[2]}, {len(lines)} lines, {preview}]'+s[b.end():]
        s = re.sub(r'!\[([^\]]*)\]\([^\n]+',r'[IMAGE: \1]',s)
        s = re.sub(r'https?://[^\s)>"<]+', '[URL]', s)
        s = re.sub(r'\n(?:<empty-block/>\n)+','\n',s)
        print(s)
