
package com.mediatek.op.sms;

import android.app.NotificationManager;
import android.app.Notification;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.provider.Telephony.SimInfo;
import android.provider.Telephony.SIMInfo;
import android.os.Handler;
import android.os.Message;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.WspTypeDecoder;
import com.mediatek.common.sms.IWapPushFwkExt;
import com.mediatek.xlog.Xlog;

class WapPushFwkExtOP02 extends WapPushFwkExt {
    private static String TAG = "WapPushFwkExtOP02";
    
    private static final int EVENT_DELAY_NOTIFY_AUTO_PUSH = 1;
    
    private static final int DELAY_NOTI_TIME = 15 * 1000;
    private static final String PACKAGE_NAME_SETTINGS = "com.android.settings";
    private static final String CLASS_NAME_SETTINGS = "com.android.settings.gemini.WapPushSettings";
    
    public WapPushFwkExtOP02(Context context, int phoneId) {
        super(context, phoneId);
        
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_SIM_INFO_UPDATE);
        ///M: add for power-off alarm
        filter.addAction(Intent.ACTION_BOOT_COMPLETED);
        filter.addAction(IWapPushFwkExt.ACTION_WAP_PUSH_NOTI_CANCEL);
        mContext.registerReceiver(mReceiver, filter);
        Xlog.d(TAG, "call WapPushFwkExtOP02 constructor");
    }
    
    public boolean allowDispatchWapPush() {
        Xlog.d(TAG, "call allowDispatchWapPush");
        SIMInfo info = null;
        int status = -1;
        
        if(mPhoneId != Phone.GEMINI_SIM_1 && mPhoneId != Phone.GEMINI_SIM_2) {
            Xlog.d(TAG, "FAIL! invalid slot id " + mPhoneId);
            return true;
        }
        
        info = SIMInfo.getSIMInfoBySlot(mContext, mPhoneId);
        if(info == null) {
            Xlog.d(TAG, "FAIL! SIMInfo is null");
            return true;
        }
        
        Xlog.d(TAG, "SIM " + mPhoneId + "Auto-Push status: " + info.mWapPush);
        printWapPushSettingInfo();
        return (info.mWapPush == SimInfo.WAP_PUSH_ENABLE);
    }
    
    public boolean isMmsWapPush(byte[] pdu) {
        Xlog.d(TAG, "call isMmsWapPush");
        if(pdu == null || pdu.length == 0) {
            Xlog.d(TAG, "FAIL! no data in pdu");
            return false;
        }
        
        int index = 1; // skip transaction id
        int pduType = pdu[index++] & 0xff;
        if(pduType != WspTypeDecoder.PDU_TYPE_PUSH
                && pduType != WspTypeDecoder.PDU_TYPE_CONFIRMED_PUSH) {
            Xlog.d(TAG, "FAIL! invalid pdu type " + pduType);
            return false;
        }
        
        WspTypeDecoder pduDecoder = new WspTypeDecoder(pdu);
        if(pduDecoder == null) {
            Xlog.d(TAG, "FAIL! can not create WspTypeDecoder");
            return false;
        }
        
        if(pduDecoder.decodeUintvarInteger(index) == false) {
            Xlog.d(TAG, "FAIL! header length error");
            return false;
        }
        int headerLength = (int)pduDecoder.getValue32();
        index += pduDecoder.getDecodedDataLength();
        
        if(pduDecoder.decodeContentType(index) == false) {
            Xlog.d(TAG, "FAIL! header content type error");
            return false;
        }
        
        String mimeType = pduDecoder.getValueString();
        Xlog.d(TAG, "mimeType = " + mimeType);
        
        if(mimeType != null) {
            return (mimeType.equals(WspTypeDecoder.CONTENT_MIME_TYPE_B_MMS));
        }
        
        int binaryContentType = (int)pduDecoder.getValue32();
        Xlog.d(TAG, "binaryContentType = " + binaryContentType);
        return (binaryContentType == WspTypeDecoder.CONTENT_TYPE_B_MMS);
    }
    
    protected boolean shouldNotifyUser() {
        Xlog.d(TAG, "call shouldNotifyUser");
        
        boolean hasSim1SettingTouched = isAutoPushSettingTouched(Phone.GEMINI_SIM_1);
        boolean hasSim2SettingTouched = isAutoPushSettingTouched(Phone.GEMINI_SIM_2);
        
        Xlog.d(TAG, "hasSim1SettingTouched = " + hasSim1SettingTouched);
        Xlog.d(TAG, "hasSim2SettingTouched = " + hasSim2SettingTouched);
        return (hasSim1SettingTouched == false || hasSim2SettingTouched == false);
    }
    
    protected void notifyForAutoPushSetting() {
        Xlog.d(TAG, "call notifyForAutoPushSetting");
        
        NotificationManager notiMgr = null;
        if(mContext != null) {
            notiMgr = (NotificationManager)mContext.
                    getSystemService(Context.NOTIFICATION_SERVICE);
        }
        
        if(notiMgr == null) {
            Xlog.d(TAG, "FAIL! can not obtain Notification Service");
            return;
        }
        
        Xlog.d(TAG, "obtain reources for Auto-Push notification");
        Resources r = Resources.getSystem();
        int iconId = android.R.drawable.stat_sys_warning;
        String tickerText = r.getString(com.mediatek.internal.R.string.wap_push_setting_noti_ticker);
        String contentTitle = r.getString(com.mediatek.internal.R.string.wap_push_setting_noti_title);
        String contentText = r.getString(com.mediatek.internal.R.string.wap_push_setting_noti_text);
        long when = System.currentTimeMillis();
        
        Xlog.d(TAG, "initial PendingIntent for Auto-Push notification");
        Intent notiIntent = Intent.makeRestartActivityTask(new ComponentName(PACKAGE_NAME_SETTINGS, CLASS_NAME_SETTINGS));
        PendingIntent contentIntent = PendingIntent.getActivity(mContext, 0, notiIntent, 0);
        
        Xlog.d(TAG, "initial Auto-Push notification");
        Notification noti = new Notification(iconId, tickerText, when);
        noti.defaults = Notification.DEFAULT_SOUND;
        noti.setLatestEventInfo(mContext, contentTitle, contentText, contentIntent);
        notiMgr.notify(AUTO_PUSH_NOTI_ID, noti);
        Xlog.d(TAG, "notify Auto-Push notification");
    }
    
    protected void cancelAutoPushNotification() {
        Xlog.d(TAG, "call cancelAutoPushNotification");
        
        NotificationManager notiMgr = null;
        if(mContext != null) {
            notiMgr = (NotificationManager)mContext.
                    getSystemService(Context.NOTIFICATION_SERVICE);
        }
        
        if(notiMgr == null) {
            Xlog.d(TAG, "FAIL! can not obtain Notification Service");
            return;
        }
        
        notiMgr.cancel(AUTO_PUSH_NOTI_ID);
    }
    
    /*
    * Check Auto-Push Service setting
    *
    * @param slotId slot id
    * 
    * @return true if the specifed SIM has been configed
    * or return false. If no card is inserted in the slot,
    * we resume that the specified SIM has been configed
    */
    private boolean isAutoPushSettingTouched(int slotId) {
        Xlog.d(TAG, "call checkAutoPushSetting " + slotId);
        boolean hasSimInterted = false;
        boolean hasSettingTouched = false;
        SIMInfo info = null;
        
        if(mContext == null) {
            Xlog.d(TAG, "FAIL! mContext is null");
            throw new RuntimeException("no valid Context object in " + TAG);
        }
        
        info = SIMInfo.getSIMInfoBySlot(mContext, slotId);
        if(info == null) {
            Xlog.d(TAG, "info is null");
        } else {
            hasSimInterted = (info.mSlot != SimInfo.SLOT_NONE);
            if(hasSimInterted == true) {
                hasSettingTouched = (info.mWapPush != SimInfo.WAP_PUSH_DEFAULT);
            }
            Xlog.d(TAG, "check SIM " + slotId + " Auto-Push status: " + hasSimInterted + "/" + hasSettingTouched);
        }
        
        return (!hasSimInterted || hasSettingTouched);
    }
    
    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            Xlog.d(TAG, "receive message " + msg.what);
            switch(msg.what) {
                case EVENT_DELAY_NOTIFY_AUTO_PUSH:
                    notifyForAutoPushSetting();
                    break;
                    
                default:
                    break;
            }
        } // end handleMessage
    }; // end mHandler
    
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        /// M: add for power-off alarm not to play wap push ringtone @{
        private boolean mSimInfoUpdated = false;        
        private boolean mBootCompleted = false;
        /// @}
        /*
        * this variable is used to record wether the Auto-Push
        * notification was shown
        */
        private boolean hasNotifiedForAutoPush = false;
        
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Xlog.d(TAG, "receive an intent: " + action);
            if(action.equals(TelephonyIntents.ACTION_SIM_INFO_UPDATE)) {
                Xlog.d(TAG, "sim info db has been updated");
                if(hasNotifiedForAutoPush == true) {
                    Xlog.d(TAG, "no need to show the notification again");
                    return;
                }
                hasNotifiedForAutoPush = true;
                
                /*
                * We just show the notification one time, so
                * we do this job in phone 1
                */
                if(mPhoneId == Phone.GEMINI_SIM_1) {
                    if(shouldNotifyUser() && mBootCompleted) {
                        mHandler.sendEmptyMessageDelayed(
                                EVENT_DELAY_NOTIFY_AUTO_PUSH,
                                DELAY_NOTI_TIME);
                    }else{
                        mSimInfoUpdated = true;
                    }
                }
            } else if(action.equals(ACTION_WAP_PUSH_NOTI_CANCEL)) {
                Xlog.d(TAG, "to cancel the notification");
                cancelAutoPushNotification();
            }else if(action.equals(Intent.ACTION_BOOT_COMPLETED)){
                if(shouldNotifyUser() && mSimInfoUpdated ) {
                    mHandler.sendEmptyMessageDelayed(
                            EVENT_DELAY_NOTIFY_AUTO_PUSH,
                            DELAY_NOTI_TIME);
                }else{
                    mBootCompleted = true;
                }
            }
        } // end onReceive
    }; // end mReceiver
}