package tcpforward;


import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Random;
import java.util.Scanner;
import java.util.concurrent.BlockingQueue;

import tcpforward.Helpers.FileInfo;
import tcpforward.Helpers.FileReq;
import tcpforward.Helpers.Helper;
import util.NetworkUtil;



public class WriteThreadServer implements Runnable{

    private int MAX_BUFFER_SIZE = 1024 ;
    private int MAX_CHUNK_SIZE = 512;
    private int MIN_CHUNK_SIZE = 128;
    
    private Thread thr;
    private HashMap<String, NetworkUtil> clientMap;
    String directory;
    private BlockingQueue<Message>messageQueue;
    


    public WriteThreadServer(HashMap<String, NetworkUtil> map, NetworkUtil util, String s, Scanner scanner,BlockingQueue<Message>messageQueue){
        this.clientMap = map;
        this.directory = s;
        this.messageQueue = messageQueue;
        
        this.thr = new Thread(this);
        thr.start();
    }

    @Override
    public synchronized void run() {
        try {
            while(true){
                Message mess = messageQueue.take();

                if(mess.getText().equalsIgnoreCase("1") && mess.getHeader().equalsIgnoreCase("WELCOME_POP_INPUT")){
                    provideTheListOfUsers(mess);
                }

                else if(mess.getText().equalsIgnoreCase("2") && mess.getHeader().equalsIgnoreCase("WELCOME_POP_INPUT")){
                    Helper.readFromFileCache();
                    String clientName = mess.getFrom();
                    String s ="\n<fileId>    <fileName>      <Type>\n";
                    if(Helper.fileListMap.containsKey(clientName)){
                        for( FileInfo fileInfo: Helper.fileListMap.get(clientName)){
                            s += "<"+fileInfo.fileId + ">    <" + fileInfo.filename + ">   <"+fileInfo.fileType+">\n";
                        }
                    }
                    clientMap.get(clientName).write(s);
                    printWelcomePop(mess);
                }
                
                else if(mess.getText().equalsIgnoreCase("3") && mess.getHeader().equalsIgnoreCase("WELCOME_POP_INPUT")){
                    String clientName = mess.getFrom();
                    String s ="\n<fileId>    <fileName>      <Owner>\n";
                    Helper.readFromFileCache();
                    for(String name:Helper.fileListMap.keySet()){
                        if(!clientName.equals(name)){
                            for(FileInfo f: Helper.fileListMap.get(name)){
                                if(f.fileType.equalsIgnoreCase("public")){
                                    s+= "<"+f.fileId+">    <" + f.filename + ">     <"+name+">\n";  
                                }
                            }
                        }
                    }
                    clientMap.get(clientName).write(s);
                    printWelcomePop(mess);
                }   

                else if(mess.getText().equalsIgnoreCase("4") && mess.getHeader().equalsIgnoreCase("WELCOME_POP_INPUT")){
                    // make File request
                    String s = Helper.createReqId();
                    Helper.fileReq.put(s, new FileReq(s, mess.getFrom()));
                    Message message = createMessage("Enter the file name", "FILE_REQ", mess.getFrom());
                    message.setReqId(s);
                    clientMap.get(mess.getFrom()).write(message);
                }
                
                else if(mess.getHeader().equalsIgnoreCase("FILE_NAME")){
                    Helper.fileReq.get(mess.getReqId()).name = mess.getText();
                    Message message = createMessage("Enter a short Description in ONE Line:", "FILE_DES", mess.getFrom());
                    message.setReqId(mess.getReqId());
                    clientMap.get(mess.getFrom()).write(message);
                }

                else if(mess.getHeader().equalsIgnoreCase("FILE_DES")){
                    Helper.fileReq.get(mess.getReqId()).description = mess.getText();
                    Helper.writeMessage(mess.getFrom(), Helper.fileReq.get(mess.getReqId())); 
                    printWelcomePop(mess);
                }
                
                else if(mess.getText().equalsIgnoreCase("5") && mess.getHeader().equalsIgnoreCase("WELCOME_POP_INPUT")){
                    // for viewing the Inbox
                    String s = Helper.provideMessages(mess.getFrom());
                    System.out.println(mess.getFrom());
                    clientMap.get(mess.getFrom()).write(s);
                    printWelcomePop(mess);   
                }
                
                else if(mess.getText().equalsIgnoreCase("6")&& mess.getHeader().equalsIgnoreCase("WELCOME_POP_INPUT")){
                    // for uploading a file
                    System.out.println("file uploading request received from " + mess.getFrom());
                    uploadFile(mess);

                }
                
                else if(mess.getHeader().equalsIgnoreCase("UPLOAD_TYPE_SELECTION")){
                    
                    System.out.println(Helper.fileInfoMap.get(mess.getFileId()));
                    Helper.fileInfoMap.get(mess.getFileId()).fileLocation = createPublicOrPrivateDirs(mess, Helper.fileInfoMap.get(mess.getFileId())); // create dirctories;
                    
                    Message message = createMessage("Write the name of the file: \n", "WRITE_FILE_NAME", mess.getFrom());
                    message.setFileId(mess.getFileId());
                    clientMap.get(mess.getFrom()).write(message);

                }

                else if(mess.getHeader().equalsIgnoreCase("WRITE_FILE_NAME")){

                    Helper.fileInfoMap.get(mess.getFileId()).filename = mess.getText();
                    Helper.fileInfoMap.get(mess.getFileId()).fileLocation += mess.getText();
                    
                    Message message = createMessage("Write the size of the file in bytes: \n", "WRITE_FILE_SIZE", mess.getFrom());
                    message.setFileId(mess.getFileId());
                    message.setReqId(mess.getReqId());
                    clientMap.get(mess.getFrom()).write(message);
                }

                else if(mess.getHeader().equalsIgnoreCase("WRITE_FILE_SIZE")){
                    Helper.fileInfoMap.get(mess.getFileId()).fileSize = Long.valueOf(mess.getText());
                    int chunk_size = new Random().nextInt(MAX_CHUNK_SIZE - MIN_CHUNK_SIZE + 1) + MIN_CHUNK_SIZE;

                    File file = new File(Helper.fileInfoMap.get(mess.getFileId()).fileLocation);
                    file.createNewFile();   // create file if not present
                    

                    Message m = createMessage("File id: "+ Helper.fileInfoMap.get(mess.getFileId()).fileId + " Chunk Size: "+ chunk_size + "\n", "UPLOAD_FILE", mess.getFrom());
                    m.setFileId(mess.getFileId());
                    m.setReqId(mess.getReqId());
                    m.setChunkSize(chunk_size);
                    clientMap.get(mess.getFrom()).write(m);
                }
                
                else if(mess.getHeader().equalsIgnoreCase("UPLOAD_FILE")){
                    FileInfo f = Helper.fileInfoMap.get(mess.getFileId());
                    FileOutputStream fos = new FileOutputStream(new File(f.fileLocation), true);
                    int bytesRead = mess.getBytesRead();
                    
                    
                    fos.write(Arrays.copyOfRange(mess.getText().getBytes(StandardCharsets.UTF_8), 0, bytesRead));
                    
                    f.readBytes += bytesRead;
                    System.out.println(f.readBytes + " bytes read");
                    fos.flush();    // clear the outputstream buffer
                    
                    Message m = createMessage("OK", "CONFIRMATION", mess.getFrom());
                    m.setReqId(mess.getReqId());
                    m.setFileId(mess.getFileId());
                    clientMap.get(mess.getFrom()).write(m);
                    
                    fos.close();

                }

                else if(mess.getHeader().equalsIgnoreCase("UPLOAD_ERROR")){
                   handleError(mess);
                }
                
                else if(mess.getHeader().equalsIgnoreCase("UPLOAD_COMPLETE")){
                    FileInfo f = Helper.fileInfoMap.get(mess.getFileId());

                    System.out.println("Inside file Complete");
                    
                    String message = "";
                    if(f.readBytes == f.fileSize){
                        System.out.println("File uploaded successfully by " + mess.getFrom());
                        message= "File uploaded successfully";
                        Helper.writeToFileCache(mess.getFileId());
                        
                        if(mess.getReqId() != null){
                            Helper.sendMessageToRequester(mess.getFrom(), mess.getReqId());
                        }
                    }else{
                        handleError(mess);
                        System.out.println("file upload error...file deleted");
                        message="Error in file upload. File is deleted in server!";
                        
                    }
                    
                    clientMap.get(mess.getFrom()).write(message);
                    printWelcomePop(mess);       
                }

                else if(mess.getText().equalsIgnoreCase("7") && mess.getHeader().equalsIgnoreCase("WELCOME_POP_INPUT")){
                    // upload file via request
                    Message message = createMessage("Enter requestId: ", "REQUEST_FILE_UPLOAD", mess.getFrom());
                    clientMap.get(mess.getFrom()).write(message);
                }
                
                else if(mess.getHeader().equalsIgnoreCase("REQUEST_FILE_UPLOAD")){
                    String reqID = mess.getText();
                    if(Helper.checkReqId(reqID)){
                        String fileId = Helper.createFileId();

                        Helper.fileInfoMap.put(fileId, new FileInfo(fileId, mess.getFrom()));
                        mess.setText("2"); // public type file
                        System.out.println(Helper.fileInfoMap.get(fileId));
                        Helper.fileInfoMap.get(fileId).fileLocation = createPublicOrPrivateDirs(mess, Helper.fileInfoMap.get(fileId)); // create dirctories;
                        
                        Message message = createMessage("Write the name of the file: \n", "WRITE_FILE_NAME", mess.getFrom());
                        message.setFileId(fileId);
                        message.setReqId(reqID);
                        clientMap.get(mess.getFrom()).write(message);
                    }else{
                        clientMap.get(mess.getFrom()).write("Request Id does not match");
                        printWelcomePop(mess);
                    }

                }

                else if(mess.getText().equalsIgnoreCase("8") && mess.getHeader().equalsIgnoreCase("WELCOME_POP_INPUT")){
                    // download file
                    Message message = createMessage("Type fileID: ", "DOWNLOAD", mess.getFrom());
                    clientMap.get(mess.getFrom()).write(message);
                }
                
                else if(mess.getHeader().equalsIgnoreCase("DOWNLOAD")){
                    FileInfo fileInfo = fileIdVerification(mess.getText(), mess.getFrom());

                    if(fileInfo != null){

                        downloadFile(fileInfo,mess);
                        System.out.println(fileInfo.fileId + " Downloaded by "+ mess.getFrom());
                        clientMap.get(mess.getFrom()).write("File downloading completed");
                        printWelcomePop(mess);
                    }
                    else{

                        clientMap.get(mess.getFrom()).write("Sorry file Id does not match");
                        printWelcomePop(mess);
                    }
                }

                else if(mess.getText().equalsIgnoreCase("9") && mess.getHeader().equalsIgnoreCase("WELCOME_POP_INPUT")){
                    logout(mess);
                }
            }

            
        } catch (Exception e) {
            System.out.println("Error in WriteThreadServer" + e);
        }
        
    }

