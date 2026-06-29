/* ── Ramp test ──────────────────────────────────────────────────────────── */

#include "syntropic/util/syn_ramp.h"

static void test_ramp(void)
{
    printf("Test: Ramp generator\n");
    SYN_Ramp r;
    syn_ramp_init(&r, 0);
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == 0);
    TEST_ASSERT_TRUE(syn_ramp_done(&r));
    syn_ramp_set_target(&r, 100, 10);
    TEST_ASSERT_TRUE(!syn_ramp_done(&r));
    int32_t val = 0;
    for (int i = 0; i < 10; i++) val = syn_ramp_update(&r);
    TEST_ASSERT_TRUE(val == 100);
    TEST_ASSERT_TRUE(syn_ramp_done(&r));
    syn_ramp_set_target(&r, 50, 25);
    syn_ramp_update(&r); syn_ramp_update(&r);
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == 50);
    syn_ramp_jump(&r, 999);
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == 999);
    syn_ramp_set_target(&r, 1002, 10);
    syn_ramp_update(&r);
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == 1002);
    syn_ramp_init(&r, 0);
    syn_ramp_set_target_scurve(&r, 1000, 100, 10);
    int iters = 0;
    while (!syn_ramp_done(&r) && iters < 500) { syn_ramp_update(&r); iters++; }
    TEST_ASSERT_TRUE(syn_ramp_done(&r));
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == 1000);
    syn_ramp_init(&r, 100);
    syn_ramp_set_target(&r, -100, 50);
    for (int i = 0; i < 4; i++) syn_ramp_update(&r);
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == -100);
}

/* ── Pack test ──────────────────────────────────────────────────────────── */

#include "syntropic/util/syn_pack.h"

static void test_pack(void)
{
    printf("Test: Binary packer\n");
    uint8_t buf[32]; size_t pos = 0;
    syn_pack_u8(buf, &pos, 0x42);
    syn_pack_u16(buf, &pos, 0x1234);
    syn_pack_i32(buf, &pos, -12345);
    TEST_ASSERT_TRUE(pos == 7);
    pos = 0;
    TEST_ASSERT(syn_unpack_u8(buf, &pos) == 0x42, "pack: u8");
    TEST_ASSERT(syn_unpack_u16(buf, &pos) == 0x1234, "pack: u16");
    TEST_ASSERT(syn_unpack_i32(buf, &pos) == -12345, "pack: i32");
    pos = 0;
    syn_pack_u16_le(buf, &pos, 0xABCD);
    syn_pack_u32_le(buf, &pos, 0x12345678);
    pos = 0;
    TEST_ASSERT(syn_unpack_u16_le(buf, &pos) == 0xABCD, "pack: le u16");
    TEST_ASSERT(syn_unpack_u32_le(buf, &pos) == 0x12345678, "pack: le u32");
    uint8_t src[] = {1,2,3,4}; uint8_t dst[4] = {0};
    pos = 0;
    syn_pack_bytes(buf, &pos, src, 4);
    pos = 0;
    syn_unpack_bytes(buf, &pos, dst, 4);
    TEST_ASSERT_TRUE(dst[0]==1 && dst[3]==4);
    pos = 0;
    syn_pack_i8(buf, &pos, -42);
    syn_pack_i16(buf, &pos, -1000);
    pos = 0;
    TEST_ASSERT(syn_unpack_i8(buf, &pos) == -42, "pack: i8");
    TEST_ASSERT(syn_unpack_i16(buf, &pos) == -1000, "pack: i16");
    pos = 0;
    syn_pack_u16(buf, &pos, 0xBEEF);
    TEST_ASSERT(syn_peek_u16(buf, 0) == 0xBEEF, "pack: peek u16");
}

/* ── Power test ─────────────────────────────────────────────────────────── */

#include "syntropic/system/syn_power.h"

static int power_bo_count = 0;
static int power_re_count = 0;
static void pwr_bo_cb(SYN_Power *p, void *c) { (void)p; (void)c; power_bo_count++; }
static void pwr_re_cb(SYN_Power *p, void *c) { (void)p; (void)c; power_re_count++; }

