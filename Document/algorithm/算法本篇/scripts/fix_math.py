# Fix math delimiters for KaTeX compatibility
import re

path = r'd:\WorkShop\EmbeddedNS-Portable\Document\algorithm\算法本篇\频域维纳滤波降噪算法.md'

with open(path, 'r', encoding='utf-8') as f:
    content = f.read()

lines = content.split('\n')
result = []
i = 0

math_indicators = [
    '\\hat', '\\frac', '\\begin', '\\sin', '\\log', '\\ell',
    '\\alpha', '\\gamma', '\\theta', '\\bar', '\\sqrt',
    '\\underbrace', '\\max', '\\exp', '\\tanh', '\\left',
    '\\right', '\\cdot', '\\sum', '\\text', 'x(n) =',
    'H(z) =', 'w(n) =', 'y(n) =', 'G(k) =', 'G_{',
    'P_{', 'I_0 =', 'I_1 =', 'I_2 =', 'S_1 =', 'S_2 =',
    'g =', 'g_{', 'D =', 'F =', 'P_s(k)', 'P_{prior}',
]

while i < len(lines):
    line = lines[i]
    stripped = line.strip()
    is_math_block = (
        stripped != ''
        and '$' not in line
        and not line.startswith('#')
        and not line.startswith('|')
        and not line.startswith('*')
        and not line.startswith('-')
        and not line.startswith('!')
        and not line.startswith('>')
        and not line.startswith('1.')
        and not line.startswith('2.')
        and not line.startswith('3.')
        and not line.startswith('4.')
        and not line.startswith('5.')
        and i > 0
        and lines[i-1].strip() == ''
        and any(cmd in line for cmd in math_indicators)
    )

    if is_math_block:
        block_lines = [line]
        j = i + 1
        while j < len(lines) and lines[j].strip() != '':
            block_lines.append(lines[j])
            j += 1
        if j < len(lines) and lines[j].strip() == '':
            result.append('$$')
            result.extend(block_lines)
            result.append('$$')
            i = j
            continue

    result.append(line)
    i += 1

content = '\n'.join(result)

with open(path, 'w', encoding='utf-8') as f:
    f.write(content)

print("Done - block math $$ delimiters restored.")
