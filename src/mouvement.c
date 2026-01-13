/*
 * Mouvement des véhicules - Navigation et parking
 */
#include "mouvement.h"
#include "mouvement/sprites.h"
#include "mouvement/collision.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdio.h>

extern unsigned long int global_frame_counter;

#define MAX_VOITURES 20
#define MAX_SCAN 10
#define RAYON_SCAN 8
#define TARGET_SORTIE -2
#define TURN_LOCK_DURATION 5

static void corriger_alignement_fleche(VEHICULE *vehicule, PlanParking *plan);

typedef struct {
    VEHICULE *vehicule;
    int target_place;
} VoitureTarget;

typedef struct {
    VEHICULE *vehicule;
    int turn_lock_counter;
} VoitureLaneLock;

static VoitureTarget targets[MAX_VOITURES];
static VoitureLaneLock lane_locks[MAX_VOITURES];
static int targets_initialized = 0;
static int lane_locks_initialized = 0;

static void init_lane_locks() {
    for (int i = 0; i < MAX_VOITURES; i++) {
        lane_locks[i].vehicule = NULL;
        lane_locks[i].turn_lock_counter = 0;
    }
    lane_locks_initialized = 1;
}

static void activer_lane_lock(VEHICULE *v) {
    if (!lane_locks_initialized) init_lane_locks();
    for (int i = 0; i < MAX_VOITURES; i++) {
        if (lane_locks[i].vehicule == v || lane_locks[i].vehicule == NULL) {
            lane_locks[i].vehicule = v;
            lane_locks[i].turn_lock_counter = TURN_LOCK_DURATION;
            return;
        }
    }
}

static int a_lane_lock_actif(VEHICULE *v) {
    if (!lane_locks_initialized) return 0;
    for (int i = 0; i < MAX_VOITURES; i++) {
        if (lane_locks[i].vehicule == v)
            return lane_locks[i].turn_lock_counter > 0;
    }
    return 0;
}

static void decrementer_lane_locks() {
    if (!lane_locks_initialized) return;
    for (int i = 0; i < MAX_VOITURES; i++) {
        if (lane_locks[i].vehicule != NULL && lane_locks[i].turn_lock_counter > 0) {
            lane_locks[i].turn_lock_counter--;
        }
    }
}

static void init_targets() {
    for (int i = 0; i < MAX_VOITURES; i++) {
        targets[i].vehicule = NULL;
        targets[i].target_place = -1;
    }
    targets_initialized = 1;
}

static int obtenir_target(VEHICULE *v) {
    if (!targets_initialized) init_targets();
    for (int i = 0; i < MAX_VOITURES; i++) {
        if (targets[i].vehicule == v) return targets[i].target_place;
    }
    return -1;
}

static void set_target(VEHICULE *v, int place_index) {
    if (!targets_initialized) init_targets();
    for (int i = 0; i < MAX_VOITURES; i++) {
        if (targets[i].vehicule == v) {
            targets[i].target_place = place_index;
            return;
        }
    }
    for (int i = 0; i < MAX_VOITURES; i++) {
        if (targets[i].vehicule == NULL) {
            targets[i].vehicule = v;
            targets[i].target_place = place_index;
            return;
        }
    }
}

static void clear_target(VEHICULE *v) {
    if (!targets_initialized) init_targets();
    for (int i = 0; i < MAX_VOITURES; i++) {
        if (targets[i].vehicule == v) {
            targets[i].vehicule = NULL;
            targets[i].target_place = -1;
            return;
        }
    }
}

void marquer_vehicule_en_sortie(VEHICULE *vehicule) {
    set_target(vehicule, TARGET_SORTIE);
}

static int est_fleche_simple(wchar_t c) {
    return (c == L'←' || c == L'→' || c == L'↑' || c == L'↓');
}

static int est_fleche_circulation(wchar_t c) {
    return (c == L'←' || c == L'→');
}

static char sens_fleche(wchar_t c) {
    if (c == L'←') return 'O';
    if (c == L'→') return 'E';
    if (c == L'↑') return 'N';
    if (c == L'↓') return 'S';
    return '\0';
}

static int compter_passes(PlanParking *plan, int cx, int cy, char dir) {
    if (!plan) return 0;
    int dx = 0, dy = 0;
    obtenir_delta_direction(dir, &dx, &dy);
    if (dx == 0 && dy == 0) return 0;
    int passes = 0, x = cx + dx, y = cy + dy;
    for (int i = 0; i < MAX_SCAN; i++) {
        if (!est_dans_limites(plan, x, y)) break;
        if (!est_cellule_roulable(plan, x, y)) break;
        passes++;
        x += dx;
        y += dy;
    }
    return passes;
}