static void test_power(void)
{
    printf("Test: Power monitor\n");
    static SYN_ADC pwr_adc;
    SYN_ADC_Config pwr_adc_cfg = { .channel = 0, .oversample = 1, .cal_scale = 1 };
    mock_adc_value = 4095; /* ~3300mV */
    syn_adc_init(&pwr_adc, &pwr_adc_cfg);
    power_bo_count = 0; power_re_count = 0;
    SYN_Power pwr;
    SYN_Power_Config pcfg = {
        .adc = &pwr_adc, .brownout_mv = 3000, .restore_mv = 3200,
        .on_brownout = pwr_bo_cb, .on_restore = pwr_re_cb,
    };
    syn_power_init(&pwr, &pcfg);
    syn_power_update(&pwr);
    TEST_ASSERT_TRUE(!syn_power_is_brownout(&pwr));
    mock_adc_value = 3600; /* ~2900mV */
    syn_power_update(&pwr);
    TEST_ASSERT_TRUE(syn_power_is_brownout(&pwr));
    TEST_ASSERT_TRUE(power_bo_count == 1);
    syn_power_update(&pwr);
    TEST_ASSERT_TRUE(power_bo_count == 1);
    mock_adc_value = 4095; /* ~3300mV */
    syn_power_update(&pwr);
    TEST_ASSERT_TRUE(!syn_power_is_brownout(&pwr));
    TEST_ASSERT_TRUE(power_re_count == 1);
    SYN_Signal pwr_stats;
    int32_t pwr_samp[8];
    syn_signal_init(&pwr_stats, pwr_samp, 8);
    syn_power_set_stats(&pwr, &pwr_stats);
    syn_power_update(&pwr);
    TEST_ASSERT_TRUE(syn_signal_count(&pwr_stats) == 1);
}

/* ── Actuator test ──────────────────────────────────────────────────────── */

#include "syntropic/motor/syn_actuator.h"

static int32_t act_mock_pos = 500;
static int32_t act_read(void *c) { (void)c; return act_mock_pos; }

static void test_actuator(void)
{
    printf("Test: Linear actuator\n");
    static SYN_DCMotor act_motor;
    syn_dc_motor_init(&act_motor, 10, 11, SYN_DC_MODE_PWM_DIR);
    act_mock_pos = 500;
    SYN_Actuator act;
    SYN_Actuator_Config acfg = {
        .dc_motor = &act_motor, .read_pos = act_read, .read_ctx = NULL,
        .stroke_min = 100, .stroke_max = 900, .update_hz = 50,
        .pid_kp = 100, .pid_scale = 4,
    };
    syn_actuator_init(&act, &acfg);
    TEST_ASSERT_TRUE(syn_actuator_position(&act) == 500);
    syn_actuator_set_position(&act, 750);
    act_mock_pos = 700;
    syn_actuator_update(&act);
    TEST_ASSERT_TRUE(syn_actuator_position(&act) == 750);
    syn_actuator_stop(&act);
    TEST_ASSERT_TRUE(act.ctrl.state == SYN_MCTRL_STOPPED);
    syn_actuator_set_position(&act, 0);
    act_mock_pos = 100;
    syn_actuator_update(&act);
    TEST_ASSERT_TRUE(syn_actuator_position(&act) == 0);
    syn_actuator_set_position(&act, 1000);
    act_mock_pos = 900;
    syn_actuator_update(&act);
    TEST_ASSERT_TRUE(syn_actuator_position(&act) == 1000);
    syn_actuator_set_position(&act, 2000);
    TEST_ASSERT_TRUE(act.target_pct == 1000);
    syn_actuator_set_position(&act, -500);
    TEST_ASSERT_TRUE(act.target_pct == 0);
}

/* ── Canvas test ────────────────────────────────────────────────────────── */

#include "syntropic/display/syn_canvas.h"

static int cvs_flush_n = 0;
static size_t cvs_flush_sz = 0;
static void cvs_flush(const uint8_t *b, size_t l, void *c) { (void)b; (void)c; cvs_flush_n++; cvs_flush_sz = l; }

