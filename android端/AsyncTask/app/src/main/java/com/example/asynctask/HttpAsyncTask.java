package com.example.asynctask;

import android.content.Context;
import android.os.AsyncTask;
import android.util.Log;
import android.widget.Toast;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

public class HttpAsyncTask extends AsyncTask<String, Void, String> {
    private Context context;

    /**
     * 自定义Dialog监听器
     */
    public interface PriorityListener {
        /**
         * 回调函数，用于在Dialog的监听事件触发后刷新Activity的UI显示
         */
        void setActivity(int code);
    }

    private PriorityListener listener;

    public HttpAsyncTask(Context context, PriorityListener listener) {
        this.context = context;
        this.listener = listener;
    }

    @Override
    protected String doInBackground(String... params) {
        String path = params[0];
        String data = params[1];
        String content = null;
        try {
            // 创建一个URL对象，参数就是网址
            URL url = new URL(path);
            // 获取HttpURLConnection连接对象
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            // 默认请求是get，要大写
            conn.setRequestMethod("POST");
            // 设置网络连接的超时时间
            conn.setConnectTimeout(5000);
            // 设置两个请求头信息
            conn.setRequestProperty("Content-Type",
                    "application/x-www-form-urlencoded");
            conn.setRequestProperty("Content-Length", data.length() + "");

            // 把组拼好的数据提交给服务器，以流的形式提交
            conn.setDoOutput(true);
            conn.getOutputStream().write(data.getBytes());

            // 获取服务器返回的状态码,200代表获取服务器资源全部成功，206请求部分资源
            int code = conn.getResponseCode();
            if (code == 200) {
                InputStream in = conn.getInputStream();
                content = readStream(in);
                Log.e("SendAsyncTask", content);
            }

        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return content;
    }

    @Override
    protected void onPostExecute(String result) {
        if (result != null) {
            try {
                JSONObject jsonObject = new JSONObject(result);
                int code = jsonObject.getInt("resultCode");

                if (code != 0) {
                    listener.setActivity(code);
                } else {
                    Toast.makeText(context, "数据类型不正确", Toast.LENGTH_SHORT).show();
                }
            } catch (JSONException e) {
                e.printStackTrace();
            }
        } else {
            Toast.makeText(context, "网络连接错误", Toast.LENGTH_SHORT).show();
        }

        super.onPostExecute(result);
    }

    public String readStream(InputStream in) throws Exception {
        //定义一个内存输出流
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        int len = -1;
        byte[] buffer = new byte[1024];
        while ((len = in.read(buffer)) != -1) {
            baos.write(buffer, 0, len);
        }
        in.close();
        String content = new String(baos.toByteArray());
        return content;
    }

}