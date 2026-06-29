/**
 * @file test_cli.c
 * @brief Unity tests for syn_cli.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/cli/syn_cli.h"

static char cli_output_buf[1024];
static size_t cli_output_pos = 0;

static void cli_test_putchar(char ch)
{
    if (cli_output_pos < sizeof(cli_output_buf) - 1) {
        cli_output_buf[cli_output_pos++] = ch;
        cli_output_buf[cli_output_pos] = '\0';
    }
}

static int led_handler_called = 0;
static int led_handler_argc = 0;
static char led_handler_arg1[32];

static int cmd_led(int argc, char *argv[])
{
    led_handler_called = 1;
    led_handler_argc = argc;
    if (argc > 1) {
        strncpy(led_handler_arg1, argv[1], sizeof(led_handler_arg1) - 1);
        led_handler_arg1[sizeof(led_handler_arg1) - 1] = '\0';
    }
    return 0;
}

static int cmd_status(int argc, char *argv[])
{
    (void)argc; (void)argv;
    return 0;
}

static const SYN_CLI_Command test_commands[] = {
    { "led",    "led <on|off>  - Control LED",   cmd_led    },
    { "status", "status        - Show status",   cmd_status },
};

static void test_cli(void)
{

    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 2, cli_test_putchar, "> ");

    led_handler_called = 0;
    led_handler_argc = 0;
    led_handler_arg1[0] = '\0';
    syn_cli_process_line(&cli, "led on");
    TEST_ASSERT_EQUAL_INT(1, led_handler_called);
    TEST_ASSERT_EQUAL_INT(2, led_handler_argc);
    TEST_ASSERT_EQUAL_STRING("on", led_handler_arg1);

    cli_output_pos = 0;
    syn_cli_process_line(&cli, "bogus");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Unknown command"));

    cli_output_pos = 0;
    syn_cli_process_line(&cli, "help");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Available commands"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "led"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "status"));

    led_handler_called = 0;
    cli_output_pos = 0;
    syn_cli_set_echo(&cli, true);
    syn_cli_print_prompt(&cli);
    {
        const char *input = "led off\r";
        while (*input) {
            syn_cli_process_char(&cli, *input++);
        }
    }
    TEST_ASSERT_EQUAL_INT(1, led_handler_called);
    TEST_ASSERT_EQUAL_STRING("off", led_handler_arg1);

    led_handler_called = 0;
    led_handler_arg1[0] = '\0';
    {
        const char *bs_input = "lex\bd off\r";
        while (*bs_input) {
            syn_cli_process_char(&cli, *bs_input++);
        }
    }
    TEST_ASSERT_EQUAL_INT(1, led_handler_called);
    TEST_ASSERT_EQUAL_STRING("off", led_handler_arg1);

    led_handler_called = 0;
    syn_cli_process_char(&cli, '\r');
    TEST_ASSERT_EQUAL_INT(0, led_handler_called);

    led_handler_called = 0;
    syn_cli_process_char(&cli, 'l');
    syn_cli_process_char(&cli, 'e');
    syn_cli_process_char(&cli, 0x03);
    syn_cli_process_char(&cli, '\r');
    TEST_ASSERT_EQUAL_INT(0, led_handler_called);

    led_handler_called = 0;
    led_handler_arg1[0] = '\0';
    syn_cli_process_line(&cli, "led \"hello world\"");
    TEST_ASSERT_EQUAL_INT(1, led_handler_called);
    TEST_ASSERT_EQUAL_STRING("hello world", led_handler_arg1);

    cli_output_pos = 0;
    syn_cli_printf(&cli, "value=%d\r\n", 99);
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "value=99"));
}

void run_cli_tests(void)
{
    RUN_TEST(test_cli);
}
