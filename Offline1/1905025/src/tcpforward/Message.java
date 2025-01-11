package tcpforward;


import java.io.Serializable;
import java.util.HashMap;

import tcpforward.Helpers.Helper;
import util.NetworkUtil;

public class Message implements Serializable {
    private String from;
    private String to;
    private String text;
    private String fileId;
    private int chunk_size;
    private int bytesRead;
    private String reqId;
    private String header;
    private String fileLocation;

    

    public Message() {
    }

    public String getFrom() {
        return from;
    }

    public void setFrom(String from) {
        this.from = from;
    }

    public String getTo() {
        return to;
    }

    public void setTo(String to) {
        this.to = to;
    }

    public String getText() {
        return text;
    }

    public void setText(String text) {
        this.text = text;
    }

    public void setFileId(String id){
        fileId = id;
    }

    public void setHeader(String s){
        header = s;
    }
    public String getHeader(){
        return header;
    }

    public String getFileId(){
        return fileId;
    }

    public void welcomeMessage(String to){
        
        from = "Server";
        this.to = to; 
        text = "\nPick up your Choice:\n";
        text += "1. Look up the client list\n";
        text += "2. Look up the uploaded files\n";
        text += "3. Look up files of other users\n";
        text += "4. Make a file request\n";
        text += "5. Read Unread Messages\n";
        text += "6. Upload a file\n";
        text += "7. Upload requested file\n";
        text += "8. Download a file\n";
        text += "9. Logout\n";
        header = "WELCOME_POP_INPUT";
    }
    
    public void listOfUsersMessage(String s, HashMap<String, NetworkUtil> map, String to){
        from = "Server";
        this.to = to;
        text = readFromFile();
        text += listOfActiveUsers(map);
    }

    private String readFromFile(){
        String txt = "List of All Previous Users: \n";
        for(String s: Helper.alltimeUsers){
            txt+= s;
            txt+="\n";
        }
        return txt;

    }

    private String listOfActiveUsers(HashMap<String, NetworkUtil> clientMap){
        String txt = "";
        txt += "The list of current users: \n";
        for(String s: clientMap.keySet()){
            txt += s;
            txt += "\n";
        }
        return txt;
    }

    public void uploadTypeSelection(String clientName) {
        from = "Server";
        to = clientName;
        text = "\nPick file type:\n";
        text += "1. Private\n";
        text += "2. Public\n";

        header = "UPLOAD_TYPE_SELECTION";
        
    }
    
    public int getChunkSize(){
        return chunk_size;
    }
    
    public void setChunkSize(int x){
        chunk_size = x;
    }
    
    public int getBytesRead() {
        return bytesRead;
    }

    public void setBytesRead(int bytesRead) {
        this.bytesRead = bytesRead;
    }

    public String getReqId() {
        return reqId;
    }

    public void setReqId(String reqId) {
        this.reqId = reqId;
    }

    public String getFileLocation() {
        return fileLocation;
    }

    public void setFileLocation(String fileLocation) {
        this.fileLocation = fileLocation;
    }
    
}