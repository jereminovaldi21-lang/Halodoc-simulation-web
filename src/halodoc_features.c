// File: halodoc_features.c
// Download manager, Halodoc features: dokter, antrian, obat

#include "../header/halodoc.h"

// DOWNLOAD MANAGER (F10) 
void initDownloadQueue(void) {
    dlQueue.front = 0;
    dlQueue.rear = 0;
    dlQueue.count = 0;
}

void downloadPage(const char* url) {
    if (dlQueue.count >= DOWNLOAD_MAX) {
        printf(RED "[E-NFB-613] Download tidak diterima, antrian sudah penuh.\n" RESET);
        return;
    }
    int idx = findPageByURL(url);
    if (idx < 0) {
        printf(RED "[E-NFB-404] Halaman %s tidak ditemukan!\n" RESET, url);
        return;
    }

    // Hitung ticks: floor(length(URL) / 5) + 2
    int ticks = (int)(strlen(url) / 5) + 2;

    DownloadItem item;
    strncpy(item.url, url, MAX_URL_LEN - 1);
    item.ticks_remaining = ticks;

    dlQueue.items[dlQueue.rear] = item;
    dlQueue.rear = (dlQueue.rear + 1) % DOWNLOAD_MAX;
    dlQueue.count++;

    if (dlQueue.count == 1) {
        printf(GREEN "Download %s (%d ticks)\n" RESET, url, ticks);
    } else {
        printf(GREEN "Download %s (%d ticks) -> antrian no %d\n" RESET, url, ticks, dlQueue.count);
    }
}

void tick(void) {
    if (dlQueue.count == 0) {
        printf(YELLOW "Antrian download saat ini kosong.\n" RESET);
        return;
    }

    DownloadItem* front = &dlQueue.items[dlQueue.front];
    front->ticks_remaining--;

    if (front->ticks_remaining <= 0) {
        // Download selesai - simpan ke file (D02)
        char filename[MAX_URL_LEN + 10];
        // Potong TLD dari URL untuk nama file
        char base[MAX_URL_LEN];
        strncpy(base, front->url, MAX_URL_LEN - 1);
        char* dot = strrchr(base, '.');
        if (dot) *dot = '\0';
        sprintf(filename, "%s.txt", base);

        // Cari konten halaman
        int idx = findPageByURL(front->url);
        if (idx >= 0) {
            FILE* f = fopen(filename, "w");
            if (f) {
                // Tulis konten dengan mengganti \\n -> newline
                char* p = pages[idx].content;
                while (*p) {
                    if (*p == '\\' && *(p+1) == 'n') { fprintf(f, "\n"); p += 2; }
                    else { fprintf(f, "%c", *p); p++; }
                }
                fclose(f);
                printf(GREEN "%s selesai didownload, ke file %s.\n" RESET, front->url, filename);
            } else {
                printf(RED "Gagal membuat file %s.\n" RESET, filename);
            }
        }

        // Dequeue
        dlQueue.front = (dlQueue.front + 1) % DOWNLOAD_MAX;
        dlQueue.count--;

        // Lanjut ke download berikutnya
        if (dlQueue.count > 0) {
            printf(CYAN "Lanjut downloading %s... (%d ticks tersisa)\n" RESET,
                   dlQueue.items[dlQueue.front].url,
                   dlQueue.items[dlQueue.front].ticks_remaining);
        }
    } else {
        printf(CYAN "Downloading %s... (%d ticks tersisa)\n" RESET,
               front->url, front->ticks_remaining);
    }
}

// ANTRIAN DOKTER (Queue Linked List)
void initAntrianDokter(AntrianDokter* q) {
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
}

void enqueueAntrian(AntrianDokter* q, const char* nama) {
    QNode* node = (QNode*)malloc(sizeof(QNode));
    if (!node) { printf(RED "Gagal alokasi memori!\n" RESET); return; }
    strncpy(node->nama_pasien, nama, 49);
    node->next = NULL;
    if (q->tail) q->tail->next = node;
    else q->head = node;
    q->tail = node;
    q->count++;
}

void dequeueAntrian(AntrianDokter* q) {
    if (!q->head) return;
    QNode* tmp = q->head;
    q->head = q->head->next;
    if (!q->head) q->tail = NULL;
    free(tmp);
    q->count--;
}

void freeAntrian(AntrianDokter* q) {
    while (q->head) dequeueAntrian(q);
}

// HALODOC - DOKTER 
void initDokter(void) {
    // Data default dokter (akan di-override saat load CSV)
    dokter_count = 0;
}

