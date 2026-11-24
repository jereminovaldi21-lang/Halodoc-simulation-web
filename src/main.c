// File: main.c
// Halodoc Browser Simulator
// Tema: Simulasi web browser dengan konten layanan kesehatan Halodoc

#include "../header/halodoc.h"

// GLOBAL VARIABLES 
WebPage pages[MAX_PAGES];
int page_count = 0;

WebGraph graph;
Cache cache;
DownloadQueue dlQueue;

Tab tabs[MAX_TABS];
int tab_count = 0;
int current_tab = 0;

Dokter dokter_list[MAX_DOKTER];
int dokter_count = 0;

Obat obat_list[MAX_OBAT];
int obat_count = 0;

unsigned int lcg_seed = LCG_SEED_INITIAL;

// MAIN 
int main(int argc, char* argv[]) {
    char config_folder[256] = "config/";
    if (argc >= 2) {
        strncpy(config_folder, argv[1], 255);
        config_folder[255] = '\0';
    }

    initCache();
    initGraph();
    initDownloadQueue();
    initTabs();
    initDokter();
    initObat();

    // Load data dari file CSV
    loadData(config_folder);

    printBanner();
    printf(GREEN "Data berhasil dimuat dari folder: %s\n" RESET, config_folder);
    printf(CYAN "Ketik 'help' untuk melihat daftar perintah.\n\n" RESET);

    char input[256];
    char cmd[64];
    char arg1[MAX_URL_LEN];
    char arg2[MAX_URL_LEN];

    int running = 1;
    while (running) {
        printf(BOLD CYAN "[%s] " RESET ">>> ", tabs[current_tab].name);
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n\r")] = '\0';

        cmd[0] = '\0'; arg1[0] = '\0'; arg2[0] = '\0';
        int parsed = sscanf(input, "%63s %99s %99s", cmd, arg1, arg2);
        if (parsed < 1) continue;

        if (strEq(cmd, "exit")) {
            printf(YELLOW "Save data sebelum keluar? (y/n): " RESET);
            char yn[8];
            fgets(yn, sizeof(yn), stdin);
            if (yn[0] == 'y' || yn[0] == 'Y') {
                printf("Masukkan folder penyimpanan (default: config/): ");
                char savefolder[256];
                fgets(savefolder, sizeof(savefolder), stdin);
                savefolder[strcspn(savefolder, "\n\r")] = '\0';
                if (strlen(savefolder) == 0) strcpy(savefolder, "config/");
                saveData(savefolder);
            }
            printf(GREEN "\nTerima kasih telah menggunakan Halodoc Browser!\n" RESET);
            printf(CYAN "Jaga kesehatan ya! 🌿\n\n" RESET);
            running = 0;
        }
        else if (strEq(cmd, "help")) {
            printSeparator();
            printf(BOLD CYAN "  DAFTAR PERINTAH HALODOC BROWSER\n" RESET);
            printSeparator();
            printf(GREEN "  NAVIGASI\n" RESET);
            printf("  discover              - Tampilkan 5 halaman acak\n");
            printf("  search <query>        - Cari halaman berdasarkan URL\n");
            printf("  open <url>            - Buka halaman web\n");
            printf("  openlinked <x>        - Buka linked page ke-x\n");
            printf("  back [x]              - Kembali x halaman (default 1)\n");
            printf("  forward [x]           - Maju x halaman (default 1)\n");
            printf("  home                  - Kembali ke menu utama\n");
            printf("  view_tab_history      - Lihat history tab aktif\n");
            printf(GREEN "  TAB\n" RESET);
            printf("  newtab                - Buka tab baru\n");
            printf("  closetab              - Tutup tab aktif\n");
            printf("  checktab              - Lihat daftar tab\n");
            printf("  prevtab <x>           - Pindah x tab ke kiri\n");
            printf("  nexttab <x>           - Pindah x tab ke kanan\n");
            printf(GREEN "  MANAJEMEN HALAMAN\n" RESET);
            printf("  add_page <url>        - Tambah halaman baru\n");
            printf("  edit_page <url>       - Edit halaman\n");
            printf("  delete_page <url>     - Hapus halaman\n");
            printf(GREEN "  DOWNLOAD\n" RESET);
            printf("  download <url>        - Unduh halaman\n");
            printf("  tick                  - Majukan waktu 1 langkah\n");
            printf(GREEN "  HALODOC FITUR\n" RESET);
            printf("  dokter                - Lihat daftar dokter\n");
            printf("  antrian <id> <nama>   - Daftar antrian dokter\n");
            printf("  lihat_antrian <id>    - Lihat antrian dokter\n");
            printf("  obat                  - Lihat daftar obat\n");
            printf(GREEN "  FILE\n" RESET);
            printf("  load <folder/>        - Load ulang data\n");
            printf("  save <folder/>        - Simpan data\n");
            printf("  exit                  - Keluar program\n");
            printSeparator();
        }
        else if (strEq(cmd, "discover")) {
            discover();
        }
        else if (strEq(cmd, "search")) {
            if (strlen(arg1) == 0) printf(RED "Usage: search <query>\n" RESET);
            else searchPage(arg1);
        }
        else if (strEq(cmd, "open")) {
            if (strlen(arg1) == 0) printf(RED "Usage: open <url>\n" RESET);
            else if (!validateURL(arg1)) printf(RED "[E-NFB-400] Format URL tidak valid!\n" RESET);
            else openPage(arg1);
        }
        else if (strEq(cmd, "openlinked")) {
            int idx = 0;
            if (strlen(arg1) == 0 || sscanf(arg1, "%d", &idx) != 1 || idx < 1 || idx > 999999) {
                printf(RED "[E-NFB-400] Usage: openlinked <x>\n" RESET);
            } else {
                TabHistory* h = &tabs[current_tab].history;
                if (h->top < 0) {
                    printf(RED "ERROR: COMMAND HANYA DAPAT DIGUNAKAN SAAT HALAMAN WEB TERBUKA!\n" RESET);
                } else {
                    int pidx = findPageByURL(h->urls[h->top]);
                    if (pidx < 0) printf(RED "Halaman saat ini tidak ditemukan di database.\n" RESET);
                    else {
                        int dest = openLinked(pidx, idx);
                        if (dest >= 0) openPage(pages[dest].url);
                    }
                }
            }
        }
        else if (strEq(cmd, "back")) {
            int steps = 1;
            if (strlen(arg1) > 0) sscanf(arg1, "%d", &steps);
            if (steps < 1 || steps > 999999) printf(RED "[E-NFB-400] Input tidak valid!\n" RESET);
            else goBack(steps);
        }
        else if (strEq(cmd, "forward")) {
            int steps = 1;
            if (strlen(arg1) > 0) sscanf(arg1, "%d", &steps);
            if (steps < 1 || steps > 999999) printf(RED "[E-NFB-400] Input tidak valid!\n" RESET);
            else goForward(steps);
        }
        else if (strEq(cmd, "home")) {
            goHome();
        }
        else if (strEq(cmd, "view_tab_history")) {
            viewTabHistory();
        }
        else if (strEq(cmd, "newtab")) {
            newTab();
        }
        else if (strEq(cmd, "closetab")) {
            closeTab();
        }
        else if (strEq(cmd, "checktab")) {
            checkTab();
        }
        else if (strEq(cmd, "prevtab")) {
            int steps = 0;
            if (sscanf(arg1, "%d", &steps) != 1 || steps < 1 || steps > 999999)
                printf(RED "[E-NFB-400] Input tidak valid!\n" RESET);
            else prevTab(steps);
        }
        else if (strEq(cmd, "nexttab")) {
            int steps = 0;
            if (sscanf(arg1, "%d", &steps) != 1 || steps < 1 || steps > 999999)
                printf(RED "[E-NFB-400] Input tidak valid!\n" RESET);
            else nextTab(steps);
        }
        else if (strEq(cmd, "add_page")) {
            if (!validateURL(arg1)) printf(RED "[E-NFB-400] Format URL tidak valid!\n" RESET);
            else addPage(arg1, NULL);
        }
        else if (strEq(cmd, "edit_page")) {
            if (!validateURL(arg1)) printf(RED "[E-NFB-400] Format URL tidak valid!\n" RESET);
            else editPage(arg1);
        }
        else if (strEq(cmd, "delete_page")) {
            if (!validateURL(arg1)) printf(RED "[E-NFB-400] Format URL tidak valid!\n" RESET);
            else deletePage(arg1);
        }
        else if (strEq(cmd, "download")) {
            if (!validateURL(arg1)) printf(RED "[E-NFB-400] Format URL tidak valid!\n" RESET);
            else downloadPage(arg1);
        }
        else if (strEq(cmd, "tick")) {
            tick();
        }
        else if (strEq(cmd, "dokter")) {
            showDokterList();
        }
        else if (strEq(cmd, "antrian")) {
            // antrian <id_dokter> <nama_pasien>
            int did = 0;
            if (sscanf(arg1, "%d", &did) != 1 || strlen(arg2) == 0) {
                printf(RED "Usage: antrian <id_dokter> <nama_pasien>\n" RESET);
            } else {
                daftarAntrian(arg2, did);
            }
        }
        else if (strEq(cmd, "lihat_antrian")) {
            int did = 0;
            if (sscanf(arg1, "%d", &did) != 1) printf(RED "Usage: lihat_antrian <id_dokter>\n" RESET);
            else lihatAntrian(did);
        }
        else if (strEq(cmd, "obat")) {
            showObatList();
        }
        else if (strEq(cmd, "load")) {
            if (strlen(arg1) == 0) printf(RED "Usage: load <folder/>\n" RESET);
            else loadData(arg1);
        }
        else if (strEq(cmd, "save")) {
            if (strlen(arg1) == 0) printf(RED "Usage: save <folder/>\n" RESET);
            else saveData(arg1);
        }
        else {
            printf(RED "Perintah '%s' tidak dikenali. Ketik 'help' untuk bantuan.\n" RESET, cmd);
        }
    }
    return 0;
}
