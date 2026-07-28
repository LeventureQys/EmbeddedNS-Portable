import re
path = r'd:\WorkShop\EmbeddedNS-Portable\Document\algorithm\算法本篇\频域维纳滤波降噪算法.md'
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()
blocks = re.findall(r'\$\$.*?\$\$', c, re.DOTALL)
inlines = re.findall(r'(?<!\$)\$(?!\$)(.+?)(?<!\$)\$(?!\$)', c)
print(f'Block math: {len(blocks)}')
print(f'Inline math: {len(inlines)}')
print(f'Total lines: {len(c.splitlines())}')
# Check for any remaining old-style delimiters
old_inline = c.count('\\(')
old_block = c.count('\\[')
print(f'Remaining backslash-paren: {old_inline}')
print(f'Remaining backslash-bracket: {old_block}')
