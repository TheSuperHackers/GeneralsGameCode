# Unicode paragraph direction

`Get_Paragraph_Base_Level` provides the initial paragraph level for complex text
layout, such as Uniscribe itemization. It implements the first-strong
decision in [UAX #9 P2/P3](https://www.unicode.org/reports/tr9/tr9-51.html#P2),
with scanning bounded to the first paragraph. Shaping and visual reordering are
left to the renderer.

The scan skips nested directional-isolate contents, decodes UTF-16 surrogate
pairs, and ignores lone surrogates. It also accepts direct supplementary scalar
values when `wchar_t` is 32 bits. The first L character selects level 0; the first
R or AL character selects level 1. A paragraph without a strong character
defaults to level 0.

Both BMP and supplementary classification use the same Unicode 17.0.0 data,
independent of the operating system. The compact table merges R and AL and
treats every other non-L class as neutral for this decision. Omitted ranges
default to L, including the source's `@missing` defaults.

## Data and license

Download [DerivedBidiClass.txt](https://www.unicode.org/Public/17.0.0/ucd/extracted/DerivedBidiClass.txt)
and regenerate or check the table from the repository root:

```
python Dependencies/UnicodeBidi/generate_bidi.py DerivedBidiClass.txt
python Dependencies/UnicodeBidi/generate_bidi.py DerivedBidiClass.txt --check
```

The generator validates the source version and SHA-256 before extracting ranges.
The generated data retains Unicode's copyright notice; `unicode-license.txt`
contains the full Unicode License v3.
