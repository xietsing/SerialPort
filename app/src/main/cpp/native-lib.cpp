#include <jni.h>
#include <string.h>

#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <pthread.h>

#include "android/log.h"

#include "MFRC522.h"

static const char *TAG="serial_port";

#define LOGE(msg, ...)	__android_log_print(ANDROID_LOG_ERROR, TAG, msg, ##__VA_ARGS__)
#define LOGI(msg, ...)	__android_log_print(ANDROID_LOG_WARN, TAG, msg, ##__VA_ARGS__)
#define LOGD(msg, ...)	__android_log_print(ANDROID_LOG_DEBUG, TAG, msg, ##__VA_ARGS__)


#ifndef NELEM
# define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

static const char *gClassName = "com/xmlenz/deviceport/DevicePort";

static bool haveuid= false;

MFRC522 *mfrc522;


jstring   stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++ asdfsdfads";
    return env->NewStringUTF(hello.c_str());
}


static speed_t getBaudrate(jint baudrate)
{
    switch(baudrate) {
        case 0: return B0;
        case 50: return B50;
        case 75: return B75;
        case 110: return B110;
        case 134: return B134;
        case 150: return B150;
        case 200: return B200;
        case 300: return B300;
        case 600: return B600;
        case 1200: return B1200;
        case 1800: return B1800;
        case 2400: return B2400;
        case 4800: return B4800;
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        case 460800: return B460800;
        case 500000: return B500000;
        case 576000: return B576000;
        case 921600: return B921600;
        case 1000000: return B1000000;
        case 1152000: return B1152000;
        case 1500000: return B1500000;
        case 2000000: return B2000000;
        case 2500000: return B2500000;
        case 3000000: return B3000000;
        case 3500000: return B3500000;
        case 4000000: return B4000000;
        default: return -1;
    }
}

/*
 * Class:     android_serialport_SerialPort
 * Method:    open
 * Signature: (Ljava/lang/String;II)Ljava/io/FileDescriptor;
 */

JNIEXPORT jobject JNICALL SerialPort_open
        (JNIEnv *env, jclass thiz, jstring path, jint baudrate, jint flags)
{
    int fd;
    speed_t speed;
    jobject mFileDescriptor;

    /* Check arguments */
    {
        speed = getBaudrate(baudrate);
        if (speed == -1) {
            /* TODO: throw an exception */
            LOGE("Invalid baudrate");
            return NULL;
        }
    }

    /* Opening device */
    {
        jboolean iscopy;
//        flags =  O_NOCTTY | O_NONBLOCK | O_NDELAY;
        const char *path_utf = env->GetStringUTFChars(path, &iscopy);
        LOGD("Opening serial port %s with flags 0x%x", path_utf, O_RDWR | flags);
        fd = open(path_utf, O_RDWR | flags);
        LOGD("open() fd = %d", fd);
        env->ReleaseStringUTFChars(path, path_utf);
        if (fd == -1)
        {
            /* Throw an exception */
            LOGE("Cannot open port");
            /* TODO: throw an exception */
            return NULL;
        }
    }

    /* Configure device */
    {
        struct termios cfg;
        LOGD("Configuring serial port");
        if (tcgetattr(fd, &cfg))
        {
            LOGE("tcgetattr() failed");
            close(fd);
            /* TODO: throw an exception */
            return NULL;
        }

        cfmakeraw(&cfg);
        cfsetispeed(&cfg, speed);
        cfsetospeed(&cfg, speed);

        if (tcsetattr(fd, TCSANOW, &cfg))
        {
            LOGE("tcsetattr() failed");
            close(fd);
            /* TODO: throw an exception */
            return NULL;
        }
    }

    /* Create a corresponding file descriptor */
    {
//        jclass cFileDescriptor = env->FindClass( "java/io/FileDescriptor");
//        jmethodID iFileDescriptor = env->GetMethodID(cFileDescriptor, "<init>", "()V");
//        jfieldID descriptorID = env->GetFieldID( cFileDescriptor, "descriptor", "I");
//        mFileDescriptor = env->NewObject(cFileDescriptor, iFileDescriptor);
//        env->SetIntField(mFileDescriptor, descriptorID, (jint)fd);
        jclass cFileDescriptor = env->FindClass("java/io/FileDescriptor");
        jmethodID iFileDescriptor = env->GetMethodID(cFileDescriptor,"<init>", "()V");
        jfieldID descriptorID = env->GetFieldID(cFileDescriptor,"descriptor", "I");
        mFileDescriptor = env->NewObject(cFileDescriptor,iFileDescriptor);
        env->SetIntField(mFileDescriptor, descriptorID, (jint) fd);
    }

    return mFileDescriptor;
}

