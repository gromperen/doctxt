## 2024-05-18 - Eliminate disk I/O bottleneck by parsing XML from memory
**Learning:** `libxml2` provides functions like `xmlReadMemory` which allow parsing XML data directly from memory buffers without needing to write it out to a file on disk first and then read it back.
**Action:** When extracting data from zip files or APIs to process with a parser, always look for parser APIs that accept memory buffers directly instead of creating intermediate temporary files.
