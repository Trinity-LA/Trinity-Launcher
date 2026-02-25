import xml.etree.ElementTree as ET
import sys

def get_unfinished(filename):
    tree = ET.parse(filename)
    root = tree.getroot()
    unfinished = set()
    for message in root.iter('message'):
        trans = message.find('translation')
        if trans is not None and trans.get('type') == 'unfinished':
            source = message.find('source')
            if source is not None and source.text:
                unfinished.add(source.text)
    return list(unfinished)

ca = get_unfinished('resources/i18n/trinity_ca.ts')
uk = get_unfinished('resources/i18n/trinity_uk.ts')

print("Unfinished texts:")
for u in set(ca + uk):
    print(repr(u))
