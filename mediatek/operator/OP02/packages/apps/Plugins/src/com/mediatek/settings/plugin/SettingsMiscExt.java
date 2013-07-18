/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.settings.plugin;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.ContentValues;
import android.content.ContentResolver;
import com.android.settings.ext.ISettingsMiscExt;
import android.preference.Preference;
import android.preference.PreferenceGroup;
import android.preference.PreferenceScreen;
import android.preference.DialogPreference;
import android.provider.Telephony;
import com.android.internal.telephony.TelephonyProperties;
import android.net.Uri;
import com.mediatek.op02.plugin.R;
import com.mediatek.xlog.Xlog;
import java.util.ArrayList;
import java.util.HashMap;
import android.database.SQLException;
import android.content.ContentUris;
import android.database.Cursor;
import com.mediatek.common.featureoption.FeatureOption;

import static com.mediatek.settings.plugin.OmacpApn.*;

public class SettingsMiscExt extends ContextWrapper implements ISettingsMiscExt{
    Context mContext;

    private static final int SIM_CARD_1 = 0;
    private static final int SIM_CARD_2 = 1;
    
    private long mApnToUseId = -1;
    private boolean mResult;
    
    private static final String CU_3GNET_NAME = "3gnet";
    private static final String CU_3GWAP_NAME = "3gwap";    
    private static final String CU_MMS_TYPE = "mms";     
    
    private static final String TAG = "SettingsMiscExt";
    
    public SettingsMiscExt(Context context) {
   	    super(context);
   	    mContext = getBaseContext();
   }
   
    public boolean isAllowEditPresetApn(String type, String apn, String Numeric) {
        return !Numeric.equals("46001");
    }

    public Preference getApnPref(PreferenceGroup apnList, int count, int[] sourTypeArray) {
        Preference apnPref = apnList.getPreference(0);
        for(int i = 0; i < count; i++){
            Preference preference =  apnList.getPreference(i);
            CharSequence strApn = preference.getSummary();
            if(CU_3GNET_NAME.equals(strApn) && (sourTypeArray[i] == 0)) {
                apnPref = preference;
            }                        
        }
        Xlog.d(TAG,"Get apn: "+apnPref.getSummary());
        return apnPref;
    }

    public void removeTetherApnSettings(PreferenceScreen prefSc, Preference preference) {
        prefSc.removePreference(preference);
    }

	/*
	  *@ return if wifi toggle button could be disabled. 
	 */
	public boolean isWifiToggleCouldDisabled(Context ctx) {
		return true;
	}

	/*
	 *return tether wifi string.
	 */
	public String getTetherWifiSSID(Context ctx){
		return mContext.getString(
                    com.android.internal.R.string.wifi_tether_configure_ssid_default);
	}
	
	public void setTimeoutPrefTitle(Preference pref) {
		DialogPreference dialogPref = (DialogPreference)pref;
		pref.setTitle(mContext.getString(R.string.screen_timeout_CU));
		dialogPref.setDialogTitle(mContext.getString(R.string.screen_timeout_CU));
	}
	
    public void updateApn(Context context, ContentValues values, String type, int SimId, Uri uri, String Numeric, String ApnId, String Apn) {
    	if(UpdateOperApn(context, values, false, Apn, type, uri, SimId, Numeric)) {
            UpdateOperApn(context, values, true, Apn, type, uri, SimId, Numeric);                        
        } else {
            insertAPN(context, values, type, SimId, uri, Numeric, ApnId);
            if(FeatureOption.MTK_GEMINI_SUPPORT){
                forTheOtherCard(context, SimId, values, Numeric, ApnId);   
            }
        }
    }

    
    /**
     * Add APN record to database
     */
    private void insertAPN(Context context, ContentValues values, String type, int SimId, Uri uri, String Numeric, String ApnId) {
        

        boolean isApnExisted = false;
        boolean isMmsApn = "mms".equalsIgnoreCase(type);
        
        OmacpApn mOmacpApn = new OmacpApn(context, SimId, uri, Numeric);
        ArrayList<HashMap<String, String>> omcpIdList = mOmacpApn.getExistOmacpId();
        int sizeApn = omcpIdList.size();
        for(int i = 0; i < sizeApn; i++){
            HashMap<String, String> map = omcpIdList.get(i);
            if(map.containsKey(ApnId)){
                
                isApnExisted = true;
                mResult = true;
                if (!isMmsApn) {
                    mApnToUseId = Long.parseLong(map.get(ApnId));
                }
                break;
            }//apnid matches
        }// end of loop:for
        
        if (isApnExisted != true) {
            
            long id = mOmacpApn.insert(context, values);
            if (id != -1) {
                mResult = true;
                if (!isMmsApn) {
                    mApnToUseId = id;
                    
                }
            }            
        }
    }
    
