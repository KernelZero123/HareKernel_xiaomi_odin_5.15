// SPDX-License-Identifier: GPL-2.0
/*
 * Novatek NT36672E based CSOT (Tianma/CSOT "k8") panel for Xiaomi Mi MIX 4 (odin)
 *
 * Panel: xiaomi 42 02 0a cmd mode dsc dsi panel (1080x2400, DSC 8bpc)
 * Derived from the vendor dtsi dsi-panel-k8-42-02-0a-dsc-cmd.dtsi
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

#include <video/mipi_display.h>

struct k8_cmd {
	u8 type;
	u8 len;
	u8 data[32];
};

static const struct k8_cmd k8_on_cmds[] = {
	{ .type = 0x39, .len = 2, .data = { 0x99, 0x10 } },
	{ .type = 0x39, .len = 2, .data = { 0x90, 0x01 } },
	{ .type = 0x39, .len = 19, .data = { 0x91, 0xab, 0x28, 0x00, 0x0c, 0xc2, 0x00, 0x03, 0x1c, 0x01, 0x7e, 0x00, 0x0f, 0x08, 0xbb, 0x04, 0x3d, 0x10, 0xf0 } },
	{ .type = 0x39, .len = 2, .data = { 0x03, 0x01 } },
	{ .type = 0x39, .len = 2, .data = { 0x2c, 0x00 } },
	{ .type = 0x39, .len = 5, .data = { 0x51, 0x00, 0x00, 0x00, 0x00 } },
	{ .type = 0x39, .len = 2, .data = { 0x35, 0x00 } },
	{ .type = 0x39, .len = 5, .data = { 0x2a, 0x00, 0x00, 0x04, 0x37 } },
	{ .type = 0x39, .len = 5, .data = { 0x2b, 0x00, 0x00, 0x09, 0x5f } },
	{ .type = 0x39, .len = 2, .data = { 0x53, 0x20 } },
	{ .type = 0x39, .len = 6, .data = { 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x02 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x03 } },
	{ .type = 0x39, .len = 2, .data = { 0xbf, 0x02 } },
	{ .type = 0x39, .len = 2, .data = { 0x2f, 0x01 } },
	{ .type = 0x39, .len = 6, .data = { 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x01 } },
	{ .type = 0x39, .len = 8, .data = { 0xc3, 0x9b, 0x01, 0x92, 0xd0, 0x20, 0x01, 0x00 } },
	{ .type = 0x39, .len = 3, .data = { 0xcd, 0x05, 0x61 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x01 } },
	{ .type = 0x39, .len = 4, .data = { 0xd2, 0x00, 0x00, 0x11 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x06 } },
	{ .type = 0x39, .len = 2, .data = { 0xd2, 0x05 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x0f } },
	{ .type = 0x39, .len = 2, .data = { 0xd2, 0x00 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x09 } },
	{ .type = 0x39, .len = 3, .data = { 0xd2, 0x00, 0x00 } },
	{ .type = 0x39, .len = 2, .data = { 0xce, 0x00 } },
	{ .type = 0x39, .len = 5, .data = { 0xff, 0xaa, 0x55, 0xa5, 0x80 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x61 } },
	{ .type = 0x39, .len = 2, .data = { 0xf3, 0x80 } },
	{ .type = 0x39, .len = 6, .data = { 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x00 } },
	{ .type = 0x39, .len = 2, .data = { 0xc0, 0x4e } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x05 } },
	{ .type = 0x39, .len = 2, .data = { 0xbe, 0x88 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x08 } },
	{ .type = 0x39, .len = 2, .data = { 0xb5, 0x32 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x0b } },
	{ .type = 0x39, .len = 4, .data = { 0xb5, 0x33, 0x23, 0x2b } },
	{ .type = 0x39, .len = 6, .data = { 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x04 } },
	{ .type = 0x39, .len = 2, .data = { 0xcb, 0x06 } },
	{ .type = 0x39, .len = 6, .data = { 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x00 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x29 } },
	{ .type = 0x39, .len = 2, .data = { 0xd9, 0x00 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x01 } },
	{ .type = 0x39, .len = 2, .data = { 0xd9, 0x80 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x07 } },
	{ .type = 0x39, .len = 3, .data = { 0xb2, 0x07, 0xff } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x17 } },
	{ .type = 0x39, .len = 3, .data = { 0xb2, 0x0f, 0xff } },
	{ .type = 0x39, .len = 6, .data = { 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x07 } },
	{ .type = 0x39, .len = 2, .data = { 0xc0, 0x05 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x00 } },
	{ .type = 0x39, .len = 6, .data = { 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x05 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x06 } },
	{ .type = 0x39, .len = 3, .data = { 0xec, 0x00, 0xff } },
	{ .type = 0x39, .len = 6, .data = { 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x00 } },
	{ .type = 0x39, .len = 2, .data = { 0xc5, 0x10 } },
	{ .type = 0x39, .len = 6, .data = { 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x00 } },
	{ .type = 0x39, .len = 3, .data = { 0xc0, 0x4e, 0xf3 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x02 } },
	{ .type = 0x39, .len = 2, .data = { 0xc0, 0x21 } },
	{ .type = 0x39, .len = 6, .data = { 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x04 } },
	{ .type = 0x39, .len = 2, .data = { 0xc2, 0x14 } },
	{ .type = 0x39, .len = 2, .data = { 0xb1, 0x03 } },
	{ .type = 0x39, .len = 2, .data = { 0xb2, 0x00 } },
	{ .type = 0x39, .len = 4, .data = { 0xb8, 0x0f, 0x00, 0x00 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x03 } },
	{ .type = 0x39, .len = 4, .data = { 0xb8, 0x07, 0x70, 0x00 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x01 } },
	{ .type = 0x39, .len = 2, .data = { 0xc7, 0xff } },
	{ .type = 0x39, .len = 4, .data = { 0xb7, 0x00, 0x40, 0x04 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x0b } },
	{ .type = 0x39, .len = 2, .data = { 0xca, 0xff } },
	{ .type = 0x39, .len = 6, .data = { 0xf0, 0x55, 0xaa, 0x52, 0x08, 0x08 } },
	{ .type = 0x39, .len = 7, .data = { 0xc1, 0xa7, 0x00, 0x3b, 0x81, 0x90, 0x1e } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x06 } },
	{ .type = 0x39, .len = 8, .data = { 0xc1, 0x64, 0x89, 0xa4, 0xbb, 0xcf, 0xe1, 0xf1 } },
	{ .type = 0x39, .len = 16, .data = { 0xc2, 0x88, 0x00, 0x00, 0x88, 0x00, 0x00, 0x88, 0x00, 0x00, 0x88, 0x00, 0x00, 0x88, 0x00, 0x00 } },
	{ .type = 0x39, .len = 13, .data = { 0xc3, 0x88, 0x00, 0x00, 0x88, 0x00, 0x00, 0x88, 0x00, 0x00, 0x88, 0x00, 0x00 } },
	{ .type = 0x39, .len = 12, .data = { 0xc4, 0x44, 0x00, 0x00, 0x44, 0x00, 0x00, 0x44, 0x00, 0x00, 0x04, 0x00 } },
	{ .type = 0x39, .len = 10, .data = { 0xc5, 0x88, 0x00, 0x00, 0x88, 0x00, 0x00, 0x88, 0x00, 0x00 } },
	{ .type = 0x39, .len = 2, .data = { 0xc6, 0x11 } },
	{ .type = 0x39, .len = 9, .data = { 0xc7, 0x50, 0x50, 0x60, 0x70, 0x70, 0x80, 0x80, 0x80 } },
	{ .type = 0x39, .len = 9, .data = { 0xc8, 0x50, 0x50, 0x60, 0x70, 0x80, 0x80, 0xa0, 0xa0 } },
	{ .type = 0x39, .len = 9, .data = { 0xc9, 0x20, 0x20, 0x30, 0x40, 0x50, 0x70, 0xb0, 0xb0 } },
	{ .type = 0x39, .len = 15, .data = { 0xca, 0x00, 0x14, 0x27, 0x00, 0x4e, 0x9d, 0x21, 0x3b, 0x74, 0xb8, 0xb0, 0x3e, 0x0f, 0xff } },
	{ .type = 0x39, .len = 15, .data = { 0xcb, 0x00, 0x13, 0x25, 0x00, 0x4a, 0x95, 0x21, 0x29, 0x52, 0xa8, 0x42, 0x94, 0x0f, 0xff } },
	{ .type = 0x39, .len = 15, .data = { 0xcd, 0x00, 0x0f, 0x1f, 0x00, 0x3d, 0x7a, 0x10, 0xf5, 0xea, 0xcb, 0x5c, 0x69, 0x0f, 0xff } },
	{ .type = 0x39, .len = 16, .data = { 0xce, 0x22, 0x00, 0x00, 0x22, 0x00, 0x00, 0x22, 0x00, 0x00, 0x22, 0x00, 0x00, 0x22, 0x00, 0x00 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x0f } },
	{ .type = 0x39, .len = 13, .data = { 0xce, 0x22, 0x00, 0x00, 0x22, 0x00, 0x00, 0x22, 0x00, 0x00, 0x22, 0x00, 0x00 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x1b } },
	{ .type = 0x39, .len = 28, .data = { 0xce, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } },
	{ .type = 0x39, .len = 21, .data = { 0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x22 } },
	{ .type = 0x39, .len = 10, .data = { 0xd0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ .type = 0x39, .len = 3, .data = { 0xd2, 0x00, 0x01 } },
	{ .type = 0x39, .len = 2, .data = { 0x6f, 0x02 } },
	{ .type = 0x39, .len = 10, .data = { 0xd2, 0x10, 0x00, 0x87, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00 } },
	{ .type = 0x05, .len = 2, .data = { 0x11, 0x00 } },
	{ .type = 0x05, .len = 2, .data = { 0x29, 0x00 } },
};

static const struct k8_cmd k8_off_cmds[] = {
	{ .type = 0x05, .len = 2, .data = { 0x28, 0x00 } },
	{ .type = 0x05, .len = 2, .data = { 0x10, 0x00 } },
};


static const struct drm_display_mode k8_display_mode = {
	.clock = 1560000,   /* kHz, DSC compressed stream */
	.hdisplay = 1080,
	.hsync_start = 1080 + 40,
	.hsync_end = 1080 + 40 + 10,
	.htotal = 1080 + 40 + 10 + 20,
	.vdisplay = 2400,
	.vsync_start = 2400 + 2704,
	.vsync_end = 2400 + 2704 + 2,
	.vtotal = 2400 + 2704 + 2 + 2000,
	.width_mm = 72,
	.height_mm = 160,
};

