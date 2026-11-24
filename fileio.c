// File: fileio.c
// Load & Save data CSV (D03, D04)

#include "../header/halodoc.h"
#include <sys/stat.h>

// Helper: trim whitespace
static void trim(char* s) {
    char* p = s;
    while (*p == ' ' || *p == '\t') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' ||
                        s[len-1] == '\n' || s[len-1] == '\r')) {
        s[--len] = '\0';
    }
}

// Helper: baca field CSV dengan memperhatikan kutip ganda
static int csvNextField(const char* line, int* pos, char* out, int max_out) {
    if (line[*pos] == '\0') return 0;
    int i = 0;

    if (line[*pos] == '"') {
        // Field dikutip
        (*pos)++;
        while (line[*pos] != '\0' && i < max_out - 1) {
            if (line[*pos] == '"' && line[*pos + 1] == '"') {
                out[i++] = '"'; (*pos) += 2;
            } else if (line[*pos] == '"') {
                (*pos)++; break;
            } else {
                out[i++] = line[(*pos)++];
            }
        }
        if (line[*pos] == ',') (*pos)++;
    } else {
        while (line[*pos] != '\0' && line[*pos] != ',' && i < max_out - 1) {
            out[i++] = line[(*pos)++];
        }
        if (line[*pos] == ',') (*pos)++;
    }
    out[i] = '\0';
    return 1;
}

// BUAT LOAD DATA (D03) 
void loadData(const char* folder) {
    char path[512];
    char line[2048];

    printf(CYAN "Loading data dari folder: %s\n" RESET, folder);

    // Reset data
    page_count = 0;
    dokter_count = 0;
    obat_count = 0;
    initCache();
    initGraph();

    // Load web_pages.csv 
    snprintf(path, sizeof(path), "%sweb_pages.csv", folder);
    FILE* f = fopen(path, "r");
    if (!f) {
        printf(RED "Error: File %s tidak ditemukan!\n" RESET, path);
    } else {
        fgets(line, sizeof(line), f); // skip header
        while (fgets(line, sizeof(line), f) && page_count < MAX_PAGES) {
            line[strcspn(line, "\n\r")] = '\0';
            if (strlen(line) == 0) continue;

            int pos = 0;
            char fid[20], furl[MAX_URL_LEN], fcontent[MAX_CONTENT_LEN];
            if (!csvNextField(line, &pos, fid, sizeof(fid))) continue;
            if (!csvNextField(line, &pos, furl, sizeof(furl))) continue;
            if (!csvNextField(line, &pos, fcontent, sizeof(fcontent))) continue;

            trim(furl);
            int idx = page_count;
            pages[idx].id = atoi(fid);
            strncpy(pages[idx].url, furl, MAX_URL_LEN - 1);
            strncpy(pages[idx].content, fcontent, MAX_CONTENT_LEN - 1);
            pages[idx].linked_count = 0;
            page_count++;
        }
        fclose(f);
        printf(GREEN "  ✓ web_pages.csv: %d halaman dimuat\n" RESET, page_count);
    }

    // Load linked_pages.csv 
    snprintf(path, sizeof(path), "%slinked_pages.csv", folder);
    f = fopen(path, "r");
    if (!f) {
        printf(YELLOW "  ⚠ File %s tidak ditemukan, skip.\n" RESET, path);
    } else {
        fgets(line, sizeof(line), f); // skip header
        int link_count = 0;
        while (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\n\r")] = '\0';
            if (strlen(line) == 0) continue;
            int pos = 0;
            char fid[20], fsrc[20], fdst[20];
            if (!csvNextField(line, &pos, fid, sizeof(fid))) continue;
            if (!csvNextField(line, &pos, fsrc, sizeof(fsrc))) continue;
            if (!csvNextField(line, &pos, fdst, sizeof(fdst))) continue;

            int src_id = atoi(fsrc);
            int dst_id = atoi(fdst);
            // Cari indeks berdasarkan ID
            int src_idx = -1, dst_idx = -1;
            for (int i = 0; i < page_count; i++) {
                if (pages[i].id == src_id) src_idx = i;
                if (pages[i].id == dst_id) dst_idx = i;
            }
            if (src_idx >= 0 && dst_idx >= 0) {
                addEdge(src_idx, dst_idx);
                link_count++;
            }
        }
        fclose(f);
        printf(GREEN "  ✓ linked_pages.csv: %d relasi dimuat\n" RESET, link_count);
    }

    // Load dokter.csv 
    snprintf(path, sizeof(path), "%sdokter.csv", folder);
    f = fopen(path, "r");
    if (!f) {
        printf(YELLOW "  ⚠ File %s tidak ditemukan, skip.\n" RESET, path);
    } else {
        fgets(line, sizeof(line), f); // skip header
        while (fgets(line, sizeof(line), f) && dokter_count < MAX_DOKTER) {
            line[strcspn(line, "\n\r")] = '\0';
            if (strlen(line) == 0) continue;
            int pos = 0;
            char fid[20], fnama[50], fspesialis[50], furl[MAX_URL_LEN];
            if (!csvNextField(line, &pos, fid, sizeof(fid))) continue;
            if (!csvNextField(line, &pos, fnama, sizeof(fnama))) continue;
            if (!csvNextField(line, &pos, fspesialis, sizeof(fspesialis))) continue;
            if (!csvNextField(line, &pos, furl, sizeof(furl))) continue;

            int i = dokter_count;
            dokter_list[i].id = atoi(fid);
            strncpy(dokter_list[i].nama, fnama, 49);
            strncpy(dokter_list[i].spesialis, fspesialis, 49);
            strncpy(dokter_list[i].url, furl, MAX_URL_LEN - 1);
            initAntrianDokter(&dokter_list[i].antrian);
            dokter_count++;
        }
        fclose(f);
        printf(GREEN "  ✓ dokter.csv: %d dokter dimuat\n" RESET, dokter_count);
    }

    // Load obat.csv 
    snprintf(path, sizeof(path), "%sobat.csv", folder);
    f = fopen(path, "r");
    if (!f) {
        printf(YELLOW "  ⚠ File %s tidak ditemukan, skip.\n" RESET, path);
    } else {
        fgets(line, sizeof(line), f); // skip header
        while (fgets(line, sizeof(line), f) && obat_count < MAX_OBAT) {
            line[strcspn(line, "\n\r")] = '\0';
            if (strlen(line) == 0) continue;
            int pos = 0;
            char fid[20], fnama[50], furl[MAX_URL_LEN], fstok[20], fharga[20];
            if (!csvNextField(line, &pos, fid, sizeof(fid))) continue;
            if (!csvNextField(line, &pos, fnama, sizeof(fnama))) continue;
            if (!csvNextField(line, &pos, furl, sizeof(furl))) continue;
            if (!csvNextField(line, &pos, fstok, sizeof(fstok))) continue;
            if (!csvNextField(line, &pos, fharga, sizeof(fharga))) continue;

            int i = obat_count;
            obat_list[i].id = atoi(fid);
            strncpy(obat_list[i].nama, fnama, 49);
            strncpy(obat_list[i].url, furl, MAX_URL_LEN - 1);
            obat_list[i].stok = atoi(fstok);
            obat_list[i].harga = atoi(fharga);
            obat_count++;
        }
        fclose(f);
        printf(GREEN "  ✓ obat.csv: %d obat dimuat\n" RESET, obat_count);
    }

    printf(GREEN "Data berhasil dimuat!\n\n" RESET);
}

