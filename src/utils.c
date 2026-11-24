// File: utils.c
// Utility functions, LCG, URL validation, Cache, Web Graph, Page Management

#include "../header/halodoc.h"

// UTILS 
int strEq(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}

void clearInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void printBanner(void) {
    printf(CYAN BOLD);
    printf("  ██╗  ██╗ █████╗ ██╗      ██████╗ ██████╗  ██████╗  ██████╗\n");
    printf("  ██║  ██║██╔══██╗██║     ██╔═══██╗██╔══██╗██╔═══██╗██╔════╝\n");
    printf("  ███████║███████║██║     ██║   ██║██║  ██║██║   ██║██║     \n");
    printf("  ██╔══██║██╔══██║██║     ██║   ██║██║  ██║██║   ██║██║     \n");
    printf("  ██║  ██║██║  ██║███████╗╚██████╔╝██████╔╝╚██████╔╝╚██████╗\n");
    printf("  ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝ ╚═════╝ ╚═════╝  ╚═════╝  ╚═════╝\n");
    printf(RESET GREEN);
    printf("         Browser Simulator - Layanan Kesehatan Digital\n");
    printf(RESET);
    printf("  ══════════════════════════════════════════════════════════\n\n");
}

void printSeparator(void) {
    printf(CYAN "  ──────────────────────────────────────────────────────\n" RESET);
}

// LCG 
unsigned int lcgNext(void) {
    lcg_seed = (unsigned int)((LCG_MULTIPLIER * (unsigned long long)lcg_seed + LCG_INCREMENT) % LCG_MODULUS);
    return lcg_seed;
}

// URL VALIDATION (E01) 
// Regex base: ^(?!-)[A-Za-z0-9-]{1,63}(?<!-)(\.[A-Za-z]{2,})+$
// Extended: support path setelah domain (misal halodoc.com/artikel)
int validateURL(const char* url) {
    if (!url || strlen(url) == 0) return 0;

    char domain[MAX_URL_LEN];
    strncpy(domain, url, MAX_URL_LEN - 1);
    domain[MAX_URL_LEN - 1] = '\0';

    char* slash = strchr(domain, '/');
    if (slash) {
        if (*(slash + 1) == '\0') return 0;
        *slash = '\0'; // potong, validasi domain saja
    }

    int len = (int)strlen(domain);
    if (len == 0) return 0;
    int i = 0;

    while (i < len) {
        if (domain[i] == '-') return 0;

        int seg_len = 0;
        while (i < len && domain[i] != '.') {
            char c = domain[i];
            if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9') || c == '-')) return 0;
            seg_len++;
            i++;
        }

        if (seg_len == 0 || seg_len > 63) return 0;
        if (domain[i - 1] == '-') return 0;

        if (i < len && domain[i] == '.') {
            i++;
            if (i >= len) return 0;
        }
    }

    const char* last_dot = strrchr(domain, '.');
    if (!last_dot) return 0;
    last_dot++;
    int tld_len = (int)strlen(last_dot);
    if (tld_len < 2) return 0;
    for (int j = 0; j < tld_len; j++) {
        char c = last_dot[j];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) return 0;
    }
    return 1;
}

//  NORMALIZE URL KEY (untuk cache)
int normalizeUrlKey(const char* url) {
    int sum = 0;
    for (int i = 0; url[i] != '\0'; i++) sum += (int)url[i];
    return sum % 997;
}

// CACHE (ADT Map - FIFO Eviction) 
void initCache(void) {
    for (int i = 0; i < CACHE_MAX; i++) {
        cache.entries[i].valid = 0;
        cache.entries[i].url[0] = '\0';
        cache.entries[i].order = 0;
    }
    cache.count = 0;
    cache.insert_order = 0;
}

int cacheGet(const char* url, char* out_content) {
    normalizeUrlKey(url);
    for (int i = 0; i < CACHE_MAX; i++) {
        if (cache.entries[i].valid && strEq(cache.entries[i].url, url)) {
            if (out_content) strcpy(out_content, cache.entries[i].content);
            return 1; // cache hit
        }
    }
    return 0;
}

