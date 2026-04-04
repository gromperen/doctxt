## 2024-04-04 - In-memory parsing vs Disk I/O
**Learning:** Writing intermediate extracted files to disk (`/tmp/doctxt-temp.txt`) and reading them back for XML parsing added unnecessary overhead and disk I/O, slowing down execution significantly, especially since the file fits in memory.
**Action:** When extracting data from archives like zip, parse the data directly from memory using APIs like `xmlReadMemory` rather than writing intermediate files to disk, to save system calls and improve performance.
