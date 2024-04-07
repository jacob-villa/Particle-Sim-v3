import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class NetworkClient {
    private Socket socket;

    NetworkClient(String serverAddress, int serverPort) throws IOException {
        try{
            this.socket = new Socket(serverAddress, serverPort);
            System.out.println("Connection established");
            Thread t = new Thread(this::readMessages);
            t.start();

        } catch(IOException e){
            e.printStackTrace();
        }
    }

    public void closeSocket() throws IOException {
        socket.close();
    }

    private void readMessages() {
        try {
            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            while(true){
                System.out.println("receiving...");
                String message = in.readLine();
                System.out.println("Message from server: " + message);
            }
        } catch (IOException ex) {
            throw new RuntimeException(ex);
        }
    }

    public void sendMessage(String s) {
        try {
            System.out.println(socket.isConnected());
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
            out.print(s);
        } catch (IOException ex) {
            throw new RuntimeException(ex);
        }
    }
}
