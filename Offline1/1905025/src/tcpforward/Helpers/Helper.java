package tcpforward.Helpers;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;


public class Helper {
    public static HashMap<String ,FileInfo>fileInfoMap = new HashMap<>(); // <fileId, file>
    public static HashMap<String, FileReq> fileReq = new HashMap<>();      //<reqId,req>
    public static HashMap<String,List<FileInfo>> fileListMap = new HashMap<>(); //<user,files>
    public static List<String> allTimeUniqueUsers = new ArrayList<>();  // unique users for 
    public static List<String> alltimeUsers = new ArrayList<>(); //all users for client req
    public static HashMap<String,List<String>> userReqMap = new HashMap<>();// to store user and his requests 
    public static int fileCount = 0;
    public static int reqCount = 0;
    
    static{
        readFilecount();
    }

    public static void readAllUsers(){

        String dir = "src/tcpforward/Server/Server_files/client_cache.txt";
        try {
            FileReader fileReader = new FileReader(dir);
            BufferedReader read = new BufferedReader(fileReader);

            String line;
            while((line = read.readLine())!= null){
                
                alltimeUsers.add(line);
                if(! allTimeUniqueUsers.contains(line)){
                    allTimeUniqueUsers.add(line);
                }
            }
        read.close();
        }catch(Exception e){
            System.out.println("Helper.readAllUsers " + e);
        }

    }
 
    private static void readFilecount() {
        try{

            File file = new File("src/tcpforward/Server/Server_files/file_count.txt");
            if(! file.exists()){
                file.createNewFile();
                System.out.println("file_count.txt created");
                reqCount = 0;
                fileCount = 0;
            }else{
                BufferedReader br = new BufferedReader(new FileReader("src/tcpforward/Server/Server_files/file_count.txt"));
                String line = br.readLine();
                br.close();
                String[] parts = line.split(">");
                fileCount = Integer.valueOf(parts[0]);
                reqCount = Integer.valueOf(parts[1]);
            }
            
            
        }catch(Exception e){
            System.out.println("Error in readFilecount " + e);
        }
    }

    public static String createDirectory(String s){
        try {
            File client_dir = new File(s);
            if(! client_dir.exists()){
                if(client_dir.mkdirs()){
                    System.out.println("New Client directory created");
                }
            }else{
                System.out.println("Client directory exists");
            } 
            
            return s;
        } catch (Exception e) {
            System.out.println("Exception in creating User Directory " + e);
        }
        return null;
    }

    public static String createFileId(){
        fileCount++;
        return "f"+ String.valueOf(fileCount); 
    }

    public static void writeMessage(String clientName, FileReq msgReq){
        String text = "User \'"+ msgReq.sentFrom + "\' requested for a file named \'"+msgReq.name+"\' with request ID: \'"+msgReq.reqId + "\' and Description: \'"+ msgReq.description + "\'";
        for(String x:allTimeUniqueUsers){
            if(! x.equalsIgnoreCase(clientName))
            {
                String s = "src/tcpforward/Server/Clients/"+ x+"/Inbox/Messages.txt";
                write(s, text);
            }

        }
        write("src/tcpforward/Server/Server_files/fileReq_cache.txt",msgReq.reqId+">"+msgReq.sentFrom);
        writeFileCount("src/tcpforward/Server/Server_files/file_count.txt",fileCount+">"+reqCount);
    }

    private static void writeFileCount(String dir, String text){
        try (FileWriter fileWriter = new FileWriter(dir)) {
            fileWriter.write(text+"\n");
        } catch (IOException e) {
            System.out.println("Error in Helper.write " + e);
        }
    }

    public static void writeToFileCache(String fileId){
        String dir = "src/tcpforward/Server/Server_files/file_cache.txt";
        FileInfo f = fileInfoMap.get(fileId);
        String txt = f.fileId +">"+f.filename+">"+f.fileOwner+">"+f.fileType+">"+f.fileSize+">"+f.fileLocation ;
        
        write(dir,txt);
        writeFileCount("src/tcpforward/Server/Server_files/file_count.txt", fileCount+">"+reqCount);
        
        fileInfoMap.remove(fileId);
    }

