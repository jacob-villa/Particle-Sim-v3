import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Main {

    static final int THREAD_COUNT = Runtime.getRuntime().availableProcessors();
    public static ExecutorService threadPool = Executors.newFixedThreadPool(THREAD_COUNT);


    public static void main(String[] args) {
        System.out.println("Hello world!");
    }
}