package com.mediatek.settings.plugin;

import android.preference.PreferenceScreen;
import android.preference.PreferenceGroup;
import com.android.settings.ext.ISimManagementExt;
import com.mediatek.xlog.Xlog;
import com.mediatek.common.featureoption.FeatureOption;

public class SimManagementExt implements ISimManagementExt{
    private static final String TAG = "SimManagementExt";
    private static final String KEY_3G_SERVICE_SETTING = "3g_service_settings";
    public void updateSimManagementPref(PreferenceGroup parent){
        Xlog.d(TAG,"SimManagementExt---updateSimManagementPref()");
        PreferenceScreen pref3GService = null;
        if (parent != null){
            pref3GService = (PreferenceScreen)parent.findPreference(KEY_3G_SERVICE_SETTING);
        }
        if (pref3GService != null){
            Xlog.d(TAG,"FeatureOption.MTK_GEMINI_3G_SWITCH="+FeatureOption.MTK_GEMINI_3G_SWITCH);
            if (!FeatureOption.MTK_GEMINI_3G_SWITCH){
                parent.removePreference(pref3GService);
            }
        }
    }
}