void showDokterList(void) {
    printSeparator();
    printf(BOLD CYAN "  DAFTAR DOKTER HALODOC\n" RESET);
    printSeparator();
    if (dokter_count == 0) {
        printf(YELLOW "  Belum ada data dokter. Load data terlebih dahulu.\n" RESET);
    } else {
        printf("  %-4s %-25s %-20s %-8s %s\n", "ID", "Nama", "Spesialis", "Antrian", "URL");
        printf("  %-4s %-25s %-20s %-8s %s\n", "----", "-------------------------", "--------------------", "--------", "---");
        for (int i = 0; i < dokter_count; i++) {
            printf("  %-4d %-25s %-20s %-8d %s\n",
                dokter_list[i].id,
                dokter_list[i].nama,
                dokter_list[i].spesialis,
                dokter_list[i].antrian.count,
                dokter_list[i].url);
        }
    }
    printSeparator();
}

void daftarAntrian(const char* nama_pasien, int dokter_id) {
    for (int i = 0; i < dokter_count; i++) {
        if (dokter_list[i].id == dokter_id) {
            enqueueAntrian(&dokter_list[i].antrian, nama_pasien);
            printf(GREEN "✓ %s berhasil didaftarkan ke antrian Dr. %s\n" RESET,
                   nama_pasien, dokter_list[i].nama);
            printf(CYAN "  Posisi antrian: %d\n" RESET, dokter_list[i].antrian.count);
            return;
        }
    }
    printf(RED "Dokter dengan ID %d tidak ditemukan!\n" RESET, dokter_id);
}

void lihatAntrian(int dokter_id) {
    for (int i = 0; i < dokter_count; i++) {
        if (dokter_list[i].id == dokter_id) {
            printSeparator();
            printf(BOLD "  Antrian Dr. %s (%s)\n" RESET,
                   dokter_list[i].nama, dokter_list[i].spesialis);
            printSeparator();
            if (dokter_list[i].antrian.count == 0) {
                printf(YELLOW "  Antrian kosong.\n" RESET);
            } else {
                QNode* cur = dokter_list[i].antrian.head;
                int no = 1;
                while (cur) {
                    printf("  [%d] %s\n", no++, cur->nama_pasien);
                    cur = cur->next;
                }
            }
            printSeparator();
            return;
        }
    }
    printf(RED "Dokter dengan ID %d tidak ditemukan!\n" RESET, dokter_id);
}

// HALODOC - OBAT
void initObat(void) {
    obat_count = 0;
}

void showObatList(void) {
    printSeparator();
    printf(BOLD CYAN "  DAFTAR OBAT HALODOC\n" RESET);
    printSeparator();
    if (obat_count == 0) {
        printf(YELLOW "  Belum ada data obat. Load data terlebih dahulu.\n" RESET);
    } else {
        printf("  %-4s %-25s %-8s %-10s %s\n", "ID", "Nama Obat", "Stok", "Harga", "URL");
        printf("  %-4s %-25s %-8s %-10s %s\n", "----", "-------------------------", "--------", "----------", "---");
        for (int i = 0; i < obat_count; i++) {
            printf("  %-4d %-25s %-8d Rp%-8d %s\n",
                obat_list[i].id,
                obat_list[i].nama,
                obat_list[i].stok,
                obat_list[i].harga,
                obat_list[i].url);
        }
    }
    printSeparator();
}

// TAMPILAN DR HALODOC HOME 
void showHalodocHome(void) {
    printf(CYAN BOLD);
    printf("  ╔══════════════════════════════════════════════════════╗\n");
    printf("  ║         SELAMAT DATANG DI HALODOC BROWSER           ║\n");
    printf("  ║      Layanan Kesehatan Digital Terpercaya            ║\n");
    printf("  ╚══════════════════════════════════════════════════════╝\n");
    printf(RESET);
    printf(GREEN);
    printf("  🏥 Konsultasi Dokter Online    - open halodoc.com/dokter\n");
    printf("  💊 Beli Obat Online            - open halodoc.com/obat\n");
    printf("  🔬 Cek Lab & Tes Kesehatan     - open halodoc.com/lab\n");
    printf("  📋 Daftar Antrian Dokter       - antrian <id> <nama>\n");
    printf("  👨‍⚕️ Lihat Dokter Tersedia        - dokter\n");
    printf(RESET);
    printSeparator();
    printf(YELLOW "  Tip: Ketik 'discover' untuk melihat halaman menarik!\n" RESET);
    printSeparator();
}
