# limitations under the License.
#

# This makefile shows how to build your own shared library that can be
# shipped on the system of a phone, and included additional examples of
# including JNI code with the library and writing client applications against it.

LOCAL_PATH := $(call my-dir)

# MediaTek op library.
# =============================================================
include $(CLEAR_VARS)

LOCAL_MODULE := mediatek-op

LOCAL_SRC_FILES := $(call all-java-files-under,java)

LOCAL_JAVA_LIBRARIES := mediatek-framework

include $(BUILD_JAVA_LIBRARY)
