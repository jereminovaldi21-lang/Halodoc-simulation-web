# Simulasi Web Kesehatan Halodoc
Program ini merupakan Tugas Besar IF1210 Algoritma dan Pemrograman 1 2026.

## Deskripsi Singkat Program
Program ini adalah simulasi web browser tentang web kesehatan Halodoc. Program memiliki berbagai fitur yang dapat digunakan oleh pengguna mulai dari menjelajah, mencari halaman, membuka halaman, berganti tab browser, melihat history tab, mengunduh halaman, menambahkan halaman, menghapus halaman, mengedit halaman dan berbagai fitur lainnya beserta simulasi layanan antrian pada dokter. 


## Struktur Folder Program

```text
.
├── Makefile                            # Instruksi kompilasi otomatis
├── readme.MD
├── header/                             # Tempat menyimpan semua file header (.h)
│   └── halodoc.h                       # ANSI Color, Constanta, Prototypes, Structs, Global state  
├── src/                                # Tempat menyimpan semua file sumber (.c)
|   ├── utils.c                         # Banner, LCG, URL regex, Cache, Web Graph, Web page management
|   ├── fileio.c                        # Read CSV, Load Data, Save Data
|   ├── browser.c                       # Tab management, History Navigation, Discover, Search
|   ├── halodoc_features.c              # Download Manager, Dokter, Antrian, Obat
│   └── main.c                          # Tempat program berjalan halaman awal, helper 
├── doc/
|   ├── IF1210_LaporanTB_MS1_K05-K.pdf # Laporan milestone 1
|   └── IF1210_LaporanTB_MS2_K05-K.pdf # Laporan milestone 2
├── config/
|   ├── dokter.csv                      # Data dokter beserta halaman
|   ├── linked_pages.csv                # Data halaman tertaut antara sumber & tujuan
|   ├── obat.csv                        # Data obat-obatan beserta halaman
|   ├── config.txt                      # Konfigurasi konstanta global & status data global
|   └── web_pages.csv                   # Data laman web beserta konten
├── bin/                              # Folder output hasil kompilasi (otomatis dibuat)
|   └── halodoc                         # Executable file (hasil build)
└── .gitignore                          # Abaikan hasil kompilasi ketika push
```

## Cara Kompilasi dan Menjalankan Program

Pastikan sistem Anda sudah terinstal:
- **gcc** (GNU Compiler Collection) - untuk kompilasi program C
- **make** - untuk automasi build process

Untuk mengecek apakah sudah terinstal, jalankan:
```bash
gcc --version
make --version
```

### Cara Menjalankan Program

#### Build Program
Untuk mengkompilasi program:
```bash
make build
```
atau cukup:
```bash
make
```

#### Run Program
Untuk build dan menjalankan program:
```bash
make run
```
Jika sudah ada file kompilasi hasil build
```bash
./bin/halodoc config/
```

#### Clean Build
Untuk menghapus hasil kompilasi:
```bash
make clean
```

## Daftar Fitur 

### Fitur Utama
| No | Fitur | Penjelasan Singkat | Status |
| -------- | -------- | -------- | -------- |
| 1 | Discover          | Menampilkan 5 url secara acak dengan algoritma LCG (*Linear Congruential Generator*)  | ✅  |
| 2 | Search            | Mencari web yang ingin dituju oleh user  | ✅  |
| 3 | Open Page         | Menampilkan konten dari url web  | ✅  |
| 4 | Caching           | Menampilkan konten dari url web dengan sistem caching berupa *Cache-Hit* dan *Cache-Miss*  | ✅  | 
| 5 | Page Management   | Menambahkan, menyunting, dan menghapus halaman  | ✅  |
| 6 | Tabs              | Membuka tab baru, menutup tab aktif, memeriksa tab aktif, dan berpindah antar tab  | ✅  |
| 7 | Back/Forward      | Navigasi halaman web sebelum dan sesudah  | ✅  |
| 8 | Tabs History      | Melihat history halaman yang dibuka  | ✅  |
| 9 | Web Graph         | Memproses relasi dan menampilkan halaman yang tertaut pada sebuah halaman | ✅  |
| 10 | Download Manager | mengunduh halaman dengan basis waktu *tick* | ✅  |
| 11 | Exit             | Menghentikan program | ✅  |

### Fitur Tambahan
| No | Fitur | Penjelasan Singkat | Status |
| -------- | -------- | -------- | -------- |
| 1 | Web Graph         | Penyimpanan data dengan ADT Matriks  | ✅ |
| 2 | Download Manager  | Sistem keluaran file txt  | ✅ |
| 3 | Load              | Load otomatis saat program mulai dijalankan dan saat di menu utama  | ✅ |
| 4 | Save              | Menyimpan Perubahan dengan perintah  | ✅ |
| 5 | Exit              | Memberikan opsi save saat keluar program  | ✅ |


## Anggota Kelompok dan Pembagian Tugas
| No | NIM | Nama | Tugas |
| -------- | -------- | -------- | -------- |
| 1 | Nathanael Hans Limas          | 13225037  | Implementasi command loop dan parser perintah, pengaturan alur menu utama, membantu debugging dan integrasi, serta menulis laporan  |
| 2 | Jordan Berlian Widanarta      | 13225039  | Implementasi fitur Web Graph, mengelola dan validasi linked pages, membantu pengujian openlinked, serta menulis laporan  |
| 3 | Faishal Ananda Haris          | 13225055  | Implementasi fitur Page Management, termasuk add_page, edit_page, dan delete_page, mengelola halaman web dan validasi URL, serta menulis laporan  |
| 4 | Alexander Abie Kuntara        | 13225097  | Implementasi Download Manager berbasis Queue, pembuatan file hasil download, serta menulis laporan  |
| 5 | Zaidan Dzaky Ibrahim	        | 13225107  | Implementasi fitur tab dan navigasi browser, termasuk newtab, closetab, nexttab, prevtab, back, forward, dan view_tab_history, serta menulis laporan  |
| 6 | Jeremy Novaldy Eka Putra      | 18325035  | Implementasi fitur Load dan Save, pembacaan dan penulisan file CSV/TXT, serta menulis laporan  |