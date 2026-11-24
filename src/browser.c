// File: browser.c
// Tab management, navigation, discover, search

#include "../header/halodoc.h"

// TAB MANAGEMENT 
void initTabs(void) {
    for (int i = 0; i < MAX_TABS; i++) {
        tabs[i].is_active = 0;
        tabs[i].history.top = -1;
        tabs[i].history.total = 0;
        tabs[i].session_token = 0xBEEF;
    }
    // Tab pertama default
    tabs[0].is_active = 1;
    sprintf(tabs[0].name, "TAB1");
    tab_count = 1;
    current_tab = 0;
}

void newTab(void) {
    if (tab_count >= MAX_TABS) {
        printf(RED "[E-NFB-507] Jumlah tab tidak bisa melebihi batas maksimum (%d)!\n" RESET, MAX_TABS);
        return;
    }
    // Cari slot kosong dan tentukan nomor tab
    int new_num = 1;
    for (int i = 0; i < MAX_TABS; i++) {
        if (tabs[i].is_active) {
            // ambil nomor dari nama TABx
            int n;
            if (sscanf(tabs[i].name, "TAB%d", &n) == 1 && n >= new_num) new_num = n + 1;
        }
    }
    // Taruh di slot pertama yang tidak aktif
    for (int i = 0; i < MAX_TABS; i++) {
        if (!tabs[i].is_active) {
            tabs[i].is_active = 1;
            sprintf(tabs[i].name, "TAB%d", new_num);
            tabs[i].history.top = -1;
            tabs[i].history.total = 0;
            tabs[i].session_token = 0xBEEF;
            tab_count++;
            printf(GREEN "Tab baru (%s) berhasil dibuat!\n" RESET, tabs[i].name);
            return;
        }
    }
}

void closeTab(void) {
    if (tab_count <= 1) {
        printf(RED "ERROR: Tidak bisa menutup tab, tab minimal berjumlah 1!\n" RESET);
        return;
    }
    printf(GREEN "%s berhasil ditutup.\n" RESET, tabs[current_tab].name);
    tabs[current_tab].is_active = 0;
    tabs[current_tab].history.top = -1;
    tabs[current_tab].history.total = 0;
    tab_count--;

    // Pindah ke tab aktif berikutnya
    int next = -1;
    for (int i = 0; i < MAX_TABS; i++) {
        if (tabs[i].is_active) { next = i; break; }
    }
    current_tab = (next >= 0) ? next : 0;
}

void checkTab(void) {
    printSeparator();
    printf(BOLD "  List of tab(s):\n" RESET);
    int num = 1;
    for (int i = 0; i < MAX_TABS; i++) {
        if (tabs[i].is_active) {
            if (i == current_tab)
                printf(GREEN "  [%d] %s  <-- AKTIF\n" RESET, num, tabs[i].name);
            else
                printf("  [%d] %s\n", num, tabs[i].name);
            num++;
        }
    }
    printf(CYAN "  Current tab: %s\n" RESET, tabs[current_tab].name);
    printSeparator();
}

static int getTabSlotByOrder(int order) {
    int count = 0;
    for (int i = 0; i < MAX_TABS; i++) {
        if (tabs[i].is_active) {
            count++;
            if (count == order) return i;
        }
    }
    return -1;
}

static int getCurrentTabOrder(void) {
    int count = 0;
    for (int i = 0; i < MAX_TABS; i++) {
        if (tabs[i].is_active) {
            count++;
            if (i == current_tab) return count;
        }
    }
    return -1;
}

void prevTab(int steps) {
    int cur_order = getCurrentTabOrder();
    int target_order = cur_order - steps;
    if (target_order < 1) {
        printf(RED "ERROR: Posisi tab tidak valid!\n" RESET);
        return;
    }
    int slot = getTabSlotByOrder(target_order);
    if (slot < 0) {
        printf(RED "ERROR: Posisi tab tidak valid!\n" RESET);
        return;
    }
    current_tab = slot;
    printf(GREEN "Tab saat ini berhasil diganti ke %s.\n" RESET, tabs[current_tab].name);
}

void nextTab(int steps) {
    int cur_order = getCurrentTabOrder();
    int target_order = cur_order + steps;
    if (target_order > tab_count) {
        printf(RED "ERROR: Posisi tab tidak valid!\n" RESET);
        return;
    }
    int slot = getTabSlotByOrder(target_order);
    if (slot < 0) {
        printf(RED "ERROR: Posisi tab tidak valid!\n" RESET);
        return;
    }
    current_tab = slot;
    printf(GREEN "Tab saat ini berhasil diganti ke %s.\n" RESET, tabs[current_tab].name);
}

// NAVIGASI
static void displayPage(int page_idx) {
    printSeparator();
    printf(BOLD CYAN "  %s\n" RESET, pages[page_idx].url);
    printSeparator();
    // Tampilkan konten — replace \\n dengan newline asli
    char* p = pages[page_idx].content;
    printf("  ");
    while (*p) {
        if (*p == '\\' && *(p+1) == 'n') { printf("\n  "); p += 2; }
        else { printf("%c", *p); p++; }
    }
    printf("\n\n");
    showLinkedPages(page_idx);
    printSeparator();
}

