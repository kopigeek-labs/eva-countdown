#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Generate compact UI lettering from the existing vendored OFL fonts."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
ROOT = Path(__file__).resolve().parents[1]
FONTS = ROOT / 'assets/fonts'

def emit(name, im):
    im = im.point(lambda p: 255 if p >= 128 else 0).convert('1', dither=Image.Dither.NONE)
    data = im.tobytes()
    return 'inline constexpr unsigned char '+name+'[] = {\n'+''.join(
        ','.join(f'0x{x:02x}' for x in data[i:i+24])+',\n' for i in range(0,len(data),24))+'};\n'

out = '// Generated OFL font-derived UI assets; see THIRD_PARTY.md.\n#pragma once\n'
font = ImageFont.truetype(str(FONTS/'BarlowCondensed-Regular.ttf'), 18)
widths=[]
for i in range(32,127):
    c=chr(i)
    w=min(12,max(3,round(font.getlength(c))))
    im=Image.new('L',(12,20))
    ImageDraw.Draw(im).text((0,-2),c,font=font,fill=255)
    out+=emit(f'kLetter{i}',im)
    widths.append(w)
out+='inline constexpr const unsigned char* kLetters[]={'+','.join(f'kLetter{i}' for i in range(32,127))+'};\n'
out+='inline constexpr unsigned char kAdvances[]={'+','.join(map(str,widths))+'};\n'
jp=ImageFont.truetype(str(FONTS/'ZenAntique-Regular.ttf'),48)
for name,text in [('Clock','現在時刻'),('Pomodoro','集中時間')]:
    box=jp.getbbox(text)
    im=Image.new('L',(box[2]-box[0],box[3]-box[1]))
    ImageDraw.Draw(im).text((-box[0],-box[1]),text,font=jp,fill=255)
    out+=emit(f'kTitle{name}',im.resize((84,20),Image.Resampling.LANCZOS))
(ROOT/'main/ui_assets.h').write_text(out)
print('Generated UI alphabet and two Japanese mode titles')
