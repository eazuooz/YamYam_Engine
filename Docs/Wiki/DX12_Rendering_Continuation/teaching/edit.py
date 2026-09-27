from pathlib import Path
import json,re
H=Path(__file__).resolve().parent
INV=json.loads((H.parent/'audit/inventory.json').read_text(encoding='utf-8'))
PROTECTED={14,15,40,41,42,43,44,45}
MEDIA=re.compile(r'(?m)^[\t ]*(?:!\[[^\n]*|<(?:video|audio|file|unknown)[^\n]*)$')
FENCE=re.compile(r'(?m)^([\t ]*)```([^\n]*)\n([\s\S]*?)^\1```')
class Edit:
    def __init__(self,i):
        assert i not in PROTECTED
        self.i=i;self.meta=INV[i]
        self.data=json.loads((H/'current'/f'{i}.json').read_text(encoding='utf-8'))
        self.original=self.data['text'].split('<content>\n')[1].split('\n</content>')[0]
        self.body=self.original;self.updates=[]
        self.codes=[m.group() for m in FENCE.finditer(self.body)]
        self.media=[m.group() for m in MEDIA.finditer(self.body)]
    def replace(self,old,new):
        assert self.body.count(old)==1,(self.i,'nonunique',old[:100],self.body.count(old))
        assert not MEDIA.search(old),(self.i,'media in target')
        self.body=self.body.replace(old,new,1)
        self.updates.append({'old_str':old,'new_str':new})
    def pattern(self,pattern,new):
        hits=list(re.finditer(pattern,self.body,re.S))
        assert len(hits)==1,(self.i,pattern,len(hits))
        self.replace(hits[0].group(),new)
    def intro(self,new):
        self.pattern(r'\A<callout[^>]*>.*?</callout>',new.strip())
    def code(self,i,before='',after='',replacement=None):
        old=self.codes[i]
        new=old if replacement is None else replacement.strip()
        self.replace(old,('\n\n'.join(x.strip() for x in (before,new,after) if x)).strip())
    def tail(self,heading,new):
        self.replace(self.body[self.body.index(heading):],new.strip())
    def rewrite(self,draft):
        assert not self.updates
        new=(H/draft).read_text(encoding='utf-8').strip()
        expected=[f'{{{{M{i}}}}}' for i in range(len(self.media))]
        actual=re.findall(r'\{\{M\d+\}\}',new)
        assert actual==expected,(self.i,actual,expected)
        old_parts=MEDIA.split(self.original)
        new_parts=re.split(r'\{\{M\d+\}\}',new)
        assert len(old_parts)==len(new_parts)
        # Every media block remains untouched. Work from bottom up so repeated
        # transitional text cannot become an accidental later replacement target.
        for old,part in reversed(list(zip(old_parts,new_parts))):
            if old==part:continue
            if not re.sub(r'<empty-block/>|\s','',old):
                if not part.strip():continue
                assert old.strip() and self.body.count(old)==1,(self.i,'ambiguous media-only gap')
            assert old.strip(),(self.i,'empty media gap')
            self.replace(old,part)
    def save(self):
        (H/'plans').mkdir(exist_ok=True)
        (H/'final').mkdir(exist_ok=True)
        (H/'plans'/f'{self.i}.json').write_text(json.dumps({'index':self.i,**self.meta,'content_updates':self.updates},ensure_ascii=False,indent=2),encoding='utf-8')
        (H/'final'/f'{self.i}.md').write_text(self.body,encoding='utf-8')
        print(self.i,len(self.updates),'edits',len(self.original),'->',len(self.body),'characters')