/*
 * Class:     cedric_serial_SerialPort
 * Method:    close
 * Signature: ()V
 */
JNIEXPORT void JNICALL SerialPort_close
        (JNIEnv *env, jobject thiz)
{
    jclass SerialPortClass = env->GetObjectClass(thiz);
    jclass FileDescriptorClass = env->FindClass("java/io/FileDescriptor");

    jfieldID mFdID = env->GetFieldID(SerialPortClass, "mFd", "Ljava/io/FileDescriptor;");
    jfieldID descriptorID = env->GetFieldID(FileDescriptorClass, "descriptor", "I");

    jobject mFd = env->GetObjectField(thiz, mFdID);
    jint descriptor = env->GetIntField(mFd, descriptorID);

    LOGD("close(fd = %d)", descriptor);
    close(descriptor);
}


JNIEXPORT jint JNICALL I2c_open(JNIEnv *env, jobject obj, jstring file)
{
    char fileName[64];

    const char *str;
    str =  env->GetStringUTFChars(file, NULL);
    if (str == NULL) {
        LOGI("Can't get file name!");
        return -1;
    }
    sprintf(fileName, "%s", str);
    LOGI("will open i2c device node %s", fileName);
    env->ReleaseStringUTFChars(file, str);
    return open(fileName, O_RDWR);
}

JNIEXPORT jint JNICALL I2c_read (JNIEnv * env, jobject obj, jint fileHander, jint slaveAddr, jintArray bufArr, jint len)
{
    jint *bufInt;
    char *bufByte;
    int res = 0, i = 0, j = 0;
    if (len <= 0) {
//        LOGE("I2C: buf len <=0");
        goto err0;
    }
    bufInt = (jint *) malloc(len * sizeof(int));
    if (bufInt == 0) {
        LOGE("I2C: nomem");
        goto err0;
    }
    bufByte = (char*) malloc(len);
    if (bufByte == 0) {
        LOGE("I2C: nomem");
        goto err1;
    }
    env->GetIntArrayRegion(bufArr, 0, len, bufInt);
    res = ioctl(fileHander, I2C_SLAVE, slaveAddr);
    if (res != 0) {
        LOGE("I2C: Can't set slave address");
        goto err2;
    }
    memset(bufByte, '\0', len);
    if ((j = read(fileHander, bufByte, len)) != len) {
        LOGE("read fail in i2c read jni i = %d buf 4", i);
        goto err2;
    } else {
        for (i = 0; i < j ; i++)
            bufInt[i] = bufByte[i];
        LOGI("return %d %d %d %d in i2c read jni", bufByte[0], bufByte[1], bufByte[2], bufByte[3]);
        (env)->SetIntArrayRegion(bufArr, 0, len, bufInt);
    }
    free(bufByte);
    free(bufInt);
    return j;
    err2:
    free(bufByte);
    err1:
    free(bufInt);
    err0:
    return -1;
}

int  I2c_write(JNIEnv *env, jobject obj, jint fileHander, jint slaveAddr, jint mode,jintArray bufArr,jint len)//
{

    jint *bufInt;
    char *bufByte;
    int res = 0, i = 0, j = 0;
    if (len <= 0) {
        LOGE("I2C: buf len <=0");
        goto err0;
    }
    bufInt = (jint *) malloc(len * sizeof(int));
    if (bufInt == 0) {
        LOGE("I2C: nomem");
        goto err0;
    }
    bufByte = (char*) malloc(len + 1);
    if (bufByte == 0) {
        LOGE("I2C: nomem");
        goto err1;
    }
    (env)->GetIntArrayRegion(bufArr, 0, len, bufInt);
    bufByte[0] = mode;
    for (i = 0; i < len; i++)
        bufByte[i + 1] = bufInt[i];
    res = ioctl(fileHander, I2C_SLAVE, slaveAddr);
    if (res != 0) {
        LOGE("I2C: Can't set slave address");
        goto err2;
    }
    if ((j = write(fileHander, bufByte, len + 1)) != len + 1) {
        LOGE("write fail in i2c");
        goto err2;
    }
    LOGI("I2C: write %d byte", j);
    free(bufByte);
    free(bufInt);
    return j - 1;

    err2:
    free(bufByte);

    err1:
    free(bufInt);

    err0:
    return -1;

}

