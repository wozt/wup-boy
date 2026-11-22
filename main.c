/* WUP Boy: the Game Boy and Game Boy Color games in this app's roms/ folder (.gb, .gbc: copied there with File Sharing), in the Homebrew Menu's
 * look (libwup's kit). A game starts the player (player.wup, next to this app: player/main.c) with the game's path;
 * the player comes back here when the game is left. No popups: an empty folder says what to do. */
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "wup_kit.h"
#include "wup_lvgl.h"
#include "littlefs/lfs.h"

WUP_APP("WUP Boy");

#define MAXG 48
static struct { char file[64], name[64], path[192]; int color; } games[MAXG];
static int ngames, started;
static char roms[80];

static int is_rom(const char *n) { size_t l = strlen(n); return (l > 3 && !strcasecmp(n + l - 3, ".gb")) || (l > 4 && !strcasecmp(n + l - 4, ".gbc")); }
static int by_name(const void *a, const void *b) { return strcasecmp(((const char *)a) + 64, ((const char *)b) + 64); }
static void add_game(const char *path, const char *base) {         /* base = the file's name, path = where it lives */
    if (ngames >= MAXG || strlen(base) >= sizeof games[0].file) return;
    snprintf(games[ngames].file, sizeof games[0].file, "%s", base);
    snprintf(games[ngames].name, sizeof games[ngames].name, "%s", base); *strrchr(games[ngames].name, '.') = 0;
    snprintf(games[ngames].path, sizeof games[0].path, "%s", path);
    games[ngames].color = 0;
    int fd = open(path, O_RDONLY);                                /* the cartridge header's colour flag, either backend */
    if (fd >= 0) { uint8_t cgb = 0; if (lseek(fd, 0x143, SEEK_SET) == 0x143 && read(fd, &cgb, 1) == 1) games[ngames].color = (cgb & 0x80) != 0; close(fd); }
    ngames++;
}
static void scan(void) {
    ngames = 0;
    snprintf(roms, sizeof roms, "%s/roms", wup_app_dir()[0] ? wup_app_dir() : "/apps/wup_boy");
    if (!wup_fs_ok()) return;
    lfs_mkdir(&fs_lfs, roms);                              /* (there for whoever looks over File Sharing) */
    lfs_dir_t d; struct lfs_info inf;
    if (!lfs_dir_open(&fs_lfs, &d, roms)) {
        while (lfs_dir_read(&fs_lfs, &d, &inf) > 0) {
            if (inf.type != LFS_TYPE_REG || !is_rom(inf.name)) continue;
            char path[192]; snprintf(path, sizeof path, "%s/%s", roms, inf.name);
            add_game(path, inf.name);
        }
        lfs_dir_close(&fs_lfs, &d);
    }
    DIR *nd = opendir("/net/roms");                         /* the network share: a walk like netfs_browse's, no fs_* calls */
    if (nd) {
        struct dirent *e;
        while ((e = readdir(nd)) && ngames < MAXG) {
            if (e->d_type != DT_REG && e->d_type != DT_UNKNOWN) continue;
            if (!is_rom(e->d_name)) continue;
            char path[192]; snprintf(path, sizeof path, "/net/roms/%s", e->d_name);
            add_game(path, e->d_name);
        }
        closedir(nd);
    }
    qsort(games, (size_t)ngames, sizeof games[0], by_name);
}
/* the player next to this app; when WUP Boy itself was fetched from /net there is none, so player.wup
   is taken from the share's wup_boy/ folder once and cached in this app's own folder (/save/net/wup_boy
   there; a subfolder of the share, so the menu's home does not list player.wup as an app of its own) */
static int ensure_player(char *player, size_t cap) {
    char dir[96];
    snprintf(dir, sizeof dir, "%s", wup_app_dir()[0] ? wup_app_dir() : "/apps/wup_boy");
    snprintf(player, cap, "%s/player.wup", dir);
    int local = fs_filesize(player);
    int fd = open("/net/wup_boy/player.wup", O_RDONLY);       /* the share's copy: its size decides if the cache is current */
    if (fd < 0) return local > 0;                             /* (offline: a cached one still plays) */
    struct stat st;
    if (fstat(fd, &st) || st.st_size <= 0 || st.st_size > (1 << 20)) { close(fd); return local > 0; }
    if (local == (int)st.st_size) { close(fd); return 1; }
    lfs_mkdir(&fs_lfs, dir);
    if (fs_stream_open(player)) { close(fd); return 0; }
    uint8_t buf[16384]; uint64_t left = (uint64_t)st.st_size; int bad = 0;
    while (left) {
        ssize_t k = read(fd, buf, left > sizeof buf ? sizeof buf : (size_t)left);
        if (k <= 0 || fs_stream_write(buf, (uint32_t)k)) { bad = 1; break; }
        left -= (uint64_t)k;
    }
    close(fd);
    if (fs_stream_close() || bad) { fs_remove(player); return 0; }
    return fs_filesize(player) == (int)st.st_size;
}
static void on_game(lv_event_t *e) {
    int i = (int)(intptr_t)lv_event_get_user_data(e);
    char player[192];
    if (!ensure_player(player, sizeof player)) { wup_alert("Player.wup Not Found", "Put player.wup next to wup_boy.wup, or in the share's wup_boy/ folder", "OK", NULL, NULL, 0); return; }
    if (!wup_exec(player, games[i].path)) started = 1;       /* (this app ends; the player is next) */
}
static void home(lv_obj_t *scr) {
    if (!wup_fs_ok()) { kit_outcome(kit_page(scr, "WUP Boy", "B Back"), "Storage Not Available", ""); return; }
    if (!ngames) {
        kit_empty(kit_page(scr, "WUP Boy", "B Back"), "No Games", "Add .gb and .gbc files to apps/wup_boy/roms (File Sharing) or /net/roms (Network Storage)");
        return;
    }
    lv_obj_t *l = kit_page(scr, "WUP Boy", "B Back    A Play");
    for (int i = 0; i < ngames; i++) kit_row(l, NULL, games[i].name, games[i].color ? "Color" : NULL, on_game, (void *)(intptr_t)i);
}
int main(void) {
    scan();
    kit_start(home);
    while (!started && kit_step()) {}
    return 0;
}