void evictCache(void) {
    int min_order = cache.entries[0].order;
    int min_idx = 0;
    for (int i = 1; i < CACHE_MAX; i++) {
        if (cache.entries[i].valid && cache.entries[i].order < min_order) {
            min_order = cache.entries[i].order;
            min_idx = i;
        }
    }
    printf(YELLOW "[Cache] Evicting: %s (FIFO)\n" RESET, cache.entries[min_idx].url);
    cache.entries[min_idx].valid = 0;
    cache.count--;
}

void cachePut(const char* url, const char* content) {
    normalizeUrlKey(url); 

    for (int i = 0; i < CACHE_MAX; i++) {
        if (cache.entries[i].valid && strEq(cache.entries[i].url, url)) return;
    }
    if (cache.count >= CACHE_MAX) {
        printf(RED "[E-NFB-511] Cache penuh, melakukan eviction...\n" RESET);
        evictCache();
    }

    for (int i = 0; i < CACHE_MAX; i++) {
        if (!cache.entries[i].valid) {
            strncpy(cache.entries[i].url, url, MAX_URL_LEN - 1);
            strncpy(cache.entries[i].content, content, MAX_CONTENT_LEN - 1);
            cache.entries[i].valid = 1;
            cache.entries[i].order = cache.insert_order++;
            cache.count++;
            return;
        }
    }
}

// WEB GRAPH (Adjacency Matrix D01) 
void initGraph(void) {
    for (int i = 0; i < MAX_PAGES; i++)
        for (int j = 0; j < MAX_PAGES; j++)
            graph.matrix[i][j] = 0;
    graph.page_count = 0;
}

void addEdge(int src_id, int dst_id) {
    if (src_id < 0 || dst_id < 0 || src_id >= MAX_PAGES || dst_id >= MAX_PAGES) return;
    graph.matrix[src_id][dst_id] = 1;
}

void removeEdgesOf(int page_id) {
    if (page_id < 0 || page_id >= MAX_PAGES) return;
    for (int j = 0; j < MAX_PAGES; j++) graph.matrix[page_id][j] = 0;
    for (int i = 0; i < MAX_PAGES; i++) graph.matrix[i][page_id] = 0;
}

void showLinkedPages(int page_id) {
    int found = 0;
    printf(CYAN "Linked pages:\n" RESET);
    int idx = 1;
    for (int j = 0; j < page_count; j++) {
        if (graph.matrix[page_id][j]) {
            printf("  [%d] %s\n", idx++, pages[j].url);
            found++;
        }
    }
    if (!found) printf(YELLOW "  (Tidak ada linked pages)\n" RESET);
}

int openLinked(int page_id, int link_index) {
    int idx = 1;
    for (int j = 0; j < page_count; j++) {
        if (graph.matrix[page_id][j]) {
            if (idx == link_index) return j;
            idx++;
        }
    }
    printf(RED "ERROR: HALAMAN TIDAK MEMILIKI TAUTAN KE INDEKS %d!\n" RESET, link_index);
    return -1;
}

//  WEB PAGES (pake ADT Set boi) 
int findPageByURL(const char* url) {
    // Linear search
    for (int i = 0; i < page_count; i++) {
        if (strEq(pages[i].url, url)) return i;
    }
    return -1;
}

int addPage(const char* url, const char* preset_content) {
    if (page_count >= MAX_PAGES) {
        printf(RED "ERROR: Kapasitas halaman penuh (MAX=%d)!\n" RESET, MAX_PAGES);
        return -1;
    }
    if (findPageByURL(url) >= 0) {
        printf(YELLOW "Sudah terdapat halaman dengan url %s. Gunakan url lain!\n" RESET, url);
        return -1;
    }

    int idx = page_count;
    pages[idx].id = idx + 1;
    strncpy(pages[idx].url, url, MAX_URL_LEN - 1);
    pages[idx].linked_count = 0;

    if (preset_content) {
        strncpy(pages[idx].content, preset_content, MAX_CONTENT_LEN - 1);
    } else {
        printf(CYAN "Masukkan konten (akhiri dengan '.' di baris baru):\n" RESET);
        char line[256];
        char content[MAX_CONTENT_LEN] = "";
        while (1) {
            printf(">>> ");
            fgets(line, sizeof(line), stdin);
            line[strcspn(line, "\n\r")] = '\0';
            if (strEq(line, ".")) break;
            if (strlen(content) + strlen(line) + 2 < MAX_CONTENT_LEN) {
                if (strlen(content) > 0) strcat(content, "\\n");
                strcat(content, line);
            }
        }
        strncpy(pages[idx].content, content, MAX_CONTENT_LEN - 1);

        // Linked pages
        printf(CYAN "Masukkan linked pages (ketik 'DONE' jika selesai):\n" RESET);
        while (1) {
            printf(">>> ");
            fgets(line, sizeof(line), stdin);
            line[strcspn(line, "\n\r")] = '\0';
            if (strEq(line, "DONE")) break;
            int dst = findPageByURL(line);
            if (dst < 0) {
                printf(RED "URL tidak ditemukan!\n" RESET);
            } else {
                addEdge(idx, dst);
            }
        }
    }

    page_count++;
    graph.page_count = page_count;
    printf(GREEN "Halaman %s berhasil ditambahkan!\n" RESET, url);
    return idx;
}

