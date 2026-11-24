#ifndef HALODOC_H
#define HALODOC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ANSI COLORS
#define CYAN    "\x1b[36m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define RED     "\x1b[31m"
#define MAGENTA "\x1b[35m"
#define BLUE    "\x1b[34m"
#define BOLD    "\x1b[1m"
#define RESET   "\x1b[0m"

// KONSTANTA 
#define MAX_URL_LEN       100
#define MAX_CONTENT_LEN   1024
#define MAX_PAGES         100
#define MAX_LINKED        10
#define MAX_TABS          10
#define MAX_HISTORY       20
#define CACHE_MAX         5
#define DOWNLOAD_MAX      5
#define MAX_QUEUE_PASIEN  20
#define MAX_DOKTER        20
#define MAX_OBAT          50

// LCG Constants
#define LCG_MULTIPLIER    1103515245
#define LCG_INCREMENT     12345
#define LCG_MODULUS       2147483648U
#define LCG_SEED_INITIAL  73939133

// STRUCT WEB PAGE 
typedef struct {
    int id;
    char url[MAX_URL_LEN];
    char content[MAX_CONTENT_LEN];
    int linked_ids[MAX_LINKED];
    int linked_count;
} WebPage;

// ADJACENCY MATRIX (D01) 
typedef struct {
    int matrix[MAX_PAGES][MAX_PAGES];
    int page_count;
} WebGraph;

// CACHE (ADT Map) 
typedef struct {
    char url[MAX_URL_LEN];
    char content[MAX_CONTENT_LEN];
    int valid;
    int order; // untuk FIFO eviction
} CacheEntry;

typedef struct {
    CacheEntry entries[CACHE_MAX];
    int count;
    int insert_order;
} Cache;

// DOWNLOAD QUEUE 
typedef struct {
    char url[MAX_URL_LEN];
    int ticks_remaining;
} DownloadItem;

typedef struct {
    DownloadItem items[DOWNLOAD_MAX];
    int front, rear, count;
} DownloadQueue;

// TAB HISTORY (ADT Stack) 
typedef struct {
    char urls[MAX_HISTORY][MAX_URL_LEN];
    int top;        // indeks halaman saat ini
    int total;      // total url di history
} TabHistory;

typedef struct {
    char name[20];
    TabHistory history;
    int session_token; // = 0xBEEF
    int is_active;
} Tab;

// ANTRIAN DOKTER (ADT Queue Linked List) 
typedef struct QNode {
    char nama_pasien[50];
    struct QNode* next;
} QNode;

typedef struct {
    QNode* head;
    QNode* tail;
    int count;
} AntrianDokter;

typedef struct {
    int id;
    char nama[50];
    char spesialis[50];
    char url[MAX_URL_LEN]; // halaman profil dokter
    AntrianDokter antrian;
} Dokter;

// OBAT 
typedef struct {
    int id;
    char nama[50];
    char url[MAX_URL_LEN];
    int stok;
    int harga;
} Obat;

// GLOBAL STATE 
extern WebPage pages[MAX_PAGES];
extern int page_count;
extern WebGraph graph;
extern Cache cache;
extern DownloadQueue dlQueue;
extern Tab tabs[MAX_TABS];
extern int tab_count;
extern int current_tab;
extern Dokter dokter_list[MAX_DOKTER];
extern int dokter_count;
extern Obat obat_list[MAX_OBAT];
extern int obat_count;
extern unsigned int lcg_seed;

// PROTOTYPES 

// Utils
int strEq(const char* a, const char* b);
void clearInput(void);
int validateURL(const char* url);
int normalizeUrlKey(const char* url);
void printBanner(void);
void printSeparator(void);

// LCG
unsigned int lcgNext(void);

// Web Pages (Set)
int findPageByURL(const char* url);
int addPage(const char* url, const char* content);
void editPage(const char* url);
void deletePage(const char* url);

// Cache (Map)
void initCache(void);
int cacheGet(const char* url, char* out_content);
void cachePut(const char* url, const char* content);
void evictCache(void);

// Web Graph (Adjacency Matrix)
void initGraph(void);
void addEdge(int src_id, int dst_id);
void removeEdgesOf(int page_id);
void showLinkedPages(int page_id);
int openLinked(int page_id, int link_index);

// Tab Management
void initTabs(void);
void newTab(void);
void closeTab(void);
void checkTab(void);
void prevTab(int steps);
void nextTab(int steps);

// Navigation
void openPage(const char* url);
void goBack(int steps);
void goForward(int steps);
void viewTabHistory(void);
void goHome(void);

// Download Manager
void initDownloadQueue(void);
void downloadPage(const char* url);
void tick(void);
void showDownloadQueue(void);

// Discover & Search
void discover(void);
void searchPage(const char* query);

// Load & Save (D03, D04)
void loadData(const char* folder);
void saveData(const char* folder);

// Halodoc Features
void initDokter(void);
void initObat(void);
void showDokterList(void);
void daftarAntrian(const char* nama_pasien, int dokter_id);
void lihatAntrian(int dokter_id);
void showObatList(void);
void showHalodocHome(void);

// Antrian Queue
void initAntrianDokter(AntrianDokter* q);
void enqueueAntrian(AntrianDokter* q, const char* nama);
void dequeueAntrian(AntrianDokter* q);
void freeAntrian(AntrianDokter* q);

#endif
