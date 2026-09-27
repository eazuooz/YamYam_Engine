from pathlib import Path
import json,re,difflib
HERE=Path(__file__).resolve().parent
items=json.loads((HERE/'inventory.json').read_text(encoding='utf-8'))
def body(d):return d['text'].split('<content>\n',1)[1].split('\n</content>',1)[0]
def tokens(s):
    lines=s.splitlines(keepends=True); out=[]; media=0
    for line in lines:
        if re.search(r'!\[|<(?:image|video|file|unknown)\b',line):
            out.append(f'__MEDIA_{media}__\n');media+=1
        else:out.append(line)
    return lines,out
plans=[]
for i in [14,15,40,41,42,43,44,45]:
    it=items[i]
    original=body(json.loads((HERE/'original'/f"{it['id']}.json").read_text(encoding='utf-8')))
    current=body(json.loads((HERE/'math_restore'/f'current_{i}.json').read_text(encoding='utf-8')))
    old,ot=tokens(original); cur,ct=tokens(current)
    assert sum(x.startswith('__MEDIA') for x in ot)==sum(x.startswith('__MEDIA') for x in ct),(i,'media count mismatch')
    changes=[]
    for tag,a,b,c,d in difflib.SequenceMatcher(a=ct,b=ot,autojunk=False).get_opcodes():
        if tag=='equal':continue
        old_str=''.join(cur[a:b]).rstrip('\n');new_str=''.join(old[c:d]).rstrip('\n')
        if not old_str.strip() and not new_str.strip():continue
        assert not any(x.startswith('__MEDIA') for x in ct[a:b]+ot[c:d]),(i,'media in replacement')
        if not old_str.strip():
            # Anchor insertion to a preceding unchanged plain-text line.
            assert a>0 and not ct[a-1].startswith('__MEDIA'),(i,'no insertion anchor')
            old_str=cur[a-1].rstrip('\n');new_str=old_str+'\n'+new_str
        if current.count(old_str)!=1:
            # Extend to following unchanged text, keeping media out.
            assert b<len(cur) and not ct[b].startswith('__MEDIA'),(i,'ambiguous anchor',old_str[:50])
            old_str+='\n'+cur[b].rstrip('\n');new_str+='\n'+cur[b].rstrip('\n')
        assert current.count(old_str)==1,(i,'ambiguous',old_str[:80])
        changes.append(dict(old_str=old_str,new_str=new_str))
    plans.append(dict(index=i,page_id=it['id'],content_updates=changes))
    print(i,len(changes),'targeted changes; native media preserved')
(HERE/'math_restore'/'plans.json').write_text(json.dumps(plans,ensure_ascii=False,indent=2),encoding='utf-8')
