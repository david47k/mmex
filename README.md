# mmex
Resource extractor for MMFW resource files:
```
  .MMP,.PIC       MMFW Pictures
  .MMB            MMFW Blobs
  .MMA,.SND       MMFW Sounds
  .MMF            MMFW Films
  .MMS            MMFW 3 Script (of type MM)
  .VEC            Vector File Version 1.0
```   
Related file types, but not extracted by this program:
```     
  .MMS            MMFW 3 Script (of type II)
  .ITX            SKU Text Binary v2 
```   
Found in a number of software programs from the late 90s.
Found in software developed by [ImageBuilder Software, Inc.](https://www.mobygames.com/company/imagebuilder-software-inc)
Found in software developed by D\*\*\*\*y Interactive.
   
This file format can vary a bit between different files, even though they have the same header.
There is a -offset command line parameter which may help in some situations.

The file format is big-endian.      

## License
GNU General Public License version 2 or any later version (GPL-2.0-or-later).

## Usage
```
mmex inputFile -offset hexOffset -dump prefix -usenames -ext extension

  inputFile               a compatible file. the only required parameter.
  -offset hexOffset       specify the file offset where the 16-bit resource count
                          is. e.g. -offset 1A
  -dump prefix            dumps the files out with the specified prefix.
                          e.g. -dump output_folder/
  -usenames               when dumping, use resource names as filenames.
  -ext extension          when dumping, use the specified file extension.
                          e.g. -ext .cgm

```