static void test_canvas(void)
{
    printf("Test: Canvas display\n");
    uint8_t fb[32 * 16 / 8];
    SYN_Canvas c;
    memset(fb, 0, sizeof(fb));
    cvs_flush_n = 0;
    syn_canvas_init(&c, fb, 32, 16, 1, cvs_flush, NULL);
    TEST_ASSERT_TRUE(c.width == 32 && c.height == 16);
    syn_canvas_fill(&c, 1);
    TEST_ASSERT_TRUE(fb[0] == 0xFF);
    syn_canvas_clear(&c);
    TEST_ASSERT_TRUE(fb[0] == 0x00);
    syn_canvas_pixel(&c, 0, 0, 1);
    TEST_ASSERT_TRUE((fb[0] & 0x01) != 0);
    syn_canvas_pixel(&c, -1, 0, 1);
    syn_canvas_pixel(&c, 100, 100, 1);
    syn_canvas_clear(&c);
    syn_canvas_line(&c, 0, 0, 31, 0, 1);
    int lok = 1;
    for (int x = 0; x < 32; x++) { if (!(fb[x] & 0x01)) { lok = 0; break; } }
    TEST_ASSERT_TRUE(lok);
    syn_canvas_clear(&c);
    syn_canvas_rect_fill(&c, 0, 0, 2, 2, 1);
    TEST_ASSERT_TRUE((fb[0]&0x01) && (fb[1]&0x01) && (fb[0]&0x02) && (fb[1]&0x02));
    syn_canvas_clear(&c);
    syn_canvas_circle(&c, 16, 8, 5, 1);
    int cpx = 0;
    for (size_t i = 0; i < sizeof(fb); i++) for (int b = 0; b < 8; b++) if (fb[i] & (1<<b)) cpx++;
    TEST_ASSERT_TRUE(cpx > 0);
    syn_canvas_clear(&c);
    syn_canvas_text(&c, 0, 0, "A", 1);
    TEST_ASSERT_TRUE(fb[0] != 0);
    TEST_ASSERT_TRUE(syn_canvas_text_width(&c) == 11, "cvs: text width");
    syn_canvas_flush(&c);
    TEST_ASSERT_TRUE(cvs_flush_n == 1);
    TEST_ASSERT_TRUE(cvs_flush_sz == sizeof(fb));
    uint8_t fbc[8*8*2];
    SYN_Canvas cc;
    syn_canvas_init(&cc, fbc, 8, 8, 16, NULL, NULL);
    syn_canvas_clear(&cc);
    syn_canvas_pixel(&cc, 0, 0, SYN_COLOR_RED);
    TEST_ASSERT_TRUE(fbc[0]==0xF8 && fbc[1]==0x00);
    TEST_ASSERT(syn_rgb565(255,0,0) == SYN_COLOR_RED, "cvs: rgb565 helper");
}

/* ── Menu test ──────────────────────────────────────────────────────────── */

#include "syntropic/ui/syn_menu.h"

static int mnu_render_n = 0;
static void mnu_render(const SYN_Menu *m, void *c) { (void)m; (void)c; mnu_render_n++; }
static int mnu_cb_n = 0;
static void mnu_cb(void *c) { (void)c; mnu_cb_n++; }

static void test_menu(void)
{
    printf("Test: Menu system\n");
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
    TEST_ASSERT_TRUE(mnu_bright == 60);
    for (int i = 0; i < 5; i++) syn_menu_up(&menu);
    TEST_ASSERT_TRUE(mnu_bright == 100);
    syn_menu_down(&menu);
    TEST_ASSERT_TRUE(mnu_bright == 90);
    syn_menu_enter(&menu);
    TEST_ASSERT_TRUE(!menu.editing);
    syn_menu_down(&menu);
    mnu_cb_n = 0;
    syn_menu_enter(&menu);
    TEST_ASSERT_TRUE(mnu_cb_n == 1);
    syn_menu_back(&menu);
    TEST_ASSERT_TRUE(menu.depth == 0);
    TEST_ASSERT_TRUE(mnu_render_n > 5);
}

/* ── CAN test ───────────────────────────────────────────────────────────── */

#include "syntropic/drivers/syn_can.h"

static SYN_CAN_Frame mock_can_rx;
static bool mock_can_rx_avail = false;
static bool mock_can_tx_ok = true;

bool syn_port_can_init(uint8_t p, uint32_t br) { (void)p; (void)br; return true; }
bool syn_port_can_send(uint8_t p, uint32_t id, bool ext, const uint8_t *d, uint8_t dl)
{ (void)p; (void)id; (void)ext; (void)d; (void)dl; return mock_can_tx_ok; }
bool syn_port_can_receive(uint8_t p, uint32_t *id, bool *ext, uint8_t *d, uint8_t *dl)
{
    (void)p;
    if (!mock_can_rx_avail) return false;
    *id = mock_can_rx.id; *ext = mock_can_rx.extended; *dl = mock_can_rx.dlc;
    memcpy(d, mock_can_rx.data, mock_can_rx.dlc);
    mock_can_rx_avail = false;
    return true;
}
void syn_port_can_set_filter(uint8_t p, uint32_t id, uint32_t m) { (void)p; (void)id; (void)m; }

