# TD3/Crave sequence file dumper/converter
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Hit Counter](https://hitcounter.pythonanywhere.com/count/tag.svg?url=https://github.com/claziss/CraveSeq)](https://github.com/brentvollebregt/hit-counter)


Simple tool that parses Behringer's Crave and TD3 SEQ file and dumps them in a text format. Also it can convert TD3 SEQ files to CRAVE SEQ files.

## Building

``` make txtdump```

## Visualize Crave SEQ file.

``` ./txtdump my.seq```

or

``` ./txtdump -c my.seq```

## Visualize TD3 SEQ file.

```./txtdump -t my.seq```

## Visiualize TD3 SEQ file and convert it to CRAVE's SEQ file.

```./txtdump -d my.seq```