struct k8_panel {
	struct drm_panel base;
	struct mipi_dsi_device *link;
	struct regulator_bulk_data supplies[2];
	struct gpio_desc *reset_gpio;
	bool prepared;
	bool enabled;
};

static inline struct k8_panel *to_k8_panel(struct drm_panel *panel)
{
	return container_of(panel, struct k8_panel, base);
}

static int k8_send_seq(struct k8_panel *ctx, const struct k8_cmd *cmds, int n)
{
	int i, ret;

	for (i = 0; i < n; i++) {
		const struct k8_cmd *c = &cmds[i];

		if (c->type == 0x05)
			ret = mipi_dsi_dcs_write_buffer(ctx->link, c->data, c->len);
		else if (c->type == 0x15)
			ret = mipi_dsi_dcs_write(ctx->link, c->data[0], NULL, 0);
		else
			ret = mipi_dsi_dcs_write_buffer(ctx->link, c->data, c->len);

		if (ret < 0) {
			dev_err(&ctx->link->dev, "cmd %d (0x%02x) failed: %d\n",
				i, c->data[0], ret);
			return ret;
		}
	}
	return 0;
}

static int k8_panel_unprepare(struct drm_panel *panel)
{
	struct k8_panel *ctx = to_k8_panel(panel);
	int ret;

	if (!ctx->prepared)
		return 0;

	ret = k8_send_seq(ctx, k8_off_cmds, ARRAY_SIZE(k8_off_cmds));
	if (ret < 0)
		dev_err(panel->dev, "failed to send off cmds: %d\n", ret);

	msleep(120);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(1000, 2000);
	regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);

	ctx->prepared = false;
	return 0;
}

