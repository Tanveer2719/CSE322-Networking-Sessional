package tcpforward;

import util.NetworkUtil;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Scanner;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.BlockingQueue;


public class WriteThreadClient implements Runnable {

    private Thread thr;
    private NetworkUtil networkUtil;
    private String name;
    private ReadThreadClient readClient;
    private Scanner input;
    private BlockingQueue<Message> messageQueue;
    private BlockingQueue<Message> fileUploadBlockingQueue;
    private Timer timer;
    private boolean timesUP = false;
    

    public WriteThreadClient(NetworkUtil networkUtil, String name, Scanner scanner,BlockingQueue<Message> messageQueue) {
        this.networkUtil = networkUtil;
        this.name = name;
        this.messageQueue = messageQueue;
        input = scanner;
        this.thr = new Thread(this);
        thr.start();
    }

    

    public synchronized void run() {

        try {
            while (true) {
                Message s = messageQueue.take();
                
                if(s.getHeader().equalsIgnoreCase("WELCOME_POP_INPUT")){
                    System.out.println(s.getText());
                    int x = takeInput(1, 9);
                    Message m = createMessage(String.valueOf(x), "WELCOME_POP_INPUT");
                    networkUtil.write(m);
                    
                }

                else if(s.getHeader().equalsIgnoreCase("USER_LIST")){
                    System.out.println(s.getText());
                }

                else if(s.getHeader().equalsIgnoreCase("UPLOAD_TYPE_SELECTION")){
                    System.out.println(s.getText());
                    int x = takeInput(1, 2);
                    Message m = createMessage(String.valueOf(x), "UPLOAD_TYPE_SELECTION");
                    m.setFileId(s.getFileId());
                    networkUtil.write(m);
                }
                
                else if(s.getHeader().equalsIgnoreCase("WRITE_FILE_NAME")){
                    System.out.println(s.getText());
                    String fileName = input.nextLine();
                    Message m = createMessage(fileName, "WRITE_FILE_NAME");
                    m.setFileId(s.getFileId());
                    m.setReqId(s.getReqId());
                    networkUtil.write(m);
                }

                else if(s.getHeader().equalsIgnoreCase("WRITE_FILE_SIZE")){
                    System.out.println(s.getText());
                    Long fileSize = input.nextLong();
                    Message m = createMessage(String.valueOf(fileSize),"WRITE_FILE_SIZE");
                    m.setFileId(s.getFileId());
                    m.setReqId(s.getReqId());
                    networkUtil.write(m);
                }
                
                else if(s.getHeader().equalsIgnoreCase("UPLOAD_FILE")){
                    System.out.println(s.getText());
                    String dir = "src/tcpforward/Files/NewStudent.txt";
                    FileInputStream fis = new FileInputStream(dir);
                    fileUploadBlockingQueue = readClient.setBQ();
                    
                    byte[] buffer = new byte[s.getChunkSize()];

                    
                    String fileId = s.getFileId();

                    while (true) {
                        int n = fis.read(buffer);
                        if(n == -1){
                            break;
                        }
                        
                        String bufferToString = new String(buffer,StandardCharsets.UTF_8);
                        Message m = new Message();
                        m.setFrom(name);
                        m.setBytesRead(n);
                        m.setTo("Server");
                        m.setText(bufferToString);
                        m.setHeader("UPLOAD_FILE");
                        m.setFileId(fileId);
                        m.setReqId(s.getReqId());
                        networkUtil.write(m);
                        
                        startTimer(fileId); // start the timer
                        // wait for the confirmation
                        
                        if(timesUP) break;
                        Message response = fileUploadBlockingQueue.take();
                        
                        if(response.getFrom().equalsIgnoreCase(s.getFrom()) && response.getHeader().equalsIgnoreCase("CONFIRMATION") && response.getText().equalsIgnoreCase("OK") && !timesUP){
                            stopTimer();
                        }else{
                            timesUP = true;
                            sendErrorMessage(fileId);
                            break;
                        }
                    }


                    fis.close();

                    if(!timesUP){
                        Message message = new Message();
                        message.setFileId(fileId);
                        message.setFrom(name);
                        message.setTo("Server");
                        message.setText("");
                        message.setHeader("UPLOAD_COMPLETE");
                        message.setReqId(s.getReqId());
                        networkUtil.write(message);
                    }
                    
                }
                
                else if(s.getHeader().equalsIgnoreCase("VIEW_INBOX")){
                    System.out.println(s.getText());
                }
                
                else if(s.getHeader().equalsIgnoreCase("FILE_REQ")){
                    System.out.println(s.getText());
                    String name = input.nextLine();
                    Message m = createMessage(name, "FILE_NAME");
                    m.setReqId(s.getReqId());
                    networkUtil.write(m);
                }
                
                else if(s.getHeader().equalsIgnoreCase("FILE_DES")){
                    System.out.println(s.getText());
                    String des = input.nextLine();
                    Message m = createMessage(des, "FILE_DES");
                    m.setReqId(s.getReqId());
                    networkUtil.write(m);
                }
                
                else if(s.getHeader().equalsIgnoreCase("REQUEST_FILE_UPLOAD")){
                    System.out.println(s.getText());
                    String reqID = input.nextLine();
                    Message message = createMessage(reqID, "REQUEST_FILE_UPLOAD");
                    networkUtil.write(message);
                }
                
                else if(s.getHeader().equalsIgnoreCase("DOWNLOAD")){
                    System.out.println(s.getText());
                    String fileID=  input.nextLine();
                    Message m = createMessage(fileID, "DOWNLOAD");
                    m.setFileLocation("src/tcpforward/Client/");
                    m.setFileId(fileID);
                    networkUtil.write(m);
                }
                
                else if(s.getHeader().equalsIgnoreCase("FILE_DOWNLOAD")){
                    try{
                        FileOutputStream fo = new FileOutputStream(new File(s.getFileLocation()),true);
                        fo.write(Arrays.copyOfRange(s.getText().getBytes(StandardCharsets.UTF_8), 0, s.getBytesRead()));
                        fo.close();
                    }catch(Exception e){
                        System.out.println("Error in file downloading "+ e);
                    }
                }

                else{
                    System.out.println(s.getText());
                }
                
            }
        } catch (Exception e) {
            System.out.println("error at writeThreadClient" + e);
        } finally {
            try {
                networkUtil.closeConnection();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private int takeInput(int low, int high){
        int x = input.nextInt();
        input.nextLine();
        while(x > high || x < low){
            System.out.println("Enter a valid choice");
            x = input.nextInt();
            input.nextLine();
        }
        return x;
    }
    
    private void startTimer(String fileId){
        timer = new Timer();
        timer.schedule(new ConfirmationTimerTask(fileId), 3000);
    }

    private void stopTimer(){
        if(timer != null){
            timer.cancel();
        }
    }
    
    private class ConfirmationTimerTask extends TimerTask{
        String fileId;
        private ConfirmationTimerTask(String fileId){
            this.fileId = fileId;
        }
        public void run(){
            timesUP = true;
            sendErrorMessage(fileId);
        }
    }

    private Message createMessage(String s, String header){
        Message m = new Message();
        m.setFrom(name);
        m.setTo("Server");
        m.setText(s);
        m.setHeader(header);
        return m;
    }

    public void setReader(ReadThreadClient readClient) {
        this.readClient = readClient;
    }

    private void sendErrorMessage(String fileID){
        System.out.println("File uploading Error. Request Time out....");
        Message message = new Message();
        message.setText("Request time out");
        message.setHeader("UPLOAD_ERROR");
        message.setFrom(name);
        message.setFileId(fileID);
        try {
            networkUtil.write(message);
        } catch (IOException e) {
            e.printStackTrace();
        }

    }
    

}