static void compter_fleches_zone(PlanParking *plan, int cx, int cy, int R, int counts[4]) {
    counts[0] = counts[1] = counts[2] = counts[3] = 0;
    if (!plan) return;
    for (int y = cy - R; y <= cy + R; y++) {
        for (int x = cx - R; x <= cx + R; x++) {
            if (!est_dans_limites(plan, x, y)) continue;
            wchar_t c = plan->plan_statique[y][x];
            if (!est_fleche_circulation(c)) continue;
            char sens = sens_fleche(c);
            switch (sens) {
                case 'N': counts[0]++; break;
                case 'S': counts[1]++; break;
                case 'E': counts[2]++; break;
                case 'O': counts[3]++; break;
            }
        }
    }
}

static int compter_voisins_roulables(PlanParking *plan, int cx, int cy) {
    int count = 0;
    if (est_cellule_roulable(plan, cx, cy - 1)) count++;
    if (est_cellule_roulable(plan, cx, cy + 1)) count++;
    if (est_cellule_roulable(plan, cx + 1, cy)) count++;
    if (est_cellule_roulable(plan, cx - 1, cy)) count++;
    return count;
}

static int lane_lock_bloque_changement(VEHICULE *vehicule, PlanParking *plan, int centre_x, int centre_y, char dir) {
    if (!a_lane_lock_actif(vehicule)) return 0;
    int dx = 0, dy = 0;
    obtenir_delta_direction(dir, &dx, &dy);
    int next_centre_x = centre_x + dx;
    int next_centre_y = centre_y + dy;
    if (est_dans_limites(plan, next_centre_x, next_centre_y)) {
        wchar_t c_next = plan->plan_statique[next_centre_y][next_centre_x];
        int allee_devant = (c_next == L' ' || c_next == L'←' || c_next == L'→' ||
                           c_next == L'↑' || c_next == L'↓' || c_next == L'.');
        if (allee_devant) return 1;
    }
    return 0;
}

static char detecter_fleche_proche(PlanParking *plan, int cx, int cy) {
    wchar_t fleche_proche = 0;
    int fleche_proche_y = -1;
    for (int dy = -3; dy <= 3; dy++) {
        for (int dx = -3; dx <= 3; dx++) {
            int check_x = cx + dx, check_y = cy + dy;
            if (est_dans_limites(plan, check_x, check_y)) {
                wchar_t c = plan->plan_statique[check_y][check_x];
                if (c == L'↑' || c == L'↓') {
                    fleche_proche = c;
                    fleche_proche_y = check_y;
                    break;
                }
            }
        }
        if (fleche_proche) break;
    }
    if (!fleche_proche) return '\0';
    int ligne_rangee = fleche_proche_y - 1;
    for (int i = 0; i < plan->places_totales; i++) {
        if (plan->places[i].occupee == 0 && plan->places[i].ligne == ligne_rangee) {
            return sens_fleche(fleche_proche);
        }
    }
    return '\0';
}

static int doit_recalculer_direction(VEHICULE *vehicule, PlanParking *plan, int fleches_zone[4], int target_x, int target_y) {
    if (!vehicule || !plan) return 1;
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;
    char dir = vehicule->direction;
    if (!est_dans_limites(plan, centre_x, centre_y)) return 1;
    wchar_t c_actuel = plan->plan_statique[centre_y][centre_x];
    if (lane_lock_bloque_changement(vehicule, plan, centre_x, centre_y, dir)) return 0;
    if (est_fleche_circulation(c_actuel)) return 1;
    char dir_fleche_parking = detecter_fleche_proche(plan, centre_x, centre_y);
    if (dir_fleche_parking != '\0') return 1;
    int fleches_perp = 0;
    if (dir == 'N' || dir == 'S') fleches_perp = fleches_zone[2] + fleches_zone[3];
    else fleches_perp = fleches_zone[0] + fleches_zone[1];
    if (fleches_perp > 0) return 1;
    int dx = 0, dy = 0;
    obtenir_delta_direction(dir, &dx, &dy);
    int nx = vehicule->posx + dx, ny = vehicule->posy + dy;
    if (!peut_deplacer_sur_allee(vehicule, plan, nx, ny)) {
        int est_hors_limites = (nx < 0 || ny < 0 || nx + largeur > plan->largeur || ny + hauteur > plan->hauteur);
        if (est_hors_limites) return 1;
        int new_centre_x = nx + largeur / 2;
        int new_centre_y = ny + hauteur / 2;
        if (est_dans_limites(plan, new_centre_x, new_centre_y)) {
            wchar_t c_new = plan->plan_statique[new_centre_y][new_centre_x];
            if (c_new == L' ' || c_new == L'←' || c_new == L'→' ||
                c_new == L'↑' || c_new == L'↓' || c_new == L'.') return 0;
        }
        return 1;
    }
    int voisins = compter_voisins_roulables(plan, centre_x, centre_y);
    if (voisins >= 3) return 1;
    if (target_x >= 0 && target_y >= 0) {
        int dx_cible = target_x - centre_x;
        int dy_cible = target_y - centre_y;
        if (abs(dy_cible) > 5 || abs(dx_cible) > 5) {
            if (dir == 'N' && dy_cible > 5) return 1;
            if (dir == 'S' && dy_cible < -5) return 1;
            if (dir == 'E' && dx_cible < -5) return 1;
            if (dir == 'O' && dx_cible > 5) return 1;
        }
    }
    return 0;
}

