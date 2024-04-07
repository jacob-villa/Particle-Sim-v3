import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;

public class NetworkClient {
    private String serverAddress;
    private int serverPort;
    private Socket socket;
    public BufferedReader in;

    NetworkClient(String serverAddress, int serverPort) throws IOException {
        this.serverAddress = serverAddress;
        this.serverPort = serverPort;
        try{
            this.socket = new Socket(this.serverAddress, this.serverPort);
            System.out.println("Connection established");
            this.in = new BufferedReader(new InputStreamReader(socket.getInputStream()), 1024);
            readMessages();
        } catch(IOException e){
            e.printStackTrace();
        }
    }

    public void closeSocket() throws IOException {
        socket.close();
    }

    private void readMessages() throws IOException {
        while(true){
            System.out.println("receiving...");
            String message = in.readLine();
            System.out.println("Message from server: " + message);
        }
    }
}
