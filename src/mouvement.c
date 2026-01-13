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
typedef struct { VEHICULE *vehicule; int target_place, lane_lock_counter; } VehiculeState;
static VehiculeState states[MAX_VOITURES];
static int states_initialized = 0;
static void init_states() {
    for (int i = 0; i < MAX_VOITURES; i++) states[i].vehicule = NULL, states[i].target_place = -1, states[i].lane_lock_counter = 0;
    states_initialized = 1;
}
static VehiculeState* get_state(VEHICULE *v) {
    if (!states_initialized) init_states();
    for (int i = 0; i < MAX_VOITURES; i++) if (states[i].vehicule == v) return &states[i];
    for (int i = 0; i < MAX_VOITURES; i++) if (!states[i].vehicule) { states[i].vehicule = v; return &states[i]; }
    return NULL;
}
static void activer_lane_lock(VEHICULE *v) { VehiculeState *s = get_state(v); if (s) s->lane_lock_counter = TURN_LOCK_DURATION; }
static int a_lane_lock_actif(VEHICULE *v) { VehiculeState *s = get_state(v); return s ? s->lane_lock_counter > 0 : 0; }
static void decrementer_lane_locks() {
    if (!states_initialized) return;
    for (int i = 0; i < MAX_VOITURES; i++) if (states[i].vehicule && states[i].lane_lock_counter > 0) states[i].lane_lock_counter--;
}
static int obtenir_target(VEHICULE *v) { VehiculeState *s = get_state(v); return s ? s->target_place : -1; }
static void set_target(VEHICULE *v, int place_index) { VehiculeState *s = get_state(v); if (s) s->target_place = place_index; }
static void clear_target(VEHICULE *v) { VehiculeState *s = get_state(v); if (s) s->target_place = -1; }
void marquer_vehicule_en_sortie(VEHICULE *vehicule) { set_target(vehicule, TARGET_SORTIE); }
static int est_fleche_simple(wchar_t c) { return (c == L'←' || c == L'→' || c == L'↑' || c == L'↓'); }
static int est_fleche_circulation(wchar_t c) { return (c == L'←' || c == L'→'); }
static char sens_fleche(wchar_t c) {
    return (c == L'←') ? 'O' : (c == L'→') ? 'E' : (c == L'↑') ? 'N' : (c == L'↓') ? 'S' : '\0';
}
static int compter_passes(PlanParking *plan, int cx, int cy, char dir) {
    if (!plan) return 0;
    int dx = 0, dy = 0;
    obtenir_delta_direction(dir, &dx, &dy);
    if (dx == 0 && dy == 0) return 0;
    int passes = 0, x = cx + dx, y = cy + dy;
    for (int i = 0; i < MAX_SCAN && est_dans_limites(plan, x, y) && est_cellule_roulable(plan, x, y); i++, x += dx, y += dy) passes++;
    return passes;
}
static void compter_fleches_zone(PlanParking *plan, int cx, int cy, int R, int counts[4]) {
    counts[0] = counts[1] = counts[2] = counts[3] = 0;
    if (!plan) return;
    for (int y = cy - R; y <= cy + R; y++)
        for (int x = cx - R; x <= cx + R; x++) {
            if (!est_dans_limites(plan, x, y) || !est_fleche_circulation(plan->plan_statique[y][x])) continue;
            char s = sens_fleche(plan->plan_statique[y][x]);
            if (s == 'N') counts[0]++; else if (s == 'S') counts[1]++; else if (s == 'E') counts[2]++; else if (s == 'O') counts[3]++;
        }
}
static int compter_voisins_roulables(PlanParking *plan, int cx, int cy) {
    return est_cellule_roulable(plan, cx, cy - 1) + est_cellule_roulable(plan, cx, cy + 1) +
           est_cellule_roulable(plan, cx + 1, cy) + est_cellule_roulable(plan, cx - 1, cy);
}
static int lane_lock_bloque_changement(VEHICULE *vehicule, PlanParking *plan, int cx, int cy, char dir) {
    if (!a_lane_lock_actif(vehicule)) return 0;
    int dx = 0, dy = 0;
    obtenir_delta_direction(dir, &dx, &dy);
    int nx = cx + dx, ny = cy + dy;
    return est_dans_limites(plan, nx, ny) && est_cellule_passable(plan->plan_statique[ny][nx]);
}
static char detecter_fleche_proche(PlanParking *plan, int cx, int cy) {
    int fx = -1, fy = -1;
    if (!trouver_cellule_dans_zone(plan, cx, cy, 3, est_fleche_parking, &fx, &fy)) return '\0';
    int ligne = fy - 1;
    for (int i = 0; i < plan->places_totales; i++)
        if (plan->places[i].occupee == 0 && plan->places[i].ligne == ligne)
            return sens_fleche(plan->plan_statique[fy][fx]);
    return '\0';
}
static int doit_recalculer_direction(VEHICULE *vehicule, PlanParking *plan, int fleches_zone[4], int target_x, int target_y) {
    if (!vehicule || !plan) return 1;
    int cx, cy, w, h; CENTRE_VEHICULE(vehicule, cx, cy); obtenir_dimensions_vehicule(vehicule, &w, &h);
    char dir = vehicule->direction;
    if (!est_dans_limites(plan, cx, cy)) return 1;
    if (lane_lock_bloque_changement(vehicule, plan, cx, cy, dir)) return 0;
    if (est_fleche_circulation(plan->plan_statique[cy][cx]) || detecter_fleche_proche(plan, cx, cy)) return 1;
    int fp = (dir == 'N' || dir == 'S') ? fleches_zone[2] + fleches_zone[3] : fleches_zone[0] + fleches_zone[1];
    if (fp > 0) return 1;
    int dx = 0, dy = 0; obtenir_delta_direction(dir, &dx, &dy);
    int nx = vehicule->posx + dx, ny = vehicule->posy + dy;
    if (!peut_deplacer_sur_allee(vehicule, plan, nx, ny)) {
        if (nx < 0 || ny < 0 || nx + w > plan->largeur || ny + h > plan->hauteur) return 1;
        int ncx = nx + w/2, ncy = ny + h/2;
        if (est_dans_limites(plan, ncx, ncy) && est_cellule_passable(plan->plan_statique[ncy][ncx])) return 0;
        return 1;
    }
    if (compter_voisins_roulables(plan, cx, cy) >= 3) return 1;
    if (target_x >= 0 && target_y >= 0) {
        int dcx = target_x - cx, dcy = target_y - cy;
        if ((dir == 'N' && dcy > 5) || (dir == 'S' && dcy < -5) || (dir == 'E' && dcx < -5) || (dir == 'O' && dcx > 5)) return 1;
    }
    return 0;
}
static char forcer_direction_si_fleche_parking(VEHICULE *vehicule, PlanParking *plan, int cx, int cy) {
    if (!vehicule || !plan) return '\0';
    int fx = -1, fy = -1;
    if (!trouver_cellule_dans_zone(plan, cx, cy, 3, est_fleche_parking, &fx, &fy)) return '\0';
    int ligne = fy - 1;
    for (int i = 0; i < plan->places_totales; i++)
        if (plan->places[i].occupee == 0 && plan->places[i].ligne == ligne) {
            activer_lane_lock(vehicule);
            return sens_fleche(plan->plan_statique[fy][fx]);
        }
    return '\0';
}
typedef struct { int y_min, y_max, x_threshold, x_min, x_max; int bonus[4]; } ZoneBonus;
static const ZoneBonus ZONES[] = {
    {20, 26, 85, -1, -1, {40, 0, 0, 0}},
    {20, 26, -1, 86, 999, {0, 0, 0, 50}},
    {17, 19, -1, -1, -1, {0, 0, 30, 60}}
};
static void calculer_scores_avec_bonus(PlanParking *plan, int cx, int cy, int target_x, int target_y, int scores[4], int *meilleur_score, char *dir_meilleure) {
    if (!plan || !scores || !meilleur_score || !dir_meilleure) return;
    int fleches[4];
    compter_fleches_zone(plan, cx, cy, RAYON_SCAN, fleches);
    int passes[4] = {compter_passes(plan, cx, cy, 'N'), compter_passes(plan, cx, cy, 'S'), compter_passes(plan, cx, cy, 'E'), compter_passes(plan, cx, cy, 'O')};
    for (int i = 0; i < 4; i++) scores[i] = 10 * passes[i] + 3 * fleches[i];
    if (target_x >= 0 && target_y >= 0) {
        int dx = target_x - cx, dy = target_y - cy;
        if (dy < -3 && passes[0] > 0) scores[0] += 20;
        if (dy > 3 && passes[1] > 0) scores[1] += 20;
        if (dx > 3 && passes[2] > 0) scores[2] += 20;
        if (dx < -3 && passes[3] > 0) scores[3] += 20;
    }
    for (int z = 0; z < 3; z++) {
        const ZoneBonus *zb = &ZONES[z];
        if (cy < zb->y_min || cy > zb->y_max) continue;
        if (zb->x_threshold > 0 && cx <= zb->x_threshold) { for (int i = 0; i < 4; i++) if (passes[i] > 0) scores[i] += zb->bonus[i]; }
        else if (zb->x_min > 0 && cx >= zb->x_min && cx <= zb->x_max) { for (int i = 0; i < 4; i++) if (passes[i] > 0) scores[i] += zb->bonus[i]; }
        else if (zb->x_threshold < 0 && zb->x_min < 0) { for (int i = 0; i < 4; i++) if (passes[i] > 0) scores[i] += zb->bonus[i]; }
    }
    *meilleur_score = scores[0]; *dir_meilleure = 'N';
    for (int i = 1; i < 4; i++) if (scores[i] > *meilleur_score) { *meilleur_score = scores[i]; *dir_meilleure = "NSEO"[i]; }
}
static int est_changement_lateral(char old, char new) {
    return ((old == 'N' || old == 'S') && (new == 'E' || new == 'O')) || ((old == 'E' || old == 'O') && (new == 'N' || new == 'S'));
}
static int est_uturn(char old, char new) {
    return (old == 'N' && new == 'S') || (old == 'S' && new == 'N') || (old == 'E' && new == 'O') || (old == 'O' && new == 'E');
}
static int est_intersection_ou_fleche(PlanParking *plan, int cx, int cy) {
    int nb = compter_voisins_roulables(plan, cx, cy);
    return nb >= 3 || est_fleche_circulation(plan->plan_statique[cy][cx]);
}
static int a_parking_libre_proche(PlanParking *plan, int cx, int cy, char dir) {
    int fx = -1, fy = -1;
    if (!trouver_cellule_dans_zone(plan, cx, cy, 3, est_fleche_parking, &fx, &fy)) return 0;
    if (sens_fleche(plan->plan_statique[fy][fx]) != dir) return 0;
    int ligne = fy - 1;
    for (int i = 0; i < plan->places_totales; i++) if (plan->places[i].occupee == 0 && plan->places[i].ligne == ligne) return 1;
    return 0;
}
static int dir_est_sure(PlanParking *plan, int cx, int cy, char dir) {
    int dx = 0, dy = 0;
    obtenir_delta_direction(dir, &dx, &dy);
    int nx = cx + dx, ny = cy + dy;
    return est_dans_limites(plan, nx, ny) && est_cellule_passable(plan->plan_statique[ny][nx]);
}
static char valider_direction_lane_keeping(VEHICULE *vehicule, PlanParking *plan, int cx, int cy, char old, char new) {
    if (!vehicule || !plan) return old;
    int old_safe = dir_est_sure(plan, cx, cy, old);
    int new_safe = dir_est_sure(plan, cx, cy, new);
    if (est_changement_lateral(old, new) && old_safe && !est_intersection_ou_fleche(plan, cx, cy) && !a_parking_libre_proche(plan, cx, cy, new)) return old;
    if (est_uturn(old, new) && old_safe) return old;
    return new_safe ? new : old;
}
static char choisir_direction_stable(VEHICULE *vehicule, PlanParking *plan, int target_x, int target_y) {
    if (!vehicule || !plan) return vehicule->direction;
    int cx, cy;
    CENTRE_VEHICULE(vehicule, cx, cy);
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
    char dp = forcer_direction_si_fleche_parking(vehicule, plan, cx, cy);
    if (dp) return dp;
    int scores[4], ms = 0; char dm = 'N';
    calculer_scores_avec_bonus(plan, cx, cy, target_x, target_y, scores, &ms, &dm);
    char old = vehicule->direction;
    char newdir = ms ? dm : old;
    int sa = (old == 'N') ? scores[0] : (old == 'S') ? scores[1] : (old == 'E') ? scores[2] : scores[3];
    if (sa >= ms - 2 && sa > 0) newdir = old;
    return valider_direction_lane_keeping(vehicule, plan, cx, cy, old, newdir);
}
void suivre_fleches(VEHICULE *vehicule, PlanParking *plan) {
    if (!vehicule || !plan || vehicule->etat != '1') return;
    int cx, cy; CENTRE_VEHICULE(vehicule, cx, cy);
    if (!est_dans_limites(plan, cx, cy)) return;
    int md = 999, mi = -1;
    for (int i = 0; i < plan->nb_fleches; i++) {
        int fx = plan->fleches[i].colonne, fy = plan->fleches[i].ligne;
        int d = abs(fx - cx) + abs(fy - cy);
        int ahead = (vehicule->direction == 'N' && fy < cy && fy >= cy - 4) ||
                    (vehicule->direction == 'S' && fy > cy && fy <= cy + 4) ||
                    (vehicule->direction == 'E' && fx > cx && fx <= cx + 4) ||
                    (vehicule->direction == 'O' && fx < cx && fx >= cx - 4);
        if ((d <= 2 || ahead) && d < md) { md = d; mi = i; }
    }
    if (mi >= 0) {
        char de = plan->fleches[mi].direction_entree;
        if (!de || vehicule->direction == de) vehicule->direction = plan->fleches[mi].direction_sortie;
    }
}
void deplacer_vehicule(VEHICULE *vehicule, PlanParking *plan) {
    if (!vehicule || !plan || vehicule->etat != '1') return;
    char old = vehicule->direction;
    suivre_fleches(vehicule, plan);
    if (old != vehicule->direction) orienter_carrosserie(vehicule);
    int dx = 0, dy = 0; obtenir_delta_direction(vehicule->direction, &dx, &dy);
    if (!dx && !dy) return;
    int nx = vehicule->posx + dx * vehicule->vitesse, ny = vehicule->posy + dy * vehicule->vitesse;
    if (peut_deplacer(vehicule, plan, nx, ny)) { vehicule->posx = nx; vehicule->posy = ny; return; }
    char dirs[] = {'N', 'S', 'E', 'O'};
    for (int i = 0; i < 4; i++) {
        if (dirs[i] == vehicule->direction) continue;
        int tx = vehicule->posx, ty = vehicule->posy, tdx = 0, tdy = 0;
        obtenir_delta_direction(dirs[i], &tdx, &tdy);
        tx += tdx * vehicule->vitesse; ty += tdy * vehicule->vitesse;
        if (peut_deplacer(vehicule, plan, tx, ty)) {
            vehicule->direction = dirs[i]; orienter_carrosserie(vehicule);
            vehicule->posx = tx; vehicule->posy = ty; return;
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
    int centre_x, centre_y;
    CENTRE_VEHICULE(vehicule, centre_x, centre_y);
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    if (!est_dans_limites(plan, centre_x, centre_y)) return 0;
    int fleche_x = -1, fleche_y = -1;
    if (!trouver_cellule_dans_zone(plan, centre_x, centre_y, 3, est_fleche_parking, &fleche_x, &fleche_y)) {
        return 0;
    }
    wchar_t fleche_trouvee = plan->plan_statique[fleche_y][fleche_x];
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
    int centre_x, centre_y;
    CENTRE_VEHICULE(vehicule, centre_x, centre_y);
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
    int centre_x, centre_y;
    CENTRE_VEHICULE(vehicule, centre_x, centre_y);
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
static void corriger_alignement_fleche(VEHICULE *vehicule, PlanParking *plan) {
    if (!vehicule || !plan) return;
    int centre_x, centre_y;
    CENTRE_VEHICULE(vehicule, centre_x, centre_y);
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    if (est_dans_limites(plan, centre_x, centre_y)) {
        wchar_t c_centre = plan->plan_statique[centre_y][centre_x];
        if (c_centre == L'←' || c_centre == L'→' || c_centre == L'↑' || c_centre == L'↓' ||
            c_centre == L' ' || c_centre == L'.') return;
    }
    int best_x = -1, best_y = -1, best_dist = 999;
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            if (dx == 0 && dy == 0) continue;
            int check_x = centre_x + dx, check_y = centre_y + dy;
            if (!est_dans_limites(plan, check_x, check_y)) continue;
            wchar_t c = plan->plan_statique[check_y][check_x];
            int compat = ((vehicule->direction == 'O' && c == L'←') || (vehicule->direction == 'E' && c == L'→') ||
                         (vehicule->direction == 'N' && c == L'↑') || (vehicule->direction == 'S' && c == L'↓')) ? 2 :
                        ((c == L' ' || c == L'.') ? 1 : 0);
            if (compat > 0) {
                int dist = abs(dx) + abs(dy);
                if ((vehicule->direction == 'O' || vehicule->direction == 'E') ? dy == 0 : dx == 0) dist -= 5;
                if (compat == 2) dist -= 3;
                if (dist < best_dist) { best_dist = dist; best_x = check_x; best_y = check_y; }
            }
        }
    }
    if (best_x < 0 || best_y < 0 || best_dist > 2) return;
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
    int centre_x, centre_y;
    CENTRE_VEHICULE(vehicule, centre_x, centre_y);
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
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
