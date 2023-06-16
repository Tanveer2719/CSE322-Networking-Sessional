package tcpforward;

import util.NetworkUtil;

import java.io.IOException;
import java.util.HashMap;
import java.util.concurrent.BlockingQueue;


public class ReadThreadServer implements Runnable {
    private Thread thr;
    private NetworkUtil networkUtil;
    public HashMap<String, NetworkUtil> clientMap;
    private BlockingQueue<Message>messageQueue;
    
    public ReadThreadServer(HashMap<String, NetworkUtil> map, NetworkUtil networkUtil ,BlockingQueue<Message>messageQueue) {
        this.clientMap = map;
        this.networkUtil = networkUtil;
        this.messageQueue = messageQueue;
        this.thr = new Thread(this);
        thr.start();
    }

    public synchronized void run() {
        try {
            while (true) {
                Object o = networkUtil.read();
                if (o != null) {
                    if (o instanceof Message) {
                        Message obj = (Message) o;
                        messageQueue.put(obj);
                    }
                }
            }
        } catch (Exception e) {
            System.out.println(e);
        } finally {
            try {
                networkUtil.closeConnection();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public void setWriter(WriteThreadServer serverWrite) {
    }


}



