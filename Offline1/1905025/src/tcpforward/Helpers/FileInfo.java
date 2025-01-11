package tcpforward.Helpers;

public class FileInfo {
    public String filename;
    public Long fileSize;
    public String fileLocation;
    public String fileId;
    public String fileType;
    public String fileOwner;
    public long readBytes = 0;
    
    public FileInfo(){}

    public FileInfo(String id,String string){
        fileId = id;
        fileOwner = string;
    }
    public String toString(){
        return fileId +" " +filename + " "+ fileType;
    }
    
}
