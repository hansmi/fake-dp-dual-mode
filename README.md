# Override detection of DisplayPort++ adaptors in Linux kernel

After upgrading a Toshiba Satellite Z30-A laptop from [Debian
9 (Stretch)][debstretch] with [Linux 4.9][kernelorg] to [Debian 10
(Buster)][debbuster] which is using the Linux 4.19 kernel the external monitor,
a Dell U2715H, would no longer be driven at its native resolution of 2560x1440
and a reduced resolution of 2048x1152 was used instead. Already with Debian
Stretch a [custom modeline][modeline] in the [X.Org][xorg] configuration was
required to compel the software into using the appropriate resolution:

```
# 2560x1440 59.95 Hz (CVT 3.69M9-R) hsync: 88.79 kHz; pclk: 241.50 MHz
Modeline "2560x1440R" 241.50 2560 2608 2640 2720 1440 1443 1448 1481 +hsync -vsync
```

For the investigation with Debian Buster I enabled verbose logging:

```
# Enable DRM driver and KMS debug messages
echo 0x6 > /sys/module/drm/parameters/debug

# Disable
echo 0 > /sys/module/drm/parameters/debug
```

The messages showed that the pixel clock rate was being limited to 165 MHz:

```
[ 1428.752898] [drm:drm_dp_dual_mode_detect [drm_kms_helper]] DP dual mode HDMI ID: DP-HDMI ADAPTOR\004 (err 0)
[ 1428.753676] [drm:drm_dp_dual_mode_detect [drm_kms_helper]] DP dual mode adaptor ID: ff (err 0)
[ 1428.753728] [drm:intel_hdmi_set_edid [i915]] DP dual mode adaptor (type 1 HDMI) detected (max TMDS clock: 165000 kHz)
```

After reading plenty of Linux kernel code ([primarily in `drivers/gpu/drm` and
`drivers/gpu/drm/i915`][linux419drm]) and comparing changes between Linux 4.9
and 4.19, as well as in [`xf86-video-intel`][xf86intel] from Debian Stretch,
I came up with an experimental kernel code change which made the native
resolution work again.

As I wasn't sure whether I was dealing with a software bug or have been driving
the hardware out of specs for over two years I filed [bug #111553 with the
`i915` graphics driver developers][bug111553]. Upstream developer Ville Syrjälä
confirmed that the hardware indeed is supposed to only support pixel clock
frequencies up to 165 MHz. Despite the monitor being attached via a HDMI-A
cable there must be a converter from DisplayPort to HDMI or the other way
around in the signal path (possibly in the monitor as it also has a DisplayPort
connector).

Considering that everything was working well before the upgrade I wanted to
achieve a working configuration under Debian Buster without changing hardware.
One way would've been to [rebuild the Debian kernel package with
a patch][customdebkernel] as already done for my experiments (albeit for those
the `i915.ko` module was sufficient). Maintaining a local kernel package is
a maintenance burden I'd rather avoid and thus looked for another solution.


## Implementation

The module uses [Kernel probes][kprobes] to hooks itself into the return path
of the [`drm_dp_dual_mode_detect`][linux419dpdualmodedetect] function. This
function is used by the `i915` driver as part of the decision whether the pixel
clock rate should be restricted and is, this is critical, exported.

The `fake_mode` parameter defaults to `DRM_DP_DUAL_MODE_NONE` ("there is no
DisplayPort conversion"), but can be set to any value.

There is packaging information for Debian using [DKMS][dkms].


[bug111553]: https://bugs.freedesktop.org/show_bug.cgi?id=111553
[customdebkernel]: https://kernel-team.pages.debian.net/kernel-handbook/ch-common-tasks.html#s-common-official
[debbuster]: https://www.debian.org/releases/buster/index.en.html
[debstretch]: https://www.debian.org/releases/stretch/index.en.html
[dkms]: https://github.com/dell/dkms
[kernelorg]: https://www.kernel.org/
[kprobes]: https://www.kernel.org/doc/Documentation/kprobes.txt
[linux419dpdualmodedetect]: https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/gpu/drm/drm_dp_dual_mode_helper.c?h=v4.19#n164
[linux419drm]: https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/gpu/drm?h=v4.19
[modeline]: https://en.wikipedia.org/wiki/XFree86_Modeline
[xf86intel]: https://cgit.freedesktop.org/xorg/driver/xf86-video-intel/
[xorg]: https://www.x.org/

<!-- vim: set sw=2 sts=2 et : -->