static int k8_panel_prepare(struct drm_panel *panel)
{
	struct k8_panel *ctx = to_k8_panel(panel);
	int ret;

	if (ctx->prepared)
		return 0;

	ret = regulator_bulk_enable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(panel->dev, "failed to enable regulators: %d\n", ret);
		return ret;
	}

	/* reset sequence */
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(1000, 2000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(1000, 2000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	msleep(20);

	ret = k8_send_seq(ctx, k8_on_cmds, ARRAY_SIZE(k8_on_cmds));
	if (ret < 0) {
		dev_err(panel->dev, "failed to send on cmds: %d\n", ret);
		goto err;
	}

	msleep(120);
	ctx->prepared = true;
	return 0;

err:
	regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
	return ret;
}

static int k8_panel_enable(struct drm_panel *panel)
{
	struct k8_panel *ctx = to_k8_panel(panel);

	if (ctx->enabled)
		return 0;

	ctx->enabled = true;
	return 0;
}

static int k8_panel_disable(struct drm_panel *panel)
{
	struct k8_panel *ctx = to_k8_panel(panel);

	ctx->enabled = false;
	return 0;
}

static int k8_panel_get_modes(struct drm_panel *panel,
			      struct drm_connector *connector)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, &k8_display_mode);
	if (!mode)
		return -ENOMEM;

	drm_mode_set_name(mode);
	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	connector->display_info.width_mm = k8_display_mode.width_mm;
	connector->display_info.height_mm = k8_display_mode.height_mm;
	drm_mode_probed_add(connector, mode);

	return 1;
}