static int can_rx_n = 0;
static void can_rxcb(const SYN_CAN_Frame *f, void *c) { (void)f; (void)c; can_rx_n++; }

static void test_can(void)
{
    printf("Test: CAN bus driver\n");
    SYN_CAN can;
    TEST_ASSERT(syn_can_init(&can, 0, 500000) == SYN_OK, "can: init");
    mock_can_tx_ok = true;
    SYN_CAN_Frame tx = { .id = 0x100, .dlc = 2 };
    tx.data[0] = 0x42;
    TEST_ASSERT(syn_can_send(&can, &tx), "can: send ok");
    TEST_ASSERT_TRUE(can.tx_count == 1);
    mock_can_tx_ok = false;
    TEST_ASSERT(!syn_can_send(&can, &tx), "can: send fail");
    TEST_ASSERT_TRUE(can.err_count == 1);
    can_rx_n = 0;
    syn_can_on_receive(&can, can_rxcb, NULL);
    mock_can_rx.id = 0x200; mock_can_rx.dlc = 1; mock_can_rx.data[0] = 0xAA;
    mock_can_rx_avail = true;
    syn_can_poll(&can);
    TEST_ASSERT_TRUE(can_rx_n == 1);
    TEST_ASSERT_TRUE(can.rx_count == 1);
    syn_can_poll(&can);
    TEST_ASSERT_TRUE(can_rx_n == 1);
    syn_can_set_filter(&can, 0x100, 0x7FF);
}

/* ── Router test ────────────────────────────────────────────────────────── */

#include "syntropic/net/syn_router.h"

static uint8_t rt_tx_buf[128]; static size_t rt_tx_len = 0;
static uint8_t rt_rx_buf[128]; static size_t rt_rx_len = 0;
static bool rt_rx_rdy = false;

static bool rt_send(const uint8_t *d, size_t l, void *c) { (void)c; memcpy(rt_tx_buf,d,l); rt_tx_len=l; return true; }
static bool rt_recv(uint8_t *d, size_t mx, size_t *ol, void *c) {
    (void)c; if (!rt_rx_rdy) return false;
    if (rt_rx_len > mx) return false;
    memcpy(d, rt_rx_buf, rt_rx_len); *ol = rt_rx_len; rt_rx_rdy = false; return true;
}

static int rt_msg_n = 0; static uint8_t rt_last_type = 0;
static void rt_handler(const SYN_Packet *p, void *c) { (void)c; rt_msg_n++; rt_last_type = p->type; }

