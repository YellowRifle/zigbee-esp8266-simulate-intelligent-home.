package com.example.asynctask;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import java.util.Arrays;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    private Button btnBlink;

    private Button btnOn;

    private Button btnOff;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        btnBlink = findViewById(R.id.Blink);
        btnBlink.setOnClickListener(this);

        btnOn = findViewById(R.id.On);
        btnOn.setOnClickListener(this);

        btnOff = findViewById(R.id.Off);
        btnOff.setOnClickListener(this);

    }

    @Override
    public void onClick(View view) {
        String name = "layne";
        String sessionId = "CONN_9527";
        switch (view.getId()){
            case R.id.Blink:
                byte[] msg1 = new byte[]{(byte) 0x01, (byte) 0x00, (byte) 0x00};

                //发出请求
                serviceCONN(msg1,name,sessionId);
                break;
            case R.id.Off:
                byte[] msg2 = new byte[]{(byte) 0x00 , (byte) 0x01, (byte) 0x00};

                //发出请求
                serviceCONN(msg2,name,sessionId);
                break;
            case R.id.On:
                byte[] msg3 = new byte[]{(byte) 0x00, (byte) 0x00, (byte) 0x01};

                //发出请求
                serviceCONN(msg3,name,sessionId);
                break;
        }
    }

    private void serviceCONN(byte[] bytes, String username, String sessionId) {
        //将byte数组转化成字符串
        String data_msg = Arrays.toString(bytesToInt(bytes));
        //服务器的url
        String url = "http://你的服务器地址:8080/zigbeeTwo/application";
        //将数据拼接起来
        String data = "userName=" + username + "&sessionId=" + sessionId + "&data=" + data_msg;
        String[] str = new String[]{url, data};

        //发出一个请求
        new HttpAsyncTask(MainActivity.this, new HttpAsyncTask.PriorityListener() {

            @Override
            public void setActivity(int code) {
                switch (code) {
                    case 200:
                        //如果返回的resultCode是200,那么说明APP的数据传送成功，并成功解析返回的json数据
                        Toast.makeText(MainActivity.this, "发送数据成功", Toast.LENGTH_SHORT).show();
                        break;
                    case 202:
                        Toast.makeText(MainActivity.this, "设备离线状态", Toast.LENGTH_SHORT).show();
                        break;
                    default:
                        Toast.makeText(MainActivity.this, "网络传输异常", Toast.LENGTH_SHORT).show();
                        break;
                }

            }
        }).execute(str);
    }

    /**
     * byte转化为int
     */
    public static int[] bytesToInt(byte[] src) {
        int value[] = new int[src.length];
        for (int i = 0; i < src.length; i++) {
            value[i] = src[i] & 0xFF;
        }
        return value;
    }
}