static char forcer_direction_si_fleche_parking(VEHICULE *vehicule, PlanParking *plan, int cx, int cy) {
    if (!vehicule || !plan) return '\0';
    wchar_t fleche_parking_proche = 0;
    int fleche_parking_y = -1;
    for (int dy = -3; dy <= 3; dy++) {
        for (int dx = -3; dx <= 3; dx++) {
            int check_x = cx + dx, check_y = cy + dy;
            if (est_dans_limites(plan, check_x, check_y)) {
                wchar_t c = plan->plan_statique[check_y][check_x];
                if (c == L'↑' || c == L'↓') {
                    fleche_parking_proche = c;
                    fleche_parking_y = check_y;
                    break;
                }
            }
        }
        if (fleche_parking_proche) break;
    }
    if (fleche_parking_proche) {
        int ligne_rangee = fleche_parking_y - 1;
        for (int i = 0; i < plan->places_totales; i++) {
            if (plan->places[i].occupee == 0 && plan->places[i].ligne == ligne_rangee) {
                char dir_parking = sens_fleche(fleche_parking_proche);
                activer_lane_lock(vehicule);
                return dir_parking;
            }
        }
    }
    return '\0';
}

static void calculer_scores_avec_bonus(PlanParking *plan, int cx, int cy, int target_x, int target_y,
                                       int scores[4], int *meilleur_score, char *dir_meilleure) {
    if (!plan || !scores || !meilleur_score || !dir_meilleure) return;
    int fleches_zone[4];
    compter_fleches_zone(plan, cx, cy, RAYON_SCAN, fleches_zone);
    int passes_N = compter_passes(plan, cx, cy, 'N');
    int passes_S = compter_passes(plan, cx, cy, 'S');
    int passes_E = compter_passes(plan, cx, cy, 'E');
    int passes_O = compter_passes(plan, cx, cy, 'O');
    scores[0] = 10 * passes_N + 3 * fleches_zone[0];
    scores[1] = 10 * passes_S + 3 * fleches_zone[1];
    scores[2] = 10 * passes_E + 3 * fleches_zone[2];
    scores[3] = 10 * passes_O + 3 * fleches_zone[3];
    if (target_x >= 0 && target_y >= 0) {
        int dx_cible = target_x - cx, dy_cible = target_y - cy;
        if (dy_cible < -3 && passes_N > 0) scores[0] += 20;
        if (dy_cible > 3 && passes_S > 0) scores[1] += 20;
        if (dx_cible > 3 && passes_E > 0) scores[2] += 20;
        if (dx_cible < -3 && passes_O > 0) scores[3] += 20;
    }
    if (cy >= 20 && cy <= 26) {
        if (cx > 85) {
            if (passes_O > 0) scores[3] += 50;
        } else {
            if (passes_N > 0) scores[0] += 40;
        }
    } else if (cy >= 17 && cy <= 19) {
        if (passes_O > 0) scores[3] += 60;
        if (passes_E > 0) scores[2] += 30;
    }
    *meilleur_score = scores[0];
    *dir_meilleure = 'N';
    if (scores[1] > *meilleur_score) { *meilleur_score = scores[1]; *dir_meilleure = 'S'; }
    if (scores[2] > *meilleur_score) { *meilleur_score = scores[2]; *dir_meilleure = 'E'; }
    if (scores[3] > *meilleur_score) { *meilleur_score = scores[3]; *dir_meilleure = 'O'; }
}

