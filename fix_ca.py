import re
with open('resources/i18n/trinity_ca.ts', 'r', encoding='utf-8') as f:
    text = f.read()

# Remove spaces inside brackets like < message > to <message>
text = re.sub(r'<\s+([/\w]+)\s+>', r'<\1>', text)
# Handle tags with attributes like < translation type="vanished" >
text = re.sub(r'<\s+([/\w]+)(.*?)\s+>', r'<\1\2>', text)
# Fix the broken Resource Pack tag
text = text.replace('<source>Resource  type="unfinished"Pack</source>', '<source>Resource Pack</source>')

with open('resources/i18n/trinity_ca.ts', 'w', encoding='utf-8') as f:
    f.write(text)
