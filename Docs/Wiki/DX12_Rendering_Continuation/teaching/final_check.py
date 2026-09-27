from edit import H,MEDIA,FENCE,INV,PROTECTED
from finish_lessons import NewEdit
import json,re,difflib

def read(p):return json.loads(p.read_text(encoding='utf-8'))
def body(d):return d['text'].split('<content>\n',1)[1].split('\n</content>',1)[0]
def clean_urls(s):return re.sub(r'https?://[^\s)"<>]+',lambda m:m.group().split('?')[0],s)
def media(s):return [clean_urls(m.group()) for m in MEDIA.finditer(s)]
def normalize(s):
    s=clean_urls(s).replace('<empty-block/>','')
    return re.sub(r'\s+','',s)

results=[]
for i in sorted(set(range(46))-PROTECTED):
    before=read(H/'current'/f'{i}.json')
    p=H/'final'/f'{i}.json'
    after=read(p if p.exists() else H/'current'/f'{i}.json')
    b=body(after)
    assert media(body(before))==media(b),(i,'changed native media')
    assert '{{M' not in b,(i,'leftover media token')
    assert len(re.findall(r'(?m)^\s*```',b))%2==0,(i,'unpaired code fence')
    assert '<content>' not in b,(i,'nested envelope')
    results.append(dict(index=i,media=len(media(b)),code_blocks=len(list(FENCE.finditer(b))),characters=len(b)))
for i in [46,47,'hub11','hub12']:
    e=NewEdit(i);d=read(H/'final'/f'{i}.json');b=body(d)
    assert media(e.original)==media(b),(i,'changed media')
    assert normalize(e.body)==normalize(e.original) # Fresh input is untouched by this helper.
    # Compare native references individually; prose formatting can be normalized by Notion.
    refs=lambda s:[normalize(m) for m in re.findall(r'<(?:page|mention-page|unknown)\b[^\n]*',clean_urls(s))]
    assert refs(e.original)==refs(b),(i,'changed native references')
    if i=='hub11':
        start=e.original.index('<unknown');end=e.original.index('<page',start)
        assert e.original[start:end] in b,'math link segment changed'
    results.append(dict(index=i,media=len(media(b)),code_blocks=len(list(FENCE.finditer(b))),characters=len(b)))

for i in sorted(PROTECTED):
    old=body(read(H.parent/'audit/original'/f"{INV[i]['id']}.json"))
    new=body(read(H/'final'/f'math_{i}.json'))
    assert media(old)==media(new),(i,'protected media changed')
    if normalize(old)!=normalize(new):
        # Print only unsiged prose excerpts for diagnosis, never token-bearing URLs.
        diff=list(difflib.unified_diff(clean_urls(old).splitlines(),clean_urls(new).splitlines()))
        print(i,'DIFF', '\n'.join(diff[:60]));raise AssertionError((i,'protected text changed'))
    print('protected',i,'original text and media preserved')

(H/'verification.json').write_text(json.dumps(results,ensure_ascii=False,indent=2),encoding='utf-8')
print('Verified',len(results),'edited lesson/hub pages; protected math pages:',len(PROTECTED))