static char valider_direction_lane_keeping(VEHICULE *vehicule, PlanParking *plan, int cx, int cy,
                                           char dir_actuelle, char nouvelle_dir) {
    if (!vehicule || !plan) return dir_actuelle;
    int dx_new = 0, dy_new = 0;
    obtenir_delta_direction(nouvelle_dir, &dx_new, &dy_new);
    int dx_old = 0, dy_old = 0;
    obtenir_delta_direction(dir_actuelle, &dx_old, &dy_old);
    int next_cx_new = cx + dx_new, next_cy_new = cy + dy_new;
    int next_cx_old = cx + dx_old, next_cy_old = cy + dy_old;
    int new_dir_safe = 0, old_dir_safe = 0;
    wchar_t c_ahead_new = L'?', c_ahead_old = L'?';
    if (est_dans_limites(plan, next_cx_new, next_cy_new)) {
        c_ahead_new = plan->plan_statique[next_cy_new][next_cx_new];
        new_dir_safe = (c_ahead_new == L' ' || c_ahead_new == L'←' || c_ahead_new == L'→' ||
                       c_ahead_new == L'↑' || c_ahead_new == L'↓' || c_ahead_new == L'.');
    }
    if (est_dans_limites(plan, next_cx_old, next_cy_old)) {
        c_ahead_old = plan->plan_statique[next_cy_old][next_cx_old];
        old_dir_safe = (c_ahead_old == L' ' || c_ahead_old == L'←' || c_ahead_old == L'→' ||
                       c_ahead_old == L'↑' || c_ahead_old == L'↓' || c_ahead_old == L'.');
    }
    char chosen_dir = nouvelle_dir;
    const char *block_reason = NULL;
    int is_lateral_change = 0;
    if ((dir_actuelle == 'N' || dir_actuelle == 'S') && (nouvelle_dir == 'E' || nouvelle_dir == 'O')) is_lateral_change = 1;
    if ((dir_actuelle == 'E' || dir_actuelle == 'O') && (nouvelle_dir == 'N' || nouvelle_dir == 'S')) is_lateral_change = 1;
    int nb_voisins_allee = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (abs(dx) + abs(dy) != 1) continue;
            int check_x = cx + dx, check_y = cy + dy;
            if (est_dans_limites(plan, check_x, check_y)) {
                wchar_t c = plan->plan_statique[check_y][check_x];
                if (c == L' ' || c == L'←' || c == L'→' || c == L'↑' || c == L'↓' || c == L'.') nb_voisins_allee++;
            }
        }
    }
    int is_intersection = (nb_voisins_allee >= 3);
    wchar_t c_actuel = plan->plan_statique[cy][cx];
    int fleche_circulation_sous_centre = est_fleche_circulation(c_actuel);
    int exception_parking = 0;
    wchar_t fleche_exception = 0;
    int fleche_exception_y = -1;
    for (int dy = -3; dy <= 3; dy++) {
        for (int dx = -3; dx <= 3; dx++) {
            int check_x = cx + dx, check_y = cy + dy;
            if (est_dans_limites(plan, check_x, check_y)) {
                wchar_t c = plan->plan_statique[check_y][check_x];
                if (c == L'↑' || c == L'↓') {
                    fleche_exception = c;
                    fleche_exception_y = check_y;
                    break;
                }
            }
        }
        if (fleche_exception) break;
    }
    if (fleche_exception) {
        char dir_parking_attendue = sens_fleche(fleche_exception);
        if (nouvelle_dir == dir_parking_attendue) {
            int ligne_rangee = fleche_exception_y - 1;
            for (int i = 0; i < plan->places_totales; i++) {
                if (plan->places[i].occupee == 0 && plan->places[i].ligne == ligne_rangee) {
                    exception_parking = 1;
                    break;
                }
            }
        }
    }
    if (is_lateral_change && old_dir_safe && !is_intersection && !fleche_circulation_sous_centre && !exception_parking) {
        chosen_dir = dir_actuelle;
        block_reason = "LANE_KEEP";
    }
    int is_uturn = 0;
    if ((dir_actuelle == 'N' && nouvelle_dir == 'S') || (dir_actuelle == 'S' && nouvelle_dir == 'N')) is_uturn = 1;
    if ((dir_actuelle == 'E' && nouvelle_dir == 'O') || (dir_actuelle == 'O' && nouvelle_dir == 'E')) is_uturn = 1;
    if (is_uturn && old_dir_safe && !block_reason) {
        chosen_dir = dir_actuelle;
        block_reason = "UTURN_FORBIDDEN";
    }
    if (!new_dir_safe && old_dir_safe && !block_reason) {
        chosen_dir = dir_actuelle;
        block_reason = "NEW_DIR_UNSAFE";
    } else if (!new_dir_safe && !old_dir_safe && !block_reason) {
        chosen_dir = dir_actuelle;
        block_reason = "BOTH_UNSAFE";
    }
    (void)block_reason;
    return chosen_dir;
}