    private FileInfo fileIdVerification(String fileId, String owner) {
        Helper.readFromFileCache();
        for(String s: Helper.fileListMap.keySet()){
            for(FileInfo f: Helper.fileListMap.get(s)){
                if(f.fileId.equals(fileId) && f.fileType.equalsIgnoreCase("public"))
                    return f; 
                else if(f.fileId.equals(fileId) && f.fileOwner.equalsIgnoreCase(owner)){
                    return f;
                }
            }
        }
        return null;
    }

    private void handleError(Message mess){
        System.out.println("file upload error..from user "+ mess.getFrom());
        FileInfo f = Helper.fileInfoMap.get(mess.getFileId());
        File file = new File(f.fileLocation);
        if(file.delete()){
            System.out.println("File deleted");
        }
        Helper.fileInfoMap.remove(mess.getFileId());

    }
    
    private void uploadFile(Message mess) throws IOException, InterruptedException{
        String clientName = mess.getFrom();
        
        Message message = new Message();

        String fId = Helper.createFileId();
        Helper.fileInfoMap.put(fId, new FileInfo(fId, mess.getFrom()));
        
        // public or private
        message.uploadTypeSelection(clientName);
        message.setFileId(fId); 
        clientMap.get(clientName).write(message);

    }

    private void logout(Message mess) throws IOException {
        String clientName = mess.getFrom();
        clientMap.get(mess.getFrom()).write("\nLogging out....\n");
        try{
            clientMap.get(clientName).closeConnection();
            System.out.println(clientName + " Logged out");
        }catch(Exception e){
            System.out.println("Error in logging out");
        }
        clientMap.remove(clientName);
    }

