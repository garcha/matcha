// +build matcha,android

#include "matchaforeign.h"
#include "matchaforeign-java.h"
#include <jni.h>
#include <stdlib.h>
#include <android/log.h>
#include <stdint.h>

JavaVM *sJavaVM;
JNIEnv *sEnv;
jint sJavaVersion;
jobject sTracker;

#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "TAG", __VA_ARGS__);

CGoBuffer MatchaStringToCGoBuffer(JNIEnv *env, jstring v) {
    const char *nativeString = (*env)->GetStringUTFChars(env, v, 0);
    
    int len = strlen(nativeString);
    char *buf = (char *)malloc(len);
    strncpy(buf, nativeString, len);
    
    (*env)->ReleaseStringUTFChars(env, v, nativeString);
   
    CGoBuffer cstr;
    cstr.ptr = buf;
    cstr.len = len;
    return cstr;
}

jstring MatchaCGoBufferToString(JNIEnv *env, CGoBuffer buf) {
    char *str = malloc(buf.len+1);
    strncpy(str, buf.ptr, buf.len);
    str[buf.len] = '\0';
    
    jstring jstrBuf = (*sEnv)->NewStringUTF(sEnv, str);
    free(buf.ptr);
    free(str);
    return jstrBuf;
}

CGoBuffer MatchaByteArrayToCGoBuffer(JNIEnv *env, jbyteArray v) {
    // int len = [data length];
    // if (len == 0) {
    //     return (CGoBuffer){0};
    // }
    
    int len = (*env)->GetArrayLength(env, v);
    char *buf = (char *)malloc(len);
    (*env)->GetByteArrayRegion(env, v, 0, len, (jbyte *)buf);
  
    CGoBuffer cstr;
    cstr.ptr = buf;
    cstr.len = len;
    return cstr;
}

jbyteArray MatchaCGoBufferToByteArray(JNIEnv *env, CGoBuffer buf) {
    jbyteArray array = (*env)->NewByteArray(env, buf.len);
    (*env)->SetByteArrayRegion(env, array, 0, buf.len, buf.ptr);
    free(buf.ptr);
    return array;
}

ObjcRef MatchaForeignBridge(CGoBuffer str) {
    jstring *string = MatchaCGoBufferToString(sEnv, str);
    
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignBridge", "(Ljava/lang/String;)J");
    return (*sEnv)->CallLongMethod(sEnv, sTracker, mid, string);
}

ObjcRef MatchaObjcBool(bool v) {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignBool", "(Z)J");
    return (*sEnv)->CallLongMethod(sEnv, sTracker, mid, v);
}

bool MatchaObjcToBool(ObjcRef v) {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignToBool", "(J)Z");
    return (*sEnv)->CallBooleanMethod(sEnv, sTracker, mid, v);
}

ObjcRef MatchaObjcInt64(int64_t v) {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignInt64", "(J)J");
    return (*sEnv)->CallLongMethod(sEnv, sTracker, mid, v);
}

int64_t MatchaObjcToInt64(ObjcRef v) {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignToInt64", "(J)J");
    return (*sEnv)->CallLongMethod(sEnv, sTracker, mid, v);
}

ObjcRef MatchaObjcFloat64(double v) {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignFloat64", "(D)J");
    return (*sEnv)->CallLongMethod(sEnv, sTracker, mid, v);
}

double MatchaObjcToFloat64(ObjcRef v) {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignToFloat64", "(J)D");
    return (*sEnv)->CallDoubleMethod(sEnv, sTracker, mid, v);
}

ObjcRef MatchaObjcGoRef(GoRef v) {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignGoRef", "(J)J");
    return (*sEnv)->CallLongMethod(sEnv, sTracker, mid, v);
}

GoRef MatchaObjcToGoRef(ObjcRef v) {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignToGoRef", "(J)J");
    return (*sEnv)->CallLongMethod(sEnv, sTracker, mid, v);
}

