# Enhanced FSBL for Sophgo CV18xx/SG200x Boards

Comparing to the upstream bootloader,

- Buildable with a common upstream toolchain (without T-Head modifications).
- Easier usage, suitable to pack with mainline OpenSBI/U-Boot.
- Support packing FDT in the image, avoiding non-portable OpenSBI binaries.

This repository probably won't work with the ARM core!

## NOTICE

***
This repository is forked from [Sophgo's FSBL](https://github.com/sophgo/fsbl),
which doesn't claim a formal open-source license. Please contact me to remove
the content if there's any type of abusing.
***

Force-pushing is expected on the branch before the
[license issue](https://github.com/sophgo/fsbl/issues/1) is completely resolved.

## Bootflow of CV18xx/SG200x SoCs

```
                        --------
                        | RTOS |
--------    --------  / --------
| BROM | => | FSBL |  |
--------    --------  \ -----------    -----------------
                        | Monitor | => | Second Loader |
                        -----------    -----------------

|                                     |                 |
|               M-Mode                |     S-Mode      |
|                                     |                 |

```

Concepts are taking from the FSBL source code, as they take both the ARM and
RISC-V core in consideration. In a common RISC-V usecase,

- `Monitor` stands for `OpenSBI`
- `Second Loader` stands for `U-Boot`

## Building the FSBL

### Requirements

- riscv64 toolchain
- python3
  - `lz4` module, if you want to use LZ4 for second loader compression
    (UNTESTED!)

### Build Steps

```shell
	$ make CHIP_ARCH=... DDR_CFG=... BOOT_CPU=riscv \
		CROSS_COMPILE=...
```

- `CHIP_ARCH` specifies which chip to use, currently all modifications are
  subject to `cv181x`.
  Valid options: `cv180x`, `cv181x`
- `DDR_CFG` specifies the DRAM configuration of the board,
  Valid options: `ddr2_1333_x16`, `ddr3_1866_x16`, `ddr3_2133_x16`
- `CROSS_COMPILE` should be the prefix of your cross-compiling toolchain.

For example, if you want to build the FSBL for Milk-V Duo 256M with a toolchain
whose GCC is called `riscv64-unknown-linux-musl-gcc`,

```shell
	$ make CHIP_ARCH=cv181x DDR_CFG=ddr3_1866_x16 BOOT_CPU=riscv \
		CROSS_COMPILE=riscv64-unknown-linux-musl-
```

### Mapping from boards to the configuration,

|     Board Name    |     CHIP_ARCH    |     DDR_CFG     |  Status   |
--------------------|------------------|-----------------|-----------|
|  Milk-V Duo-256M  |       cv181x     |  ddr3_1866_x16  |  tested   |
|  Milk-V Duo       |       cv180x     |  ddr2_1333_x16  |  tested   |

## Packing FSBL and other images together

`fip.sh` serves as a wrapper over multiple files, to ease image creation,
all arguments are passed through environment variables.

```shell
	$ CHIP_ARCH=... \
		OPENSBI=...	\
		NEXTLOADER=...	\
		FDT=...		\
	 ./fip.sh
```

- `CHIP_ARCH`: same as above
- `OPENSBI`: Path to the monitor binary
- `NEXTLOADER`: Path to the secondary bootloader
- `FDT`: (Optional), path to the FDT to be passed to monitor

This will generate `fip.bin`. Choose a SDCard, partition it with MBR tables,
format the first partition as FAT32 and copy `fip.bin` into it. The board
should boot.

## Example Usage

```shell
	# Build FSBL
	# git clone https://github.com/ziyao233/sophgo-fsbl.git
	$ make CROSS_COMPILE=riscv64-unknown-linux-musl- \
		CHIP_ARCH=cv181x	\
		DDR_CFG=ddr3_1866_x16	\
		BOOT_CPU=riscv

	# Build Mainline U-Boot
	# git clone https://github.com/u-boot/u-boot.git
	$ make CROSS_COMPILE=riscv64-unknown-linux-musl- \
		ARCH=riscv	\
		milkv_duo_defconfig u-boot-dtb.bin u-boot.dtb

	# Build OpenSBI
	# git clone https://github.com/riscv-software-src/opensbi.git
	$ make CROSS_COMPILE=riscv64-unknown-linux-musl \
		PLATFORM=generic FW_FDT_PATH=u-boot/u-boot.dtb

	# Generate the image
	# In sophgo-fsbl
	$ CHIP_ARCH=cv181x \
	  OPENSBI=opensbi/build/platform/generic/firmware/fw_dynamic.bin \
	  NEXTLOADER=u-boot/u-boot.bin					\
	  ./fip.sh

	# The following doesn't work with mainline U-Boot (v2025.04-rc2)
	# The milkv_duo port doesn't respect the devicetree passed by previous
	# firwmare (OpenSBI), have to bundle them together.
	# Unless you apply https://lore.kernel.org/u-boot/20250227144734.61458-1-ziyao@disroot.org/,
	# to U-Boot, and you could avoid FW_FDT_PATH when building OpenSBI
	$ CHIP_ARCH=cv181x \
	  OPENSBI=opensbi/build/platform/generic/firmware/fw_dynamic.bin \
	  NEXTLOADER=u-boot/u-boot-nodtb.bin				\
	  FDT=u-boot.dtb						\
	  ./fip.sh
```

## TODOs

- ~~Test with Milk-V Duo (64MiB DRAM)~~
- Dynamically detecting SoC and choosing appropriate devicetree
- Ultimately, upstream it as U-Boot SPL!