    private static void write(String dir, String txt){

        try (FileWriter fileWriter = new FileWriter(dir, true)) {
            fileWriter.write(txt+"\n");
        } catch (IOException e) {
            System.out.println("Error in Helper.write " + e);
        }

    }

    public static void readFromFileCache(){
        fileListMap.clear();
        String dir = "src/tcpforward/Server/Server_files/file_cache.txt";
        try {
            FileReader fileReader = new FileReader(dir);
            BufferedReader read = new BufferedReader(fileReader);

            String line;
            while((line = read.readLine())!= null){
                String[] parts = line.split(">");
                FileInfo newFile = new FileInfo(parts[0],parts[2]);
                newFile.filename = parts[1];
                newFile.fileLocation = parts[5]; 
                newFile.fileType = parts[3];
                newFile.fileSize = Long.valueOf(parts[4]);

                if(! fileListMap.containsKey(parts[2])){
                    fileListMap.put(parts[2], new ArrayList<>());    
                }
                fileListMap.get(parts[2]).add(newFile);
            }
            read.close();
        } catch (Exception e) {
            
            e.printStackTrace();
        }
        
    }

    public static String provideMessages(String clientName){
        String text = "";
        String dir = "src/tcpforward/Server/Clients/"+ clientName+"/Inbox/Messages.txt";
        try
        {
            BufferedReader br = new BufferedReader(new FileReader(dir));
            String line;
            
            while((line = br.readLine()) != null){
                text = line;
            }
            br.close();
        }catch(Exception e){
            System.out.println("Helper.provideMessage() " + e);
        }
        if(text.equalsIgnoreCase("")){
            return "No Messages to show";
        }else{
            // clear the message.txt file
            try {
                BufferedWriter writer = new BufferedWriter(new FileWriter(dir));
                writer.write("");
                writer.close();
            } catch (IOException e) {
                System.out.println("Error in Helper.provideMessages "+ e);
            }
            return text;
        }

    } 

    public static String createReqId() {
        reqCount++;
        return "r"+reqCount;
    }

    public static void createFile(String string) {
        File file = new File(string);
        try {
            file.createNewFile();
        } catch (IOException e) {
            System.out.println("error in Helper.createFile "+ e);
        }
    }
    
    // update when new user added
    public static void updateUserList(String user){
        if(! allTimeUniqueUsers.contains(user)){
            allTimeUniqueUsers.add(user);
        }
    }

    public static boolean checkReqId(String reqID) {
        boolean flag = false;
        userReqMap.clear();
        String dir = "src/tcpforward/Server/Server_files/fileReq_cache.txt";
        try{
           BufferedReader br = new BufferedReader(new FileReader(dir));
            String line;
            
            while((line = br.readLine()) != null){
                String[] parts = line.split(">");
                System.out.println("Parts length: " + parts.length);
                if(! userReqMap.containsKey(parts[1])){
                    userReqMap.put(parts[1], new ArrayList<>());
                }
                if(parts[0].equals(reqID)){
                    flag = true;
                }
                userReqMap.get(parts[1]).add(parts[0]);
            }
            br.close();
        }catch(Exception e){
            System.out.println("Helper.checkReqId " + e);
        }

        return flag;
    }

    public static void sendMessageToRequester(String user, String reqID, String filename) {
        for(String s: userReqMap.keySet()){
            if(userReqMap.get(s).contains(reqID)){
                String dir = "src/tcpforward/Server/Clients/"+ s+"/Inbox/Messages.txt";
                write(dir, "file named as '"+ filename+"' uploaded by user '"+ user + "' for your request ID: "+ reqID);
            }
        }

    }
}
