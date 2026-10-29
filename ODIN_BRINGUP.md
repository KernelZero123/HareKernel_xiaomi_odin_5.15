# Xiaomi Mi MIX 4 (odin) — Linux 5.15 bring-up

Base: **ACK android13-5.15-lts = Linux 5.15.195**

## Why not stock 5.4

sm8350/lahaina has **no 5.15 BSP from Qualcomm**. Highest CAF tag for lahaina is
LA.UM.9.14 (5.4). LineageOS `android_kernel_xiaomi_sm8350` is still 5.4.302 even
on lineage-24.0. So this tree is a forward-port, not a merge.

## Verified status

| Item | State |
|---|---|
| GKI 5.15.195 build | ✅ Image 41MB, 0 errors |
| sm8350-odin.dts | ✅ compiles, 41KB dtb |
| Kernel boots to userspace | ✅ **verified in qemu** |
| boot.img packaging | ✅ header v2 + v3 both produced |
| Display (DPU) | ❌ ACK 5.15 has no DPU 7.0.0 catalog |
| GPU (gpucc) | ❌ no gpucc-sm8350 |
| Camera / venus | ❌ absent |
| SD card | ❌ sdhci-msm has no sm8350 |

## What ACK 5.15 already gives us for free

- `sm8350.dtsi` (2257 lines) — SoC skeleton
- `gcc-sm8350.c` — main clock controller
- `interconnect/qcom/sm8350.c` — bus fabric
- `arm-smmu-qcom.c` — `qcom,sm8350-smmu-500`
- `pinctrl-sm8350.c`
- `rpmhpd.c` — 12 sm8350 power domains
- `qcom-rpmh-regulator.c` — pm8350 / pm8350c
- PMIC dtsi: pm8350, pm8350b, pm8350c, pmk8350, pmr735a, pmr735b
- UFS: matches via generic `qcom,ufshc`

Boot chain (CPU → RPMh → PMIC → UFS → init) is therefore complete.

## odin UFS regulators (from 5.4 xiaomi-sm8350-common.dtsi)

| Rail | Source | Voltage |
|---|---|---|
| UFS VCC | PM8350 L7 | 2.504–2.95 V |
| UFS VCCQ | PM8350 L9 | 1.2 V |
| VCCQ parent | PM8350 S12 | 1.25 V |
| PHY VDDA | PM8350 L5 | 0.88 V |
| PHY PLL | PM8350 L6 | 1.2 V |

## Blind-debug channel: ramoops

MIX4 stock DTS has **no ramoops**. Added at `0x85100000`, 1MB, carved from the
gap between `cdsp_secure_heap` (ends 0x85200000) and `pil_camera` (starts
0x85200000).

```
record-size  = 0x40000
console-size = 0x40000
pmsg-size    = 0x20000
```

After a failed boot, recover the log from a working system / TWRP:

```
cat /sys/fs/pstore/console-ramoops-0
```

## Toolchain caveat

Host clang is 21.1.8; ACK 5.15 expects clang-r450784e (~14). Newer clang turns
warnings into hard errors, e.g.

```
arch/arm64/kvm/sys_regs.c:2817: error: variable 'clidr' is uninitialized
```

Workaround used: `KCFLAGS="-Wno-error"` plus `# CONFIG_WERROR is not set`.
Ideally swap in the real prebuilt clang later.

## Build

```sh
make O=out ARCH=arm64 CC=clang LLVM=1 \
     KCFLAGS="-Wno-error" Image qcom/sm8350-odin.dtb
```

## Packaging

```sh
# v2: dtb embedded, usable with `fastboot boot`
python3 tools/mkbootimg.py --header_version 2 \
  --kernel out/arch/arm64/boot/Image \
  --ramdisk initramfs.cpio.gz \
  --dtb out/arch/arm64/boot/dts/qcom/sm8350-odin.dtb \
  --cmdline "console=ttyMSM0,115200n8 earlycon loglevel=8" \
  --pagesize 4096 --base 0x0 --ramdisk_offset 0x01000000 \
  --tags_offset 0x100 --dtb_offset 0x01f00000 -o boot-odin-v2.img
```

Note: the ROM uses header v3 + vendor_boot (`BOARD_INCLUDE_DTB_IN_BOOTIMG :=
true`), but the stock vendor_boot image is not available here, so v3 cannot carry
the odin DTB yet. v2 is the practical test artifact.

## Next steps (priority order)

1. Add sm8350 DPU catalog — lahaina is `SDE_HW_VER_700` (7.0.0); ACK 5.15
   `dpu_hw_catalog.c` stops at `DPU_HW_VER_720` (sc7280) and has no 700 entry.
2. Port `dispcc-sm8350` clock controller.
3. Port `gpucc-sm8350` + adreno support.
4. Camera (camcc + CCI + CAMSS) — largest single chunk.

---

# Progress log (2026-10-29)

## v1 — kernel boots (verified in qemu)

`Run /init as init process` reached. Chain CPU -> memory -> DT -> initramfs
works. But a bare GKI kernel drives nothing on real hardware.

## v2/v3 — no USB device appeared

Root cause found by auditing the built-in symbols, not the config file:

```
CONFIG_PHY_QCOM_QMP        was NOT set   <- USB PHY, nothing works without it
CONFIG_QCOM_RPMH           was NOT set   <- root of power/clock
CONFIG_QCOM_SMEM           was NOT set
CONFIG_INTERCONNECT_QCOM   was NOT set
CONFIG_ARM_SMMU            was NOT set
CONFIG_PINCTRL_SM8350      was NOT set   <- no GPIO, reset line unusable
CONFIG_SM_GCC_8350         was NOT set
CONFIG_SM_DISPCC_8250      was NOT set
```

`CONFIG_ARCH_QCOM=y` alone is not enough: it only opens the door.

## v4 — platform drivers enabled

`arch/arm64/configs/odin_defconfig` (6948 lines) is the new baseline.
Verified present in vmlinux: qmp phy, dwc3-qcom, rpmhpd, sm8350 tlmm,
dpu_kms, k8_panel_probe.

## Kconfig fix required for display clocks

`SM_DISPCC_8250` depends on `SM_GCC_8150 || SM_GCC_8250`, but sm8350 uses
`SM_GCC_8350`, so olddefconfig silently dropped it. Patched:

```
-depends on SM_GCC_8150 || SM_GCC_8250
+depends on SM_GCC_8150 || SM_GCC_8250 || SM_GCC_8350
```

Without this, dispcc-sm8250.c (which carries `qcom,sm8350-dispcc`) is never
compiled and the display clock tree is absent.

## Build

```sh
OUT=out ./build-odin.sh odin_defconfig
OUT=out ./build-odin.sh Image qcom/sm8350-odin.dtb
```

Use `-j3`. `-j8` segfaults during LTO vmlinux link on an 8-core/27G host.

## Panel

CSOT k8 (xiaomi 42 02 0a), 1080x2400 DSC cmd mode.
Sequences parsed from vendor `dsi-panel-k8-42-02-0a-dsc-cmd.dtsi`:
100 on-commands + 2 off-commands.

Unverified guesses that still need a real device to confirm:
- panel supplies: vddio = PM8350 L9 (1.2V), vddd = PM8350 L5 (0.88V)
- reset gpio: tlmm 6