    public long getApnUserId() {
    	return mApnToUseId;
    }
    
    public boolean getResult() {
    	return mResult;
    }
    
    private boolean forTheOtherCard(Context context, int simId, ContentValues values, String Numeric, String ApnId){
        Xlog.d(TAG,"for the other card");
        
        int theOtherSimId = 1 - simId;
        Uri theOtherUri = null;
        
        switch (theOtherSimId) {
            case SIM_CARD_1:
            	theOtherUri = Telephony.Carriers.CONTENT_URI;
                break;
    
            case SIM_CARD_2:
            	theOtherUri = Telephony.Carriers.GeminiCarriers.CONTENT_URI;
                break;
        }
        if(theOtherUri == null) {
            return false;
        }
        OmacpApn theOtherOmacpApn = new OmacpApn(context, theOtherSimId, theOtherUri, Numeric);
        ArrayList<HashMap<String, String>> omcpIdList = theOtherOmacpApn.getExistOmacpId();
        int size = omcpIdList.size();
        for(int i = 0; i < size; i++){
            HashMap<String, String> map = omcpIdList.get(i);
            if(map.containsKey(ApnId)){
                Xlog.d(TAG,"The other card: this apn already exists!");
                return true;
            }//apnid matches
        }// end of loop:for
        
        long theOtherId = -1;
        theOtherId = theOtherOmacpApn.insert(context, values);
        Xlog.d(TAG,"The other id = " + theOtherId);
        
        return theOtherId == -1 ? false : true;
    }
    
	private boolean UpdateOperApn(Context context, final ContentValues values, boolean IsTheOther, String Apn, String type, Uri ApnUri, int SimId, String Numeric) {
	        
           boolean bSuc = false;
           if(!IsDefaultApn(Apn, type)) {                
               Xlog.d(TAG,"not oper APN");
               return false;
           }
           ContentResolver mContentResolver = context.getContentResolver();
           
           Uri uri = ApnUri;
           if (IsTheOther) {
               int theOtherSimId = 1 - SimId;    
               
               switch (theOtherSimId) {
                   case SIM_CARD_1:
                       uri = Telephony.Carriers.CONTENT_URI;
                       break;
           
                   case SIM_CARD_2:
                       uri = Telephony.Carriers.GeminiCarriers.CONTENT_URI;
                       break;
               }
           }
           String[] whereArgs = new String[]{Numeric,Apn};
           
           String where = Telephony.Carriers.NUMERIC+ "=?" + " AND "
           + Telephony.Carriers.APN+ "=?" + " AND "
           + Telephony.Carriers.SOURCE_TYPE + "=0";

           Xlog.d(TAG,where);

           Cursor cursor = mContentResolver.query(
                   uri, 
                   new String[] { Telephony.Carriers._ID, Telephony.Carriers.TYPE }, 
                   where, 
                   whereArgs, 
                   null);
           if(cursor == null) {
               Xlog.i(TAG, "query in updated cursor == null");
           }

           if(cursor != null){
               cursor.moveToFirst();
               while (!cursor.isAfterLast()) {

                   String typeTemp = cursor.getString(1);
                   if(((type==null||!type.equals(CU_MMS_TYPE))
                       &&(typeTemp==null||!typeTemp.equals(CU_MMS_TYPE)))
                       ||((type!=null&&type.equals(CU_MMS_TYPE))
                               &&(typeTemp!=null&&typeTemp.equals(CU_MMS_TYPE)))) {

                       Uri mOperUri = ContentUris.withAppendedId(uri, 
                               Integer.parseInt( cursor.getString(0)));
                       try {
                           values.put(sProjection[SOURCE_TYPE_INDEX], 0);
                           if (mContentResolver.update(mOperUri, values, null, null)<0) {
                               
                               values.put(sProjection[SOURCE_TYPE_INDEX], 2);
                               cursor.close();
                               return false;
                           }
                           if (!IsTheOther) {
                               mApnToUseId = Long.parseLong(cursor.getString(0));                              
                           }
                           bSuc = true;
                           mResult = true;

                          } catch (SQLException e) {
                              cursor.close();
                              return false;
                          }
                       }

                   cursor.moveToNext();
               }// end of while
               cursor.close();
               Xlog.e(TAG, "cursor end");               
           } else {
               return false;
       }
           Xlog.e(TAG, "return bSuc = " +bSuc); 
           return bSuc;
   }
	
    private boolean IsDefaultApn(String Apn, String type) {

        if (((Apn!=null)&&(Apn.equals(CU_3GNET_NAME)))
            &&((type==null)||(!type.equals(CU_MMS_TYPE)))) {

            return true;
        } else if ((Apn!=null)&&(Apn.equals(CU_3GWAP_NAME))) {

            return true;
        }               
        return false;
            
    }
}
