// SPDX-License-Identifier: GPL-2.0-only

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <drm/drm_dp_dual_mode_helper.h>

static int fake_mode = DRM_DP_DUAL_MODE_NONE;

MODULE_PARM_DESC(fake_mode, "drm_dp_dual_mode_detect return value");
module_param(fake_mode, int, 0600);

static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
  regs_set_return_value(regs, (enum drm_dp_dual_mode_type)fake_mode);

  return 0;
}

static struct kretprobe probe = {
  .kp.addr = (kprobe_opcode_t *)drm_dp_dual_mode_detect,
  .handler = ret_handler,
  .maxactive = 1,
};

static int __init fake_dp_dual_mode_init(void)
{
  int ret;

  ret = register_kretprobe(&probe);
  if (ret < 0) {
    pr_err("register_kretprobe failed, returned %d\n", ret);
    return -1;
  }

  return 0;
}

static void __exit fake_dp_dual_mode_exit(void)
{
  unregister_kretprobe(&probe);
}

module_init(fake_dp_dual_mode_init)
module_exit(fake_dp_dual_mode_exit)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael Hanselmann");
MODULE_SOFTDEP("pre: i915");

/* vim: set sw=2 sts=2 et : */
