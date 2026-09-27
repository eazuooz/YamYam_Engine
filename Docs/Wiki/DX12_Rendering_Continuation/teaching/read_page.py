from pathlib import Path
import json,re,sys
H=Path(__file__).resolve().parent
i=int(sys.argv[1]);d=json.loads((H/'current'/f'{i}.json').read_text(encoding='utf-8'))
s=d['text'].split('<content>\n')[1].split('\n</content>')[0]
if len(sys.argv)>2 and sys.argv[2]=='code':
    for j,b in enumerate(re.finditer(r'(?m)^([\t ]*)```([^\n]*)\n([\s\S]*?)^\1```',s)):print('CODE',j,b[2],b[3])
else:
    for j,b in reversed(list(enumerate(re.finditer(r'(?m)^([\t ]*)```([^\n]*)\n([\s\S]*?)^\1```',s)))):
        s=s[:b.start()]+f'[CODE {j}: {b[2]}, '+b[3].strip().split('\n')[0]+']'+s[b.end():]
    s=re.sub(r'!\[([^\]]*)\]\([^\n]+',r'[IMAGE:\1]',s)
    s=re.sub(r'<(?:video|file|unknown)[^\n]+','[MEDIA]',s)
    s=re.sub(r'https?://[^\s)>"<]+','[URL]',s)
    a=int(sys.argv[2]) if len(sys.argv)>2 else 0
    b=int(sys.argv[3]) if len(sys.argv)>3 else 20000
    print(s[a:b])
