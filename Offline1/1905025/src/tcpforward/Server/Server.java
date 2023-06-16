package tcpforward.Server;

import util.NetworkUtil;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.HashMap;
import java.util.Scanner;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import tcpforward.*;
import tcpforward.Helpers.Helper;



public class Server {

    private ServerSocket serverSocket;
    public HashMap<String, NetworkUtil> clientMap;
    public Scanner scanner = new Scanner(System.in);
    private BlockingQueue<Message> messageQueue;

    Server() {
        messageQueue = new ArrayBlockingQueue<>(10);
        clientMap = new HashMap<>();    // for current user tracking
        try {
            serverSocket = new ServerSocket(33333);
            // create necessary files for server
            createServerFiles();
            while (true) {
                Socket clientSocket = serverSocket.accept();
                serve(clientSocket);
            }
        } catch (Exception e) {
            System.out.println("Server starts:" + e);
        }

        
    }
    
    private void createServerFiles() throws IOException {
        String dir = "src/tcpforward/Server/Server_files/";
        File file1 = new File(dir+"client_cache.txt");
        File file2 = new File(dir+"file_cache.txt");
        File file3 = new File(dir+"fileReq_cache.txt");
        if(! file1.exists() || !file2.exists() || !file3.exists()){
            file1.createNewFile();
            file2.createNewFile();
            file3.createNewFile();
        }
        Helper.readAllUsers();         // read users from the file 
    }

    // add to client_cache.txt
    private void addUserToFile(String clientName) {
        // used to list all the users who have used this system
        String filename = "src/tcpforward/Server/Server_files/client_cache.txt";
        try (FileWriter fileWriter = new FileWriter(filename, true)) {
            fileWriter.write(clientName+"\n");
            System.out.println("Client name added to the cache");
            Helper.updateUserList(clientName);
        } catch (IOException e) {
            System.out.println("Error in adding the client name to the cache" + e);
        }
        
    }
    
    // check if there is duplicate user in the system
    private boolean checkClientValidity(String clientName, NetworkUtil networkUtil) throws IOException{
        
        if(clientMap.containsKey(clientName)){
            System.out.println("There is duplicate client "+ clientName);
            networkUtil.write("Please use another username");
            networkUtil.closeConnection();
            return false;
        }
        return true;
    } 

    // now we need to create the directory for the client in the "Server/Clients" folder
    private String createClientDirectory(String clientName){
        String s = "src/tcpforward/Server/Clients/"+ clientName;
        Helper.createDirectory(s);
        Helper.createDirectory(s+"/Inbox");
        Helper.createFile(s+"/Inbox/Messages.txt");
        addUserToFile(clientName);
        return s;
    }
    
    public void serve(Socket clientSocket) throws IOException, ClassNotFoundException {
        NetworkUtil networkUtil = new NetworkUtil(clientSocket);
        String clientName = (String) networkUtil.read();
        System.out.println("Client name: " + clientName);
        
        if(!checkClientValidity(clientName, networkUtil)){
            //invalid client
            return;
        }

        clientMap.put(clientName, networkUtil);

        String clientDirectory = createClientDirectory(clientName);
        

        // send the client, the welcome popup
        Message message = new Message();
        message.welcomeMessage(clientName);
        networkUtil.write(message);

        WriteThreadServer serverWrite = new WriteThreadServer(clientMap, networkUtil, clientDirectory , scanner, messageQueue);
        ReadThreadServer serverRead =  new ReadThreadServer(clientMap, networkUtil, messageQueue);
        serverRead.setWriter(serverWrite);
        
    }

    public static void main(String args[]) {
        new Server();
    }
}