ObjcRef MatchaObjcString(CGoBuffer buf) {
    jstring jstrBuf = MatchaCGoBufferToString(sEnv, buf);
    
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignString", "(Ljava/lang/String;)J");
    long a = (*sEnv)->CallLongMethod(sEnv, sTracker, mid, jstrBuf);
    
    (*sEnv)->DeleteLocalRef(sEnv, jstrBuf);
    return a;
}

CGoBuffer MatchaObjcToString(ObjcRef v) {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignToString", "(J)Ljava/lang/String;");
    jstring str = (jstring)(*sEnv)->CallObjectMethod(sEnv, sTracker, mid, v);
    
    return MatchaStringToCGoBuffer(sEnv, str);
}

ObjcRef MatchaObjcBytes(CGoBuffer bytes) {
    jbyteArray array = MatchaCGoBufferToByteArray(sEnv, bytes);
    
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignBytes", "([B)J");
    long a = (*sEnv)->CallLongMethod(sEnv, sTracker, mid, array);
    
    (*sEnv)->DeleteLocalRef(sEnv, array);
    return a;
}

CGoBuffer MatchaObjcToBytes(ObjcRef v) {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignToBytes", "(J)[B");
    jbyteArray str = (jbyteArray)(*sEnv)->CallObjectMethod(sEnv, sTracker, mid, v);
    
    return MatchaByteArrayToCGoBuffer(sEnv, str);
}

ObjcRef MatchaObjcArray(int64_t len) {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignArray", "(I)J");
    return (*sEnv)->CallLongMethod(sEnv, sTracker, mid, len);
}

int64_t MatchaObjcArrayLen(ObjcRef v) {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignArrayLen", "(J)J");
    int64_t x = (*sEnv)->CallLongMethod(sEnv, sTracker, mid, v);
    return x;
}

void MatchaObjcArraySet(ObjcRef v, ObjcRef a, int64_t idx) {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignArraySet", "(JJI)V");
    (*sEnv)->CallVoidMethod(sEnv, sTracker, mid, v, a, idx);
}

ObjcRef MatchaObjcArrayAt(ObjcRef v, int64_t idx) {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignArrayAt", "(JI)J");
    return (*sEnv)->CallLongMethod(sEnv, sTracker, mid, v, idx);
}

ObjcRef MatchaObjcCallSentinel() {
    // Not necessary on android.
    return 0;
}

ObjcRef MatchaObjcCall(ObjcRef v, CGoBuffer str, ObjcRef args) {
    jstring method = MatchaCGoBufferToString(sEnv, str);
    
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignCall", "(JLjava/lang/String;J)J");
    return (*sEnv)->CallLongMethod(sEnv, sTracker, mid, v, method, args);
}

void MatchaForeignPanic() {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "foreignPanic", "()V");
    return (*sEnv)->CallVoidMethod(sEnv, sTracker, mid);
}

// Tracker
ObjcRef MatchaTrackObjc(jobject v) {
    jclass cls = (*sEnv)->GetObjectClass(sEnv, sTracker);
    jmethodID mid = (*sEnv)->GetMethodID(sEnv, cls, "track", "(Ljava/lang/Object;)J");
    return (*sEnv)->CallLongMethod(sEnv, sTracker, mid, v);
}

// id MatchaGetObjc(ObjcRef key) {
// }

void MatchaUntrackObjc(ObjcRef key) {
    JNIEnv *env = NULL;
    jint success = (*sJavaVM)->GetEnv(sJavaVM, (void **)&env, sJavaVersion);
    if (success == JNI_EDETACHED) {
        (*sJavaVM)->AttachCurrentThread(sJavaVM, &env, NULL);
    }

    jclass cls = (*env)->GetObjectClass(env, sTracker);
    jmethodID mid = (*env)->GetMethodID(env, cls, "untrack", "(J)V");
    (*env)->CallVoidMethod(env, sTracker, mid, key);
    (*env)->DeleteLocalRef(env, cls);
}