static char choisir_direction_stable(VEHICULE *vehicule, PlanParking *plan, int target_x, int target_y) {
    if (!vehicule || !plan) return vehicule->direction;
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    int cx = vehicule->posx + largeur / 2;
    int cy = vehicule->posy + hauteur / 2;
    if (!est_dans_limites(plan, cx, cy)) return vehicule->direction;
    wchar_t c_actuel = plan->plan_statique[cy][cx];
    if (est_fleche_simple(c_actuel)) {
        char nouvelle_dir = sens_fleche(c_actuel);
        return nouvelle_dir;
    }
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            int check_y = cy + dy, check_x = cx + dx;
            if (est_dans_limites(plan, check_x, check_y)) {
                wchar_t c = plan->plan_statique[check_y][check_x];
                if (est_fleche_circulation(c)) {
                    char dir_fleche = sens_fleche(c);
                    int sur_ligne_horizontale = (dir_fleche == 'E' || dir_fleche == 'O') && (abs(dy) <= 1);
                    int sur_colonne_verticale = (dir_fleche == 'N' || dir_fleche == 'S') && (abs(dx) <= 1);
                    if (sur_ligne_horizontale || sur_colonne_verticale) return dir_fleche;
                }
            }
        }
    }
    char dir_parking = forcer_direction_si_fleche_parking(vehicule, plan, cx, cy);
    if (dir_parking != '\0') return dir_parking;
    int scores[4], meilleur_score = 0;
    char dir_meilleure = 'N';
    calculer_scores_avec_bonus(plan, cx, cy, target_x, target_y, scores, &meilleur_score, &dir_meilleure);
    char dir_actuelle = vehicule->direction, nouvelle_dir = dir_actuelle;
    if (meilleur_score == 0) {
        nouvelle_dir = dir_actuelle;
    } else {
        nouvelle_dir = dir_meilleure;
    }
    int score_actuel = 0;
    if (dir_actuelle == 'N') score_actuel = scores[0];
    else if (dir_actuelle == 'S') score_actuel = scores[1];
    else if (dir_actuelle == 'E') score_actuel = scores[2];
    else if (dir_actuelle == 'O') score_actuel = scores[3];
    if (score_actuel >= meilleur_score - 2 && score_actuel > 0) nouvelle_dir = dir_actuelle;
    char chosen_dir = valider_direction_lane_keeping(vehicule, plan, cx, cy, dir_actuelle, nouvelle_dir);
    return chosen_dir;
}

void suivre_fleches(VEHICULE *vehicule, PlanParking *plan) {
    if (!vehicule || !plan || vehicule->etat != '1') return;
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;
    if (!est_dans_limites(plan, centre_x, centre_y)) return;
    int anticipation = 4, meilleure_distance = 999, meilleur_index = -1;
    for (int i = 0; i < plan->nb_fleches; i++) {
        int fx = plan->fleches[i].colonne, fy = plan->fleches[i].ligne;
        int dist_centre = abs(fx - centre_x) + abs(fy - centre_y);
        int est_devant = 0;
        switch (vehicule->direction) {
            case 'N': est_devant = (fy < centre_y && fy >= centre_y - anticipation); break;
            case 'S': est_devant = (fy > centre_y && fy <= centre_y + anticipation); break;
            case 'E': est_devant = (fx > centre_x && fx <= centre_x + anticipation); break;
            case 'O': est_devant = (fx < centre_x && fx >= centre_x - anticipation); break;
        }
        if ((dist_centre <= 2) || est_devant) {
            if (dist_centre < meilleure_distance) {
                meilleure_distance = dist_centre;
                meilleur_index = i;
            }
        }
    }
    if (meilleur_index >= 0) {
        if (plan->fleches[meilleur_index].direction_entree == '\0') {
            vehicule->direction = plan->fleches[meilleur_index].direction_sortie;
        } else {
            if (vehicule->direction == plan->fleches[meilleur_index].direction_entree) {
                vehicule->direction = plan->fleches[meilleur_index].direction_sortie;
            }
        }
    }
}

void deplacer_vehicule(VEHICULE *vehicule, PlanParking *plan) {
    if (!vehicule || !plan || vehicule->etat != '1') return;
    char ancienne_direction = vehicule->direction;
    suivre_fleches(vehicule, plan);
    if (ancienne_direction != vehicule->direction) orienter_carrosserie(vehicule);
    int nouveau_x = vehicule->posx, nouveau_y = vehicule->posy;
    int dx = 0, dy = 0;
    obtenir_delta_direction(vehicule->direction, &dx, &dy);
    if (dx == 0 && dy == 0) return;
    nouveau_x += dx * vehicule->vitesse;
    nouveau_y += dy * vehicule->vitesse;
    if (peut_deplacer(vehicule, plan, nouveau_x, nouveau_y)) {
        vehicule->posx = nouveau_x;
        vehicule->posy = nouveau_y;
    } else {
        char nouvelles_directions[] = {'N', 'S', 'E', 'O'};
        for (int i = 0; i < 4; i++) {
            if (nouvelles_directions[i] == vehicule->direction) continue;
            int test_x = vehicule->posx, test_y = vehicule->posy;
            int test_dx = 0, test_dy = 0;
            obtenir_delta_direction(nouvelles_directions[i], &test_dx, &test_dy);
            test_x += test_dx * vehicule->vitesse;
            test_y += test_dy * vehicule->vitesse;
            if (peut_deplacer(vehicule, plan, test_x, test_y)) {
                vehicule->direction = nouvelles_directions[i];
                orienter_carrosserie(vehicule);
                vehicule->posx = test_x;
                vehicule->posy = test_y;
                return;
            }
        }
    }
}

