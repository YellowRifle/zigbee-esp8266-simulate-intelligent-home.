package servlet;

import org.json.JSONException;
import org.json.JSONObject;
import service.WifiServerSocket;
import utils.ToolUtils;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.io.PrintWriter;

public class ApplicationServlet extends HttpServlet {

    /**
     *
     */
    private static final long serialVersionUID = -582634537189366787L;

    @Override
    public void doGet(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        //让doGet请求也归类为doPost请求
        doPost(request, response);
    }

    @Override
    public void doPost(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {

        response.setContentType("text/html;charset=UTF-8");
        //获取doPost请求中的传过来的数据
        String userName = request.getParameter("userName");
        String sessionId = request.getParameter("sessionId");
        String data = request.getParameter("data");

        byte[] msg = ToolUtils.stringToByte(data);
        System.out.println(msg);

        JSONObject jObject = new JSONObject();
        if (sessionId != null) {
            // TODO 将获取的数据打印出来
            System.out.println("userName:" + userName);
            System.out.println("sessionId:" + sessionId);
            System.out.println("data:" + data);
            WifiServerSocket.ProcessSocketData psd = WifiServerSocket.getSocketMap().get(
                    new String(sessionId));
            if (psd != null){
                psd.send(msg);
                System.out.println("数据已经发送到8266");
                try {
                    //返回json数据
                    JSONObject record = new JSONObject();
                    record.put("userName", userName);

                    jObject.put("reason", "SUCCESSED");
                    jObject.put("resultCode", 200);
                    jObject.put("totalNum", 1);
                    jObject.put("data", record);
                } catch (JSONException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }

            }else {
                System.out.println("socket为空,8266未连接服务器");
                try {
                    //返回json数据
                    JSONObject record = new JSONObject();
                    record.put("userName", userName);

                    jObject.put("reason", "SUCCESSED");
                    jObject.put("resultCode", 202);
                    jObject.put("totalNum", 0);
                    jObject.put("data", record);
                } catch (JSONException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }

        } else {
            // 未接收到该设备的id
            try {
                jObject.put("resultCode", 204);
                jObject.put("reason", "NULL");
                jObject.put("data", "");
            } catch (JSONException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                try {
                    jObject.put("resultCode", 400);
                    jObject.put("reason", "ERROR");
                    jObject.put("data", "");

                } catch (JSONException ex) {
                    // TODO: handle exception
                    ex.printStackTrace();
                }
            }
        }
        PrintWriter out = response.getWriter();
        out.print(jObject);
        out.flush();
        out.close();

    }
}