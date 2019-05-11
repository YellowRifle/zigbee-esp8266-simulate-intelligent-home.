package utils;

public class ToolUtils {
    public static byte[] stringToByte(String str) {
        byte[] bytes = null;
        if (str != null) {
            // 去掉头尾的中括号
            String str1 = str.replace("[", "").replace("]", "");
            // 以逗号分割每个字符，生成新的字符数组
            String[] str_msg = str1.split(",");
            bytes = new byte[str_msg.length];
            // 强制转换并生成byte数组
            for (int i = 0; i < str_msg.length; i++) {
                int msg = Integer.valueOf(str_msg[i].trim());
                bytes[i] = (byte) msg;
            }

        }
        return bytes;
    }

    public int[] bytesToInt(byte[] src) {
        int value[] = new int[src.length];
        for (int i = 0; i < src.length; i++) {
            value[i] = src[i] & 0xFF;
        }
        return value;
    }
}