package com.mediatek.camera.ext;

import com.mediatek.camera.ext.CameraExtension;
import com.mediatek.camera.ext.ICameraExtension;
import com.mediatek.camera.ext.ISavingPath;


public class Op02CameraExtension extends CameraExtension {
    private static final String TAG = "Op02CameraExtension";
    private static final boolean LOG = true;

    @Override
    public ISavingPath getSavingPath() {
        return new SavingPathImpl();
    }

    class SavingPathImpl implements ISavingPath {
    	public String getDCIM(String mountPoint, boolean stillCapture) {
    		if (stillCapture) {
    			return mountPoint + "/Photo";
    		} else {
    			return mountPoint + "/Video";
    		}
    	}

    	public String getDirectory(String mountPoint, boolean stillCapture) {
    		if (stillCapture) {
    			return mountPoint + "/Photo";
    		} else {
    			return mountPoint + "/Video";
    		}
    	}

    	public String getDirectory3D(String mountPoint, boolean stillCapture) {
    		if (stillCapture) {
    			return mountPoint + "/Photo";
    		} else {
    			return mountPoint + "/Video";
    		}
    	}

	public boolean isMultiDirectory() {
	    return true;
	}
    }
}
