package com.mediatek.systemui.plugin;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.res.Resources;

import com.mediatek.op02.plugin.R;
import com.mediatek.systemui.ext.DataType;
import com.mediatek.systemui.ext.DefaultStatusBarPlugin;
import com.mediatek.systemui.ext.NetworkType;

/**
 * M: OP02 implementation of Plug-in definition of Status bar.
 */
public class Op02StatusBarPlugin extends DefaultStatusBarPlugin {

    public Op02StatusBarPlugin(Context context) {
        super(context);
    }

    public Resources getPluginResources() {
        return this.getResources();
    }

    public int getSignalStrengthIcon(boolean roaming, int inetCondition, int level) {
        return -1;
    }

    public String getSignalStrengthDescription(int level) {
        return null;
    }

    public int getSignalStrengthIconGemini(int simColorId, int level) {
        return -1;
    }

    public int getSignalStrengthIconGemini(int simColorId, int type, int level) {
        return -1;
    }

    public int getSignalStrengthNullIconGemini(int slotId) {
        if (slotId == 0) {
            return R.drawable.stat_sys_gemini_signal_null_sim1;
        } else if (slotId == 1) {
            return R.drawable.stat_sys_gemini_signal_null_sim2;
        }
        return -1;
    }

    public int getSignalStrengthSearchingIconGemini(int slotId) {
        return -1;
    }

    public int getSignalIndicatorIconGemini(int slotId) {
        if (slotId == 0) {
            return R.drawable.stat_sys_gemini_signal_indicator_sim1;
        } else if (slotId == 1) {
            return R.drawable.stat_sys_gemini_signal_indicator_sim2;
        }
        return -1;
    }

    public int[] getDataTypeIconListGemini(boolean roaming, DataType dataType) {
        int[] iconList = null;
        if (roaming) {
            iconList = TelephonyIcons.DATA_ROAM[dataType.getTypeId()];
        }
        return iconList;
    }

    public boolean isHspaDataDistinguishable() {
        return true;
    }
    
    public int getDataNetworkTypeIconGemini(NetworkType networkType, int simColorId) {
        int typeId = networkType.getTypeId();
        if (typeId >= 0 && typeId <= 1) {
            return TelephonyIcons.NETWORK_TYPE[typeId][simColorId];
        }
        return -1;
    }

    public int[] getDataActivityIconList() {
        return null;
    }

    public boolean supportDataTypeAlwaysDisplayWhileOn() {
        return false;
    }

    public boolean supportDisableWifiAtAirplaneMode() {
        return false;
    }

    public String get3gDisabledWarningString() {
        return getString(R.string.gemini_3g_disable_warning);
    }

}
