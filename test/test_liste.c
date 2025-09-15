#include "unity.h"
#include "liste_car.h"

void setUp(void) {}
void tearDown(void) {}

void test_nv_liste_est_vide()
{
    l_car *liste = nv_liste_car();
    TEST_ASSERT_NOT_NULL(liste);
    TEST_ASSERT_EQUAL(1, est_vide_liste_car(liste));
    detruire_liste_car(&liste);
}

void test_ajouter_tete()
{
    l_car *l = nv_liste_car();
    char *caro[4] = {
        "######",
        "#    #",
        "#    #",
        "######"};
    VEHICULE *v = nv_vehicule('N', 0, 0, 50, 'g', 'v', caro, 1, '1', 0);

    ajouter_tete_liste_car(v, l);

    TEST_ASSERT_EQUAL(0, est_vide_liste_car(l));
    TEST_ASSERT_EQUAL_PTR(v, l->premier);
    TEST_ASSERT_EQUAL_PTR(v, l->dernier);

    detruire_liste_car(&l);
}

void test_ajouter_queue()
{
    l_car *l = nv_liste_car();
    char *caro[4] = {
        "######",
        "#    #",
        "#    #",
        "######"};
    VEHICULE *v1 = nv_vehicule('N', 0, 0, 50, 'g', 'v', caro, 1, '1', 0);
    VEHICULE *v2 = nv_vehicule('S', 1, 1, 30, 'd', 'v', caro, 2, '1', 0);

    ajouter_queue_liste_car(v1, l);
    ajouter_queue_liste_car(v2, l);

    TEST_ASSERT_EQUAL_PTR(v1, l->premier);
    TEST_ASSERT_EQUAL_PTR(v2, l->dernier);
    TEST_ASSERT_EQUAL_PTR(v2, v1->NXT);

    detruire_liste_car(&l);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_nv_liste_est_vide);
    RUN_TEST(test_ajouter_tete);
    RUN_TEST(test_ajouter_queue);

    return UNITY_END();
}