package android.nfc.api;

public class NFC {

    public NFC( ) {
    }

    public native void close();

    public native  static int open(String path);
    public native  static int init();
    public native  static String readCard();
    static {
        System.loadLibrary("nfc_fm175xx");
    }
}
