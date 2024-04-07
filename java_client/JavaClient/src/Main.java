import java.io.IOException;
import java.util.concurrent.DelayQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Main {

    static final int THREAD_COUNT = Runtime.getRuntime().availableProcessors();
    public static ExecutorService threadPool = Executors.newFixedThreadPool(THREAD_COUNT);
    public static DelayQueue<Task> tasksQueue = new DelayQueue<>();


    public static void main(String[] args) throws InterruptedException, IOException {
        Controller.initListeners();
        MainGUI.initGUI();
        RunnableTimer renderTask = new RunnableTimer();
        Thread renderThread = new Thread(renderTask);
        renderThread.start();

        NetworkClient tcpClient = new NetworkClient("127.0.0.1", 4160);

        threadPool.execute(new RunnableTask(tasksQueue.take()) {
        });

        while(true) {
            threadPool.execute(new RunnableTask(tasksQueue.take()));
        }
    }
}