int trouver_place_libre_proche(VEHICULE *vehicule, PlanParking *plan) {
    if (!vehicule || !plan) return -1;
    int meilleur_index = -1, meilleure_distance = 999999;
    int cx = vehicule->posx, cy = vehicule->posy;
    for (int i = 0; i < plan->places_totales; i++) {
        if (plan->places[i].occupee == 0) {
            int place_x = plan->places[i].colonne, place_y = plan->places[i].ligne;
            int distance = abs(cx - place_x) + abs(cy - place_y);
            if (distance < meilleure_distance) {
                meilleure_distance = distance;
                meilleur_index = i;
            }
        }
    }
    return meilleur_index;
}

static int tenter_parking_automatique(VEHICULE *vehicule, PlanParking *plan) {
    if (!vehicule || !plan) return 0;
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;
    if (!est_dans_limites(plan, centre_x, centre_y)) return 0;
    wchar_t fleche_trouvee = 0;
    int fleche_x = -1, fleche_y = -1;
    for (int dy = -3; dy <= 3; dy++) {
        for (int dx = -3; dx <= 3; dx++) {
            int check_x = centre_x + dx, check_y = centre_y + dy;
            if (est_dans_limites(plan, check_x, check_y)) {
                wchar_t c = plan->plan_statique[check_y][check_x];
                if (c == L'↓' || c == L'↑') {
                    fleche_trouvee = c;
                    fleche_x = check_x;
                    fleche_y = check_y;
                    break;
                }
            }
        }
        if (fleche_trouvee) break;
    }
    if (!fleche_trouvee) return 0;
    char dir_parking = (fleche_trouvee == L'↓') ? 'S' : 'N';
    if (vehicule->direction != dir_parking) {
        activer_lane_lock(vehicule);
        return 0;
    }
    int place_trouvee = -1, dist_min = 99999;
    int ligne_rangee = fleche_y - 1;
    for (int i = 0; i < plan->places_totales; i++) {
        if (plan->places[i].occupee != 0) continue;
        if (plan->places[i].ligne != ligne_rangee) continue;
        int dx = abs(plan->places[i].colonne - fleche_x);
        if (dx < dist_min && dx <= 15) {
            dist_min = dx;
            place_trouvee = i;
        }
    }
    if (place_trouvee == -1) return 0;
    vehicule->posx = plan->places[place_trouvee].colonne - largeur / 2;
    vehicule->posy = plan->places[place_trouvee].ligne + 1;
    vehicule->direction = dir_parking;
    orienter_carrosserie(vehicule);
    marquer_place_occupee(plan, place_trouvee);
    vehicule->etat = '0';
    vehicule->tps = global_frame_counter;
    return 1;
}

void deplacer_vers_place(VEHICULE *vehicule, PlanParking *plan, int index_place) {
    if (!vehicule || !plan || vehicule->etat != '1') return;
    if (index_place < 0 || index_place >= plan->places_totales) return;
    int target_x = plan->places[index_place].colonne;
    int target_y = plan->places[index_place].ligne;
    if (vehicule->posx == target_x && vehicule->posy == target_y) {
        marquer_place_occupee(plan, index_place);
        clear_target(vehicule);
        return;
    }
    int nouveau_x = vehicule->posx, nouveau_y = vehicule->posy;
    if (vehicule->posx < target_x) {
        nouveau_x = vehicule->posx + 1;
        if (est_cellule_roulable(plan, nouveau_x, nouveau_y)) {
            vehicule->posx = nouveau_x;
            return;
        }
    } else if (vehicule->posx > target_x) {
        nouveau_x = vehicule->posx - 1;
        if (est_cellule_roulable(plan, nouveau_x, nouveau_y)) {
            vehicule->posx = nouveau_x;
            return;
        }
    }
    nouveau_x = vehicule->posx;
    nouveau_y = vehicule->posy;
    if (vehicule->posy < target_y) {
        nouveau_y = vehicule->posy + 1;
        if (est_cellule_roulable(plan, nouveau_x, nouveau_y)) {
            vehicule->posy = nouveau_y;
            return;
        }
    } else if (vehicule->posy > target_y) {
        nouveau_y = vehicule->posy - 1;
        if (est_cellule_roulable(plan, nouveau_x, nouveau_y)) {
            vehicule->posy = nouveau_y;
            return;
        }
    }
}

