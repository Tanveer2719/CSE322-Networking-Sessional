package tcpforward.Helpers;

public class FileReq {
    public FileReq(String s, String from) {
        reqId = s;
        sentFrom = from;
    }
    public FileReq(){}
    public String reqId;
    public String sentFrom;
    public String description;
    public String name;
}
