=====================
i.MX93 Evaluation Kit
=====================

.. tags:: chip:imx93, arch:arm64

The kit i.MX93 Evaluation Kit has a pre-installed Linux image which contains
u-boot and the i.MX93 reference Linux installation.

NuttX may work as the bootloader, replacing u-boot completely. Currently it
doesn't initialize the DDR memory yet. In other words, DDR training is still
missing.

Below is a set of instructions on how to run NuttX on the i.MX93 EVK, on top
of the u-boot. Also, instructions on running NuttX as the bootloader will
follow.

Pre-requisites
--------------

* ``imx93_ca55.JLinkScript`` which is a custom file, put it wherever you want

U-Boot configuration
--------------------

Two things need to be configured on u-boot before NuttX can be loaded:

* u-boot data cache must be turned off
* u-boot must stop to the u-boot console, i.e. the Linux payload must not be
  loaded

--------------
Manual option:
--------------

1. Disable u-boot autostart (needs to be done only once):

   .. code:: console

      Hit any key to stop autoboot:  0
      u-boot=> setenv bootdelay -1
      u-boot=> saveenv
      Saving Environment to MMC... Writing to MMC(0)... OK
      u-boot=> reset

2. On every boot, the data cache must be disabled for options 2 and 3 to work

   .. code:: console

      u-boot=> dcache off

-----------------
Automated option:
-----------------

Replace the default bootcmd to disable dcache automatically:

.. code:: console

   u-boot=> setenv bootdelay 0
   u-boot=> setenv bootcmd dcache off
   u-boot=> saveenv
   Saving Environment to MMC... Writing to MMC(0)... OK
   u-boot=> reset

To restore the default bootcmd which starts Linux automatically:

.. code:: console

    u-boot=> setenv bootcmd run distro_bootcmd;run bsp_bootcmd
    u-boot=> saveenv
    Saving Environment to MMC... Writing to MMC(0)... OK
    u-boot=> reset

The default bootcmd is:

.. code:: console

   u-boot=> env print bootcmd
   bootcmd=run distro_bootcmd;run bsp_bootcmd

Loading and running the NuttX image
===================================

You have four options:

1. Load via u-boot from SD-card
2. Load via gdb
3. Load via JLink
4. Run from SD-card, without u-boot
5. Kernel build, via AHAB boot


Option 1: load via u-boot from SD-card:
---------------------------------------

1. Build nuttx, and move ``nuttx.bin`` to SD-card

2. Load from SD-card and start nuttx payload

   .. code:: console

      u-boot=> dcache off; fatload mmc 1 0x80000000 nuttx.bin; go 0x80000000

Option 2: start via ``gdb``:
----------------------------

1. Start JLinkGDBServer

   .. code:: console

      $ JLinkGDBServer -device CORTEX-A55 -JLinkScriptFile <path_to>/imx93_ca55.JLinkScript

2. Start gdb

   .. code:: console

      $ aarch64-none-elf-gdb

   a. Attach and load nuttx

     .. code:: console

        (gdb) target remote localhost:2331
        (gdb) set mem inaccessible-by-default off
        (gdb) load <path_to>/nuttx
        (gdb) monitor go

Option 3: load with JLink:
--------------------------

1. Start JLink 

   .. code:: console

      $ JLinkExe -device CORTEX-A55 -if JTAG -jtagconf -1,-1 -speed 4000 -JLinkScriptFile <path_to>/imx93_ca55.JLinkScript

   a. Add -AutoConnect 1 to connect automatically

      .. code:: console

         $ JLinkExe -device CORTEX-A55 -if JTAG -jtagconf -1,-1 -speed 4000 -JLinkScriptFile <path_to>/imx93_ca55.JLinkScript -AutoConnect 1

2. Connect JLink
    
   a. Connect to the debugger

      .. code:: console

         Type "connect" to establish a target connection, '?' for help
         J-Link>connect

      You should now have a JLink prompt.

      .. code:: console
    
         Cortex-A55 identified.
         J-Link>

3. Load nuttx. Note that JLink expects the .elf extension, the default build
   output of nuttx is just "nuttx" without the extension, so it must be added to
   the file...

   .. code:: console

      J-Link>LoadFile <path_to>/nuttx.elf

Option 4: Run from SD-card, without u-boot
------------------------------------------

1. Make sure ``CONFIG_IMX9_BOOTLOADER`` is set and system is configured properly
   for bootloader operation:

   .. code:: console

      $ tools/configure.sh imx93-evk:bootloader

2. The build outputs a file ``imx9-sdimage.img``. This image also contains the
   Ahab container. It's required to grant Trusted Resource Domain Controller
   (TRDC) permissions. Flash it to an SD-card, where sdX may be sda or something
   else; verify the block device name properly (eg. ``/dev/sda``, ``/dev/sdb`` etc):

   .. code:: console

      $ sudo dd if=imx9-sdimage.img of=/dev/sdX bs=1k && sync

3. Insert the SD-card into the imx93-evk, make sure BMODE switch is [1,2,3,4] =
   [Off, On, Off, Off] so that it boots from the SD-card. This should boot into
   NuttShell in EL3 level.

Option 5: Kernel build, via AHAB boot
-------------------------------------

1. Follow the instructions at:
   https://spsdk.readthedocs.io/en/latest/examples/ahab/imx93/imx93_ahab_uboot.html
   to create an eMMC-bootable image (latest version of instructions tested is
   v2.6.1). We will be replacing the u-boot binary in step 2.3 with NuttX.

2. Clone both NuttX and NuttX-Apps in same level directories `nuttx` and `apps`
   respectively.

3. Configure and build NuttX:

   .. code:: console

      $ cd nuttx
      $ tools/configure.sh imx93-evk:knsh
      $ make
      $ make export

4. Build NuttX apps and prepare the ``/bin`` ROMFS image:

   .. code:: console

      $ pushd ../apps
      $ tools/mkimport.sh -z -x ../nuttx/nuttx-export-*.tar.gz
      $ make import
      $ tools/mkromfsimg.sh
      $ mv boot_romfsimg.h ../nuttx/boards/arm64/imx9/imx93-evk/include/bin_romfsimg.h
      $ popd

5. Re-build NuttX embedding the generated /bin ROMFS image:

   .. code:: console
   
      $ make clean clean_context
      $ make

6. Replace the value of the `u-boot:` entry in ``workspace/ahab_template.yaml``
   created in step 1 above with the path to ``nuttx.bin``.
