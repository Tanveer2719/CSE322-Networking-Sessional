package tcpforward;

import util.NetworkUtil;

import java.io.IOException;
import java.util.Scanner;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

public class ReadThreadClient implements Runnable {
    private Thread thr;
    private NetworkUtil networkUtil;
    private BlockingQueue<Message> messageQueue;
    private BlockingQueue<Message> fileUploadBlockingQueue;
    private String name;

    public ReadThreadClient(NetworkUtil networkUtil,String clientName, Scanner scanner, BlockingQueue<Message> messageQueue) {
        this.networkUtil = networkUtil;
        this.messageQueue = messageQueue;
        this.name = clientName;
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
                        if(obj.getTo().equalsIgnoreCase(name)){
                            if(obj.getHeader() != null && obj.getHeader().equalsIgnoreCase("CONFIRMATION"))
                                fileUploadBlockingQueue.put(obj);
                            else if(obj.getHeader() != null)
                                messageQueue.put(obj);
                        }
                    }
                    else if(o instanceof String){
                        // for the error message during login
                        String s = (String) o;
                        System.out.println("Message From server: "+ s);
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

    public void setWriter(WriteThreadClient writeClient) {
    }


    public BlockingQueue<Message> setBQ() {
        fileUploadBlockingQueue = new ArrayBlockingQueue<>(10);
        return fileUploadBlockingQueue;
    }
}