void  I2c_close(JNIEnv *env, jobject obj, jint fileHander)
{
    close(fileHander);

}

void * pthread(void *arg)
{
    while (1)
    {
        sleep(10);
        if ( ! mfrc522->PICC_IsNewCardPresent()) {
            haveuid= false;
            continue;
        }
//     Select one of the cards
        if ( ! mfrc522->PICC_ReadCardSerial()) {
            haveuid= false;
            continue;
        }
        haveuid=true;
    }
}

static jbyteArray byarray;
//static jbyteArray byarray22;
void rc522_open(JNIEnv *env, jobject obj)
{

//    byarray = env->NewByteArray(1);
//    byarray22 = env->NewByteArray(4);
//    memset(byarray,0,1);
//    memset(byarray22,0,4);
    mfrc522 = new MFRC522(0);//
    mfrc522->PCD_Init();
    mfrc522->PCD_DumpVersionToSerial();
//    pthread_t tidp;
//    pthread_create(&tidp, NULL, pthread, (void *) 1);


}





JNIEXPORT jbyteArray JNICALL rc522_read(JNIEnv *env, jobject obj) {
//    mfrc522->PCD_Reset();
//    for (int i=0;i<5;i++){
    if ( ! mfrc522->PICC_IsNewCardPresent()) {
//        jbyteArray byarray = env->NewByteArray(1);
        return byarray;
    }
//     Select one of the cards
    if ( ! mfrc522->PICC_ReadCardSerial()) {
//        jbyteArray byarray = env->NewByteArray(1);
        return byarray;
    }


//    mfrc522->PICC_DumpToSerial(&(mfrc522->uid));
    haveuid=true;
    if (haveuid) {
        int size = mfrc522->uid.size;
        LOGD("size%d",size);
        jbyteArray byarray = env->NewByteArray(size);
        jsize len = size;
//        jbyte *jbarray = (jbyte *) malloc(len * sizeof(jbyte));
//        memset(byarray2, 0, sizeof(byarray2));
//        memcpy(byarray2, mfrc522->uid.uidByte, mfrc522->uid.size);



        env->SetByteArrayRegion(byarray, 0, mfrc522->uid.size, (jbyte *)mfrc522->uid.uidByte);
        return byarray;

    }
//    jbyteArray byarray = env->NewByteArray(1);
    return byarray;



}
//
//==================================================
static JNINativeMethod gMethods[] = {
        {"stringFromJNI",	"()Ljava/lang/String;",	(void *)stringFromJNI},
        {"SerialPort_open",	"(Ljava/lang/String;II)Ljava/io/FileDescriptor;",	(void *)SerialPort_open},
        {"SerialPort_close","()V",	(void *)SerialPort_close},
        {"I2c_open",        "(Ljava/lang/String;)I",	(void *)I2c_open},
        {"I2c_write",      "(III[II)I",	(void *)I2c_write },
        {"I2c_read",       "(II[II)I",	(void *)I2c_read },
        {"I2c_close",       "(I)V",	(void *)I2c_close},


//        {"rc522_reset",      "()V",	(void *)rc522_reset},
        {"rc522_open",      "()V",	(void *)rc522_open},
        {"rc522_read",      "()[B",	(void *)rc522_read }


};


static jint registerNativeMethods(JNIEnv* env, const char* className, JNINativeMethod* methods, int numMethods) {

    jclass clazz;

    clazz = env->FindClass(className);

    if(env->ExceptionCheck()){
        env->ExceptionDescribe();
        env->ExceptionClear();
    }

    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }

    if (env->RegisterNatives(clazz, methods, numMethods) < 0) {
        LOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static jint registerNatives(JNIEnv* env){

    jint ret = JNI_FALSE;

    if (registerNativeMethods(env, gClassName, gMethods, NELEM(gMethods))){
        ret = JNI_TRUE;
    }

    return ret;
}

void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved) {
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed\n");
        goto bail;
    }

    assert(env != NULL);

    if (registerNatives(env) != JNI_TRUE) {
        LOGE("ERROR: registerNatives failed");
        goto bail;
    }

    result = JNI_VERSION_1_4;
    bail:
    return result;
}
