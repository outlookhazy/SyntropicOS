/**
 * @file test_menu.c
 * @brief Unity tests for syn_menu.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"

#include "syntropic/ui/syn_menu.h"

static int mnu_render_n = 0;
static void mnu_render(const SYN_Menu *m, void *c) { (void)m; (void)c; mnu_render_n++; }
static int mnu_cb_n = 0;
static void mnu_cb(void *c) { (void)c; mnu_cb_n++; }

static void test_menu(void)
{
    static bool mnu_led = false;
    static int32_t mnu_bright = 50;
    static const SYN_MenuItem s_items[] = {
        SYN_MENU_TOGGLE("LED", &mnu_led),
        SYN_MENU_VALUE("Bright", &mnu_bright, 0, 100, 10),
        SYN_MENU_CALLBACK("Save", mnu_cb, NULL),
    };
    static const SYN_MenuItem r_items[] = {
        SYN_MENU_SUBMENU("Settings", s_items),
        SYN_MENU_CALLBACK("Reboot", mnu_cb, NULL),
    };
    SYN_MENU_ROOT(root, r_items);
    SYN_Menu menu;
    mnu_render_n = 0; mnu_cb_n = 0; mnu_led = false; mnu_bright = 50;
    syn_menu_init(&menu, &root, mnu_render, NULL);
    TEST_ASSERT_TRUE(menu.selected == 0);
    syn_menu_down(&menu);
    TEST_ASSERT_TRUE(menu.selected == 1);
    syn_menu_down(&menu);
    TEST_ASSERT_TRUE(menu.selected == 0);
    syn_menu_enter(&menu);
    TEST_ASSERT_TRUE(menu.depth == 1);
    TEST_ASSERT_TRUE(mnu_led == false);
    syn_menu_enter(&menu);
    TEST_ASSERT_TRUE(mnu_led == true);
    syn_menu_enter(&menu);
    TEST_ASSERT_TRUE(mnu_led == false);
    syn_menu_down(&menu);
    syn_menu_enter(&menu);
    TEST_ASSERT_TRUE(menu.editing);
    syn_menu_up(&menu);
    TEST_ASSERT_EQUAL_INT(60, mnu_bright);
    for (int i = 0; i < 5; i++) syn_menu_up(&menu);
    TEST_ASSERT_EQUAL_INT(100, mnu_bright);
    syn_menu_down(&menu);
    TEST_ASSERT_EQUAL_INT(90, mnu_bright);
    syn_menu_enter(&menu);
    TEST_ASSERT_TRUE(!menu.editing);
    syn_menu_down(&menu);
    mnu_cb_n = 0;
    syn_menu_enter(&menu);
    TEST_ASSERT_EQUAL_INT(1, mnu_cb_n);
    syn_menu_back(&menu);
    TEST_ASSERT_TRUE(menu.depth == 0);
    TEST_ASSERT_TRUE(mnu_render_n > 5);
}

void run_menu_tests(void)
{
    RUN_TEST(test_menu);
}