static int detecter_intersection(VEHICULE *vehicule, PlanParking *plan) {
    if (!vehicule || !plan) return 0;
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;
    int rayon = 5, directions_trouvees = 0;
    int a_nord = 0, a_sud = 0, a_est = 0, a_ouest = 0;
    for (int dy = -rayon; dy <= rayon; dy++) {
        for (int dx = -rayon; dx <= rayon; dx++) {
            int check_x = centre_x + dx, check_y = centre_y + dy;
            if (!est_dans_limites(plan, check_x, check_y)) continue;
            wchar_t c = plan->plan_statique[check_y][check_x];
            if (c == L'↑' && !a_nord) { a_nord = 1; directions_trouvees++; }
            if (c == L'↓' && !a_sud) { a_sud = 1; directions_trouvees++; }
            if (c == L'→' && !a_est) { a_est = 1; directions_trouvees++; }
            if (c == L'←' && !a_ouest) { a_ouest = 1; directions_trouvees++; }
        }
    }
    return (directions_trouvees >= 3);
}

static int calculer_vitesse_adaptative(VEHICULE *vehicule, PlanParking *plan) {
    if (!vehicule || !plan) return 1;
    int vitesse_base = vehicule->vitesse;
    int target = obtenir_target(vehicule);
    int mode_sortie = (target == TARGET_SORTIE);
    if (mode_sortie) return vitesse_base;
    if (detecter_intersection(vehicule, plan)) return 1;
    return vitesse_base;
}

static int valider_cible_parking(VEHICULE *vehicule, PlanParking *plan) {
    int target = obtenir_target(vehicule);
    int mode_sortie = (target == TARGET_SORTIE);
    if (!mode_sortie && target >= 0 && target < plan->places_totales) {
        if (plan->places[target].occupee != 0) {
            clear_target(vehicule);
            return 0;
        }
        return 1;
    } else if (mode_sortie) {
        return 1;
    }
    return 0;
}

static void recalculer_direction_vers_cible(VEHICULE *vehicule, PlanParking *plan, l_car *tous_vehicules) {
    int target = obtenir_target(vehicule);
    int mode_sortie = (target == TARGET_SORTIE);
    int target_x, target_y;
    if (mode_sortie) {
        target_x = plan->sortie_x;
        target_y = plan->sortie_y;
    } else if (target >= 0) {
        target_x = plan->places[target].colonne;
        target_y = plan->places[target].ligne;
    } else {
        target_x = -1;
        target_y = -1;
    }
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;
    int fleches_zone[4];
    compter_fleches_zone(plan, centre_x, centre_y, RAYON_SCAN, fleches_zone);
    char ancienne_direction = vehicule->direction, nouvelle_direction = ancienne_direction;
    if (doit_recalculer_direction(vehicule, plan, fleches_zone, target_x, target_y)) {
        nouvelle_direction = choisir_direction_stable(vehicule, plan, target_x, target_y);
    }
    if (ancienne_direction != nouvelle_direction) {
        if (voie_libre_direction(vehicule, tous_vehicules, nouvelle_direction, plan)) {
            vehicule->direction = nouvelle_direction;
            orienter_carrosserie(vehicule);
        }
    }
}

void deplacer_vehicule_parking_auto(VEHICULE *vehicule, PlanParking *plan, l_car *tous_vehicules) {
    if (!vehicule || !plan || vehicule->etat != '1') return;
    int target = obtenir_target(vehicule);
    int mode_sortie = (target == TARGET_SORTIE);
    int cible_valide = valider_cible_parking(vehicule, plan);
    if (!cible_valide && !mode_sortie) {
        int nouvelle_cible = trouver_place_libre_proche(vehicule, plan);
        if (nouvelle_cible >= 0) {
            set_target(vehicule, nouvelle_cible);
            target = nouvelle_cible;
        }
    }
    recalculer_direction_vers_cible(vehicule, plan, tous_vehicules);
    char nouvelle_direction = vehicule->direction;
    if (!mode_sortie && tenter_parking_automatique(vehicule, plan)) {
        clear_target(vehicule);
        return;
    }
    if (target >= 0 || 1) {
        int vitesse_effective = calculer_vitesse_adaptative(vehicule, plan);
        int nouveau_x = vehicule->posx, nouveau_y = vehicule->posy;
        int dx = 0, dy = 0;
        obtenir_delta_direction(nouvelle_direction, &dx, &dy);
        nouveau_x += dx * vitesse_effective;
        nouveau_y += dy * vitesse_effective;
        int peut_bouger = peut_deplacer_sur_allee(vehicule, plan, nouveau_x, nouveau_y);
        if (peut_bouger) {
            vehicule->posx = nouveau_x;
            vehicule->posy = nouveau_y;
        }
    }
}