void editPage(const char* url) {
    int idx = findPageByURL(url);
    if (idx < 0) {
        printf(RED "Tidak ada halaman dengan url %s!\n" RESET, url);
        return;
    }


    char cached_content[MAX_CONTENT_LEN];
    if (cacheGet(url, cached_content)) {
        printf(GREEN "[Status: Cache-Hit] Mengambil data dari cache...\n" RESET);
    } else {
        printf(YELLOW "[Status: Cache-Miss] Mengambil data dari database...\n" RESET);
    }

    printf(CYAN "Konten saat ini:\n" RESET);

    char* p = pages[idx].content;
    while (*p) {
        if (*p == '\\' && *(p+1) == 'n') { printf("\n"); p += 2; }
        else { printf("%c", *p); p++; }
    }
    printf("\n");

    showLinkedPages(idx);

    printf(CYAN "\nMasukkan konten baru (ketik '.' untuk skip):\n" RESET);
    char line[256];
    char new_content[MAX_CONTENT_LEN] = "";
    while (1) {
        printf(">>> ");
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n\r")] = '\0';
        if (strEq(line, ".")) break;
        if (strlen(new_content) + strlen(line) + 3 < MAX_CONTENT_LEN) {
            if (strlen(new_content) > 0) strcat(new_content, "\\n");
            strcat(new_content, line);
        }
    }
    if (strlen(new_content) > 0) {
        strncpy(pages[idx].content, new_content, MAX_CONTENT_LEN - 1);
        // Update cache
        for (int i = 0; i < CACHE_MAX; i++) {
            if (cache.entries[i].valid && strEq(cache.entries[i].url, url)) {
                strncpy(cache.entries[i].content, new_content, MAX_CONTENT_LEN - 1);
            }
        }
    }

    printf(CYAN "Masukkan linked pages baru (ketik 'SKIP' untuk tidak ubah, 'DONE' selesai):\n" RESET);
    fgets(line, sizeof(line), stdin);
    line[strcspn(line, "\n\r")] = '\0';
    if (!strEq(line, "SKIP")) {
        removeEdgesOf(idx);
        while (!strEq(line, "DONE")) {
            int dst = findPageByURL(line);
            if (dst < 0) printf(RED "URL tidak ditemukan!\n" RESET);
            else addEdge(idx, dst);
            printf(">>> ");
            fgets(line, sizeof(line), stdin);
            line[strcspn(line, "\n\r")] = '\0';
        }
    }

    printf(GREEN "Halaman %s berhasil diperbarui!\n" RESET, url);
}

void deletePage(const char* url) {
    int idx = findPageByURL(url);
    if (idx < 0) {
        printf(RED "Tidak ada halaman dengan url %s!\n" RESET, url);
        return;
    }


    for (int i = 0; i < CACHE_MAX; i++) {
        if (cache.entries[i].valid && strEq(cache.entries[i].url, url)) {
            printf(GREEN "[Status: Cache-Hit] URL ditemukan di cache dan telah dibersihkan.\n" RESET);
            cache.entries[i].valid = 0;
            cache.count--;
        }
    }

    printf(CYAN "Membersihkan relasi linked pages...\n" RESET);
    removeEdgesOf(idx);

    for (int i = idx; i < page_count - 1; i++) {
        pages[i] = pages[i + 1];
    }
    page_count--;
    graph.page_count = page_count;

    printf(GREEN "Halaman %s berhasil dihapus!\n" RESET, url);
}
