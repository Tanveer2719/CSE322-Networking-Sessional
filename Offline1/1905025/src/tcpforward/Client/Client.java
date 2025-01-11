package tcpforward.Client;

import util.NetworkUtil;

import java.util.Scanner;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

import tcpforward.Message;
import tcpforward.ReadThreadClient;
import tcpforward.WriteThreadClient;

public class Client {

    public Client(String serverAddress, int serverPort) {
        try {
            Scanner scanner = new Scanner(System.in);
            System.out.print("Enter name of the client: ");
            String clientName = scanner.nextLine();
            NetworkUtil networkUtil = new NetworkUtil(serverAddress, serverPort);
            networkUtil.write(clientName);

            BlockingQueue<Message>messageQueue = new ArrayBlockingQueue<>(10);
            ReadThreadClient readClient = new ReadThreadClient(networkUtil,clientName, scanner, messageQueue);
            WriteThreadClient writeClient = new WriteThreadClient(networkUtil, clientName, scanner, messageQueue);
            readClient.setWriter(writeClient);
            writeClient.setReader(readClient);

        } catch (Exception e) {
            System.out.println(e);
        }
    }

    public static void main(String args[]) {
        String serverAddress = "127.0.0.1";
        int serverPort = 33333;
        new Client(serverAddress, serverPort);
    }
}


