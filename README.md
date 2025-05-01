# NUFS: A Tiny Filesystem Shell in C

This project implements a simplified Unix-like filesystem shell, referred to as **NUFS** (Northeastern University File System), built in C for macOS. It is layered over FUSE (Filesystem in Userspace) and handles core filesystem operations including directory navigation, file creation/deletion, reading/writing, linking, and attribute management.

The implementation is backed by a minimalist disk abstraction, inodes, directory entries, and a bitmap-managed block storage system. It includes a deterministic finite state tokenizer to parse and resolve path components and simulate a realistic shell interface.

---

## ✨ Features

- Full FUSE-based mountable filesystem
- Single-block file support (4KB max per file)
- Directory creation (`mkdir`) and deletion (`rmdir`)
- File creation, writing, reading, and truncation
- Hard links (`link`) and file renaming (`rename`)
- Deterministic tokenizer for path parsing (via linked string list)
- Manual inode and block management (like a real filesystem)
- Debug-friendly logging for every syscall

---

## 🗂 File Overview

| File              | Purpose                                                |
|-------------------|--------------------------------------------------------|
| `nufs.c`          | Entry point and FUSE operation bindings                |
| `storage.c/.h`    | High-level filesystem logic (backing implementation)   |
| `inode.h`         | Inode abstraction (reference count, mode, size, blocks)|
| `slist.c/.h`      | Linked list utility for path tokenization              |
| `directory.*`     | Directory management and entries (if included)         |
| `blocks.*`        | Block allocation and management (if included)          |
| `bitmap.*`        | Bitmap allocator for inodes/blocks (if included)       |
| `Makefile`        | Build system using `gcc` and FUSE                      |
| `test.pl`         | Perl script for automated testing (optional)           |

---

## ⚙️ Build Instructions (macOS)

> Requires [FUSE for macOS](https://osxfuse.github.io/) and `gcc`.

```bash
make
```

Then to mount the filesystem:

```bash
mkdir mnt
./nufs mnt
```

To unmount:

```bash
umount mnt
```

---

## 🧪 Example Usage

Once mounted:

```bash
cd mnt
touch file.txt
echo "hello world" > file.txt
cat file.txt
mkdir mydir
ln file.txt mydir/hardlink.txt
rm file.txt
cat mydir/hardlink.txt
```

All filesystem operations (including metadata and data changes) are handled by the custom NUFS layer.

---

## 🧠 Internals

- **Storage**: a file-based block device (`data.nufs`) simulates physical disk
- **Inodes**: track file metadata, block mappings, and reference counts
- **Tokenizer**: implemented via `slist` to tokenize path strings
- **Directories**: map file names to inode numbers with local path resolution
- **Syscalls**: traced via `printf` for step-by-step debugging

---

## 📁 Persistent Data

On first run, a file called `data.nufs` is created to simulate persistent disk storage. The root directory is initialized with standard permissions and link count.

---

## ✅ Project Status

| Component            | Status       |
|----------------------|--------------|
| File I/O             | ✅ Working   |
| Directory operations | ✅ Working   |
| Hard links           | ✅ Working   |
| FUSE integration     | ✅ Working   |
| Multi-block files    | ❌ Not yet   |
| Symbolic links       | ❌ Not implemented |

---

## 🧑‍💻 Author

**Ryan Mahshie**  
Computer Science Student, Northeastern University  
Concentration: Artificial Intelligence  
Website: [ryan-mahshie.xyz](https://ryan-mahshie.xyz)

---

## 📄 License

This project is licensed under the MIT License. You are free to use, modify, and distribute it with attribution.