static int chercher_alignement_optimal(VEHICULE *vehicule, PlanParking *plan, int *best_x, int *best_y) {
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;
    int search_radius = 2, best_dist = 999;
    *best_x = -1;
    *best_y = -1;
    for (int dy = -search_radius; dy <= search_radius; dy++) {
        for (int dx = -search_radius; dx <= search_radius; dx++) {
            if (dx == 0 && dy == 0) continue;
            int check_x = centre_x + dx, check_y = centre_y + dy;
            if (!est_dans_limites(plan, check_x, check_y)) continue;
            wchar_t c = plan->plan_statique[check_y][check_x];
            int compatible = 0;
            if ((vehicule->direction == 'O' && c == L'←') ||
                (vehicule->direction == 'E' && c == L'→') ||
                (vehicule->direction == 'N' && c == L'↑') ||
                (vehicule->direction == 'S' && c == L'↓')) {
                compatible = 2;
            } else if (c == L' ' || c == L'.') {
                compatible = 1;
            }
            if (compatible > 0) {
                int dist = abs(dx) + abs(dy);
                int priorite_bonus = 0;
                if (vehicule->direction == 'O' || vehicule->direction == 'E') {
                    if (dy == 0) priorite_bonus = -5;
                } else {
                    if (dx == 0) priorite_bonus = -5;
                }
                if (compatible == 2) priorite_bonus -= 3;
                dist += priorite_bonus;
                if (dist < best_dist) {
                    best_dist = dist;
                    *best_x = check_x;
                    *best_y = check_y;
                }
            }
        }
    }
    return (*best_x >= 0 && *best_y >= 0 && best_dist <= 2);
}

static void corriger_alignement_fleche(VEHICULE *vehicule, PlanParking *plan) {
    if (!vehicule || !plan) return;
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;
    if (est_dans_limites(plan, centre_x, centre_y)) {
        wchar_t c_centre = plan->plan_statique[centre_y][centre_x];
        if (c_centre == L'←' || c_centre == L'→' || c_centre == L'↑' || c_centre == L'↓' ||
            c_centre == L' ' || c_centre == L'.') return;
    }
    int best_x, best_y;
    if (!chercher_alignement_optimal(vehicule, plan, &best_x, &best_y)) return;
    int nouveau_coin_x = best_x - largeur / 2;
    int nouveau_coin_y = best_y - hauteur / 2;
    int position_ok = 1;
    for (int dy = 0; dy < hauteur; dy++) {
        for (int dx = 0; dx < largeur; dx++) {
            int test_x = nouveau_coin_x + dx, test_y = nouveau_coin_y + dy;
            if (!est_dans_limites(plan, test_x, test_y)) {
                position_ok = 0;
                break;
            }
            if (!est_cellule_roulable(plan, test_x, test_y)) {
                position_ok = 0;
                break;
            }
        }
        if (!position_ok) break;
    }
    if (position_ok) {
        vehicule->posx = nouveau_coin_x;
        vehicule->posy = nouveau_coin_y;
    }
}

void corriger_alignement_vehicule(VEHICULE *vehicule, PlanParking *plan) {
    corriger_alignement_fleche(vehicule, plan);
}

static int vehicule_a_sortie(VEHICULE *vehicule, PlanParking *plan) {
    if (!vehicule || !plan || vehicule->etat != '1') return 0;
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;
    for (int dy = 0; dy < hauteur; dy++) {
        for (int dx = 0; dx < largeur; dx++) {
            int check_x = vehicule->posx + dx, check_y = vehicule->posy + dy;
            if (est_dans_limites(plan, check_x, check_y)) {
                wchar_t c = plan->plan_statique[check_y][check_x];
                if (c == L'S' || c == L's') return 1;
            }
        }
    }
    int dx = abs(centre_x - plan->sortie_x);
    int dy = abs(centre_y - plan->sortie_y);
    return (dx <= 6 && dy <= 6);
}

int deplacer_tous_vehicules(l_car *vehicules, PlanParking *plan) {
    if (!vehicules || est_vide_liste_car(vehicules)) return 0;
    decrementer_lane_locks();
    VEHICULE *v_check = vehicules->premier;
    VEHICULE *vehicules_a_supprimer[50];
    int nb_a_supprimer = 0;
    while (v_check != NULL) {
        if (vehicule_a_sortie(v_check, plan) && plan->barriere_sortie_ouverte) {
            vehicules_a_supprimer[nb_a_supprimer++] = v_check;
        }
        v_check = v_check->NXT;
    }
    VEHICULE *curr = vehicules->premier;
    while (curr != NULL) {
        if (curr->etat == '1') {
            deplacer_vehicule_parking_auto(curr, plan, vehicules);
        }
        curr = curr->NXT;
    }
    for (int i = 0; i < nb_a_supprimer; i++) {
        detruire_vehicule_specifique(vehicules, vehicules_a_supprimer[i]);
    }
    VEHICULE *v1 = vehicules->premier;
    while (v1 != NULL) {
        if (v1->etat == '1') {
            VEHICULE *v2 = v1->NXT;
            while (v2 != NULL) {
                if (v2->etat == '1' && vehicules_en_collision(v1, v2)) return 1;
                v2 = v2->NXT;
            }
        }
        v1 = v1->NXT;
    }
    return 0;
}
