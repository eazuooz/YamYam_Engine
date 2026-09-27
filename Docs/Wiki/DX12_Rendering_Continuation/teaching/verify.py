from edit import H,MEDIA,Edit
import re,json,sys
def media_keys(body):
    out=[]
    for m in MEDIA.finditer(body):
        s=re.sub(r'https?://[^\s)"<>]+',lambda u:u.group().split('?')[0],m.group())
        out.append(s)
    return out
for a in sys.argv[1:]:
    i=int(a);e=Edit(i)
    d=json.loads((H/'final'/f'{i}.json').read_text(encoding='utf-8'))
    body=d['text'].split('<content>\n')[1].split('\n</content>')[0]
    old=media_keys(e.original);new=media_keys(body)
    print(i, 'media_preserved',old==new,'counts',len(old),len(new),
          'unknown_blocks',e.data.get('unknown_block_count',0),d.get('unknown_block_count',0),
          'code_fences',len(re.findall(r'(?m)^```',body)))
    assert old==new
