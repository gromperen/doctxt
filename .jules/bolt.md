## 2025-03-31 - Parsing XML directly from memory in C
**Learning:** Using an intermediate temporary disk file for `zip` to `xmlRead` was creating unnecessary I/O latency. Reading zip files directly into a memory buffer and providing it to `xmlReadMemory` bypasses disk.
**Action:** When extracting data from zip files to be processed by an external library in C, avoid temporary disk files if the library supports memory buffer parsing.