    private void provideTheListOfUsers(Message mess) throws IOException {
        String cacheDir = "src/tcpforward/Server/Server_files/client_cache.txt";
        Message message = new Message();
        message.listOfUsersMessage(cacheDir, clientMap, mess.getFrom());
        message.setHeader("USER_LIST");
        clientMap.get(mess.getFrom()).write(message);
        
        // previous requested data is provided. we again need to send the selection 
        printWelcomePop(mess);

    }

    private void printWelcomePop(Message mess) throws IOException{
        Message message = new Message();
        message.welcomeMessage(mess.getFrom());
        message.setHeader("WELCOME_POP_INPUT");
        clientMap.get(mess.getFrom()).write(message);
    }

    private String createPublicOrPrivateDirs(Message response, FileInfo f) {
        if(response.getText().equalsIgnoreCase("1")){
            // create private directory
            f.fileType = "Private";
            return Helper.createDirectory(directory + "/Private/");

        }else{
            // create public directory
            f.fileType = "Public";
            return Helper.createDirectory(directory + "/Public/");
        }
    }

    private Message createMessage(String text, String header, String to){
        Message message = new Message();
        message.setText(text);
        message.setHeader(header);
        message.setTo(to);
        message.setFrom("Server");

        return message;
    }

    private void downloadFile(FileInfo f, Message mess){
        byte[] buffer = new byte[1024];
        Helper.createFile(mess.getFileLocation()+f.filename);
        try (BufferedInputStream bi = new BufferedInputStream(new FileInputStream(f.fileLocation))) {
           int bytesRead;
            while( (bytesRead = bi.read(buffer)) != -1){
                Message m = new Message();
                m.setFrom("Server");
                m.setHeader("FILE_DOWNLOAD");
                m.setText(new String(buffer, StandardCharsets.UTF_8));
                m.setTo(mess.getFrom());
                m.setFileLocation(mess.getFileLocation()+f.filename);
                m.setBytesRead(bytesRead);
                clientMap.get(mess.getFrom()).write(m);
            }
            bi.close();
            
        } catch (IOException e) {
            
            e.printStackTrace();
        }

    }

    

    
}
