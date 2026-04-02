## 2024-05-24 - Avoid Disk I/O for Intermediate XML Parsing
**Learning:** The `doctxt` tool previously extracted XML data from zip archives, wrote it to a temporary disk file, and read it back for libxml2 parsing. This not only caused unnecessary disk I/O overhead but also posed a security risk due to predictable temporary filenames.
**Action:** Always parse extracted archive contents directly from memory using functions like `xmlReadMemory` rather than writing intermediate data to disk.