static void test_router(void)
{
    printf("Test: Packet router\n");
    SYN_Transport tr = { .send = rt_send, .recv = rt_recv, .ctx = NULL };
    SYN_RouterHandler rh[8]; SYN_Router rtr;
    syn_router_init(&rtr, 0x01, &tr, rh, 8);
    TEST_ASSERT_TRUE(rtr.node_id == 0x01);
    TEST_ASSERT(syn_router_register(&rtr, 0x10, rt_handler, NULL), "rtr: register");
    uint8_t pl[] = {0xAA, 0xBB};
    TEST_ASSERT(syn_router_send(&rtr, 0x02, 0x10, pl, 2, false), "rtr: send");
    TEST_ASSERT_TRUE(rt_tx_len == SYN_ROUTER_HEADER_SIZE + 2);
    TEST_ASSERT_TRUE(rt_tx_buf[0] == 0x01);
    TEST_ASSERT_TRUE(rt_tx_buf[1] == 0x02);
    TEST_ASSERT_TRUE(rtr.tx_count == 1);
    rt_msg_n = 0;
    rt_rx_buf[0]=0x02; rt_rx_buf[1]=0x01; rt_rx_buf[2]=0x10;
    rt_rx_buf[3]=0; rt_rx_buf[4]=0; rt_rx_buf[5]=1; rt_rx_buf[6]=0xCC;
    rt_rx_len = 7; rt_rx_rdy = true;
    syn_router_poll(&rtr);
    TEST_ASSERT_TRUE(rt_msg_n == 1);
    TEST_ASSERT_TRUE(rt_last_type == 0x10);
    rt_rx_buf[1] = 0x05; rt_rx_len = 7; rt_rx_rdy = true;
    syn_router_poll(&rtr);
    TEST_ASSERT_TRUE(rt_msg_n == 1);
    rt_rx_buf[1] = 0xFF; rt_rx_len = 7; rt_rx_rdy = true;
    syn_router_poll(&rtr);
    TEST_ASSERT_TRUE(rt_msg_n == 2);
    rt_rx_buf[1]=0x01; rt_rx_buf[2]=0x99; rt_rx_len=7; rt_rx_rdy=true;
    syn_router_poll(&rtr);
    TEST_ASSERT_TRUE(rtr.drop_count == 1);
    SYN_PendingAck pend[4];
    syn_router_enable_ack(&rtr, pend, 4, 500, 3);
    TEST_ASSERT(syn_router_send(&rtr, 0x02, 0x10, pl, 2, true), "rtr: reliable send");
    TEST_ASSERT_TRUE((rt_tx_buf[4] & SYN_PKT_FLAG_ACK_REQ) != 0);
    uint8_t aseq = rt_tx_buf[3];
    rt_rx_buf[0]=0x02; rt_rx_buf[1]=0x01; rt_rx_buf[2]=SYN_MSG_ACK;
    rt_rx_buf[3]=aseq; rt_rx_buf[4]=SYN_PKT_FLAG_IS_ACK; rt_rx_buf[5]=0;
    rt_rx_len=6; rt_rx_rdy=true;
    syn_router_poll(&rtr);
    TEST_ASSERT_TRUE(!pend[0].active);
}

/* ── Heartbeat test ─────────────────────────────────────────────────────── */

#include "syntropic/net/syn_heartbeat.h"

static int hb_lost_n = 0, hb_found_n = 0;
static void hb_lost(uint8_t n, void *c) { (void)n; (void)c; hb_lost_n++; }
static void hb_found(uint8_t n, void *c) { (void)n; (void)c; hb_found_n++; }

static void test_heartbeat(void)
{
    printf("Test: Heartbeat monitor\n");
    SYN_Transport hbt = { .send = rt_send, .recv = rt_recv, .ctx = NULL };
    SYN_RouterHandler hbh[8]; SYN_Router hbr;
    syn_router_init(&hbr, 0x01, &hbt, hbh, 8);
    SYN_HB_Peer peers[4]; SYN_Heartbeat hb;
    syn_heartbeat_init(&hb, &hbr, peers, 4, 1000, 3000);
    hb_lost_n = 0; hb_found_n = 0;
    syn_heartbeat_on_peer_lost(&hb, hb_lost, NULL);
    syn_heartbeat_on_peer_found(&hb, hb_found, NULL);
    TEST_ASSERT(syn_heartbeat_add_peer(&hb, 0x02), "hb: add peer");
    TEST_ASSERT(syn_heartbeat_add_peer(&hb, 0x03), "hb: add peer2");
    TEST_ASSERT(syn_heartbeat_peer_alive(&hb, 0x02), "hb: alive init");
    TEST_ASSERT(syn_heartbeat_add_peer(&hb, 0x02), "hb: dup ok");
    mock_tick_ms += 1100;
    rt_rx_rdy = false;
    syn_heartbeat_update(&hb);
    TEST_ASSERT_TRUE(rt_tx_len > 0);
    mock_tick_ms += 3100;
    rt_rx_rdy = false;
    syn_heartbeat_update(&hb);
    TEST_ASSERT(!syn_heartbeat_peer_alive(&hb, 0x02), "hb: peer timeout");
    TEST_ASSERT_TRUE(hb_lost_n >= 1);
    rt_rx_buf[0]=0x02; rt_rx_buf[1]=0x01; rt_rx_buf[2]=SYN_MSG_HEARTBEAT;
    rt_rx_buf[3]=0; rt_rx_buf[4]=0; rt_rx_buf[5]=0;
    rt_rx_len=6; rt_rx_rdy=true;
    syn_router_poll(&hbr);
    TEST_ASSERT(syn_heartbeat_peer_alive(&hb, 0x02), "hb: peer revived");
    TEST_ASSERT_TRUE(hb_found_n >= 1);
}
