# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.


include $(MTK_ROOT_BUILD)/libs/gmsl
# all-subdir-src-or-make
# - set obj-y to all subdir with a makefile inside, or all sources in subdirs without a makefile insde.
# - parameter: none
define all-subdir-src-or-makefile
$(eval object := $(if $(filter yes,$(KBUILD_OUTPUT_SUPPORT)),$(srctree)/$(obj),$(obj))) \
$(eval dir := $(subst $(object)/,,$(subst Makefile,,$(call wildcard2,\
              $(addsuffix Makefile, $(call wildcard2,$(object)/*/)))))) \
$(eval file := $(subst $(object)/,,$(call wildcard2,$(addsuffix /*.c,\
              $(filter-out $(object)/Makefile $(patsubst %/,%,$(addprefix \
              $(object)/,$(dir))),$(call wildcard2,$(object)/*)))))) \
$(eval obj-y := $(dir) $(patsubst %.c,%.o,$(file)))
endef

# all-subdirs-with-makefile
# - get all subdir with a Makefile in it
define all-subdirs-with-makefile
$(eval object := $(if $(filter yes,$(KBUILD_OUTPUT_SUPPORT)),$(srctree)/$(obj),$(obj))) \
$(eval $(if $(1),subdirs:=$(1),subdirs:=*))\
$(foreach d,$(subdirs),$(subst $(object)/,,$(subst Makefile,,$(call wildcard2,\
$(addsuffix Makefile, $(call wildcard2,$(object)/$(d)/))))))
endef

# all-subdirs-src
# - get all c files in the folder
define all-subdirs-src
$(if $(1),$(subst $(obj)/,,$(call wildcard2,$(addprefix $(obj)/,$(1)))),\
$(subst $(obj)/,,$(call wildcard2,$(addsuffix /*.c,$(obj)))))
endef

# modules
# - select paths according to provided variables
# - parameter:
#   1) root directory, e.g., mediatek/module/kernel/drivers
#   2) variable set prefix, e.g., CUSTOM_KERNEL_
define modules
$(strip $(patsubst $(1)/./%,%,$(call wildcard2,$(addprefix $(1)/./,\
$(foreach item,$(patsubst %/,%,$(call subdirs,$(1))), $(if \
$($(2)$(call uc,$(item))),$(item),))))))
endef
define subdirs-have-module
$(foreach item,$(patsubst %/,%,$(call subdirs,$(1))), $(if \
$($(2)$(call uc,$(item))),$(item)/,))
endef
#old version: expand to (e.g.) touchpanel/eeti_pcap7200
#define modules
#$(strip $(patsubst $(1)/./%,%,$(call wildcard2,$(addprefix $(1)/./,\
#$(foreach item,$(patsubst %/,%,$(call subdirs,$(1))), $(if \
#$($(2)$(call uc,$(item))),$(item)/$($(2)$(call uc,$(item))),))))))
#endef

#$(strip $(subst $(1)/./,,$(dir $(call wildcard2,$(1)/./*/))))

define all-modules-src-or-makefile
$(eval object := $(if $(filter yes,$(KBUILD_OUTPUT_SUPPORT)),$(srctree)/$(1),$(obj))) \
$(eval _ := $(call modules,$(if $(filter yes,$(KBUILD_OUTPUT_SUPPORT)),$(object),$(1)),$(2))) \
$(eval _ += $(filter-out $(_),$(patsubst $(object)/%,%,\
  $(call wildcard2,$(addprefix $(object)/,$(addsuffix src,$(call subdirs-have-module,$(1),$(2)))))))) \
$(eval dir := $(subst $(object)/,,$(subst Makefile,,$(call wildcard2,\
              $(addsuffix /Makefile,$(addprefix $(object)/,$(_))))))) \
$(eval file := $(subst $(object)/,,$(call wildcard2,$(addsuffix /*.c,\
              $(addprefix $(object)/,$(filter-out \
              $(patsubst %/,%,$(dir)),$(_))))))) \
$(eval obj-y := $(dir) $(patsubst %.c,%.o,$(file))) \
$(eval dir_create := $(dir $(addprefix $(1)/,$(obj-y)))) \
$(shell [ -d $(dir_create) ] || mkdir -p $(dir_create))
endef

define my-dir
$(if $(filter yes,$(KBUILD_OUTPUT_SUPPORT)),$(call to-root,$(obj))$(obj),.)
endef