static const struct drm_panel_funcs k8_panel_funcs = {
	.prepare = k8_panel_prepare,
	.unprepare = k8_panel_unprepare,
	.enable = k8_panel_enable,
	.disable = k8_panel_disable,
	.get_modes = k8_panel_get_modes,
};

static const char * const k8_supply_names[] = { "vddio", "vddd" };

static int k8_panel_add(struct k8_panel *ctx)
{
	struct device *dev = &ctx->link->dev;
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(ctx->supplies); i++)
		ctx->supplies[i].supply = k8_supply_names[i];

	ret = devm_regulator_bulk_get(dev, ARRAY_SIZE(ctx->supplies), ctx->supplies);
	if (ret < 0)
		return dev_err_probe(dev, ret, "failed to get regulators\n");

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio), "failed to get reset gpio\n");

	drm_panel_init(&ctx->base, dev, &k8_panel_funcs, DRM_MODE_CONNECTOR_DSI);
	drm_panel_add(&ctx->base);
	return 0;
}

static int k8_panel_probe(struct mipi_dsi_device *dsi)
{
	struct k8_panel *ctx;
	int ret;

	ctx = devm_kzalloc(&dsi->dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->link = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE |
			  MIPI_DSI_MODE_LPM |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS;

	ret = k8_panel_add(ctx);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		drm_panel_remove(&ctx->base);
		return ret;
	}

	dev_info(&dsi->dev, "CSOT k8 panel (xiaomi 42 02 0a) attached\n");
	return 0;
}

static int k8_panel_remove(struct mipi_dsi_device *dsi)
{
	struct k8_panel *ctx = mipi_dsi_get_drvdata(dsi);

	drm_panel_unprepare(&ctx->base);
	drm_panel_remove(&ctx->base);
	mipi_dsi_detach(dsi);
	return 0;
}

static const struct of_device_id k8_of_match[] = {
	{ .compatible = "xiaomi,odin-k8-panel" },
	{ }
};
MODULE_DEVICE_TABLE(of, k8_of_match);

static struct mipi_dsi_driver k8_panel_driver = {
	.driver = {
		.name = "panel-xiaomi-odin-k8",
		.of_match_table = k8_of_match,
	},
	.probe = k8_panel_probe,
	.remove = k8_panel_remove,
};
module_mipi_dsi_driver(k8_panel_driver);

MODULE_AUTHOR("yuexusi");
MODULE_DESCRIPTION("CSOT k8 (xiaomi 42 02 0a) DSI panel for Mi MIX 4");
MODULE_LICENSE("GPL");