static void pushHistory(const char* url) {
    TabHistory* h = &tabs[current_tab].history;
    // Hapus forward history
    h->total = h->top + 1;
    // Tambah URL baru
    if (h->total >= MAX_HISTORY) {
        // Geser ke kiri
        for (int i = 0; i < MAX_HISTORY - 1; i++)
            strncpy(h->urls[i], h->urls[i+1], MAX_URL_LEN-1);
        h->top = MAX_HISTORY - 2;
        h->total = MAX_HISTORY - 1;
    }
    h->top++;
    strncpy(h->urls[h->top], url, MAX_URL_LEN - 1);
    h->total = h->top + 1;
}

void openPage(const char* url) {
    char content[MAX_CONTENT_LEN];
    int idx = findPageByURL(url);
    if (idx < 0) {
        printf(RED "[E-NFB-404] 404 Not Found! Halaman tidak ditemukan: %s\n" RESET, url);
        return;
    }

    if (cacheGet(url, content)) {
        printf(GREEN "[Status: Cache-Hit] Mengambil data dari cache...\n" RESET);
    } else {
        printf(YELLOW "[Status: Cache-Miss] Mengambil data dari database...\n" RESET);
        cachePut(url, pages[idx].content);
    }

    pushHistory(url);
    displayPage(idx);
}

void goBack(int steps) {
    TabHistory* h = &tabs[current_tab].history;
    if (h->top < 0) {
        printf(RED "ERROR: BACK TIDAK BISA DIJALANKAN KARENA TIDAK ADA HALAMAN SEBELUMNYA!\n" RESET);
        return;
    }
    if (h->top - steps < 0) {
        printf(RED "ERROR: BACK TIDAK BISA DIGUNAKAN LAGI KARENA TIDAK ADA HALAMAN SEBELUMNYA!\n" RESET);
        // Tampilkan halaman paling awal
        int idx = findPageByURL(h->urls[0]);
        if (idx >= 0) displayPage(idx);
        h->top = 0;
        return;
    }
    h->top -= steps;
    printf(GREEN "BACK: KEMBALI KE HALAMAN %s\n" RESET, h->urls[h->top]);
    int idx = findPageByURL(h->urls[h->top]);
    if (idx >= 0) displayPage(idx);
}

void goForward(int steps) {
    TabHistory* h = &tabs[current_tab].history;
    if (h->top < 0 || h->top >= h->total - 1) {
        printf(RED "ERROR: FORWARD TIDAK BISA DIJALANKAN KARENA TIDAK ADA HALAMAN SELANJUTNYA!\n" RESET);
        if (h->top >= 0) {
            int idx = findPageByURL(h->urls[h->top]);
            if (idx >= 0) displayPage(idx);
        }
        return;
    }
    if (h->top + steps >= h->total) {
        printf(RED "ERROR: TIDAK ADA HALAMAN SELANJUTNYA SEBANYAK %d LANGKAH!\n" RESET, steps);
        return;
    }
    h->top += steps;
    printf(GREEN "FORWARD: MENUJU HALAMAN %s\n" RESET, h->urls[h->top]);
    int idx = findPageByURL(h->urls[h->top]);
    if (idx >= 0) displayPage(idx);
}

void viewTabHistory(void) {
    TabHistory* h = &tabs[current_tab].history;
    printSeparator();
    printf(BOLD "  Riwayat Tab: %s\n" RESET, tabs[current_tab].name);
    if (h->total == 0) {
        printf(YELLOW "  (Belum ada halaman yang dibuka)\n" RESET);
    } else {
        for (int i = 0; i < h->total; i++) {
            if (i == h->top)
                printf(GREEN "  [%d] %s  <- YOU ARE HERE\n" RESET, i, h->urls[i]);
            else
                printf("  [%d] %s\n", i, h->urls[i]);
        }
    }
    printSeparator();
    // Tampilkan halaman saat ini juga
    if (h->top >= 0) {
        int idx = findPageByURL(h->urls[h->top]);
        if (idx >= 0) displayPage(idx);
    }
}

void goHome(void) {
    printf(CYAN "\n  Kembali ke menu utama Halodoc Browser.\n" RESET);
    printf(CYAN "  Ketik 'help' untuk melihat semua perintah.\n\n" RESET);
    showHalodocHome();
}

// DISCOVER (F01 - LCG)
void discover(void) {
    if (page_count == 0) {
        printf(YELLOW "Belum ada halaman yang tersedia.\n" RESET);
        return;
    }
    printSeparator();
    printf(BOLD CYAN "  Berikut halaman yang mungkin menarik untukmu:\n" RESET);
    printSeparator();

    int shown = 0;
    int max_show = (page_count < 5) ? page_count : 5;
    int visited[MAX_PAGES] = {0};

    while (shown < max_show) {
        int r = (int)(lcgNext() % (unsigned int)page_count);
        if (!visited[r]) {
            visited[r] = 1;
            printf("  - %s\n", pages[r].url);
            shown++;
        }
    }
    printSeparator();
}

// SEARCH (F02) 
void searchPage(const char* query) {
    printSeparator();
    printf(BOLD "  Search result(s) untuk \"%s\":\n" RESET, query);
    int found = 0;
    int qlen = (int)strlen(query);
    for (int i = 0; i < page_count; i++) {
        // Cocokkan dari awal URL (prefix match)
        if (strncmp(pages[i].url, query, qlen) == 0) {
            printf(GREEN "  - %s\n" RESET, pages[i].url);
            found++;
        }
    }
    if (!found) printf(YELLOW "  Tidak Ditemukan\n" RESET);
    printSeparator();
}