// SAVE DATA (D04) 
void saveData(const char* folder) {
    char path[512];
    printf(CYAN "Saving data ke folder: %s\n" RESET, folder);

    // Buat folder jika belum ada
    mkdir(folder, 0755);

    // Simpan web_pages.csv 
    snprintf(path, sizeof(path), "%sweb_pages.csv", folder);
    FILE* f = fopen(path, "w");
    if (!f) {
        printf(RED "Error: Gagal membuka %s untuk ditulis!\n" RESET, path);
        return;
    }
    fprintf(f, "id,web_url,content\n");
    for (int i = 0; i < page_count; i++) {
        fprintf(f, "%d,\"%s\",\"%s\"\n",
                pages[i].id, pages[i].url, pages[i].content);
    }
    fclose(f);

    // Simpan linked_pages.csv 
    snprintf(path, sizeof(path), "%slinked_pages.csv", folder);
    f = fopen(path, "w");
    if (!f) {
        printf(RED "Error: Gagal membuka %s!\n" RESET, path);
        return;
    }
    fprintf(f, "id,id_sumber,id_tujuan\n");
    int link_id = 1;
    for (int i = 0; i < page_count; i++) {
        for (int j = 0; j < page_count; j++) {
            if (graph.matrix[i][j]) {
                fprintf(f, "%d,%d,%d\n", link_id++, pages[i].id, pages[j].id);
            }
        }
    }
    fclose(f);

    // Simpan dokter.csv 
    snprintf(path, sizeof(path), "%sdokter.csv", folder);
    f = fopen(path, "w");
    if (f) {
        fprintf(f, "id,nama,spesialis,url\n");
        for (int i = 0; i < dokter_count; i++) {
            fprintf(f, "%d,\"%s\",\"%s\",\"%s\"\n",
                dokter_list[i].id,
                dokter_list[i].nama,
                dokter_list[i].spesialis,
                dokter_list[i].url);
        }
        fclose(f);
    }

    // Simpan obat.csv 
    snprintf(path, sizeof(path), "%sobat.csv", folder);
    f = fopen(path, "w");
    if (f) {
        fprintf(f, "id,nama,url,stok,harga\n");
        for (int i = 0; i < obat_count; i++) {
            fprintf(f, "%d,\"%s\",\"%s\",%d,%d\n",
                obat_list[i].id,
                obat_list[i].nama,
                obat_list[i].url,
                obat_list[i].stok,
                obat_list[i].harga);
        }
        fclose(f);
    }

    printf(GREEN "Data berhasil disimpan ke %s\n" RESET, folder);
}
